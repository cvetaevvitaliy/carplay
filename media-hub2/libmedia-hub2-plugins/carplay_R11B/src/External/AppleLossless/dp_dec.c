/*
	File:    	dp_dec.c
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
	
	Dynamic Predictor decode routines.
*/

#include "APSCommonServices.h"
#include "dplib.h"
#include <string.h>

#if __GNUC__
#define ALWAYS_INLINE		__attribute__((always_inline))
#else
#define ALWAYS_INLINE
#endif

#if TARGET_CPU_PPC && (__MWERKS__ >= 0x3200)
// align loops to a 16 byte boundary to make the G5 happy
#pragma function_align 16
#define LOOP_ALIGN			asm { align 16 }
#else
#define LOOP_ALIGN
#endif

STATIC_INLINE int ALWAYS_INLINE sign_of_int( int32_t i )
{
    int32_t negishift;
	
    negishift = ((uint32_t)-i) >> 31;
    return negishift | (i >> 31);
}

void unpc_block( int32_t * pc1, int32_t * out, int num, int16_t * coefs, int numactive, unsigned int chanbits, unsigned int denshift )
{
	int					j, k, lim;
	int32_t				sum1, sg, sgn, top, dd;
	int32_t *			pout;
	int32_t				del, del0;
	uint32_t			chanshift = 32 - chanbits;
	int32_t				denhalf = 1<<(denshift-1);

	out[0] = pc1[0];
	if ( numactive == 0 )
	{
		// just copy if numactive == 0 (but don't bother if in/out pointers the same)
		if ( (num > 1)  && (pc1 != out) )
			memcpy( &out[1], &pc1[1], (num - 1) * sizeof(int32_t) );
		return;
	}
	if ( numactive == 31 )
	{
		// short-circuit if numactive == 31
		int32_t		prev;
		
		/*	this code is written such that the in/out buffers can be the same
			to conserve buffer space on embedded devices like the iPod
			
			(original code)
			for ( j = 1; j < num; j++ )
				del = pc1[j] + out[j-1];
				out[j] = (del << chanshift) >> chanshift;
		*/
		prev = out[0];
		for ( j = 1; j < num; j++ )
		{
			del = pc1[j] + prev;
			prev = (del << chanshift) >> chanshift;
			out[j] = prev;
		}
		return;
	}

	for ( j = 1; j <= numactive; j++ )
	{
		del = pc1[j] + out[j-1];
		out[j] = (del << chanshift) >> chanshift;
	}

	lim = numactive + 1;

	if ( numactive == 4 )
	{
		// optimization for numactive == 4
		register int16_t	a0, a1, a2, a3;
		register int32_t	b0, b1, b2, b3;

		a0 = coefs[0];
		a1 = coefs[1];
		a2 = coefs[2];
		a3 = coefs[3];

		for ( j = lim; j < num; j++ )
		{
			LOOP_ALIGN

			top = out[j - lim];
			pout = out + j - 1;

			b0 = top - pout[0];
			b1 = top - pout[-1];
			b2 = top - pout[-2];
			b3 = top - pout[-3];

			sum1 = (denhalf - a0 * b0 - a1 * b1 - a2 * b2 - a3 * b3) >> denshift;

			del = pc1[j];
			del0 = del;
			sg = sign_of_int(del);
			del += top + sum1;

			out[j] = (del << chanshift) >> chanshift;

#if HAVE_VISUAL_STUDIO
	#pragma warning(push)
	#pragma warning(disable:4244)	// conversion from 'int32_t' to 'int16_t', possible loss of data
#endif
			if ( sg > 0 )
			{
				sgn = sign_of_int( b3 );
				a3 -= sgn;
				del0 -= (4 - 3) * ((sgn * b3) >> denshift);
				if ( del0 <= 0 )
					continue;
				
				sgn = sign_of_int( b2 );
				a2 -= sgn;
				del0 -= (4 - 2) * ((sgn * b2) >> denshift);
				if ( del0 <= 0 )
					continue;
				
				sgn = sign_of_int( b1 );
				a1 -= sgn;
				del0 -= (4 - 1) * ((sgn * b1) >> denshift);
				if ( del0 <= 0 )
					continue;

				a0 -= sign_of_int( b0 );
			}
			else if ( sg < 0 )
			{
				// note: to avoid unnecessary negations, we flip the value of "sgn"
				sgn = -sign_of_int( b3 );
				a3 -= sgn;
				del0 -= (4 - 3) * ((sgn * b3) >> denshift);
				if ( del0 >= 0 )
					continue;
				
				sgn = -sign_of_int( b2 );
				a2 -= sgn;
				del0 -= (4 - 2) * ((sgn * b2) >> denshift);
				if ( del0 >= 0 )
					continue;
				
				sgn = -sign_of_int( b1 );
				a1 -= sgn;
				del0 -= (4 - 1) * ((sgn * b1) >> denshift);
				if ( del0 >= 0 )
					continue;

				a0 += sign_of_int( b0 );
			}
		}
#if HAVE_VISUAL_STUDIO
	#pragma warning(pop)
#endif

		coefs[0] = a0;
		coefs[1] = a1;
		coefs[2] = a2;
		coefs[3] = a3;
	}
	else if ( numactive == 8 )
	{
		register int16_t	a0, a1, a2, a3;
		register int16_t	a4, a5, a6, a7;
		register int32_t	b0, b1, b2, b3;
		register int32_t	b4, b5, b6, b7;

		// optimization for numactive == 8
		a0 = coefs[0];
		a1 = coefs[1];
		a2 = coefs[2];
		a3 = coefs[3];
		a4 = coefs[4];
		a5 = coefs[5];
		a6 = coefs[6];
		a7 = coefs[7];

		for ( j = lim; j < num; j++ )
		{
			LOOP_ALIGN

			top = out[j - lim];
			pout = out + j - 1;

			b0 = top - (*pout--);
			b1 = top - (*pout--);
			b2 = top - (*pout--);
			b3 = top - (*pout--);
			b4 = top - (*pout--);
			b5 = top - (*pout--);
			b6 = top - (*pout--);
			b7 = top - (*pout);

			sum1 = (denhalf - a0 * b0 - a1 * b1 - a2 * b2 - a3 * b3
					- a4 * b4 - a5 * b5 - a6 * b6 - a7 * b7) >> denshift;

			del = pc1[j];
			del0 = del;
			sg = sign_of_int(del);
			del += top + sum1;

			out[j] = (del << chanshift) >> chanshift;

#if HAVE_VISUAL_STUDIO
	#pragma warning(push)
	#pragma warning(disable:4244)	// conversion from 'int32_t' to 'int16_t', possible loss of data
#endif
			if ( sg > 0 )
			{
				sgn = sign_of_int( b7 );
				a7 -= sgn;
				del0 -= 1 * ((sgn * b7) >> denshift);
				if ( del0 <= 0 )
					continue;
				
				sgn = sign_of_int( b6 );
				a6 -= sgn;
				del0 -= 2 * ((sgn * b6) >> denshift);
				if ( del0 <= 0 )
					continue;
				
				sgn = sign_of_int( b5 );
				a5 -= sgn;
				del0 -= 3 * ((sgn * b5) >> denshift);
				if ( del0 <= 0 )
					continue;

				sgn = sign_of_int( b4 );
				a4 -= sgn;
				del0 -= 4 * ((sgn * b4) >> denshift);
				if ( del0 <= 0 )
					continue;
				
				sgn = sign_of_int( b3 );
				a3 -= sgn;
				del0 -= 5 * ((sgn * b3) >> denshift);
				if ( del0 <= 0 )
					continue;
				
				sgn = sign_of_int( b2 );
				a2 -= sgn;
				del0 -= 6 * ((sgn * b2) >> denshift);
				if ( del0 <= 0 )
					continue;
				
				sgn = sign_of_int( b1 );
				a1 -= sgn;
				del0 -= 7 * ((sgn * b1) >> denshift);
				if ( del0 <= 0 )
					continue;

				a0 -= sign_of_int( b0 );
			}
			else if ( sg < 0 )
			{
				// note: to avoid unnecessary negations, we flip the value of "sgn"
				sgn = -sign_of_int( b7 );
				a7 -= sgn;
				del0 -= 1 * ((sgn * b7) >> denshift);
				if ( del0 >= 0 )
					continue;
				
				sgn = -sign_of_int( b6 );
				a6 -= sgn;
				del0 -= 2 * ((sgn * b6) >> denshift);
				if ( del0 >= 0 )
					continue;
				
				sgn = -sign_of_int( b5 );
				a5 -= sgn;
				del0 -= 3 * ((sgn * b5) >> denshift);
				if ( del0 >= 0 )
					continue;

				sgn = -sign_of_int( b4 );
				a4 -= sgn;
				del0 -= 4 * ((sgn * b4) >> denshift);
				if ( del0 >= 0 )
					continue;
				
				sgn = -sign_of_int( b3 );
				a3 -= sgn;
				del0 -= 5 * ((sgn * b3) >> denshift);
				if ( del0 >= 0 )
					continue;
				
				sgn = -sign_of_int( b2 );
				a2 -= sgn;
				del0 -= 6 * ((sgn * b2) >> denshift);
				if ( del0 >= 0 )
					continue;
				
				sgn = -sign_of_int( b1 );
				a1 -= sgn;
				del0 -= 7 * ((sgn * b1) >> denshift);
				if ( del0 >= 0 )
					continue;

				a0 += sign_of_int( b0 );
			}
		}

#if HAVE_VISUAL_STUDIO
	#pragma warning(pop)
#endif

		coefs[0] = a0;
		coefs[1] = a1;
		coefs[2] = a2;
		coefs[3] = a3;
		coefs[4] = a4;
		coefs[5] = a5;
		coefs[6] = a6;
		coefs[7] = a7;
	}
	else
	{
		// general case
		for ( j = lim; j < num; j++ )
		{
			LOOP_ALIGN

			sum1 = 0;
			pout = out + j - 1;
			top = out[j-lim];

			for ( k = 0; k < numactive; k++ )
				sum1 += coefs[k] * (pout[-k] - top);

			del = pc1[j];
			del0 = del;
			sg = sign_of_int( del );
			del += top + ((sum1 + denhalf) >> denshift);
			out[j] = (del << chanshift) >> chanshift;

#if HAVE_VISUAL_STUDIO
	#pragma warning(push)
	#pragma warning(disable:4244)	// conversion from 'int32_t' to 'int16_t', possible loss of data
#endif
			if ( sg > 0 )
			{
				for ( k = (numactive - 1); k >= 0; k-- )
				{
					dd = top - pout[-k];
					sgn = sign_of_int( dd );
					coefs[k] -= sgn;
					del0 -= (numactive - k) * ((sgn * dd) >> denshift);
					if ( del0 <= 0 )
						break;
				}
			}
			else if ( sg < 0 )
			{
				for ( k = (numactive - 1); k >= 0; k-- )
				{
					dd = top - pout[-k];
					sgn = sign_of_int( dd );
					coefs[k] += sgn;
					del0 -= (numactive - k) * ((-sgn * dd) >> denshift);
					if ( del0 >= 0 )
						break;
				}
			}
#if HAVE_VISUAL_STUDIO
	#pragma warning(pop)
#endif
		}
	}
}
