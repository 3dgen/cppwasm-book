////////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                 License For Embedded Computer Vision Library
//
// Copyright (c) 2008, EMCV Project,
// Copyright (c) 2000-2007, Intel Corporation,
// All rights reserved.
// Third party copyrights are property of their respective owners.
//
// Redistribution and use in source and binary forms, with or without modification, 
// are permitted provided that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright notice, 
//      this list of conditions and the following disclaimer.
//    * Redistributions in binary form must reproduce the above copyright notice, 
//      this list of conditions and the following disclaimer in the documentation 
//      and/or other materials provided with the distribution.
//    * Neither the name of the copyright holders nor the names of their contributors 
//      may be used to endorse or promote products derived from this software 
//      without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
// NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
// OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
// OF SUCH DAMAGE.
//
// Contributors:
//    * Shiqi Yu (Shenzhen Institute of Advanced Technology, Chinese Academy of Sciences)



#ifndef _CXCORE_INTERNAL_H_
#define _CXCORE_INTERNAL_H_

#if defined _MSC_VER && _MSC_VER >= 1200
    /* disable warnings related to inline functions */
    #pragma warning( disable: 4711 4710 4514 )
#endif

typedef unsigned long ulong;

#ifdef __BORLANDC__
    #define     WIN32
    #define     CV_DLL
    #undef      _CV_ALWAYS_PROFILE_
    #define     _CV_ALWAYS_NO_PROFILE_
#endif

#include "cxcore.h"
#include "cxmisc.h"
#include <math.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <float.h>

// -128.f ... 255.f
extern const float icv8x32fTab[];
#define CV_8TO32F(x)  icv8x32fTab[(x)+128]

extern const ushort icv8x16uSqrTab[];
#define CV_SQR_8U(x)  icv8x16uSqrTab[(x)+255]

extern const char* icvHersheyGlyphs[];

extern const signed char icvDepthToType[];

#define icvIplToCvDepth( depth ) \
    icvDepthToType[(((depth) & 255) >> 2) + ((depth) < 0)]

extern const uchar icvSaturate8u[];
#define CV_FAST_CAST_8U(t)   (assert(-256 <= (t) && (t) <= 512), icvSaturate8u[(t)+256])
#define CV_MIN_8U(a,b)       ((a) - CV_FAST_CAST_8U((a) - (b)))
#define CV_MAX_8U(a,b)       ((a) + CV_FAST_CAST_8U((b) - (a)))

typedef CvFunc2D_3A1I CvArithmBinMaskFunc2D;
typedef CvFunc2D_2A1P1I CvArithmUniMaskFunc2D;


/****************************************************************************************\
*                                   Complex arithmetics                                  *
\****************************************************************************************/

struct CvComplex32f;
struct CvComplex64f;

struct CvComplex32f
{
    float re, im;

    CvComplex32f() {}
    CvComplex32f( float _re, float _im=0 ) : re(_re), im(_im) {}
    explicit CvComplex32f( const CvComplex64f& v );
    //CvComplex32f( const CvComplex32f& v ) : re(v.re), im(v.im) {}
    //CvComplex32f& operator = (const CvComplex32f& v ) { re = v.re; im = v.im; return *this; }
    operator CvComplex64f() const;
};

struct CvComplex64f
{
    double re, im;

    CvComplex64f() {}
    CvComplex64f( double _re, double _im=0 ) : re(_re), im(_im) {}
    explicit CvComplex64f( const CvComplex32f& v );
    //CvComplex64f( const CvComplex64f& v ) : re(v.re), im(v.im) {}
    //CvComplex64f& operator = (const CvComplex64f& v ) { re = v.re; im = v.im; return *this; }
    operator CvComplex32f() const;
};

inline CvComplex32f::CvComplex32f( const CvComplex64f& v ) : re((float)v.re), im((float)v.im) {}
inline CvComplex64f::CvComplex64f( const CvComplex32f& v ) : re(v.re), im(v.im) {}

