/*
	File:    	MathUtils.c
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
*/

#include "MathUtils.h"

#include <float.h>
#include <limits.h>
#include <math.h>

//===========================================================================================================================
//	gcd64
//
//	Finds the Greatest Common Denominator using Euclid's algorithm.
//===========================================================================================================================

int64_t	gcd64( int64_t a, int64_t b )
{
	int64_t		c;
	
	while( a > 0 )
	{
		if( a < b )
		{
			 c = a;
			 a = b;
			 b = c;
		}
		a -= b;
	}
	return( b );
}

//===========================================================================================================================
//	iceil2
//
//	Rounds x up to the closest integral power of 2. Based on code from the book Hacker's Delight.
//===========================================================================================================================

uint32_t	iceil2( uint32_t x )
{
	check( x <= UINT32_C( 0x80000000 ) );
	
	x = x - 1;
	x |= ( x >> 1 );
	x |= ( x >> 2 );
	x |= ( x >> 4 );
	x |= ( x >> 8 );
	x |= ( x >> 16 );
	return( x + 1 );
}

uint64_t	iceil2_64( uint64_t x )
{
	check( x <= UINT64_C( 0x8000000000000000 ) );
	
	x = x - 1;
	x |= ( x >> 1 );
	x |= ( x >> 2 );
	x |= ( x >> 4 );
	x |= ( x >> 8 );
	x |= ( x >> 16 );
	x |= ( x >> 32 );
	return( x + 1 );
}

//===========================================================================================================================
//	ifloor2
//
//	Rounds x down to the closest integral power of 2. Based on code from the book Hacker's Delight.
//===========================================================================================================================

uint32_t	ifloor2( uint32_t x )
{
	x |= ( x >> 1 );
	x |= ( x >> 2 );
	x |= ( x >> 4 );
	x |= ( x >> 8 );
	x |= ( x >> 16 );
	return( x - ( x >> 1 ) );
}

uint64_t	ifloor2_64( uint64_t x )
{
	x |= ( x >> 1 );
	x |= ( x >> 2 );
	x |= ( x >> 4 );
	x |= ( x >> 8 );
	x |= ( x >> 16 );
	x |= ( x >> 32 );
	return( x - ( x >> 1 ) );
}

//===========================================================================================================================
//	ilog2
//
//	Integer logarithm base 2. Note: ilog2( 0 ) returns 0. Based on code from the book Hacker's Delight.
//===========================================================================================================================

#if( !HAS_INLINE_ILOG2 )

#if( _MSC_VER )
	#pragma warning( disable:4146 )	// Disable "unary minus operator applied to unsigned type, result still unsigned".
#endif

uint32_t	ilog2(uint32_t x)
{
	uint32_t y, m, n;

	y = -(x >> 16);		// If left half of x is 0,
	m = (y >> 16) & 16;	// set n = 16.  If left half
	n = 16 - m;			// is nonzero, set n = 0 and
	x = x >> m;			// shift x right 16.
						// Now x is of the form 0000xxxx.
	y = x - 0x100;		// If positions 8-15 are 0,
	m = (y >> 16) & 8;	// add 8 to n and shift x left 8.
	n = n + m;
	x = x << m;

	y = x - 0x1000;		// If positions 12-15 are 0,
	m = (y >> 16) & 4;	// add 4 to n and shift x left 4.
	n = n + m;
	x = x << m;

	y = x - 0x4000;		// If positions 14-15 are 0,
	m = (y >> 16) & 2;	// add 2 to n and shift x left 2.
	n = n + m;
	x = x << m;

	y = x >> 14;		// Set y = 0, 1, 2, or 3.
	m = y & ~(y >> 1);	// Set m = 0, 1, 2, or 2 resp.
	x = n + 2 - m;
	
	return 31 - x;
}

#if( _MSC_VER )
	#pragma warning( default:4201 )	// Re-enable "unary minus operator applied to unsigned type, result still unsigned".
#endif

#endif // !HAS_INLINE_ILOG2

