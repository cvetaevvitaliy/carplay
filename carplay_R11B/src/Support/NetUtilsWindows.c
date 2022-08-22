/*
	File:    	NetUtilsWindows.c
	Package: 	Apple CarPlay Communication Plug-in.
	Abstract: 	n/a 
	Version: 	210.81
	
	Disclaimer: IMPORTANT: This Apple software is supplied to you, by Apple Inc. ("Apple"), in your
	capacity as a current, and in good standing, Licensee in the MFi Licensing Program. Use of this
	Apple software is governed by and subject to the terms and conditions of your MFi License,
	including, but not limited to, the restrictions specified in the provision entitled ”Public 
	Software”, and is further subject to your agreement to the following additional terms, and your 
	agreement that the use, installation, modification or redistribution of this Apple software
	constitutes acceptance of these additional terms. If you do not agree with these additional terms,
	please do not use, install, modify or redistribute this Apple software.
	
	Subject to all of these terms and in consideration of your agreement to abide by them, Apple grants
	you, for as long as you are a current and in good-standing MFi Licensee, a personal, non-exclusive 
	license, under Apple's copyrights in this original Apple software (the "Apple Software"), to use, 
	reproduce, and modify the Apple Software in source form, and to use, reproduce, modify, and 
	redistribute the Apple Software, with or without modifications, in binary form. While you may not 
	redistribute the Apple Software in source form, should you redistribute the Apple Software in binary
	form, you must retain this notice and the following text and disclaimers in all such redistributions
	of the Apple Software. Neither the name, trademarks, service marks, or logos of Apple Inc. may be
	used to endorse or promote products derived from the Apple Software without specific prior written
	permission from Apple. Except as expressly stated in this notice, no other rights or licenses, 
	express or implied, are granted by Apple herein, including but not limited to any patent rights that
	may be infringed by your derivative works or by other works in which the Apple Software may be 
	incorporated.  
	
	Unless you explicitly state otherwise, if you provide any ideas, suggestions, recommendations, bug 
	fixes or enhancements to Apple in connection with this software (“Feedback”), you hereby grant to
	Apple a non-exclusive, fully paid-up, perpetual, irrevocable, worldwide license to make, use, 
	reproduce, incorporate, modify, display, perform, sell, make or have made derivative works of,
	distribute (directly or indirectly) and sublicense, such Feedback in connection with Apple products 
	and services. Providing this Feedback is voluntary, but if you do provide Feedback to Apple, you 
	acknowledge and agree that Apple may exercise the license granted above without the payment of 
	royalties or further consideration to Participant.
	
	The Apple Software is provided by Apple on an "AS IS" basis. APPLE MAKES NO WARRANTIES, EXPRESS OR 
	IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY 
	AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR
	IN COMBINATION WITH YOUR PRODUCTS.
	
	IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL DAMAGES 
	(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
	PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION 
	AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
	(INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE 
	POSSIBILITY OF SUCH DAMAGE.
	
	Copyright (C) 2007-2014 Apple Inc. All Rights Reserved.
*/

#include "NetUtilsWindows.h"

#include "APSCommonServices.h"
#include "APSDebugServices.h"

#include <Iphlpapi.h>
#if( !TARGET_OS_WINDOWS_CE )
	#include <mswsock.h>
	#include <process.h>
#endif
#include <Qos2.h>

#if( !TARGET_OS_WINDOWS_CE )
	#pragma comment( lib, "IPHlpApi" )
	#pragma comment( lib, "ws2_32" )
#endif

//===========================================================================================================================
//	Private
//===========================================================================================================================

#define DEBUG_NAME					"[NetUtils] "
#define	NETUTILS_DEBUG_NO_GAA		0 // 1=Don't use GetAdaptersAddresses to simulate older systems.

typedef DWORD
	( WINAPI * GetAdaptersAddressesFunctionPtr )( 
		ULONG 					inFamily, 
		DWORD 					inFlags, 
		PVOID 					inReserved, 
		PIP_ADAPTER_ADDRESSES 	inAdapter, 
		PULONG					outBufferSize );

static HMODULE								gIPHelperLibraryInstance			= NULL;
static GetAdaptersAddressesFunctionPtr		gGetAdaptersAddressesFunctionPtr	= NULL;

static int	getifaddrs_ipv6( struct ifaddrs **outAddrs );
#if( TARGET_OS_WINDOWS_CE )
	static int	getifaddrs_ce( struct ifaddrs **outAddrs );
#else
	static int	getifaddrs_ipv4( struct ifaddrs **outAddrs );
#endif

