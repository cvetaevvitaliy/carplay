/*
	File:    	CommandLineUtils.c
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
	
	Copyright (C) 2008-2014 Apple Inc. All Rights Reserved.
	
	To Do:
	
		* Support command-specific -h to show help for just that option.
		* Reorder argv directly on platforms that can (e.g. POSIX) instead of making a mutable copy.
		* HeaderDoc and usage for each option time.
*/

// Microsoft deprecated standard C APIs like fopen so disable those warnings because the replacement APIs are not portable.

#if( !defined( _CRT_SECURE_NO_DEPRECATE ) )
	#define _CRT_SECURE_NO_DEPRECATE		1
#endif

#include "CommandLineUtils.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>

#include "APSCommonServices.h"
#include "APSDebugServices.h"
#include "StringUtils.h"

//===========================================================================================================================
//	Constants
//===========================================================================================================================

#define kCLIIndentWidth		4

typedef uint32_t		CLIFindFlags;
#define kCLIFindFlags_None					0
#define kCLIFindFlag_NonGlobal				( 1 << 0 ) // Find only options that aren't marked as global only.

typedef uint32_t		CLIPrintOptionsFlags;
#define kCLIPrintOptionsFlags_None			0
#define kCLIPrintOptionsFlag_LongHelp		( 1 << 0 )
#define kCLIPrintOptionsFlag_NonGlobal		( 1 << 1 )

typedef uint32_t		CLIValueFlags;
#define kCLIValueFlags_None					0
#define kCLIValueFlags_Short				( 1 << 0 )
#define kCLIValueFlags_Unset				( 1 << 1 )

//===========================================================================================================================
//	Prototypes
//===========================================================================================================================

static OSStatus		_CLIParseShortOption( CLIOption inOptions[] );
static OSStatus		_CLIParseLongOption( CLIOption inOptions[], const char *inArg );
static OSStatus		_CLIGetValue( CLIOption *inOption, CLIValueFlags inFlags );
static OSStatus		_CLIGetArg( const CLIOption *inOption, CLIValueFlags inFlags, const char **outArg );
static void			_CLIOptionError( const CLIOption *inOption, const char *inReason, CLIValueFlags inFlags );
static OSStatus		_CLIReorderArg( const char *inArg );
static CLIOption *	_CLIFindOption( CLIOption *inOptions, CLIOptionType inType, CLIFindFlags inFlags, const char *inName );
static OSStatus		_CLICheckMissingOptions( CLIOption *inOption );
static OSStatus		_CLIPrepareForMetaCommand( CLIOption *inOptions );

static void			_CLIHelp_PrintSummary( Boolean inFull );
static void			_CLIHelp_PrintCommand( CLIOption *inOption, int inShowGlobals );
static void			_CLIHelp_PrintUsageLine( CLIOption *inOption, const char *inPrefix, int inIndent );
static int			_CLIHelp_PrintOptions( const CLIOption *inOptions, int inIndent, const char *inLabel, CLIPrintOptionsFlags inFlags );
static int			_CLIHelp_PrintOptionName( const CLIOption *inOption, FILE *inFile );

static void			PrintIndentedString( FILE *inFile, int inIndent, const char *inStr );

//===========================================================================================================================
//	Globals
//===========================================================================================================================

int							gArgI				= 0;
int							gArgC				= 0;
const char **				gArgV				= NULL;
const char **				gArgVAlt			= NULL;
static int					gMutableArgC		= 0;
static const char **		gMutableArgV		= NULL;

const char *				gProgramPath		= "???";
const char *				gProgramName		= "???";
const char *				gProgramLongName	= NULL;
volatile int				gExitCode			= 0;

static CLIOption *			gRootOptions		= NULL;
static const char *			gOptionPtr			= NULL;
CLIOption *					gCLICurrentCommand	= NULL;
CLIOption *					gCLICurrentOption	= NULL;

//===========================================================================================================================
//	CLIInit
//===========================================================================================================================

void	CLIInit( int inArgC, const void *inArgV )
{
	const char *		s;
	
	gArgI = 0;
	gArgC = inArgC;
	gArgV = (const char **) inArgV;
	gProgramPath = ( inArgC > 0 ) ? gArgV[ gArgI++ ] : "?";
	s = strrchr( gProgramPath, '/' );
#if( TARGET_OS_WINDOWS )
	if( !s ) s = strrchr( gProgramPath, '\\' );
#endif
	gProgramName = s ? ( s + 1 ) : gProgramPath;
}

//===========================================================================================================================
//	CLIFree
//===========================================================================================================================

void	CLIFree( void )
{
	gArgC = 0;
	if( gArgVAlt )
	{
		free( (void *) gArgVAlt );
		gArgVAlt = NULL;
	}
	gMutableArgC = 0;
	if( gMutableArgV )
	{
		free( (void *) gMutableArgV );
		gMutableArgV = NULL;
	}
}

//===========================================================================================================================
//	CLIParse
//===========================================================================================================================