uint32_t	ilog2_64( uint64_t x )
{
	uint32_t		y;
	
	if( x <= UINT32_MAX )
	{
		return( ilog2( (uint32_t) x ) );
	}
	
	for( y = 0; ( x >>= 1 ) != 0; ) ++y;
	return( y );
}

//===========================================================================================================================
//	ipow10
//===========================================================================================================================

uint32_t	ipow10( uint32_t inExponent )
{
	uint32_t		x;
	
	check( inExponent <= 9 );
	if( inExponent <= 9 )	for( x = 1; inExponent-- > 0; x *= 10 ) {}
	else					x = 1000000000; // Saturate at the largest power of 10 (10^9).
	return( x );
}

uint64_t	ipow10_64( uint64_t inExponent )
{
	uint64_t		x;
	
	check( inExponent <= 19 );
	if( inExponent <= 19 )	for( x = 1; inExponent-- > 0; x *= 10 ) {}
	else					x = UINT64_C( 10000000000000000000 ); // Saturate at the largest power of 10 (10^19).
	return( x );
}

//===========================================================================================================================
//	isin
//
//	Integer sine approximation. Based on isin_S4() code from <http://www.coranac.com/2009/07/sines/>
//===========================================================================================================================

#define ISIN_qN		13
#define ISIN_qA		12
#define ISIN_B		19900
#define ISIN_C		3516

int32_t	isin( int32_t x )
{
	int c, y;
	
	c= x<<(30-ISIN_qN);			// Semi-circle info into carry.
	x -= 1<<ISIN_qN;			// sine -> cosine calc
	
	x= x<<(31-ISIN_qN);			// Mask with PI
	x= x>>(31-ISIN_qN);			// Note: SIGNED shift! (to qN)
	x= x*x>>(2*ISIN_qN-14);		// x=x^2 To Q14
	
	y= ISIN_B - (x*ISIN_C>>14);	// B - x^2*C
	y= (1<<ISIN_qA)-(x*y>>16);	// A - x^2*(B-x^2*C)
	
	return c>=0 ? y : -y;
}

//===========================================================================================================================
//	Multiply64x64
//
//	Multiplies two 64-bit unsigned integers and returns the upper and lower halves of the 128-bit result.
//===========================================================================================================================

void	Multiply64x64( uint64_t a, uint64_t b, uint64_t *outHi, uint64_t *outLo )
{
	uint32_t	u1, u0;
	uint32_t	v1, v0;
	uint64_t	t;
	uint32_t	k;
	uint32_t	w3, w2, w1, w0;
	
	// See Knuth V2 Section 4.3.1 Algorithm M for details.
	
	u1 = (uint32_t)( a >> 32 );
	u0 = (uint32_t)( a & 0xFFFFFFFFU );
	
	v1 = (uint32_t)( b >> 32 );
	v0 = (uint32_t)( b & 0xFFFFFFFFU );
	
	t  = u0;
	t *= v0;
	w0 = (uint32_t)( t & 0xFFFFFFFFU );
	k  = (uint32_t)( t >> 32 );
	
	t  = u1;
	t *= v0;
	t += k;
	w1 = (uint32_t)( t & 0xFFFFFFFFU );
	w2 = (uint32_t)( t >> 32 );
	
	t  = u0;
	t *= v1;
	t += w1;
	w1 = (uint32_t)( t & 0xFFFFFFFFU );
	k  = (uint32_t)( t >> 32 );
	
	t  = u1;
	t *= v1;
	t += w2;
	t += k;
	w2 = (uint32_t)( t & 0xFFFFFFFFU );
	w3 = (uint32_t)( t >> 32 );
	
	*outHi = ( ( (uint64_t) w3 ) << 32 ) | w2;
	*outLo = ( ( (uint64_t) w1 ) << 32 ) | w0;
}

#if 0
#pragma mark -
#endif

//===========================================================================================================================
//	EuclideanDistance
//
//	See <http://en.wikipedia.org/wiki/Euclidean_distance>.
//===========================================================================================================================

double	EuclideanDistance( const double x[], const double *y, int n )
{
	double		sum;
	double		distance;
	int			i;
	
	sum = 0;
	for( i = 0; i < n; ++i )
	{
		double		tmp;
		
		tmp = x[ i ] - y[ i ];
		sum += ( tmp * tmp );
	}
	distance = sqrt( sum );
	return( distance );
}