static OSStatus	GetIPv4AddressInfo( struct sockaddr *inAddr, uint32_t *outIndex, struct sockaddr_in *ioSubnetMask );
static OSStatus	WCHARtoUTF8Copy( const WCHAR *inString, char **outUTF8 );

//===========================================================================================================================
//	getifaddrs
//===========================================================================================================================

int	getifaddrs( struct ifaddrs **outAddrs )
{
	int		err;
	
	// Try to the load the GetAdaptersAddresses function from the IP Helpers DLL. This API is only available on Windows
	// XP or later. Looking up the symbol at runtime allows the code to still work on older systems without that API.

#if( !NETUTILS_DEBUG_NO_GAA )
	if( !gIPHelperLibraryInstance )
	{
		gIPHelperLibraryInstance = LoadLibrary( TEXT( "Iphlpapi" ) );
		if( gIPHelperLibraryInstance )
		{
			gGetAdaptersAddressesFunctionPtr = 
				(GetAdaptersAddressesFunctionPtr) GetProcAddressCompat( gIPHelperLibraryInstance, "GetAdaptersAddresses" );
			if( !gGetAdaptersAddressesFunctionPtr )
			{
				BOOL		ok;
				
				ok = FreeLibrary( gIPHelperLibraryInstance );
				err = map_global_value_errno( ok, ok );
				check_noerr( err );
				gIPHelperLibraryInstance = NULL;
			}
		}
	}
#endif
	
	// Use the new IPv6-capable routine if supported. Otherwise, fall back to the old and compatible IPv4-only code.
	
	if( gGetAdaptersAddressesFunctionPtr )
	{
		err = getifaddrs_ipv6( outAddrs );
		require_noerr( err, exit );
	}
	else
	{
		#if( TARGET_OS_WINDOWS_CE )
			err = getifaddrs_ce( outAddrs );
			require_noerr( err, exit );
		#else
			err = getifaddrs_ipv4( outAddrs );
			require_noerr( err, exit );
		#endif
	}
	
exit:
	return( err );
}

//===========================================================================================================================
//	getifaddrs_ipv6
//===========================================================================================================================