OSStatus	CLIParse( CLIOption inOptions[], CLIFlags inFlags )
{
	OSStatus			err;
	const char *		arg;
	CLIOption *			option;
	int					top, leaf;
	
	top = !gRootOptions;
	if( top )
	{
		_CLIPrepareForMetaCommand( inOptions );
		gRootOptions = inOptions;
	}
	gOptionPtr = NULL;
	
	// Parse options.
	
	for( ; gArgI < gArgC; ++gArgI )
	{
		arg = gArgV[ gArgI ];
		if( ( arg[ 0 ] != '-' ) || ( arg[ 1 ] == '\0' ) || isdigit_safe( arg[ 1 ] ) ) // Non-option.
		{
			if( !( inFlags & kCLIFlags_ReorderArgs ) ) break;
			err = _CLIReorderArg( arg );
			require_noerr( err, exit );
			continue;
		}
		if( arg[ 1 ] != '-' ) // Short options.
		{
			for( gOptionPtr = arg + 1; gOptionPtr; )
			{
				err = _CLIParseShortOption( inOptions );
				if( err ) goto exit;
			}
		}
		else if( arg[ 2 ] != '\0' ) // Long option.
		{
			err = _CLIParseLongOption( inOptions, arg + 2 );
			if( err ) goto exit;
		}
		else // "--" (end of options).
		{
			++gArgI;
			break;
		}
	}
	_CLIReorderArg( NULL ); // Reorder the rest of the args if needed.
	
	// Print usage if there is no command and there's a possible command in the list.
	
	if( inFlags & kCLIFlags_DontProcessCommands )
	{
		err = kNoErr;
		goto exit;
	}
	if( gArgI >= gArgC )
	{
		if( !_CLIFindOption( inOptions, kCLIOptionType_Command, kCLIFindFlags_None, NULL ) )
		{
			err = kNoErr;
			goto exit;
		}
		else if( inOptions->parentOption )
		{
			fprintf( stderr, "error: no %s command specified. See '%s help %s' for more information.\n", 
				inOptions->parentOption->longName, gProgramName, inOptions->parentOption->longName );
			err = kCLIArgErr;
			goto exit;
		}
		
		_CLIHelp_PrintSummary( false );
		err = kCLIArgErr;
		goto exit;
	}
	
	// Process commands.
	
	arg = gArgV[ gArgI ];
	option = _CLIFindOption( inOptions, kCLIOptionType_Command, kCLIFindFlags_None, arg );
	if( option )
	{
		gCLICurrentCommand = option;
		++gArgI;
		if( option->subOptions )
		{
			leaf = !_CLIFindOption( option->subOptions, kCLIOptionType_Command, kCLIFindFlags_None, NULL );
			if( !option->parentOption ) option->parentOption = gRootOptions;
			option->subOptions->parentOption = option; // Save parent for walking back up to the root.
			err = CLIParse( option->subOptions, leaf ? ( inFlags | kCLIFlags_ReorderArgs ) : inFlags );
			if( err ) goto exit;
		}
		else
		{
			leaf = true;
		}
		if( leaf )
		{
			// Parse and reorder any remaining args to handle options after the last command.
			
			err = CLIParse( inOptions, inFlags | kCLIFlags_ReorderArgs | kCLIFlags_DontProcessCommands );
			if( err ) goto exit;
		}
		
		err = _CLICheckMissingOptions( option );
		if( err ) goto exit;
		
		gCLICurrentOption = option;
		if( option->commandCallBack ) option->commandCallBack();
		err = kNoErr;
		goto exit;
	}
	else if( !_CLIFindOption( inOptions, kCLIOptionType_Command, kCLIFindFlags_None, NULL ) )
	{
		err = kNoErr;
		goto exit;
	}
	
	if( inOptions->parentOption )
	{
		fprintf( stderr, "error: unknown %s command '%s'. See '%s help %s' for a list of commands.\n", 
			inOptions->parentOption->longName, arg, gProgramName, inOptions->parentOption->longName );
	}
	else
	{
		fprintf( stderr, "error: unknown command '%s'. See '%s help' for a list of commands.\n", arg, gProgramName );
	}
	err = kCLIArgErr;
	goto exit;
	
exit:
	// If we're exiting from the top level and there's no error then warn about missing args. This is done last to give 
	// each level a chance to consume arguments. This also converts kEndingErr to kNoErr only at the top level so 
	// sub-levels don't process commands while unwinding, but still returns success to the caller.
	
	if( top )
	{
		if( !err ) for( ; gArgI < gArgC; ++gArgI ) FPrintF( stderr, "warning: unused argument '%s'.\n", gArgV[ gArgI ] );
		if( err == kEndingErr ) err = kNoErr;
	}
	return( err );
}

//===========================================================================================================================
//	_CLIParseShortOption
//===========================================================================================================================

static OSStatus	_CLIParseShortOption( CLIOption inOptions[] )
{
	OSStatus		err;
	CLIOption *		option;
	
	for( ; inOptions; inOptions = inOptions->parentOption )
	{
		for( option = inOptions; option->type != kCLIOptionType_End; ++option )
		{
			if( CLIOption_IsOption( option ) && ( option->shortName == gOptionPtr[ 0 ] ) )
			{
				gOptionPtr = ( gOptionPtr[ 1 ] != '\0' ) ? ( gOptionPtr + 1 ) : NULL;
				err = _CLIGetValue( option, kCLIValueFlags_Short );
				goto exit;
			}
		}
	}
	
	fprintf( stderr, "error: unknown option '%c'.\n", *gOptionPtr );
	err = kCLIArgErr;
	
exit:
	return( err );
}