//===========================================================================================================================
//	InterquartileRange
//
//	See <http://en.wikipedia.org/wiki/Interquartile_Range>.
//	See <http://en.wikipedia.org/wiki/Quartile>
//===========================================================================================================================

double	InterquartileRange( const double *set, double *tmp, size_t n, double *outLower, double *outUpper )
{
	size_t		mid, mid2;
	double		q1, q3, iqr, err;
	
	if( n < 4 )
	{
		if( outLower ) *outLower = DBL_MIN;
		if( outUpper ) *outUpper = DBL_MAX;
		return( DBL_MAX );
	}
	
	if( tmp )
	{
		memcpy( tmp, set, n * sizeof( *set ) );
		qsort( tmp, n, sizeof( *tmp ), qsort_cmp_double );
		set = tmp;
	}
	
	mid = n / 2;
	if( ( n % 2 ) == 0 )
	{
		if( ( mid % 2 ) == 0 )
		{
			mid2 = mid / 2;
			q1 = ( set[ mid2 - 1 ] + set[ mid2 ] ) / 2;
			
			mid2 = ( n + mid ) / 2;
			q3 = ( set[ mid2 - 1 ] + set[ mid2 ] ) / 2;
		}
		else
		{
			q1 = set[ mid / 2 ];
			q3 = set[ ( n + mid ) / 2 ];
		}
	}
	else
	{
		if( ( mid % 2 ) == 0 )
		{
			mid2 = mid / 2;
			q1 = ( set[ mid2 - 1 ] + set[ mid2 ] ) / 2;
			
			mid2 = ( n + ( mid + 1 ) ) / 2;
			q3 = ( set[ mid2 - 1 ] + set[ mid2 ] ) / 2;
		}
		else
		{
			q1 = set[ mid / 2 ];
			q3 = set[ ( n + ( mid + 1 ) ) / 2 ];
		}
	}
	
	iqr = q3 - q1;
	err = 1.5 * iqr;
	if( outLower ) *outLower = q1 - err;
	if( outUpper ) *outUpper = q3 + err;
	return( iqr );
}

//===========================================================================================================================
//	MedianAbsoluteDeviation
//===========================================================================================================================

double	MedianAbsoluteDeviation( const double *set, double *tmp1, double *tmp2, size_t n, double *outMedian )
{
	double		median, mad;
	size_t		i;
	
	if( n == 0 )
	{
		if( outMedian ) *outMedian = 0;
		return( 0 );
	}
	else if( n == 1 )
	{
		if( outMedian ) *outMedian = set[ 0 ];
		if( tmp1 )		tmp1[ 0 ]  = set[ 0 ];
		return( 0 );
	}
	
	if( tmp1 )
	{
		memcpy( tmp1, set, n * sizeof( *set ) );
		qsort( tmp1, n, sizeof( *tmp1 ), qsort_cmp_double );
		set = tmp1;
	}
	if( n % 2 ) median = set[ n / 2 ];
	else		median = ( set[ ( n / 2 ) - 1 ] + set[ n / 2 ] ) / 2;
	if( outMedian ) *outMedian = median;
	
	for( i = 0; i < n; ++i )
	{
		tmp2[ i ] = fabs( set[ i ] - median );
	}
	qsort( tmp2, n, sizeof( *tmp2 ), qsort_cmp_double );
	if( n % 2 ) mad = tmp2[ n / 2 ];
	else		mad = ( tmp2[ ( n / 2 ) - 1 ] + tmp2[ n / 2 ] ) / 2;
	return( mad );
}

//===========================================================================================================================
//	PearsonCorrelation
//
//	Converted from pseudocode on <http://en.wikipedia.org/wiki/Pearson_product-moment_correlation_coefficient>.
//===========================================================================================================================