static int	getifaddrs_ipv6( struct ifaddrs **outAddrs )
{
	int							err;
	int							i;
	DWORD						flags;
	struct ifaddrs *			head;
	struct ifaddrs **			next;
	IP_ADAPTER_ADDRESSES *		iaaList;
	ULONG						iaaListSize;
	IP_ADAPTER_ADDRESSES *		iaa;
	struct ifaddrs *			ifa;
	
	check( gGetAdaptersAddressesFunctionPtr );
	
	head	= NULL;
	next	= &head;
	iaaList	= NULL;
	
	// Get the list of interfaces. The first call gets the size and the second call gets the actual data.
	// This loops to handle the case where the interface changes in the window after getting the size, but before the
	// second call completes. A limit of 100 retries is enforced to prevent infinite loops if something else is wrong.
	
	flags = GAA_FLAG_SKIP_ANYCAST 		| 
			GAA_FLAG_SKIP_MULTICAST 	| 
			GAA_FLAG_SKIP_DNS_SERVER 	| 
			GAA_FLAG_INCLUDE_PREFIX		|
			GAA_FLAG_SKIP_FRIENDLY_NAME;
	i = 0;
	for( ;; )
	{
		iaaListSize = 0;
		err = gGetAdaptersAddressesFunctionPtr( AF_UNSPEC, flags, NULL, NULL, &iaaListSize );
		check( err == ERROR_BUFFER_OVERFLOW );
		
		iaaList = (IP_ADAPTER_ADDRESSES *) calloc( 1, iaaListSize );
		require_action( iaaList, exit, err = ERROR_NOT_ENOUGH_MEMORY );
		
		err = gGetAdaptersAddressesFunctionPtr( AF_UNSPEC, flags, NULL, iaaList, &iaaListSize );
		if( err == ERROR_SUCCESS ) break;
		
		free( iaaList );
		iaaList = NULL;
		++i;
		require( i < 100, exit );
		dlog( kLogLevelWarning, DEBUG_NAME "retrying GetAdaptersAddresses after %d failure(s): %#m\n", i, err );
	}
	
	for( iaa = iaaList; iaa; iaa = iaa->Next )
	{
		int									addrIndex;
		IP_ADAPTER_UNICAST_ADDRESS *		addr;
		size_t								minSize;
		DWORD								ipv6IfIndex;
		IP_ADAPTER_PREFIX *					firstPrefix;
		
		// Certain versions of Windows (XP without any service packs) return short 72 byte structures so we cannot
		// access fields beyond that if we've got one of these short structures. To workaround this, separate locals
		// are used for fields we need and they are initialized to sane values when we encounter a short structure.
		
		minSize = offsetof( IP_ADAPTER_ADDRESSES, Ipv6IfIndex ) + sizeof_field( IP_ADAPTER_ADDRESSES, Ipv6IfIndex );
		if( iaa->Length >= minSize )
		{
			ipv6IfIndex = iaa->Ipv6IfIndex;
			firstPrefix = iaa->FirstPrefix;
		}
		else
		{
			ipv6IfIndex = 0;
			firstPrefix = NULL;
		}
		if( ( ipv6IfIndex == 1 ) || ( iaa->IfType == IF_TYPE_TUNNEL ) ) continue;	// Skip psuedo and tunnel interfaces.
		
		// Add an AF_LINK interface for the physical address.
		
		if( iaa->PhysicalAddressLength == 6 )
		{
			struct sockaddr_dl *		sdl;
			
			ifa = (struct ifaddrs *) calloc( 1, sizeof( *ifa ) );
			require_action( ifa, exit, err = WSAENOBUFS );
			
			*next = ifa;
			next  = &ifa->ifa_next;
			
			ASPrintF( &ifa->ifa_name, "%s.AF_LINK", iaa->AdapterName );
			require_action( ifa->ifa_name, exit, err = kNoMemoryErr );
			
			ifa->displayName = strdup( ifa->ifa_name );
			require_action( ifa->displayName, exit, err = WSAENOBUFS );
			
			sdl = (struct sockaddr_dl *) calloc( 1, sizeof( *sdl ) );
			require_action( sdl, exit, err = kNoMemoryErr );
			
			sdl->sa.sa_family	= AF_LINK;
			sdl->sdl_type		= IFT_ETHER;
			sdl->sdl_alen		= 6;
			memcpy( sdl->sdl_data, iaa->PhysicalAddress, 6 );
			
			ifa->ifa_addr = (struct sockaddr *) sdl;
		}
		
		// Add each address as a separate interface to emulate the way getifaddrs works.
		
		for( addrIndex = 0, addr = iaa->FirstUnicastAddress; addr; ++addrIndex, addr = addr->Next )
		{
			int						family;
			int						prefixIndex;
			IP_ADAPTER_PREFIX *		prefix;
			ULONG					prefixLength;
			
			family = addr->Address.lpSockaddr->sa_family;
			if( ( family != AF_INET ) && ( family != AF_INET6 ) ) continue;	// Must be IPv4 or IPv6.
			if( ( ipv6IfIndex == 0 )  && ( family == AF_INET6 ) ) continue;	// Can't use IPv6 if v6 index is 0.
			
			ifa = (struct ifaddrs *) calloc( 1, sizeof( *ifa ) );
			require_action( ifa, exit, err = WSAENOBUFS );
			
			*next = ifa;
			next  = &ifa->ifa_next;
			
			// Get the interface name. This is the unfriendly GUID name.
			
			ifa->ifa_name = strdup( iaa->AdapterName );
			require_action( ifa->ifa_name, exit, err = WSAENOBUFS );
			
			// Get the display name as UTF-8.
			
			err = (DWORD) WCHARtoUTF8Copy( iaa->FriendlyName, &ifa->displayName );
			check_noerr( err ); 
			if( err ) ifa->displayName = strdup( "?" );
			require_action( ifa->displayName, exit, err = WSAENOBUFS );
			
			// Get the interface flags.
			
			ifa->ifa_flags = 0;
			if( iaa->OperStatus == IfOperStatusUp ) 		ifa->ifa_flags |= IFF_UP;
			if( iaa->IfType == IF_TYPE_SOFTWARE_LOOPBACK )	ifa->ifa_flags |= IFF_LOOPBACK;
			if( !( iaa->Flags & IP_ADAPTER_NO_MULTICAST ) )	ifa->ifa_flags |= IFF_MULTICAST;
			
			// Get the interface index. If IPv6 is not available, use the IPv4 index as the scope ID.
			
			ifa->ipv4Index	= iaa->IfIndex;
			ifa->scopeID	= ipv6IfIndex ? ipv6IfIndex : iaa->IfIndex;
			
			// Get the address.
			
			switch( family )
			{
				case AF_INET:
				case AF_INET6:
					ifa->ifa_addr = (struct sockaddr *) malloc( (size_t) addr->Address.iSockaddrLength );
					require_action( ifa->ifa_addr, exit, err = WSAENOBUFS );
					memcpy( ifa->ifa_addr, addr->Address.lpSockaddr, (size_t) addr->Address.iSockaddrLength );
					break;
				
				default:
					break;
			}
			check( ifa->ifa_addr );
			
			// Get the subnet mask (IPv4)/link prefix (IPv6). It is specified as a bit length (e.g. 24 for 255.255.255.0).
			
			prefixLength = 0;
			for( prefixIndex = 0, prefix = firstPrefix; prefix; ++prefixIndex, prefix = prefix->Next )
			{
				if( prefix->Address.lpSockaddr->sa_family == family )
				{
					prefixLength = prefix->PrefixLength;
					if( prefixIndex == addrIndex ) break;
				}
			}
			if( !prefix )
			{
				dlog( kLogLevelNotice, DEBUG_NAME "%s: bad prefix list, last family prefix: %d\n", __ROUTINE__, prefixLength );
			}
			switch( family )
			{
				case AF_INET:
				{
					struct sockaddr_in *		sa4;
					
					require_action( prefixLength <= 32, exit, err = ERROR_INVALID_DATA );
					
					sa4 = (struct sockaddr_in *) calloc( 1, sizeof( *sa4 ) );
					require_action( sa4, exit, err = WSAENOBUFS );
					
					sa4->sin_family = AF_INET;
					if( iaa->IfType == IF_TYPE_SOFTWARE_LOOPBACK )
					{
						sa4->sin_addr.s_addr = htonl( 0xFF000000U );
					}
					else
					{
						err = GetIPv4AddressInfo( ifa->ifa_addr, NULL, sa4 );
						if( err )
						{
							dlog( kLogLevelNotice, "%s: %##a not found in IPv4 table\n", __ROUTINE__, ifa->ifa_addr );
							sa4->sin_addr.s_addr = htonl( 0xFFFF0000U );
						}
					}
					ifa->ifa_netmask = (struct sockaddr *) sa4;
					break;
				}
				
				case AF_INET6:
				{
					struct sockaddr_in6 *		sa6;
					int							len;
					int							maskIndex;
					uint8_t						maskByte;
					
					require_action( prefixLength <= 128, exit, err = ERROR_INVALID_DATA );
					
					sa6 = (struct sockaddr_in6 *) calloc( 1, sizeof( *sa6 ) );
					require_action( sa6, exit, err = WSAENOBUFS );
					sa6->sin6_family = AF_INET6;
					
					if( prefixLength == 0 )
					{
						if( iaa->IfType != IF_TYPE_SOFTWARE_LOOPBACK )
						{
							dlog( kLogLevelWarning, DEBUG_NAME "%s: no IPv6 link prefix for %##a, using /128\n", 
								__ROUTINE__, ifa->ifa_addr );
						}
						prefixLength = 128;
					}
					maskIndex = 0;
					for( len = (int) prefixLength; len > 0; len -= 8 )
					{
						if( len >= 8 ) maskByte = 0xFF;
						else		   maskByte = (uint8_t)( ( 0xFFU << ( 8 - len ) ) & 0xFFU );
						sa6->sin6_addr.s6_addr[ maskIndex++ ] = maskByte;
					}
					ifa->ifa_netmask = (struct sockaddr *) sa6;
					break;
				}
				
				default:
					break;
			}
		}
	}
	
	// Success!
	
	if( outAddrs )
	{
		*outAddrs = head;
		head = NULL;
	}
	err = ERROR_SUCCESS;
	
exit:
	if( head )		freeifaddrs( head );
	if( iaaList )	free( iaaList );
	return( (int) err );
}