//===========================================================================================================================
//	_CLIParseLongOption
//===========================================================================================================================

static OSStatus	_CLIParseLongOption( CLIOption inOptions[], const char *inArg )
{
	OSStatus			err;
	const char *		namePtr;
	const char *		nameEnd;
	size_t				nameLen;
	CLIValueFlags		flags;
	CLIOption *			option;
	
	namePtr = inArg;
	nameEnd = strchr( namePtr, '=' );
	if( !nameEnd ) nameEnd = namePtr + strlen( namePtr );
	nameLen = (size_t)( nameEnd - namePtr );
	
	flags = kCLIValueFlags_None;
	if( ( nameLen >= 3 ) && ( strncmp( namePtr, "no-", 3 ) == 0 ) )
	{
		flags |= kCLIValueFlags_Unset;
		namePtr += 3;
		nameLen -= 3;
	}
	
	for( ; inOptions; inOptions = inOptions->parentOption )
	{
		for( option = inOptions; option->type != kCLIOptionType_End; ++option )
		{
			if( !CLIOption_IsOption( option ) )							continue;
			if( !option->longName )										continue;
			if( strncmp( option->longName, namePtr, nameLen ) != 0 )	continue;
			if( option->longName[ nameLen ] != '\0' )					continue;
		
			if( *nameEnd != '\0' ) gOptionPtr = nameEnd + 1;
			err = _CLIGetValue( option, flags );
			goto exit;
		}
	}
	
	fprintf( stderr, "error: unknown option '%s'.\n", inArg );
	err = kCLIArgErr;
	
exit:
	return( err );
}

//===========================================================================================================================
//	_CLIGetValue
//===========================================================================================================================

static OSStatus	_CLIGetValue( CLIOption *inOption, CLIValueFlags inFlags )
{
	OSStatus			err;
	int					unset;
	char *				end;
	const char *		arg;
	
	unset = inFlags & kCLIValueFlags_Unset;
	if( unset && ( inOption->flags & kCLIOptionFlags_NoNegate ) )
	{
		_CLIOptionError( inOption, "can't be negated", inFlags & ~kCLIValueFlags_Unset );
		err = kCLIArgErr;
		goto exit;
	}
	if( ( unset && gOptionPtr ) || 
		( !( inFlags & kCLIValueFlags_Short ) && gOptionPtr && ( inOption->flags & kCLIOptionFlags_NoArgument ) ) )
	{
		_CLIOptionError( inOption, "doesn't take a value", inFlags );
		err = kCLIArgErr;
		goto exit;
	}
	switch( inOption->type )
	{
		case kCLIOptionType_Boolean:
			*( (int *)( inOption->valuePtr ) ) = unset ? 0 : *( (int *)( inOption->valuePtr ) ) + 1;
			err = 0;
			break;
		
		case kCLIOptionType_String:
			if( unset )
			{
				*( (const char ** )( inOption->valuePtr ) ) = NULL;
				err = 0;
			}
			else if( inOption->flags & kCLIOptionFlags_OptionalArgument && !gOptionPtr )
			{
				*( (const char ** )( inOption->valuePtr ) ) = (const char *)( inOption->defaultValue );
				err = 0;
			}
			else
			{
				err = _CLIGetArg( inOption, inFlags, (const char **)( inOption->valuePtr ) );
			}
			break;
		
		case kCLIOptionType_MultiString:
			if( unset )
			{
				StringArray_Free( *( (char ***) inOption->valuePtr ), *inOption->valueCountPtr );
				*inOption->valueCountPtr = 0;
				err = 0;
			}
			else if( inOption->flags & kCLIOptionFlags_OptionalArgument && !gOptionPtr )
			{
				err = StringArray_Append( (char ***) inOption->valuePtr, inOption->valueCountPtr, 
					(const char *)( inOption->defaultValue ) );
				require_noerr_action( err, exit, err = kCLINoMemoryErr );
			}
			else
			{
				err = _CLIGetArg( inOption, inFlags, &arg );
				if( err ) goto exit;
				
				err = StringArray_Append( (char ***) inOption->valuePtr, inOption->valueCountPtr, arg );
				require_noerr_action( err, exit, err = kCLINoMemoryErr );
			}
			break;
		
		case kCLIOptionType_CallBack:
			gCLICurrentOption = inOption;
			if( unset )
			{
				err = inOption->optionCallBack( inOption, NULL, 1 );
			}
			else if( inOption->flags & kCLIOptionFlags_NoArgument )
			{
				err = inOption->optionCallBack( inOption, NULL, 0 );
			}
			else if( ( inOption->flags & kCLIOptionFlags_OptionalArgument ) && !gOptionPtr )
			{
				err = inOption->optionCallBack( inOption, NULL, 0 );
			}
			else
			{
				err = _CLIGetArg( inOption, inFlags, &arg );
				if( !err ) err = inOption->optionCallBack( inOption, arg, 0 );
			}
			break;
		
		case kCLIOptionType_Integer:
			if( unset )
			{
				*( (int *)( inOption->valuePtr ) ) = 0;
				err = 0;
			}
			else if( ( inOption->flags & kCLIOptionFlags_OptionalArgument ) && !gOptionPtr )
			{
				*( (int *)( inOption->valuePtr ) ) = (int) inOption->defaultValue;
				err = 0;
			}
			else
			{
				err = _CLIGetArg( inOption, inFlags, &arg );
				if( err ) break;
				
				*( (int *)( inOption->valuePtr ) ) = (int) strtol( arg, &end, 0 );
				if( *end == '\0' ) break;
				
				_CLIOptionError( inOption, "expects a numerical value", inFlags );
				err = kCLIArgErr;
			}
			break;
		
		default:
			fprintf( stderr, "BUG: unknown option type %d\n", inOption->type );
			err = kCLIInternalErr;
			break;
	}
	if( !err ) inOption->flags |= kCLIOptionFlags_Specified;
	
exit:
	return( err );
}

