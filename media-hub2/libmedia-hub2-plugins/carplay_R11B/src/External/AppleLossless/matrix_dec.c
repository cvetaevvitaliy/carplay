/*
	File:    	matrix_dec.c
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
	
	Copyright (C) 2004-2014 Apple Inc. All Rights Reserved.
	
	DPAG mixing/matrixing decode routines.
*/

#include "matrixlib.h"
	#include "APSCommonServices.h"
	#include "APSDebugServices.h"
	
	#define Assert( a )		check( a )

// up to 24-bit "offset" macros for the individual bytes of a 20/24-bit word
#if TARGET_RT_BIG_ENDIAN
	#define LBYTE	2
	#define MBYTE	1
	#define HBYTE	0
#else
	#define LBYTE	0
	#define MBYTE	1
	#define HBYTE	2
#endif

/*
    There is no plain middle-side option; instead there are various mixing
    modes including middle-side, each lossless, as embodied in the mix()
    and unmix() functions.  These functions exploit a generalized middle-side
    transformation:
    
    u := [(rL + (m-r)R)/m];
    v := L - R;
    
    where [ ] denotes integer floor.  The (lossless) inverse is
    
    L = u + v - [rV/m];
    R = L - v;
*/

// 16-bit routines

void unmix16( int32_t * u, int32_t * v, int16_t * out, unsigned int stride, int numSamples, int mixbits, int mixres )
{
	int16_t *	op = out;
	int 		j;

	if ( mixres != 0 )
	{
		/* matrixed stereo */
		for ( j = 0; j < numSamples; j++ )
		{
			int32_t		l, r;

			l = u[j] + v[j] - ((mixres * v[j]) >> mixbits);
			r = l - v[j];

			op[0] = (int16_t) l;
			op[1] = (int16_t) r;
			op += stride;
		} 
	}
	else
	{
		/* Conventional separated stereo. */
		for ( j = 0; j < numSamples; j++ )
		{
			op[0] = (int16_t) u[j];
			op[1] = (int16_t) v[j];
			op += stride;
		}
	}
}

// 20-bit routines
// - the 20 bits of data are left-justified in 3 bytes of storage but right-aligned for input/output predictor buffers

void unmix20( int32_t * u, int32_t * v, uint8_t * out, unsigned int stride, int numSamples, int mixbits, int mixres )
{
	uint8_t *	op = out;
	int 		j;

	if ( mixres != 0 )
	{
		/* matrixed stereo */
		for ( j = 0; j < numSamples; j++ )
		{
			int32_t		l, r;

			l = u[j] + v[j] - ((mixres * v[j]) >> mixbits);
			r = l - v[j];

			l <<= 4;
			r <<= 4;

			op[HBYTE] = (uint8_t)((l >> 16) & 0xffu);
			op[MBYTE] = (uint8_t)((l >>  8) & 0xffu);
			op[LBYTE] = (uint8_t)((l >>  0) & 0xffu);
			op += 3;

			op[HBYTE] = (uint8_t)((r >> 16) & 0xffu);
			op[MBYTE] = (uint8_t)((r >>  8) & 0xffu);
			op[LBYTE] = (uint8_t)((r >>  0) & 0xffu);

			op += (stride - 1) * 3;
		}
	}
	else 
	{
		/* Conventional separated stereo. */
		for ( j = 0; j < numSamples; j++ )
		{
			int32_t		val;

			val = u[j] << 4;
			op[HBYTE] = (uint8_t)((val >> 16) & 0xffu);
			op[MBYTE] = (uint8_t)((val >>  8) & 0xffu);
			op[LBYTE] = (uint8_t)((val >>  0) & 0xffu);
			op += 3;

			val = v[j] << 4;
			op[HBYTE] = (uint8_t)((val >> 16) & 0xffu);
			op[MBYTE] = (uint8_t)((val >>  8) & 0xffu);
			op[LBYTE] = (uint8_t)((val >>  0) & 0xffu);

			op += (stride - 1) * 3;
		}
	}
}

// 24-bit routines
// - the 24 bits of data are right-justified in the input/output predictor buffers