#if( !TARGET_OS_WINDOWS_CE )
//===========================================================================================================================
//	getifaddrs_ipv4
//===========================================================================================================================

static int	getifaddrs_ipv4( struct ifaddrs **outAddrs )
{
	int						err;
	SocketRef				sock;
	DWORD					size;
	DWORD					actualSize;
	INTERFACE_INFO *		buffer;
	INTERFACE_INFO *		tempBuffer;
	INTERFACE_INFO *		ifInfo;
	int						n;
	int						i;
	struct ifaddrs *		head;
	struct ifaddrs **		next;
	struct ifaddrs *		ifa;
	struct sockaddr_in *	sa4;
	
	sock	= kInvalidSocketRef;
	buffer	= NULL;
	head	= NULL;
	next	= &head;
	
	// Get the interface list. WSAIoctl is called with SIO_GET_INTERFACE_LIST, but since this does not provide a 
	// way to determine the size of the interface list beforehand, we have to start with an initial size guess and
	// call WSAIoctl repeatedly with increasing buffer sizes until it succeeds. Limit this to 100 tries for safety.
	
	sock = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
	err = translate_errno( IsValidSocket( sock ), errno_compat(), kUnknownErr );
	require_noerr( err, exit );
	
	n = 0;
	size = 16 * sizeof( INTERFACE_INFO );
	for( ;; )
	{
		tempBuffer = (INTERFACE_INFO *) realloc( buffer, size );
		require_action( tempBuffer, exit, err = WSAENOBUFS );
		buffer = tempBuffer;
		
		err = WSAIoctl( sock, SIO_GET_INTERFACE_LIST, NULL, 0, buffer, size, &actualSize, NULL, NULL );
		if( err == 0 ) break;
		
		++n;
		require_action( n < 100, exit, err = WSAEADDRNOTAVAIL );
		size += ( 16 * sizeof( INTERFACE_INFO ) );
	}
	check( actualSize <= size );
	check( ( actualSize % sizeof( INTERFACE_INFO ) ) == 0 );
	n = (int)( actualSize / sizeof( INTERFACE_INFO ) );
	
	// Process the raw interface list and build a linked list of IPv4 interfaces.
	
	for( i = 0; i < n; ++i )
	{
		ifInfo = &buffer[ i ];
		if( ifInfo->iiAddress.Address.sa_family != AF_INET ) continue;
		
		ifa = (struct ifaddrs *) calloc( 1, sizeof( *ifa ) );
		require_action( ifa, exit, err = WSAENOBUFS );
		
		*next = ifa;
		next  = &ifa->ifa_next;
		
		// Get the addresses.
		
		sa4 = &ifInfo->iiAddress.AddressIn;
		ifa->ifa_addr = (struct sockaddr *) malloc( sizeof( *sa4 ) );
		require_action( ifa->ifa_addr, exit, err = WSAENOBUFS );
		memcpy( ifa->ifa_addr, sa4, sizeof( *sa4 ) );
		
		sa4 = &ifInfo->iiNetmask.AddressIn;
		ifa->ifa_netmask = (struct sockaddr *) malloc( sizeof( *sa4 ) );
		require_action( ifa->ifa_netmask, exit, err = WSAENOBUFS );
		memcpy( ifa->ifa_netmask, sa4, sizeof( *sa4 ) );
		
		// Get the interface index. 
		
		err = GetIPv4AddressInfo( ifa->ifa_addr, &ifa->ipv4Index, NULL );
		check_noerr_string( err, "addr not found in IP table" );
		if( err ) ifa->ipv4Index = (uint32_t)( i + 1 );
		ifa->scopeID = ifa->ipv4Index;
		
		// Get the name. No name is available so just use the scopeID.
		
		ifa->ifa_name = (char *) calloc( 1, 16 );
		require_action( ifa->ifa_name, exit, err = WSAENOBUFS );
		snprintf( ifa->ifa_name, 16, "%u", ifa->scopeID );
		
		// Get the display name as UTF-8. Just copy the unfriendly name because nothing better is available.
		
		ifa->displayName = strdup( ifa->ifa_name );
		require_action( ifa->ifa_name, exit, err = WSAENOBUFS );
		
		// Get the interface flags.
		
		ifa->ifa_flags = (u_int) ifInfo->iiFlags;
	}
	
	// Success!
	
	if( outAddrs )
	{
		*outAddrs = head;
		head = NULL;
	}
	err = 0;
	
exit:
	if( head )					freeifaddrs( head );
	if( buffer )				free( buffer );
	if( IsValidSocket( sock ) )	close_compat( sock );
	return( err );
}
#endif	// !TARGET_OS_WINDOWS_CE )