//===========================================================================================================================
//	_CLIGetArg
//===========================================================================================================================

static OSStatus	_CLIGetArg( const CLIOption *inOption, CLIValueFlags inFlags, const char **outArg )
{
	if( gOptionPtr )
	{
		*outArg = gOptionPtr;
		gOptionPtr = NULL;
	}
	else if( ( gArgI + 1 ) < gArgC )
	{
		*outArg = gArgV[ ++gArgI ];
	}
	else
	{
		_CLIOptionError( inOption, "requires a value", inFlags );
		return( kCLIArgErr );
	}
	return( kNoErr );
}

//===========================================================================================================================
//	_CLIOptionError
//===========================================================================================================================

static void	_CLIOptionError( const CLIOption *inOption, const char *inReason, CLIValueFlags inFlags )
{
	fprintf( stderr, "error: option " );
	if(      inFlags & kCLIValueFlags_Short )	fprintf( stderr, "'%c' ",    inOption->shortName );
	else if( inFlags & kCLIValueFlags_Unset )	fprintf( stderr, "'no-%s' ", inOption->longName );
	else										fprintf( stderr, "'%s' ",    inOption->longName );
	fprintf( stderr, "%s.\n", inReason );
}

//===========================================================================================================================
//	_CLIReorderArg
//===========================================================================================================================

static OSStatus	_CLIReorderArg( const char *inArg )
{
	OSStatus		err;
	
	if( inArg )
	{
		if( gMutableArgV == NULL )
		{
			gMutableArgV = (const char **) malloc( ( gArgC + 1 ) * sizeof( *gMutableArgV ) );
			require_action( gMutableArgV, exit, err = kCLINoMemoryErr );
		}
		gMutableArgV[ gMutableArgC++ ] = inArg;
	}
	else if( gMutableArgC > 0 ) // NULL input means to reorder the rest of the args if needed.
	{
		for( ; gArgI < gArgC; ++gArgI )
		{
			gMutableArgV[ gMutableArgC++ ] = gArgV[ gArgI ];
		}
		gMutableArgV[ gMutableArgC ] = NULL;
		
		gArgI = 0;
		gArgC = gMutableArgC;
		gArgV = gMutableArgV;
		gMutableArgC = 0;
	}
	err = kNoErr; 
	
exit:
	return( err );
}

//===========================================================================================================================
//	_CLIFindOption
//===========================================================================================================================

static CLIOption *	_CLIFindOption( CLIOption *inOptions, CLIOptionType inType, CLIFindFlags inFlags, const char *inName )
{
	CLIOption *		option;
	
	if( inOptions )
	{
		for( option = inOptions; option->type != kCLIOptionType_End; ++option )
		{
			if( ( ( inType == kCLIOptionType_Any ) || ( inType == option->type ) ||
				  ( ( inType == kCLIOptionType_AnyOption ) && CLIOption_IsOption( option ) ) ) &&
				( !( inFlags & kCLIFindFlag_NonGlobal ) || !( option->flags & kCLIOptionFlags_GlobalOnly ) ) &&
				( ( inName == NULL ) || ( strcmp( option->longName, inName ) == 0 ) ) )
			{
				return( option );
			}
		}
	}
	return( NULL );
}

//===========================================================================================================================
//	_CLICheckMissingOptions
//===========================================================================================================================

static OSStatus	_CLICheckMissingOptions( CLIOption *inOption )
{
	CLIOption *		option;
	int				widest, width;
	int				missing;
	
	missing = 0;
	widest = 0;
	for( option = inOption->subOptions; option && ( option->type != kCLIOptionType_End ); ++option )
	{
		if( !( option->flags & kCLIOptionFlags_Required ) )	continue;
		if(    option->flags & kCLIOptionFlags_Specified )	continue;
		
		width = _CLIHelp_PrintOptionName( option, NULL );
		if( width > widest ) widest = width;
		
		missing = 1;
	}
	if( missing )
	{
		for( option = inOption->subOptions; option && ( option->type != kCLIOptionType_End ); ++option )
		{
			if( !( option->flags & kCLIOptionFlags_Required ) )	continue;
			if(    option->flags & kCLIOptionFlags_Specified )	continue;
			
			fprintf( stderr, "error: " );
			width = _CLIHelp_PrintOptionName( option, stderr );
			fprintf( stderr, "%*s not specified\n", widest - width, "" );
		}
	}
	return( missing ? kCLIArgErr : kNoErr );
}

