/*
	File:    	ag_dec.c
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
	
	Copyright (C) 2001-2014 Apple Inc. All Rights Reserved.
	
	Adaptive Golomb decode routines.
*/

#include "aglib.h"
#include "BitUtilities.h"
	#include "APSCommonServices.h"
	#include "APSDebugServices.h"
	
	#define Assert(a)		check(a)

	#define	RequireAction(X, ACTION) \
		do { \
			if( !( X ) ) { \
				dlog( kLogLevelAssert, "ASSERT FAILED: " #X " %s, line %d\n", __FILE__, __LINE__ ); \
				ACTION \
			} \
		} while( 0 )
	
	#if( !defined( __MACERRORS__ ) )
		#define noErr			0
		#define paramErr		-50
		#define badFileFormat	-208
	#endif
	
	#if( !defined( nil ) )
		#define nil		NULL
	#endif

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if( __GNUC__ && TARGET_CPU_PPC )
	#include <ppc_intrinsics.h>
#endif

#define CODE_TO_LONG_MAXBITS	32
#define N_MAX_MEAN_CLAMP		0xffff
#define N_MEAN_CLAMP_VAL		0xffff
#define REPORT_VAL  40

#if __GNUC__ || __clang__
#define ALWAYS_INLINE		__attribute__((always_inline))
#elif _MSC_VER
#define ALWAYS_INLINE		__forceinline
#else
#define ALWAYS_INLINE
#endif

#define USE_x86_ASM			(TARGET_OS_WIN32 && (! DEBUG))

#if __MWERKS__ && __INTEL__
// the CodeWarrior x86 compiler fails to inline some of these routines so turn off the warning
// to avoid cluttering up the code with conditionals
#pragma warn_notinlined off
#endif

/*	And on the subject of the CodeWarrior x86 compiler and inlining, I reworked a lot of this
	to help the compiler out.   In many cases this required manual inlining or a macro.  Sorry
	if it is ugly but the performance gains are well worth it.
	- WSK 5/19/04
*/

void set_standard_ag_params(AGParamRecPtr params, unsigned int fullwidth, unsigned int sectorwidth)
{
	/* Use
		fullwidth = sectorwidth = numOfSamples, for analog 1-dimensional type-short data,
		but use
		fullwidth = full image width, sectorwidth = sector (patch) width
		for such as image (2-dim.) data.
	*/
	set_ag_params( params, MB0, PB0, KB0, fullwidth, sectorwidth, MAX_RUN_DEFAULT );
}

void set_ag_params(AGParamRecPtr params, unsigned int m, unsigned int p, unsigned int k, unsigned int f, unsigned int s, unsigned int maxrun)
{
	params->mb = params->mb0 = m;
	params->pb = p;
	params->kb = k;
	params->wb = (1u<<params->kb)-1;
	params->qb = QB-params->pb; 
	params->fw = f;
	params->sw = s;
	params->maxrun = maxrun;
}

#if PRAGMA_MARK
#pragma mark -
#endif

#if TARGET_CPU_PPC

STATIC_INLINE int ALWAYS_INLINE lead( int x )
{
	return __cntlzw( x );
}

#elif USE_x86_ASM

STATIC_INLINE int lead( int m )
{
	unsigned long result;
	__asm
	{
		bsr eax, [m]
		jz thereWereNoBitsSet
		mov [result], eax
	}
	return 31 - result;

thereWereNoBitsSet:
	return 32;
}

#elif( __ARMCC_VERSION )

STATIC_INLINE int lead( int m )
{
	int		result;
	
	__asm { clz	result, m }
	return result;
}

#elif( TARGET_CPU_ARM && !__ARM_ARCH_4__ && !__ARM_ARCH_4T__ && ( __GNUC__ || __clang__ ) )

STATIC_INLINE int ALWAYS_INLINE lead( int m )
{
	int		result;
	
	__asm__( "clz %0, %1" : "=r" (result) : "r" (m) );
	return( result );
}

#elif( __ARM__ && !__ARM_ARCH_4__ && !__ARM_ARCH_4T__ )

STATIC_INLINE int lead( int m )
{
	int		result;
	
	asm { clz	result, m }
	return result;
}

#elif( TARGET_HAS_BUILTIN_CLZ )

STATIC_INLINE int ALWAYS_INLINE lead( int m )
{
	return __builtin_clz( m );
}

#else

// note: implementing this with some kind of "count leading zeros" assembly is a big performance win
STATIC_INLINE int lead( int m )
{
	long j;
	unsigned long c = (1ul << 31);

	for(j=0; j < 32; j++)
	{
		if((c & m) != 0)
			break;
		c >>= 1;
	}
	return (j);
}

#endif /* end cpu-specific lead() routines */

#if ! TARGET_CPU_PPC

#define arithmin(a, b) ((a) < (b) ? (a) : (b))

#else

STATIC_INLINE int32_t ALWAYS_INLINE arithmin(int32_t a, int32_t b)
{
    int32_t diff;
    int32_t mask;
    int32_t result;
    
    diff = a - b;
    
    // mask is -1 if a < b, else 0
    mask = diff >> 31;
    
    // if a < b, result = b + (a-b) = a
    // if a > b result = b + 0 = b
    result = b + (diff & mask);

    return result;
}

#endif	/* TARGET_CPU_PPC */

STATIC_INLINE int32_t ALWAYS_INLINE lg3a( int32_t x)
{
    int32_t result;

    x += 3;
    result = lead(x);

    return 31 - result;
}

STATIC_INLINE uint32_t ALWAYS_INLINE read32bit( uint8_t * buffer )
{
#if TARGET_CPU_PPC || TARGET_CPU_PPC64

	// just read the 32-bit word
	return *(uint32_t *)buffer;

#elif USE_x86_ASM

	// read 32-bit word and byte-swap it
	uint32_t		value = *(uint32_t *)buffer;
	__asm
	{
		mov eax, [value]
		bswap	eax
		mov [value], eax
	}
	return value;

#else

	// embedded CPUs typically can't read unaligned 32-bit words so just read the bytes
	uint32_t		value;
	
	value = ((uint32_t)buffer[0] << 24) | ((uint32_t)buffer[1] << 16) |
			 ((uint32_t)buffer[2] << 8) | (uint32_t)buffer[3];
	return value;

#endif	
}

#if PRAGMA_MARK
#pragma mark -
#endif

/*
STATIC_INLINE uint32_t ALWAYS_INLINE
get_next_fromlong(uint32_t inlong, uint32_t suff)
{
    return inlong >> (32-suff);
}
*/

#define get_next_fromlong(inlong, suff)		((inlong) >> (32 - (suff)))


STATIC_INLINE uint32_t ALWAYS_INLINE
getstreambits( uint8_t *in, int bitoffset, int numbits )
{
	uint32_t	load1, load2;
	uint32_t	byteoffset = bitoffset / 8;
	uint32_t	result;
	
	Assert( numbits <= 32 );

#if USE_x86_ASM
	load1 = *(uint32_t *)( in + byteoffset );
	__asm
	{
		mov eax, [load1]
		bswap	eax
		mov [load1], eax
	}
#else
	load1 = read32bit( in + byteoffset );
#endif

	if ( (numbits + (bitoffset & 0x7)) > 32)
	{
		int load2shift;

		result = load1 << (bitoffset & 0x7);
		load2 = (uint32_t) in[byteoffset+4];
		load2shift = (8-(numbits + (bitoffset & 0x7)-32));
		load2 >>= load2shift;
		result >>= (32-numbits);
		result |= load2;		
	}
	else
	{
		result = load1 >> (32-numbits-(bitoffset & 7));
	}

	// a shift of >= "the number of bits in the type of the value being shifted" results in undefined
	// behavior so don't try to shift by 32
	if ( numbits != (sizeof(result) * 8) )
		result &= ~(UINT32_C(0xffffffff) << numbits);
	
	return result;
}


STATIC_INLINE int32_t dyn_get(unsigned char *in, unsigned int *bitPos, unsigned int m, unsigned int k)
{
    unsigned int	tempbits = *bitPos;
    uint32_t		result;
    uint32_t		pre = 0, v;
    uint32_t		streamlong;

#if USE_x86_ASM
	streamlong = *(uint32_t *)( in + (tempbits >> 3) );
	__asm
	{
		mov eax, [streamlong]
		bswap	eax
		mov [streamlong], eax
	}
#else
	streamlong = read32bit( in + (tempbits >> 3) );
#endif
    streamlong <<= (tempbits & 7);

    /* find the number of bits in the prefix */ 
    {
        uint32_t	notI = ~streamlong;
    	pre = lead( notI);
    }

    if(pre >= MAX_PREFIX_16)
    {
        pre = MAX_PREFIX_16;
        tempbits += pre;
        streamlong <<= pre;
        result = get_next_fromlong(streamlong,MAX_DATATYPE_BITS_16);
        tempbits += MAX_DATATYPE_BITS_16;

    }
    else
    {
        // all of the bits must fit within the long we have loaded
        Assert(pre+1+k <= 32);

        tempbits += pre;
        tempbits += 1;
        streamlong <<= pre+1;
        v = get_next_fromlong(streamlong, k);
        tempbits += k;
    
        result = pre*m + v-1;

        if(v<2) {
            result -= (v-1);
            tempbits -= 1;
        }
    }

    *bitPos = tempbits;
    return result;
}


STATIC_INLINE int32_t dyn_get_32bit( uint8_t * in, unsigned int * bitPos, int m, int k, int maxbits )
{
	unsigned int	tempbits = *bitPos;
	uint32_t		v;
	uint32_t		streamlong;
	uint32_t		result;
	
#if USE_x86_ASM
	streamlong = *(uint32_t *)( in + (tempbits >> 3) );
	__asm
	{
		mov eax, [streamlong]
		bswap	eax
		mov [streamlong], eax
	}
#else
	streamlong = read32bit( in + (tempbits >> 3) );
#endif
	streamlong <<= (tempbits & 7);

	/* find the number of bits in the prefix */ 
	{
		uint32_t notI = ~streamlong;
#if USE_x86_ASM
		{
			__asm
			{
				bsr eax, [notI]
				jz thereWereNoBitsSet
				mov [result], eax
			}
			result = 31 - result;
			goto gotResult;

		thereWereNoBitsSet:
			result = 32;
		gotResult:
			;
		}
#else
		result = lead( notI);
#endif
	}
	
	if(result >= MAX_PREFIX_32)
	{
		result = getstreambits(in, tempbits+MAX_PREFIX_32, maxbits);
		tempbits += MAX_PREFIX_32 + maxbits;
	}
	else
	{		
		/* all of the bits must fit within the long we have loaded*/
		Assert(k<=14);
		Assert(result<MAX_PREFIX_32);
		Assert(result+1+k <= 32);
		
		tempbits += result;
		tempbits += 1;
		
		if (k != 1)
		{
			streamlong <<= result+1;
			v = get_next_fromlong(streamlong, k);
			tempbits += k;
			tempbits -= 1;
			result = result*m;
			
			if(v>=2)
			{
				result += (v-1);
				tempbits += 1;
			}
		}
	}

	*bitPos = tempbits;

	return result;
}

int dyn_decomp( AGParamRecPtr params, BitBuffer * bitstream, int32_t * pc, int numSamples, int maximumSize, unsigned int * outNumBits )
{
    uint8_t 		*in;
    int32_t			*outPtr = pc;
    unsigned int 	bitPos, startPos, maxPos;
    uint32_t		j, m, k, n, c, mz;
    int32_t			del, zmode;
    unsigned int 	mb;
    unsigned int	pb_local = params->pb;
    unsigned int	kb_local = params->kb;
    unsigned int	wb_local = params->wb;
    int				status;

	RequireAction( (bitstream != nil) && (pc != nil) && (outNumBits != nil), return paramErr; );
	*outNumBits = 0;

	in = bitstream->cur;
	startPos = bitstream->bitIndex;
	maxPos = bitstream->byteSize * 8;
	bitPos = startPos;

    mb = params->mb0;
    zmode = 0;

    c = 0;
	status = noErr;

    while (c < (uint32_t) numSamples)
    {
		// bail if we've run off the end of the buffer
    	RequireAction( bitPos < maxPos, status = badFileFormat; goto Exit; );

        m = (mb)>>QBSHIFT;
        k = lg3a(m);

        k = arithmin(k, kb_local);
        m = (1<<k)-1;
        
		n = dyn_get_32bit( in, &bitPos, m, k, maximumSize );

        // least significant bit is sign bit
        {
        	uint32_t	ndecode = n + zmode;
#if _MSC_VER
	#pragma warning(push)
	#pragma warning(disable:4146)	// unary minus operator applied to unsigned type, result still unsigned
#endif
            int32_t		multiplier = (- (ndecode&1));
#if _MSC_VER
	#pragma warning(pop)
#endif

            multiplier |= 1;
            del = ((ndecode+1) >> 1) * (multiplier);
        }

        *outPtr++ = del;

        c++;

        mb = pb_local*(n+zmode) + mb - ((pb_local*mb)>>QBSHIFT);

		// update mean tracking
		if (n > N_MAX_MEAN_CLAMP)
			mb = N_MEAN_CLAMP_VAL;

        zmode = 0;

        if (((mb << MMULSHIFT) < QB) && (c < (uint32_t) numSamples))
        {
            zmode = 1;
            k = lead(mb) - BITOFF+((mb+MOFF)>>MDENSHIFT);
            mz = ((1<<k)-1) & wb_local;

            n = dyn_get(in, &bitPos, mz, k);

            RequireAction(c+n <= (uint32_t) numSamples, status = badFileFormat; goto Exit; );

            for(j=0; j < n; j++)
            {
                *outPtr++ = 0;
                ++c;                    
            }

            if(n >= 65535)
            	zmode = 0;

            mb = 0;
        }
    }

Exit:
	*outNumBits = (bitPos - startPos);
	BitBufferAdvance( bitstream, *outNumBits );
	RequireAction( bitstream->cur <= bitstream->end, status = badFileFormat; );

    return status;
}