#if( TARGET_OS_WINDOWS_CE )
//===========================================================================================================================
//	getifaddrs_ce
//===========================================================================================================================

static int	getifaddrs_ce( struct ifaddrs **outAddrs )
{
	int							err;
	SocketRef					sock;
	DWORD						size;
	void *						buffer;
	SOCKET_ADDRESS_LIST *		addressList;
	struct ifaddrs *			head;
	struct ifaddrs **			next;
	struct ifaddrs *			ifa;
	int							n;
	int							i;
	
	sock 	= kInvalidSocketRef;
	buffer	= NULL;
	head	= NULL;
	next	= &head;
	
	// Open a temporary socket because one is needed to use WSAIoctl (we'll close it before exiting this function).
	
	sock = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
	err = translate_errno( IsValidSocket( sock ), errno_compat(), kUnknownErr );
	require_noerr( err, exit );
	
	// Call WSAIoctl with SIO_ADDRESS_LIST_QUERY and pass a null buffer. This call will fail, but the size needed to 
	// for the request will be filled in. Once we know the size, allocate a buffer to hold the entire list.
	//
	// NOTE: Due to a bug in Windows CE, the size returned by WSAIoctl fails when trying to actually get the list, 
	// so increase the returned size to at least the size of a single-entry list then double it as a workaround.
	
	size = 0;
	WSAIoctl( sock, SIO_ADDRESS_LIST_QUERY, NULL, 0, NULL, 0, &size, NULL, NULL );
	require_action( size > 0, exit, err = -1 );
	if( size < sizeof( SOCKET_ADDRESS_LIST ) ) size = sizeof( SOCKET_ADDRESS_LIST );
	size *= 2;
	
	buffer = calloc( 1, size );
	require_action( buffer, exit, err = -1 );
	
	// We now know the size of the list and have a buffer to hold so call WSAIoctl again to get it.
	
	err = WSAIoctl( sock, SIO_ADDRESS_LIST_QUERY, NULL, 0, buffer, size, &size, NULL, NULL );
	require_noerr( err, exit );
	addressList = (SOCKET_ADDRESS_LIST *) buffer;
	
	// Process the raw interface list and build a linked list of interfaces.
	//
	// NOTE: Due to a bug in Windows CE, the iAddressCount field may be 0. If so, try to dynamically determine the count
	// by calculating how many addresses can fit in the returned size. It's not perfect, but it shouldn't be too unsafe
	// because we allocated 2x the requested size and zeroed it so we should be able to detect when going beyond the end.
	
	n = addressList->iAddressCount;
	if( n == 0 )
	{
		n = (int)( ( size - offsetof( SOCKET_ADDRESS_LIST, Address ) ) / ( sizeof( SOCKET_ADDRESS ) + sizeof( struct sockaddr ) ) );
	}
	for( i = 0; i < n; ++i )
	{
		struct sockaddr_in *		sa4;
		
		// Exit the loop if the sockaddr looks invalid. This may happen because iAddressCount is incorrect.
		
		if( addressList->Address[ i ].iSockaddrLength != sizeof( struct sockaddr_in ) ) break;		// Bad sockaddr?
		if( addressList->Address[ i ].lpSockaddr->sa_family != AF_INET ) 				continue;	// IPv4 only.
		
		ifa = (struct ifaddrs *) calloc( 1, sizeof( *ifa ) );
		require_action( ifa, exit, err = WSAENOBUFS );
		
		*next = ifa;
		next  = &ifa->ifa_next;
				
		// Get the flags. Note: SIO_ADDRESS_LIST_QUERY does not report flags so just fake IFF_UP and IFF_MULTICAST.
		
		ifa->ifa_flags = IFF_UP | IFF_MULTICAST;
		
		// Get the address.
		
		sa4 = (struct sockaddr_in *) addressList->Address[ i ].lpSockaddr;
		ifa->ifa_addr = (struct sockaddr *) malloc( sizeof( *sa4 ) );
		require_action( ifa->ifa_addr, exit, err = WSAENOBUFS );
		memcpy( ifa->ifa_addr, sa4, sizeof( *sa4 ) );
		
		// Get the subnet mask and interface index. 
		
		sa4 = (struct sockaddr_in *) calloc( 1, sizeof( *sa4 ) );
		require_action( sa4, exit, err = WSAENOBUFS );
		sa4->sin_family = AF_INET;
		err = GetIPv4AddressInfo( ifa->ifa_addr, &ifa->ipv4Index, sa4 );
		check_noerr_string( err, "addr not found in IP table" );
		if( err )
		{
			ifa->ipv4Index = (uint32_t)( i + 1 );
			sa4->sin_addr.s_addr = htonl( 0xFFFF0000U );
		}
		ifa->scopeID = ifa->ipv4Index;
		ifa->ifa_netmask = (struct sockaddr *) sa4;
		
		// Get the name. No name is available so just use the scopeID.
		
		ifa->ifa_name = (char *) calloc( 1, 16 );
		require_action( ifa->ifa_name, exit, err = WSAENOBUFS );
		sprintf( ifa->ifa_name, "%u", ifa->scopeID );
		
		// Get the display name as UTF-8. Just copy the unfriendly name because nothing better is available.
		
		ifa->displayName = strdup( ifa->ifa_name );
		require_action( ifa->ifa_name, exit, err = WSAENOBUFS );
	}
	
	// Success!
	
	if( outAddrs )
	{
		*outAddrs = head;
		head = NULL;
	}
	err = 0;
	
exit:
	if( head )					freeifaddrs( head );
	if( buffer ) 				free( buffer );
	if( IsValidSocket( sock ) )	closesocket( sock );
	return( err );
}
#endif	// TARGET_OS_WINDOWS_CE )