//===========================================================================================================================
//	_CLIPrepareForMetaCommand
//===========================================================================================================================

static OSStatus	_CLIPrepareForMetaCommand( CLIOption *inOptions )
{
	OSStatus			err;
	CLIOption *			option;
	int					i, j;
	const char **		newArgV;
	
	for( option = inOptions; option->type != kCLIOptionType_End; ++option )
	{
		if( option->type != kCLIOptionType_Command )			continue;
		if( !( option->flags & kCLIOptionFlags_MetaCommand ) )	continue;
		if( strcmp( option->longName, gProgramName ) != 0 )		continue;
		
		// Rebuild argv with the first arg being the program name and backing up to point to it.
		// This is handle cases where there may be multiple links to the same binary, but each with a different name.
		// For example, you could symlink /usr/local/bin/test.01 to /usr/local/bin/test and then have a meta command
		// named "test.01". So when /usr/local/bin/test.01 is invoked, "test.01" will be inserted as argv[1]. This 
		// makes it look like the tool was invoked with "test.01" as the command and it's processed normally.
		
		newArgV = (const char **) malloc( ( gArgC + 2 ) * sizeof( *newArgV ) );
		require_action( newArgV, exit, err = kCLINoMemoryErr );
		
		i = 0;
		j = 0;
		if( gArgC > 0 ) newArgV[ j++ ] = gArgV[ i++ ];
		newArgV[ j++ ] = gProgramName;
		while( i < gArgC ) newArgV[ j++ ] = gArgV[ i++ ];
		newArgV[ j ] = NULL;
		
		gArgI = ( gArgC > 0 ) ? 1 : 0;
		gArgC = j;
		gArgV = newArgV;
		if( gArgVAlt ) free( (void *) gArgVAlt );
		gArgVAlt = newArgV;
		break;
	}
	err = kNoErr;
	
exit:
	return( err );
}

#if 0
#pragma mark -
#endif

//===========================================================================================================================
//	_CLIHelpOption
//===========================================================================================================================

OSStatus	_CLIHelpOption( CLIOption *inOption, const char *inArg, int inUnset )
{
	CLIOption *		option;
	
	(void) inOption;
	(void) inUnset;
	
	if( inArg )
	{
		option = _CLIFindOption( gRootOptions, kCLIOptionType_Command, kCLIFindFlags_None, inArg );
		if( option )	_CLIHelp_PrintCommand( option, 1 );
		else			fprintf( stderr, "error: unknown command '%s'.\n", inArg );
	}
	else if( gCLICurrentCommand && ( gCLICurrentCommand->commandCallBack != _CLIHelpCommand ) )
	{
		_CLIHelp_PrintCommand( gCLICurrentCommand, 1 );
	}
	else
	{
		_CLIHelp_PrintSummary( true );
	}
	return( kEndingErr );
}

//===========================================================================================================================
//	CLIHelpCommand
//===========================================================================================================================

void	CLIHelpCommand( const char *inCmd )
{
	_CLIHelpOption( NULL, inCmd, 0 );
}

void	_CLIHelpCommand( void )
{
	_CLIHelpOption( NULL, ( gArgI < gArgC ) ? gArgV[ gArgI++ ] : NULL, 0 );
}

//===========================================================================================================================
//	_CLIHelp_PrintSummary
//===========================================================================================================================

static void	_CLIHelp_PrintSummary( Boolean inFull )
{
	const CLIOption *		command;
	int						nShownOptions, nShownCommands, nHiddenCommands;
	int						widest, width;
	
	_CLIHelp_PrintUsageLine( gRootOptions, "\nUsage: ", 0 );
	nShownOptions = _CLIHelp_PrintOptions( gRootOptions, 1 * kCLIIndentWidth, NULL, kCLIPrintOptionsFlags_None );
	widest = 0;
	for( command = gRootOptions; command->type != kCLIOptionType_End; ++command )
	{
		if( command->type != kCLIOptionType_Command ) continue;
		width = (int) strlen( command->longName );
		if( width > widest ) widest = width;
	}
	nShownCommands  = 0;
	nHiddenCommands = 0;
	for( command = gRootOptions; command->type != kCLIOptionType_End; ++command )
	{
		if( command->type != kCLIOptionType_Command ) continue;
		if( !inFull && ( command->flags & kCLIOptionFlags_NotCommon ) ) { nHiddenCommands = true; continue; }
		if( nShownCommands == 0 ) fprintf( stderr, "\n%s:\n\n", inFull ? "Commands" : "Commonly used commands" );
		fprintf( stderr, "%*s%-*s    %s\n", 1 * kCLIIndentWidth, "", widest, command->longName, command->shortHelp );
		++nShownCommands;
	}
	if( nShownOptions || nShownCommands ) fputc( '\n', stderr );
	if( nHiddenCommands )		fprintf( stderr, "See '%s help' for a full list of commands.\n", gProgramName );
	if( nShownCommands )		fprintf( stderr, "See '%s help <command>' for more info about a command.\n\n", gProgramName );
	else if( nHiddenCommands )	fputc( '\n', stderr );
}