inline CvComplex32f operator + (CvComplex32f a, CvComplex32f b)
{
    return CvComplex32f( a.re + b.re, a.im + b.im );
}

inline CvComplex32f& operator += (CvComplex32f& a, CvComplex32f b)
{
    a.re += b.re;
    a.im += b.im;
    return a;
}

inline CvComplex32f operator - (CvComplex32f a, CvComplex32f b)
{
    return CvComplex32f( a.re - b.re, a.im - b.im );
}

inline CvComplex32f& operator -= (CvComplex32f& a, CvComplex32f b)
{
    a.re -= b.re;
    a.im -= b.im;
    return a;
}

inline CvComplex32f operator - (CvComplex32f a)
{
    return CvComplex32f( -a.re, -a.im );
}

inline CvComplex32f operator * (CvComplex32f a, CvComplex32f b)
{
    return CvComplex32f( a.re*b.re - a.im*b.im, a.re*b.im + a.im*b.re );
}

inline double abs(CvComplex32f a)
{
    return sqrt( (double)a.re*a.re + (double)a.im*a.im );
}

inline CvComplex32f conj(CvComplex32f a)
{
    return CvComplex32f( a.re, -a.im );
}


inline CvComplex32f operator / (CvComplex32f a, CvComplex32f b)
{
    double t = 1./((double)b.re*b.re + (double)b.im*b.im);
    return CvComplex32f( (float)((a.re*b.re + a.im*b.im)*t),
                         (float)((-a.re*b.im + a.im*b.re)*t) );
}

inline CvComplex32f operator * (double a, CvComplex32f b)
{
    return CvComplex32f( (float)(a*b.re), (float)(a*b.im) );
}

inline CvComplex32f operator * (CvComplex32f a, double b)
{
    return CvComplex32f( (float)(a.re*b), (float)(a.im*b) );
}

inline CvComplex32f::operator CvComplex64f() const
{
    return CvComplex64f(re,im);
}


inline CvComplex64f operator + (CvComplex64f a, CvComplex64f b)
{
    return CvComplex64f( a.re + b.re, a.im + b.im );
}

inline CvComplex64f& operator += (CvComplex64f& a, CvComplex64f b)
{
    a.re += b.re;
    a.im += b.im;
    return a;
}

inline CvComplex64f operator - (CvComplex64f a, CvComplex64f b)
{
    return CvComplex64f( a.re - b.re, a.im - b.im );
}

inline CvComplex64f& operator -= (CvComplex64f& a, CvComplex64f b)
{
    a.re -= b.re;
    a.im -= b.im;
    return a;
}

inline CvComplex64f operator - (CvComplex64f a)
{
    return CvComplex64f( -a.re, -a.im );
}

inline CvComplex64f operator * (CvComplex64f a, CvComplex64f b)
{
    return CvComplex64f( a.re*b.re - a.im*b.im, a.re*b.im + a.im*b.re );
}

inline double abs(CvComplex64f a)
{
    return sqrt( (double)a.re*a.re + (double)a.im*a.im );
}

inline CvComplex64f operator / (CvComplex64f a, CvComplex64f b)
{
    double t = 1./((double)b.re*b.re + (double)b.im*b.im);
    return CvComplex64f( (a.re*b.re + a.im*b.im)*t,
                         (-a.re*b.im + a.im*b.re)*t );
}

inline CvComplex64f operator * (double a, CvComplex64f b)
{
    return CvComplex64f( a*b.re, a*b.im );
}

inline CvComplex64f operator * (CvComplex64f a, double b)
{
    return CvComplex64f( a.re*b, a.im*b );
}

inline CvComplex64f::operator CvComplex32f() const
{
    return CvComplex32f((float)re,(float)im);
}

inline CvComplex64f conj(CvComplex64f a)
{
    return CvComplex64f( a.re, -a.im );
}

inline CvComplex64f operator + (CvComplex64f a, CvComplex32f b)
{
    return CvComplex64f( a.re + b.re, a.im + b.im );
}