void unmix24( int32_t * u, int32_t * v, uint8_t * out, unsigned int stride, int numSamples,
				int mixbits, int mixres, uint16_t * shiftUV, int bytesShifted )
{
	uint8_t *	op = out;
	int			shift = bytesShifted * 8;
	int32_t		l, r;
	int 		j, k;

	if ( mixres != 0 )
	{
		/* matrixed stereo */
		if ( bytesShifted != 0 )
		{
			for ( j = 0, k = 0; j < numSamples; j++, k += 2 )
			{
				l = u[j] + v[j] - ((mixres * v[j]) >> mixbits);
				r = l - v[j];

				l = (l << shift) | (uint32_t) shiftUV[k + 0];
				r = (r << shift) | (uint32_t) shiftUV[k + 1];

				op[HBYTE] = (uint8_t)((l >> 16) & 0xffu);
				op[MBYTE] = (uint8_t)((l >>  8) & 0xffu);
				op[LBYTE] = (uint8_t)((l >>  0) & 0xffu);
				op += 3;

				op[HBYTE] = (uint8_t)((r >> 16) & 0xffu);
				op[MBYTE] = (uint8_t)((r >>  8) & 0xffu);
				op[LBYTE] = (uint8_t)((r >>  0) & 0xffu);

				op += (stride - 1) * 3;
			}
		}
		else
		{
			for ( j = 0; j < numSamples; j++ )
			{
				l = u[j] + v[j] - ((mixres * v[j]) >> mixbits);
				r = l - v[j];

				op[HBYTE] = (uint8_t)((l >> 16) & 0xffu);
				op[MBYTE] = (uint8_t)((l >>  8) & 0xffu);
				op[LBYTE] = (uint8_t)((l >>  0) & 0xffu);
				op += 3;

				op[HBYTE] = (uint8_t)((r >> 16) & 0xffu);
				op[MBYTE] = (uint8_t)((r >>  8) & 0xffu);
				op[LBYTE] = (uint8_t)((r >>  0) & 0xffu);

				op += (stride - 1) * 3;
			}
		}
	}
	else 
	{
		/* Conventional separated stereo. */
		if ( bytesShifted != 0 )
		{
			for ( j = 0, k = 0; j < numSamples; j++, k += 2 )
			{
				l = u[j];
				r = v[j];

				l = (l << shift) | (uint32_t) shiftUV[k + 0];
				r = (r << shift) | (uint32_t) shiftUV[k + 1];

				op[HBYTE] = (uint8_t)((l >> 16) & 0xffu);
				op[MBYTE] = (uint8_t)((l >>  8) & 0xffu);
				op[LBYTE] = (uint8_t)((l >>  0) & 0xffu);
				op += 3;

				op[HBYTE] = (uint8_t)((r >> 16) & 0xffu);
				op[MBYTE] = (uint8_t)((r >>  8) & 0xffu);
				op[LBYTE] = (uint8_t)((r >>  0) & 0xffu);

				op += (stride - 1) * 3;
			}
		}
		else
		{
			for ( j = 0; j < numSamples; j++ )
			{
				int32_t		val;

				val = u[j];
				op[HBYTE] = (uint8_t)((val >> 16) & 0xffu);
				op[MBYTE] = (uint8_t)((val >>  8) & 0xffu);
				op[LBYTE] = (uint8_t)((val >>  0) & 0xffu);
				op += 3;

				val = v[j];
				op[HBYTE] = (uint8_t)((val >> 16) & 0xffu);
				op[MBYTE] = (uint8_t)((val >>  8) & 0xffu);
				op[LBYTE] = (uint8_t)((val >>  0) & 0xffu);

				op += (stride - 1) * 3;
			}
		}
	}
}

// 32-bit routines
// - note that these really expect the internal data width to be < 32 but the arrays are 32-bit
// - otherwise, the calculations might overflow into the 33rd bit and be lost
// - therefore, these routines deal with the specified "unused lower" bytes in the "shift" buffers