//===========================================================================================================================
//	freeifaddrs
//===========================================================================================================================

void	freeifaddrs( struct ifaddrs *inIFAs )
{
	struct ifaddrs *		p;
	struct ifaddrs *		q;
		
	for( p = inIFAs; p; p = q )
	{
		q = p->ifa_next;
		
		ForgetMem( &p->ifa_name );
		ForgetMem( &p->ifa_addr );
		ForgetMem( &p->ifa_netmask );
		ForgetMem( &p->ifa_broadaddr );
		ForgetMem( &p->ifa_dstaddr );
		ForgetMem( &p->ifa_data );
		ForgetMem( &p->displayName );
		free( p );
	}
}

//===========================================================================================================================
//	if_nametoindex
//===========================================================================================================================

uint32_t	if_nametoindex( const char *inIfName )
{
	uint32_t				ifindex;
	struct ifaddrs *		ifList;
	struct ifaddrs *		ifa;
	
	ifindex = 0;
	ifList = NULL;
	getifaddrs( &ifList );
	for( ifa = ifList; ifa; ifa = ifa->ifa_next )
	{
		if( ifa->ifa_name && ( strcmp( ifa->ifa_name, inIfName ) == 0 ) )
		{
			ifindex = ifa->scopeID;
			break;
		}
	}
	if( ifList ) freeifaddrs( ifList );
	return( ifindex );
}