inline CvComplex64f operator + (CvComplex32f a, CvComplex64f b)
{
    return CvComplex64f( a.re + b.re, a.im + b.im );
}

inline CvComplex64f operator - (CvComplex64f a, CvComplex32f b)
{
    return CvComplex64f( a.re - b.re, a.im - b.im );
}

inline CvComplex64f operator - (CvComplex32f a, CvComplex64f b)
{
    return CvComplex64f( a.re - b.re, a.im - b.im );
}

inline CvComplex64f operator * (CvComplex64f a, CvComplex32f b)
{
    return CvComplex64f( a.re*b.re - a.im*b.im, a.re*b.im + a.im*b.re );
}

inline CvComplex64f operator * (CvComplex32f a, CvComplex64f b)
{
    return CvComplex64f( a.re*b.re - a.im*b.im, a.re*b.im + a.im*b.re );
}


typedef CvStatus (CV_STDCALL * CvCopyMaskFunc)(const void* src, int src_step,
                                               void* dst, int dst_step, CvSize size,
                                               const void* mask, int mask_step);

CvCopyMaskFunc icvGetCopyMaskFunc( int elem_size );

CvStatus CV_STDCALL icvSetZero_8u_C1R( uchar* dst, int dststep, CvSize size );

CvStatus CV_STDCALL icvScale_32f( const float* src, float* dst, int len, float a, float b );
CvStatus CV_STDCALL icvScale_64f( const double* src, double* dst, int len, double a, double b );

CvStatus CV_STDCALL icvLUT_Transform8u_8u_C1R( const uchar* src, int srcstep, uchar* dst,
                                               int dststep, CvSize size, const uchar* lut );
CvStatus CV_STDCALL icvLUT_Transform8u_16u_C1R( const uchar* src, int srcstep, ushort* dst,
                                                int dststep, CvSize size, const ushort* lut );
CvStatus CV_STDCALL icvLUT_Transform8u_32s_C1R( const uchar* src, int srcstep, int* dst,
                                                int dststep, CvSize size, const int* lut );
CvStatus CV_STDCALL icvLUT_Transform8u_64f_C1R( const uchar* src, int srcstep, double* dst,
                                                int dststep, CvSize size, const double* lut );

CvStatus CV_STDCALL icvLUT_Transform8u_8u_C2R( const uchar* src, int srcstep, uchar* dst,
                                               int dststep, CvSize size, const uchar* lut );
CvStatus CV_STDCALL icvLUT_Transform8u_8u_C3R( const uchar* src, int srcstep, uchar* dst,
                                               int dststep, CvSize size, const uchar* lut );
CvStatus CV_STDCALL icvLUT_Transform8u_8u_C4R( const uchar* src, int srcstep, uchar* dst,
                                               int dststep, CvSize size, const uchar* lut );

typedef CvStatus (CV_STDCALL * CvLUT_TransformFunc)( const void* src, int srcstep, void* dst,
                                                     int dststep, CvSize size, const void* lut );

CV_INLINE CvStatus
icvLUT_Transform8u_8s_C1R( const uchar* src, int srcstep, char* dst,
                            int dststep, CvSize size, const char* lut )
{
    return icvLUT_Transform8u_8u_C1R( src, srcstep, (uchar*)dst,
                                      dststep, size, (const uchar*)lut );
}

CV_INLINE CvStatus
icvLUT_Transform8u_16s_C1R( const uchar* src, int srcstep, short* dst,
                            int dststep, CvSize size, const short* lut )
{
    return icvLUT_Transform8u_16u_C1R( src, srcstep, (ushort*)dst,
                                       dststep, size, (const ushort*)lut );
}

CV_INLINE CvStatus
icvLUT_Transform8u_32f_C1R( const uchar* src, int srcstep, float* dst,
                            int dststep, CvSize size, const float* lut )
{
    return icvLUT_Transform8u_32s_C1R( src, srcstep, (int*)dst,
                                       dststep, size, (const int*)lut );
}

#endif /*_CXCORE_INTERNAL_H_*/