//===========================================================================================================================
//	_CLIHelp_PrintCommand
//===========================================================================================================================

static void	_CLIHelp_PrintCommand( CLIOption *inOption, int inShowGlobals )
{
	CLIOption *		option;
	
	fprintf( stderr, "\n" );
	fprintf( stderr, "NAME\n" );
	fprintf( stderr, "%*s%s %s -- %s\n", 1 * kCLIIndentWidth, "", gProgramName, inOption->longName, inOption->shortHelp );
	fprintf( stderr, "\n" );
	
	fprintf( stderr, "SYNOPSIS\n" );
	_CLIHelp_PrintUsageLine( inOption, "", 1 * kCLIIndentWidth );
	fprintf( stderr, "\n" );
	
	if( inOption->longHelp )
	{
		fprintf( stderr, "DESCRIPTION\n" );
		PrintIndentedString( stderr, 1 * kCLIIndentWidth, inOption->longHelp );
		fprintf( stderr, "\n" );
	}
	if( inShowGlobals && _CLIFindOption( gRootOptions, kCLIOptionType_AnyOption, kCLIFindFlag_NonGlobal, NULL ) )
	{
		_CLIHelp_PrintOptions( gRootOptions, 1 * kCLIIndentWidth, "GLOBAL OPTIONS", 
			kCLIPrintOptionsFlag_LongHelp | kCLIPrintOptionsFlag_NonGlobal );
	}
	if( inOption->subOptions )
	{
		_CLIHelp_PrintOptions( inOption->subOptions, 1 * kCLIIndentWidth, "OPTIONS", kCLIPrintOptionsFlag_LongHelp );
	}
	for( option = inOption->subOptions; 
		( option = _CLIFindOption( option, kCLIOptionType_Section, kCLIFindFlags_None, NULL ) ) != NULL; 
		++option )
	{
		fprintf( stderr, "%s\n", option->shortHelp );
		PrintIndentedString( stderr, 1 * kCLIIndentWidth, option->longHelp );
		fprintf( stderr, "\n" );
	}
}

//===========================================================================================================================
//	_CLIHelp_PrintUsageLine
//===========================================================================================================================

static void	_CLIHelp_PrintUsageLine( CLIOption *inOption, const char *inPrefix, int inIndent )
{
	if( !inOption->commandCallBack && inOption->subOptions )
	{
		CLIOption *		option;
		
		for( option = inOption->subOptions; 
			 ( option = _CLIFindOption( option, kCLIOptionType_Command, kCLIFindFlags_None, NULL ) ) != NULL;
			 ++option )
		{
			_CLIHelp_PrintUsageLine( option, "", inIndent );
		}
	}
	else
	{
		int		root;
		
		root = ( inOption == gRootOptions );
		fprintf( stderr, "%s%*s%s", inPrefix, inIndent, "", gProgramName );
		if( _CLIFindOption( gRootOptions, kCLIOptionType_AnyOption, root ? kCLIFindFlags_None : kCLIFindFlag_NonGlobal, NULL ) )
		{
			fprintf( stderr, " [global options]" );
		}
		if( root )
		{
			if( _CLIFindOption( gRootOptions, kCLIOptionType_Command, kCLIFindFlags_None, NULL ) )
			{
				fprintf( stderr, " <command> [options] [args]" );
			}
		}
		else
		{
			const CLIOption *		option;
			
			fprintf( stderr, " %s", inOption->longName );
			if( inOption->subOptions )
			{
				if( _CLIFindOption( inOption->subOptions, kCLIOptionType_AnyOption, kCLIFindFlags_None, NULL ) )
				{
					fprintf( stderr, " [options]" );
				}
				for( option = inOption->subOptions; option->type != kCLIOptionType_End; ++option )
				{
					char		startChar, endChar;
					
					if( option->type != kCLIOptionType_Argument ) continue;
					startChar = ( option->flags & kCLIOptionFlags_OptionalArgument ) ? '[' : '<';
					endChar   = ( option->flags & kCLIOptionFlags_OptionalArgument ) ? ']' : '>';
					fprintf( stderr, " %c%s%c", startChar, option->longName, endChar );
				}
			}
		}
		fputc( '\n', stderr );
	}
}

//===========================================================================================================================
//	_CLIHelp_PrintOptions
//===========================================================================================================================