#if 0
#pragma mark -
#pragma mark == QOS ==
#endif

//===========================================================================================================================
//	QOS
//===========================================================================================================================

typedef BOOL ( WINAPI *QOSCreateHandleFunc )( PQOS_VERSION Version, PHANDLE QOSHandle );

typedef BOOL ( WINAPI *QOSCloseHandleFunc )( HANDLE QOSHandle );

typedef BOOL
	( WINAPI *QOSAddSocketToFlowFunc )( 
		HANDLE				QOSHandle, 
		SOCKET				Socket, 
		PSOCKADDR			DestAddr, 
		QOS_TRAFFIC_TYPE	TrafficType, 
		DWORD				Flags, 
		PQOS_FLOWID			FlowId );

DEBUG_STATIC OSStatus	__QOSEnsureInitialized( void );

static BOOL								gQOSInitialized					= FALSE;
static HMODULE							gQOSLibraryHandle				= NULL;
static QOSCreateHandleFunc				gQOSCreateHandleFunc			= NULL;
static QOSCloseHandleFunc				gQOSCloseHandleFunc				= NULL;
static QOSAddSocketToFlowFunc			gQOSAddSocketToFlowFunc			= NULL;

//===========================================================================================================================
//	__QOSEnsureInitialized
//===========================================================================================================================

DEBUG_STATIC OSStatus	__QOSEnsureInitialized( void )
{
	OSStatus		err;
	
	require_action_quiet( !gQOSInitialized, exit, err = kNoErr );
	
	gQOSLibraryHandle = LoadLibrary( TEXT( "Qwave.dll" ) );
	err = map_global_value_errno( gQOSLibraryHandle, gQOSLibraryHandle );
	require_noerr_quiet( err, exit );
	
	gQOSCreateHandleFunc = (QOSCreateHandleFunc) GetProcAddressCompat( gQOSLibraryHandle, "QOSCreateHandle" );
	err = map_global_value_errno( gQOSCreateHandleFunc, gQOSCreateHandleFunc );
	require_noerr( err, exit );
	
	gQOSCloseHandleFunc = (QOSCloseHandleFunc) GetProcAddressCompat( gQOSLibraryHandle, "QOSCloseHandle" );
	err = map_global_value_errno( gQOSCloseHandleFunc, gQOSCloseHandleFunc );
	require_noerr( err, exit );
	
	gQOSAddSocketToFlowFunc = (QOSAddSocketToFlowFunc) GetProcAddressCompat( gQOSLibraryHandle, "QOSAddSocketToFlow" );
	err = map_global_value_errno( gQOSAddSocketToFlowFunc, gQOSAddSocketToFlowFunc );
	require_noerr( err, exit );
	
	gQOSInitialized = TRUE;
	
exit:
	return( err );
}

//===========================================================================================================================
//	QOSCreate
//===========================================================================================================================