double	PearsonCorrelation( const double x[], const double *y, int n )
{
	double		sum_sq_x, sum_sq_y;
	double		sum_coproduct;
	double		mean_x, mean_y;
	double		pop_sd_x, pop_sd_y;
	double		cov_x_y;
	double		correlation;
	int			i;
	
	if( n < 1 ) return( 1.0 );
	
	sum_sq_x		= 0;
	sum_sq_y		= 0;
	sum_coproduct	= 0;
	mean_x			= x[ 0 ];
	mean_y			= y[ 0 ];
	
	for( i = 2; i <= n; ++i )
	{
		double		sweep;
		double		delta_x, delta_y;
		
		sweep			= ( i - 1.0 ) / i;
		delta_x			= x[ i - 1 ] - mean_x;
		delta_y			= y[ i - 1 ] - mean_y;
		sum_sq_x		+= delta_x * delta_x * sweep;
		sum_sq_y		+= delta_y * delta_y * sweep;
		sum_coproduct	+= delta_x * delta_y * sweep;
		mean_x			+= delta_x / i;
		mean_y			+= delta_y / i;
	}
	pop_sd_x	= sqrt( sum_sq_x / n );
	pop_sd_y	= sqrt( sum_sq_y / n );
	cov_x_y		= sum_coproduct / n;
	correlation = cov_x_y / ( pop_sd_x * pop_sd_y );
	return( correlation );
}

//===========================================================================================================================
//	StandardDeviation
//===========================================================================================================================

double	StandardDeviation( const double *x, size_t n )
{
	size_t		i;
	double		sum, avg, tmp;
	
	if( n <= 1 ) return( 0 );
	
	sum = 0;
	for( i = 0; i < n; ++i )
	{
		sum += x[ i ];
	}
	avg = sum / n;
	
	sum = 0;
	for( i = 0; i < n; ++i )
	{
		tmp = x[ i ] - avg;
		sum += ( tmp * tmp );
	}
	sum /= ( n - 1 ); // n - 1 makes this a sample standard deviation instead of a population standard deviation.
	return( sqrt( sum ) );
}

//===========================================================================================================================
//	TorbenMedian
//
//	Algorithm by Torben Mogensen, implementation by N. Devillard.
//	See <http://ndevilla.free.fr/median/median/median.html> for more information and alternate implementations.
//===========================================================================================================================

#define TorbenMedianImplementation( TORBEN_TYPE, TORBEN_NAME ) \
	TORBEN_TYPE	TORBEN_NAME( const TORBEN_TYPE m[], int n ) \
	{ \
		int i, less, greater, equal; \
		TORBEN_TYPE min, max, guess, maxltguess, mingtguess; \
		\
		min = max = m[0] ; \
		for (i=1 ; i<n ; i++) { \
			if (m[i]<min) min=m[i]; \
			if (m[i]>max) max=m[i]; \
		} \
		\
		for (;;) { \
			guess = (min+max)/2; \
			less = 0; greater = 0; equal = 0; \
			maxltguess = min ; \
			mingtguess = max ; \
			for (i=0; i<n; i++) { \
				if (m[i]<guess) { \
					less++; \
					if (m[i]>maxltguess) maxltguess = m[i] ; \
				} else if (m[i]>guess) { \
					greater++; \
					if (m[i]<mingtguess) mingtguess = m[i] ; \
				} else equal++; \
			} \
			if (less <= (n+1)/2 && greater <= (n+1)/2) break ; \
			else if (less>greater) max = maxltguess ; \
			else min = mingtguess; \
		} \
		if (less >= (n+1)/2) return maxltguess; \
		else if (less+equal >= (n+1)/2) return guess; \
		else return mingtguess; \
	}

TorbenMedianImplementation( int32_t, TorbenMedian32 )
TorbenMedianImplementation( int64_t, TorbenMedian64 )
TorbenMedianImplementation( double,  TorbenMedianF )

//===========================================================================================================================
//	TranslateValue
//===========================================================================================================================

double	TranslateValue( double inValue, double inOldMin, double inOldMax, double inNewMin, double inNewMax )
{
	return( ( ( ( inValue - inOldMin ) / ( inOldMax - inOldMin ) ) * ( inNewMax - inNewMin ) ) + inNewMin );
}

#if 0
#pragma mark -
#pragma mark == Debugging ==
#endif