static int	_CLIHelp_PrintOptions( const CLIOption *inOptions, int inIndent, const char *inLabel, CLIPrintOptionsFlags inFlags )
{
	const CLIOption *		option;
	int						labelShown;
	int						nShownOptions;
	int						widest, width;
	
	labelShown = 0;
	widest = 0;
	for( option = inOptions; option->type != kCLIOptionType_End; ++option )
	{
		if( !CLIOption_IsOption( option ) && ( option->type != kCLIOptionType_Argument ) ) continue;
		if( ( inFlags & kCLIPrintOptionsFlag_NonGlobal ) && ( option->flags & kCLIOptionFlags_GlobalOnly ) ) continue;
		if( !labelShown && inLabel )
		{
			fprintf( stderr, "%s", inLabel );
			labelShown = 1;
		}
		width = _CLIHelp_PrintOptionName( option, NULL );
		if( width > widest ) widest = width;
	}
	nShownOptions = 0;
	for( option = inOptions; option->type != kCLIOptionType_End; ++option )
	{
		if( ( inFlags & kCLIPrintOptionsFlag_NonGlobal ) && ( option->flags & kCLIOptionFlags_GlobalOnly ) ) continue;
		
		if( option->type == kCLIOptionType_Group )
		{
			fputc( '\n', stderr );
			if( *option->shortHelp ) fprintf( stderr, "%*s%s\n", inIndent, "", option->shortHelp );
			++nShownOptions;
			continue;
		}
		if( !CLIOption_IsOption( option ) && ( option->type != kCLIOptionType_Argument ) ) continue;
		
		if( nShownOptions == 0 ) fputc( '\n', stderr );
		fprintf( stderr, "%*s", inIndent, "" );
		width = _CLIHelp_PrintOptionName( option, stderr );
		fprintf( stderr, "%*s    %s\n", widest - width, "", option->shortHelp );
		if( ( inFlags & kCLIPrintOptionsFlag_LongHelp ) && option->longHelp )
		{
			PrintIndentedString( stderr, inIndent + kCLIIndentWidth, option->longHelp );
		}
		++nShownOptions;
	}
	if( labelShown ) fputc( '\n', stderr );
	return( nShownOptions );
}

//===========================================================================================================================
//	_CLIHelp_PrintOptionName
//===========================================================================================================================

static int	_CLIHelp_PrintOptionName( const CLIOption *inOption, FILE *inFile )
{
	int		width;
	
	width = 0;
	if( inOption->shortName != '\0' )
	{
		width += FPrintF( inFile, "-%c", inOption->shortName );
		if( inOption->longName ) width += FPrintF( inFile, ", " );
	}
	if( inOption->type == kCLIOptionType_Argument )
	{
		char		startChar, endChar;
		
		startChar = ( inOption->flags & kCLIOptionFlags_OptionalArgument ) ? '[' : '<';
		endChar   = ( inOption->flags & kCLIOptionFlags_OptionalArgument ) ? ']' : '>';
		width += FPrintF( inFile, "%c%s%c", startChar, inOption->longName, endChar );
	}
	else
	{
		if( inOption->longName ) width += FPrintF( inFile, "--%s", inOption->longName );
		if( !( inOption->flags & kCLIOptionFlags_NoArgument ) )
		{
			int					literal;
			const char *		format;
			
			literal = ( inOption->flags & kCLIOptionFlags_LiteralArgHelp ) || !inOption->argHelp;
			if( inOption->flags & kCLIOptionFlags_OptionalArgument )
			{
				if( inOption->longName ) format = literal ? "[=%s]" : "[=<%s>]";
				else					 format = literal ? "[%s]"  : "[<%s>]";
			}
			else						 format = literal ? " %s" : " <%s>";
			width += FPrintF( inFile, format, inOption->argHelp ? inOption->argHelp : "..." );
		}
	}
	return( width );
}

//===========================================================================================================================
//	Built-in Version
//===========================================================================================================================

OSStatus	_CLIVersionOption( CLIOption *inOption, const char *inArg, int inUnset )
{
	(void) inArg;
	(void) inUnset;
	
	fprintf( stdout, "%s version %s (%s)\n", gProgramLongName ? gProgramLongName : gProgramName, 
		(const char *) inOption->valuePtr, (const char *) inOption->valueCountPtr );
	return( kEndingErr );
}

void	_CLIVersionCommand( void )
{
	_CLIVersionOption( gCLICurrentOption, NULL, 0 );
}

#if 0
#pragma mark -
#endif

//===========================================================================================================================
//	CLIArgToValueOrErrQuit
//===========================================================================================================================