OSStatus	QOSCreate( QOSRef *outRef )
{
	OSStatus		err;
	QOS_VERSION		version;
	HANDLE			qosHandle;
	BOOL			good;
	
	err = __QOSEnsureInitialized();
	require_noerr_quiet( err, exit );
	
	version.MajorVersion = 1;
    version.MinorVersion = 0;
	good = gQOSCreateHandleFunc( &version, &qosHandle );
	err = map_global_value_errno( good, good );
	require_noerr( err, exit );
	
	*outRef = qosHandle;
	
exit:
	return( err );
}

//===========================================================================================================================
//	QOSDelete
//===========================================================================================================================

OSStatus	QOSDelete( QOSRef inRef )
{
	if( inRef )
	{
		check( gQOSCloseHandleFunc );
		if( gQOSCloseHandleFunc )
		{
			gQOSCloseHandleFunc( inRef );
		}
	}
	return( kNoErr );
}

//===========================================================================================================================
//	QOSSet
//===========================================================================================================================

OSStatus	QOSSet( QOSRef inRef, SocketRef inSock, QOSTrafficType inType )
{
	OSStatus		err;
	BOOL			good;
	QOS_FLOWID		flowID;
	
	require_action( inRef, exit, err = kParamErr );
	require_action( gQOSAddSocketToFlowFunc, exit, err = kNotInitializedErr );
	
	good = gQOSAddSocketToFlowFunc( inRef, inSock, NULL, inType, 0, &flowID );
	err = map_global_value_errno( good, good );
	require_noerr( err, exit );
	
exit:
	return( err );	
}

#if 0
#pragma mark -
#endif

//===========================================================================================================================
//	GetIPv4AddressInfo
//===========================================================================================================================

static OSStatus	GetIPv4AddressInfo( struct sockaddr *inAddr, uint32_t *outIndex, struct sockaddr_in *ioSubnetMask )
{
	OSStatus				err;
	PMIB_IPADDRTABLE		table;
	PMIB_IPADDRTABLE		tmpTable;
	DWORD					size;
	DWORD					tmpSize;
	DWORD					i;
	DWORD					n;
	
	table = NULL;
	require_action( inAddr->sa_family == AF_INET, exit, err = kUnsupportedErr );
	
	// Get the IP address table. First with no buffer to get the size and realloc until the buffer is big enough.
	// This needs to loop to avoid a window where the table changes in-between the realloc and GetIpAddrTable calls.
	
	size = 0;
	while( GetIpAddrTable( table, &size, 0 ) == ERROR_INSUFFICIENT_BUFFER )
	{
		tmpTable = (MIB_IPADDRTABLE *) realloc( table, size );
		require_action( tmpTable, exit, err = kNoMemoryErr );
		table = tmpTable;
	}
	require_action( size >= offsetof( MIB_IPADDRTABLE, table ), exit, err = kSizeErr );
	n = table->dwNumEntries;
	tmpSize = offsetof( MIB_IPADDRTABLE, table ) + ( n * sizeof( table->table[ 0 ] ) );
	require_action( size >= tmpSize, exit, err = kSizeErr );
	
	// Search the table for a matching address.
	
	err = kParamErr;
	for( i = 0; i < n; i++ )
	{
		if ( ( (struct sockaddr_in *) inAddr )->sin_addr.s_addr == table->table[ i ].dwAddr )
		{
			if( outIndex ) *outIndex							= table->table[ i ].dwIndex;
			if( ioSubnetMask ) ioSubnetMask->sin_addr.s_addr	= table->table[ i ].dwMask;
			err = kNoErr;
			break;
		}
	}
	
exit:
	if( table ) free( table );
	return( err );
}

//===========================================================================================================================
//	WCHARtoUTF8Copy
//===========================================================================================================================

static OSStatus	WCHARtoUTF8Copy( const WCHAR *inString, char **outUTF8 )
{
	OSStatus		err;
	int				len;
	char *			utf8;
	
	utf8 = NULL;
	
	len = WideCharToMultiByte( CP_UTF8, 0, inString, -1, NULL, 0, NULL, NULL );
	err = translate_errno( len > 0, errno_compat(), kUnknownErr );
	require_noerr( err, exit );
	
	utf8 = (char *) malloc( (size_t) len );
	require_action( utf8, exit, err = kNoMemoryErr );
	
	len = WideCharToMultiByte( CP_UTF8, 0, inString, -1, utf8, len, NULL, NULL );
	err = translate_errno( len > 0, errno_compat(), kUnknownErr );
	require_noerr( err, exit );
	
	*outUTF8 = utf8;
	utf8 = NULL;
	
exit:
	if( utf8 ) free( utf8 );
	return( err );
}