void unmix32( int32_t * u, int32_t * v, int32_t * out, unsigned int stride, int numSamples,
				int mixbits, int mixres, uint16_t * shiftUV, int bytesShifted )
{
	int32_t *	op = out;
	int			shift = bytesShifted * 8;
	int32_t		l, r;
	int 		j, k;

	if ( mixres != 0 )
	{
		Assert( bytesShifted != 0 );

		/* matrixed stereo with shift */
		for ( j = 0, k = 0; j < numSamples; j++, k += 2 )
		{
			int32_t		lt, rt;

			lt = u[j];
			rt = v[j];
			
			l = lt + rt - ((mixres * rt) >> mixbits);
			r = l - rt;

			op[0] = (l << shift) | (uint32_t) shiftUV[k + 0];
			op[1] = (r << shift) | (uint32_t) shiftUV[k + 1];
			op += stride;
		} 
	}
	else
	{
		if ( bytesShifted == 0 )
		{
			/* interleaving w/o shift */
			for ( j = 0; j < numSamples; j++ )
			{
				op[0] = u[j];
				op[1] = v[j];
				op += stride;
			}
		}
		else
		{
			/* interleaving with shift */
			for ( j = 0, k = 0; j < numSamples; j++, k += 2 )
			{
				op[0] = (u[j] << shift) | (uint32_t) shiftUV[k + 0];
				op[1] = (v[j] << shift) | (uint32_t) shiftUV[k + 1];
				op += stride;
			}
		}
	}
}

// 20/24-bit <-> 32-bit helper routines (not really matrixing but convenient to put here)

void copyPredictorTo24( int32_t * in, uint8_t * out, unsigned int stride, int numSamples )
{
	uint8_t *	op = out;
	int			j;

	for ( j = 0; j < numSamples; j++ )
	{
		int32_t		val = in[j];

		op[HBYTE] = (uint8_t)((val >> 16) & 0xffu);
		op[MBYTE] = (uint8_t)((val >>  8) & 0xffu);
		op[LBYTE] = (uint8_t)((val >>  0) & 0xffu);
		op += (stride * 3);
	}
}

void copyPredictorTo24Shift( int32_t * in, uint16_t * shift, uint8_t * out, unsigned int stride, int numSamples, int bytesShifted )
{
	uint8_t *	op = out;
	int			shiftVal = bytesShifted * 8;
	int			j;

	Assert( bytesShifted != 0 );

	for ( j = 0; j < numSamples; j++ )
	{
		int32_t		val = in[j];

		val = (val << shiftVal) | (uint32_t) shift[j];

		op[HBYTE] = (uint8_t)((val >> 16) & 0xffu);
		op[MBYTE] = (uint8_t)((val >>  8) & 0xffu);
		op[LBYTE] = (uint8_t)((val >>  0) & 0xffu);
		op += (stride * 3);
	}
}

void copyPredictorTo20( int32_t * in, uint8_t * out, unsigned int stride, int numSamples )
{
	uint8_t *	op = out;
	int			j;

	// 32-bit predictor values are right-aligned but 20-bit output values should be left-aligned
	// in the 24-bit output buffer
	for ( j = 0; j < numSamples; j++ )
	{
		int32_t		val = in[j];

		op[HBYTE] = (uint8_t)((val >> 12) & 0xffu);
		op[MBYTE] = (uint8_t)((val >>  4) & 0xffu);
		op[LBYTE] = (uint8_t)((val <<  4) & 0xffu);
		op += (stride * 3);
	}
}

void copyPredictorTo32( int32_t * in, int32_t * out, unsigned int stride, int numSamples )
{
	int			i, j;

	// this is only a subroutine to abstract the "iPod can only output 16-bit data" problem
	for ( i = 0, j = 0; i < numSamples; i++, j += stride )
		out[j] = in[i];
}

void copyPredictorTo32Shift( int32_t * in, uint16_t * shift, int32_t * out, unsigned int stride, int numSamples, int bytesShifted )
{
	int32_t *		op = out;
	uint32_t		shiftVal = bytesShifted * 8;
	int				j;

	Assert( bytesShifted != 0 );

	// this is only a subroutine to abstract the "iPod can only output 16-bit data" problem
	for ( j = 0; j < numSamples; j++ )
	{
		op[0] = (in[j] << shiftVal) | (uint32_t) shift[j];
		op += stride;
	}
}