int	CLIArgToValueOrErrQuit( const char *inLabel, ... )
{
	va_list				args;
	const char *		arg;
	const char *		name;
	int					value;
	char *				errorStr;
	int					count, totalCount;
	
	arg = ( gArgI < gArgC ) ? gArgV[ gArgI++ ] : NULL;
	require_quiet( arg, error );
	
	va_start( args, inLabel );
	for( ;; )
	{
		name = va_arg( args, const char * );
		if( !name ) break;
		
		if( stricmp( name, kCLIArg_AnyTrueish ) == 0 )
		{
			if( IsTrueString( arg, SIZE_MAX ) )
			{
				value = 1;
				break;
			}
		}
		else if( stricmp( name, kCLIArg_AnyFalseish ) == 0 )
		{
			if( IsFalseString( arg, SIZE_MAX ) )
			{
				value = 0;
				break;
			}
		}
		else if( stricmp( name, kCLIArg_AnyInt ) == 0 )
		{
			if( sscanf( arg, "%d", &value ) == 1 )
			{
				break;
			}
		}
		else
		{
			value = va_arg( args, int );
			if( stricmp( arg, name ) == 0 )
			{
				break;
			}
		}
	}
	va_end( args );
	if( name ) goto exit;
	
error:
	errorStr = NULL;
	if( arg )	AppendPrintF( &errorStr, "error: bad %s: '%s'. It must be ", inLabel, arg );
	else		AppendPrintF( &errorStr, "error: no %s specified. It must be ", inLabel );
	
	// Count the number of items to know if we should use ", " or "or ".
	
	totalCount = 0;
	va_start( args, inLabel );
	for( ;; )
	{
		name = va_arg( args, const char * );
		if( !name ) break;
		
		if(      ( stricmp( name, kCLIArg_AnyTrueish )	== 0 ) ) {}
		else if( ( stricmp( name, kCLIArg_AnyFalseish )	== 0 ) ) {}
		else if( ( stricmp( name, kCLIArg_AnyInt )		== 0 ) ) {}
		else va_arg( args, int );
		 ++totalCount;
	}
	va_end( args );
	
	// Add a label for each allowed option.
	
	count = 0;
	va_start( args, inLabel );
	for( ;; )
	{
		name = va_arg( args, const char * );
		if( !name ) break;
		
		if(      ( stricmp( name, kCLIArg_AnyTrueish )	== 0 ) ) name = "true|yes|y|on|1";
		else if( ( stricmp( name, kCLIArg_AnyFalseish )	== 0 ) ) name = "false|no|n|off|0";
		else if( ( stricmp( name, kCLIArg_AnyInt )		== 0 ) ) name = "<number>";
		else va_arg( args, int );
		
		AppendPrintF( &errorStr, "%s%s", MapCountToOrSeparator( count, totalCount ), name );
		++count;
	}
	va_end( args );
	
	ErrQuit( 1, "%s.\n", errorStr ? errorStr : "internal failure" );
	if( errorStr ) free( errorStr );
	value = -1;
	
exit:
	return( value );
}

//===========================================================================================================================
//	ErrQuit
//===========================================================================================================================

void	ErrQuit( int inExitCode, const char *inFormat, ... )
{
	va_list		args;
	
	va_start( args, inFormat );
	VFPrintF( stderr, inFormat, args );
	va_end( args );
	
	exit( inExitCode );
}

//===========================================================================================================================
//	PrintIndentedString
//===========================================================================================================================

static void	PrintIndentedString( FILE *inFile, int inIndent, const char *inStr )
{
	const char *		line;
	const char *		eol;
	
	for( line = inStr; *line != '\0'; line = eol )
	{
		for( eol = line; ( *eol != '\0' ) && ( *eol != '\n' ); ++eol ) {}
		if( *eol  != '\0' ) ++eol;
		if( *line != '\n' ) fprintf( inFile, "%*s", inIndent, "" );
		fwrite( line, 1, (size_t)( eol - line ), inFile );
	}
}

//===========================================================================================================================
//	StringArray_Append
//===========================================================================================================================

OSStatus	StringArray_Append( char ***ioArray, size_t *ioCount, const char *inStr )
{
	OSStatus		err;
	char *			newStr;
	size_t			oldCount;
	size_t			newCount;
	char **			oldArray;
	char **			newArray;
	
	newStr = strdup( inStr );
	require_action( newStr, exit, err = kNoMemoryErr );
	
	oldCount = *ioCount;
	newCount = oldCount + 1;
	newArray = (char **) malloc( newCount * sizeof( *newArray ) );
	require_action( newArray, exit, err = kNoMemoryErr );
	
	if( oldCount > 0 )
	{
		oldArray = *ioArray;
		memcpy( newArray, oldArray, oldCount * sizeof( *oldArray ) );
		free( oldArray );
	}
	newArray[ oldCount ] = newStr;
	newStr = NULL;
	
	*ioArray = newArray;
	*ioCount = newCount;
	err = kNoErr;
	
exit:
	if( newStr ) free( newStr );
	return( err );
}

//===========================================================================================================================
//	StringArray_Free
//===========================================================================================================================

void	StringArray_Free( char **inArray, size_t inCount )
{
	size_t		i;
	
	for( i = 0; i < inCount; ++i )
	{
		free( inArray[ i ] );
	}
	if( inCount > 0 ) free( inArray );
}

//===========================================================================================================================
//	StringToInteger64
//===========================================================================================================================

int64_t	StringToInteger64( const char *inStr, int64_t inMin, uint64_t inMax, int inBase, const char **outEnd, int *outErr )
{
	int				err;
	long long		x;
	char *			end;
	
	errno = 0;
	x = strtoll( inStr, &end, inBase );
	err = errno;
	if( !err && ( ( x < inMin ) || ( ( (uint64_t) x ) > inMax ) ) )
	{
		err = ERANGE;
	}
	if( outEnd ) *outEnd = end;
	if( outErr ) *outErr = err;
	return( x );
}
