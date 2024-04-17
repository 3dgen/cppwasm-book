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
// Copyright (c) 2012, EMCV Project,
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


/* ////////////////////////////////////////////////////////////////////
//
//  Geometrical transforms on images and matrices: rotation, zoom etc.
//
// */

#include "_cv.h"


/************** interpolation constants and tables ***************/

#define ICV_WARP_MUL_ONE_8U(x)  ((x) << ICV_WARP_SHIFT)
#define ICV_WARP_DESCALE_8U(x)  CV_DESCALE((x), ICV_WARP_SHIFT*2)
#define ICV_WARP_CLIP_X(x)      ((unsigned)(x) < (unsigned)ssize.width ? \
                                (x) : (x) < 0 ? 0 : ssize.width - 1)
#define ICV_WARP_CLIP_Y(y)      ((unsigned)(y) < (unsigned)ssize.height ? \
                                (y) : (y) < 0 ? 0 : ssize.height - 1)

float icvLinearCoeffs[(ICV_LINEAR_TAB_SIZE+1)*2];

void icvInitLinearCoeffTab()
{
    static int inittab = 0;
    if( !inittab )
    {
        for( int i = 0; i <= ICV_LINEAR_TAB_SIZE; i++ )
        {
            float x = (float)i/ICV_LINEAR_TAB_SIZE;
            icvLinearCoeffs[i*2] = x;
            icvLinearCoeffs[i*2+1] = 1.f - x;
        }

        inittab = 1;
    }
}


float icvCubicCoeffs[(ICV_CUBIC_TAB_SIZE+1)*2];

void icvInitCubicCoeffTab()
{
    static int inittab = 0;
    if( !inittab )
    {
#if 0
        // classical Mitchell-Netravali filter
        const double B = 1./3;
        const double C = 1./3;
        const double p0 = (6 - 2*B)/6.;
        const double p2 = (-18 + 12*B + 6*C)/6.;
        const double p3 = (12 - 9*B - 6*C)/6.;
        const double q0 = (8*B + 24*C)/6.;
        const double q1 = (-12*B - 48*C)/6.;
        const double q2 = (6*B + 30*C)/6.;
        const double q3 = (-B - 6*C)/6.;

        #define ICV_CUBIC_1(x)  (((x)*p3 + p2)*(x)*(x) + p0)
        #define ICV_CUBIC_2(x)  ((((x)*q3 + q2)*(x) + q1)*(x) + q0)
#else
        // alternative "sharp" filter
        const double A = -0.75;
        #define ICV_CUBIC_1(x)  (((A + 2)*(x) - (A + 3))*(x)*(x) + 1)
        #define ICV_CUBIC_2(x)  (((A*(x) - 5*A)*(x) + 8*A)*(x) - 4*A)
#endif
        for( int i = 0; i <= ICV_CUBIC_TAB_SIZE; i++ )
        {
            float x = (float)i/ICV_CUBIC_TAB_SIZE;
            icvCubicCoeffs[i*2] = (float)ICV_CUBIC_1(x);
            x += 1.f;
            icvCubicCoeffs[i*2+1] = (float)ICV_CUBIC_2(x);
        }

        inittab = 1;
    }
}


/****************************************************************************************\
*                                         Resize                                         *
\****************************************************************************************/

static CvStatus CV_STDCALL
icvResize_NN_8u_C1R( const uchar* src, int srcstep, CvSize ssize,
                     uchar* dst, int dststep, CvSize dsize, int pix_size )
{
    int* x_ofs = (int*)cvAlloc( dsize.width * sizeof(x_ofs[0]) );
    int pix_size4 = pix_size / sizeof(int);
    int x, y, t;

    for( x = 0; x < dsize.width; x++ )
    {
        t = (ssize.width*x*2 + MIN(ssize.width, dsize.width) - 1)/(dsize.width*2);
        t -= t >= ssize.width;
        x_ofs[x] = t*pix_size;
    }

    for( y = 0; y < dsize.height; y++, dst += dststep )
    {
        const uchar* tsrc;
        t = (ssize.height*y*2 + MIN(ssize.height, dsize.height) - 1)/(dsize.height*2);
        t -= t >= ssize.height;
        tsrc = src + srcstep*t;

        switch( pix_size )
        {
        case 1:
            for( x = 0; x <= dsize.width - 2; x += 2 )
            {
                uchar t0 = tsrc[x_ofs[x]];
                uchar t1 = tsrc[x_ofs[x+1]];

                dst[x] = t0;
                dst[x+1] = t1;
            }

            for( ; x < dsize.width; x++ )
                dst[x] = tsrc[x_ofs[x]];
            break;
        case 2:
            for( x = 0; x < dsize.width; x++ )
                *(ushort*)(dst + x*2) = *(ushort*)(tsrc + x_ofs[x]);
            break;
        case 3:
            for( x = 0; x < dsize.width; x++ )
            {
                const uchar* _tsrc = tsrc + x_ofs[x];
                dst[x*3] = _tsrc[0]; dst[x*3+1] = _tsrc[1]; dst[x*3+2] = _tsrc[2];
            }
            break;
        case 4:
            for( x = 0; x < dsize.width; x++ )
                *(int*)(dst + x*4) = *(int*)(tsrc + x_ofs[x]);
            break;
        case 6:
            for( x = 0; x < dsize.width; x++ )
            {
                const ushort* _tsrc = (const ushort*)(tsrc + x_ofs[x]);
                ushort* _tdst = (ushort*)(dst + x*6);
                _tdst[0] = _tsrc[0]; _tdst[1] = _tsrc[1]; _tdst[2] = _tsrc[2];
            }
            break;
        default:
            for( x = 0; x < dsize.width; x++ )
                CV_MEMCPY_INT( dst + x*pix_size, tsrc + x_ofs[x], pix_size4 );
        }
    }

	if(x_ofs) cvFree(&x_ofs);
    return CV_OK;
}


typedef struct CvResizeAlpha
{
    int idx;
    union
    {
        float alpha;
        int ialpha;
    };
}
CvResizeAlpha;


#define  ICV_DEF_RESIZE_BILINEAR_FUNC( flavor, arrtype, worktype, alpha_field,  \
                                       mul_one_macro, descale_macro )           \
static CvStatus CV_STDCALL                                                      \
icvResize_Bilinear_##flavor##_CnR( const arrtype* src, int srcstep, CvSize ssize,\
                                   arrtype* dst, int dststep, CvSize dsize,     \
                                   int cn, int xmax,                            \
                                   const CvResizeAlpha* xofs,                   \
                                   const CvResizeAlpha* yofs,                   \
                                   worktype* buf0, worktype* buf1 )             \
{                                                                               \
    int prev_sy0 = -1, prev_sy1 = -1;                                           \
    int k, dx, dy;                                                              \
                                                                                \
    srcstep /= sizeof(src[0]);                                                  \
    dststep /= sizeof(dst[0]);                                                  \
    dsize.width *= cn;                                                          \
    xmax *= cn;                                                                 \
                                                                                \
    for( dy = 0; dy < dsize.height; dy++, dst += dststep )                      \
    {                                                                           \
        worktype fy = yofs[dy].alpha_field, *swap_t;                            \
        int sy0 = yofs[dy].idx, sy1 = sy0 + (fy > 0 && sy0 < ssize.height-1);   \
                                                                                \
        if( sy0 == prev_sy0 && sy1 == prev_sy1 )                                \
            k = 2;                                                              \
        else if( sy0 == prev_sy1 )                                              \
        {                                                                       \
            CV_SWAP( buf0, buf1, swap_t );                                      \
            k = 1;                                                              \
        }                                                                       \
        else                                                                    \
            k = 0;                                                              \
                                                                                \
        for( ; k < 2; k++ )                                                     \
        {                                                                       \
            worktype* _buf = k == 0 ? buf0 : buf1;                              \
            const arrtype* _src;                                                \
            int sy = k == 0 ? sy0 : sy1;                                        \
            if( k == 1 && sy1 == sy0 )                                          \
            {                                                                   \
                memcpy( buf1, buf0, dsize.width*sizeof(buf0[0]) );              \
                continue;                                                       \
            }                                                                   \
                                                                                \
            _src = src + sy*srcstep;                                            \
            for( dx = 0; dx < xmax; dx++ )                                      \
            {                                                                   \
                int sx = xofs[dx].idx;                                          \
                worktype fx = xofs[dx].alpha_field;                             \
                worktype t = _src[sx];                                          \
                _buf[dx] = mul_one_macro(t) + fx*(_src[sx+cn] - t);             \
            }                                                                   \
                                                                                \
            for( ; dx < dsize.width; dx++ )                                     \
                _buf[dx] = mul_one_macro(_src[xofs[dx].idx]);                   \
        }                                                                       \
                                                                                \
        prev_sy0 = sy0;                                                         \
        prev_sy1 = sy1;                                                         \
                                                                                \
        if( sy0 == sy1 )                                                        \
            for( dx = 0; dx < dsize.width; dx++ )                               \
                dst[dx] = (arrtype)descale_macro( mul_one_macro(buf0[dx]));     \
        else                                                                    \
            for( dx = 0; dx < dsize.width; dx++ )                               \
                dst[dx] = (arrtype)descale_macro( mul_one_macro(buf0[dx]) +     \
                                                  fy*(buf1[dx] - buf0[dx]));    \
    }                                                                           \
                                                                                \
    return CV_OK;                                                               \
}


typedef struct CvDecimateAlpha
{
    int si, di;
    float alpha;
}
CvDecimateAlpha;


#define  ICV_DEF_RESIZE_AREA_FAST_FUNC( flavor, arrtype, worktype, cast_macro ) \
static CvStatus CV_STDCALL                                                      \
icvResize_AreaFast_##flavor##_CnR( const arrtype* src, int srcstep, CvSize ssize,\
                              arrtype* dst, int dststep, CvSize dsize, int cn,  \
                              const int* ofs, const int* xofs )                 \
{                                                                               \
    int dy, dx, k = 0;                                                          \
    int scale_x = ssize.width/dsize.width;                                      \
    int scale_y = ssize.height/dsize.height;                                    \
    int area = scale_x*scale_y;                                                 \
    float scale = 1.f/(scale_x*scale_y);                                        \
                                                                                \
    srcstep /= sizeof(src[0]);                                                  \
    dststep /= sizeof(dst[0]);                                                  \
    dsize.width *= cn;                                                          \
                                                                                \
    for( dy = 0; dy < dsize.height; dy++, dst += dststep )                      \
        for( dx = 0; dx < dsize.width; dx++ )                                   \
        {                                                                       \
            const arrtype* _src = src + dy*scale_y*srcstep + xofs[dx];          \
            worktype sum = 0;                                                   \
                                                                                \
            for( k = 0; k <= area - 4; k += 4 )                                 \
                sum += _src[ofs[k]] + _src[ofs[k+1]] +                          \
                       _src[ofs[k+2]] + _src[ofs[k+3]];                         \
                                                                                \
            for( ; k < area; k++ )                                              \
                sum += _src[ofs[k]];                                            \
                                                                                \
            dst[dx] = (arrtype)cast_macro( sum*scale );                         \
        }                                                                       \
                                                                                \
    return CV_OK;                                                               \
}


#define  ICV_DEF_RESIZE_AREA_FUNC( flavor, arrtype, load_macro, cast_macro )    \
static CvStatus CV_STDCALL                                                      \
icvResize_Area_##flavor##_CnR( const arrtype* src, int srcstep, CvSize ssize,   \
                               arrtype* dst, int dststep, CvSize dsize,         \
                               int cn, const CvDecimateAlpha* xofs,             \
                               int xofs_count, float* buf, float* sum )         \
{                                                                               \
    int k, sy, dx, cur_dy = 0;                                                  \
    float scale_y = (float)ssize.height/dsize.height;                           \
                                                                                \
    srcstep /= sizeof(src[0]);                                                  \
    dststep /= sizeof(dst[0]);                                                  \
    dsize.width *= cn;                                                          \
                                                                                \
    for( sy = 0; sy < ssize.height; sy++, src += srcstep )                      \
    {                                                                           \
        if( cn == 1 )                                                           \
            for( k = 0; k < xofs_count; k++ )                                   \
            {                                                                   \
                int dxn = xofs[k].di;                                           \
                float alpha = xofs[k].alpha;                                    \
                buf[dxn] = buf[dxn] + load_macro(src[xofs[k].si])*alpha;        \
            }                                                                   \
        else if( cn == 2 )                                                      \
            for( k = 0; k < xofs_count; k++ )                                   \
            {                                                                   \
                int sxn = xofs[k].si;                                           \
                int dxn = xofs[k].di;                                           \
                float alpha = xofs[k].alpha;                                    \
                float t0 = buf[dxn] + load_macro(src[sxn])*alpha;               \
                float t1 = buf[dxn+1] + load_macro(src[sxn+1])*alpha;           \
                buf[dxn] = t0; buf[dxn+1] = t1;                                 \
            }                                                                   \
        else if( cn == 3 )                                                      \
            for( k = 0; k < xofs_count; k++ )                                   \
            {                                                                   \
                int sxn = xofs[k].si;                                           \
                int dxn = xofs[k].di;                                           \
                float alpha = xofs[k].alpha;                                    \
                float t0 = buf[dxn] + load_macro(src[sxn])*alpha;               \
                float t1 = buf[dxn+1] + load_macro(src[sxn+1])*alpha;           \
                float t2 = buf[dxn+2] + load_macro(src[sxn+2])*alpha;           \
                buf[dxn] = t0; buf[dxn+1] = t1; buf[dxn+2] = t2;                \
            }                                                                   \
        else                                                                    \
            for( k = 0; k < xofs_count; k++ )                                   \
            {                                                                   \
                int sxn = xofs[k].si;                                           \
                int dxn = xofs[k].di;                                           \
                float alpha = xofs[k].alpha;                                    \
                float t0 = buf[dxn] + load_macro(src[sxn])*alpha;               \
                float t1 = buf[dxn+1] + load_macro(src[sxn+1])*alpha;           \
                buf[dxn] = t0; buf[dxn+1] = t1;                                 \
                t0 = buf[dxn+2] + load_macro(src[sxn+2])*alpha;                 \
                t1 = buf[dxn+3] + load_macro(src[sxn+3])*alpha;                 \
                buf[dxn+2] = t0; buf[dxn+3] = t1;                               \
            }                                                                   \
                                                                                \
        if( (cur_dy + 1)*scale_y <= sy + 1 || sy == ssize.height - 1 )          \
        {                                                                       \
            float beta = sy + 1 - (cur_dy+1)*scale_y, beta1;                    \
            beta = MAX( beta, 0 );                                              \
            beta1 = 1 - beta;                                                   \
            if( fabs(beta) < 1e-3 )                                             \
                for( dx = 0; dx < dsize.width; dx++ )                           \
                {                                                               \
                    dst[dx] = (arrtype)cast_macro(sum[dx] + buf[dx]);           \
                    sum[dx] = buf[dx] = 0;                                      \
                }                                                               \
            else                                                                \
                for( dx = 0; dx < dsize.width; dx++ )                           \
                {                                                               \
                    dst[dx] = (arrtype)cast_macro(sum[dx] + buf[dx]*beta1);     \
                    sum[dx] = buf[dx]*beta;                                     \
                    buf[dx] = 0;                                                \
                }                                                               \
            dst += dststep;                                                     \
            cur_dy++;                                                           \
        }                                                                       \
        else                                                                    \
            for( dx = 0; dx < dsize.width; dx += 2 )                            \
            {                                                                   \
                float t0 = sum[dx] + buf[dx];                                   \
                float t1 = sum[dx+1] + buf[dx+1];                               \
                sum[dx] = t0; sum[dx+1] = t1;                                   \
                buf[dx] = buf[dx+1] = 0;                                        \
            }                                                                   \
    }                                                                           \
                                                                                \
    return CV_OK;                                                               \
}


#define  ICV_DEF_RESIZE_BICUBIC_FUNC( flavor, arrtype, worktype, load_macro,    \
                                      cast_macro1, cast_macro2 )                \
static CvStatus CV_STDCALL                                                      \
icvResize_Bicubic_##flavor##_CnR( const arrtype* src, int srcstep, CvSize ssize,\
                                  arrtype* dst, int dststep, CvSize dsize,      \
                                  int cn, int xmin, int xmax,                   \
                                  const CvResizeAlpha* xofs, float** buf )      \
{                                                                               \
    float scale_y = (float)ssize.height/dsize.height;                           \
    int dx, dy, sx, sy, sy2, ify;                                               \
    int prev_sy2 = -2;                                                          \
                                                                                \
    xmin *= cn; xmax *= cn;                                                     \
    dsize.width *= cn;                                                          \
    ssize.width *= cn;                                                          \
    srcstep /= sizeof(src[0]);                                                  \
    dststep /= sizeof(dst[0]);                                                  \
                                                                                \
    for( dy = 0; dy < dsize.height; dy++, dst += dststep )                      \
    {                                                                           \
        float w0, w1, w2, w3;                                                   \
        float fy, x, sum;                                                       \
        float *row, *row0, *row1, *row2, *row3;                                 \
        int k1, k = 4;                                                          \
                                                                                \
        fy = dy*scale_y;                                                        \
        sy = cvFloor(fy);                                                       \
        fy -= sy;                                                               \
        ify = cvRound(fy*ICV_CUBIC_TAB_SIZE);                                   \
        sy2 = sy + 2;                                                           \
                                                                                \
        if( sy2 > prev_sy2 )                                                    \
        {                                                                       \
            int delta = prev_sy2 - sy + 2;                                      \
            for( k = 0; k < delta; k++ )                                        \
                CV_SWAP( buf[k], buf[k+4-delta], row );                         \
        }                                                                       \
                                                                                \
        for( sy += k - 1; k < 4; k++, sy++ )                                    \
        {                                                                       \
            const arrtype* _src = src + sy*srcstep;                             \
                                                                                \
            row = buf[k];                                                       \
            if( sy < 0 )                                                        \
                continue;                                                       \
            if( sy >= ssize.height )                                            \
            {                                                                   \
                assert( k > 0 );                                                \
                memcpy( row, buf[k-1], dsize.width*sizeof(row[0]) );            \
                continue;                                                       \
            }                                                                   \
                                                                                \
            for( dx = 0; dx < xmin; dx++ )                                      \
            {                                                                   \
                int ifx = xofs[dx].ialpha, sx0 = xofs[dx].idx;                  \
                sx = sx0 + cn*2;                                                \
                while( sx >= ssize.width )                                      \
                    sx -= cn;                                                   \
                x = load_macro(_src[sx]);                                       \
                sum = x*icvCubicCoeffs[(ICV_CUBIC_TAB_SIZE - ifx)*2 + 1];       \
                if( (unsigned)(sx = sx0 + cn) < (unsigned)ssize.width )         \
                    x = load_macro(_src[sx]);                                   \
                sum += x*icvCubicCoeffs[(ICV_CUBIC_TAB_SIZE - ifx)*2];          \
                if( (unsigned)(sx = sx0) < (unsigned)ssize.width )              \
                    x = load_macro(_src[sx]);                                   \
                sum += x*icvCubicCoeffs[ifx*2];                                 \
                if( (unsigned)(sx = sx0 - cn) < (unsigned)ssize.width )         \
                    x = load_macro(_src[sx]);                                   \
                row[dx] = sum + x*icvCubicCoeffs[ifx*2 + 1];                    \
            }                                                                   \
                                                                                \
            for( ; dx < xmax; dx++ )                                            \
            {                                                                   \
                int ifx = xofs[dx].ialpha;                                      \
                int sx0 = xofs[dx].idx;                                         \
                row[dx] = _src[sx0 - cn]*icvCubicCoeffs[ifx*2 + 1] +            \
                    _src[sx0]*icvCubicCoeffs[ifx*2] +                           \
                    _src[sx0 + cn]*icvCubicCoeffs[(ICV_CUBIC_TAB_SIZE-ifx)*2] + \
                    _src[sx0 + cn*2]*icvCubicCoeffs[(ICV_CUBIC_TAB_SIZE-ifx)*2+1];\
            }                                                                   \
                                                                                \
            for( ; dx < dsize.width; dx++ )                                     \
            {                                                                   \
                int ifx = xofs[dx].ialpha, sx0 = xofs[dx].idx;                  \
                x = load_macro(_src[sx0 - cn]);                                 \
                sum = x*icvCubicCoeffs[ifx*2 + 1];                              \
                if( (unsigned)(sx = sx0) < (unsigned)ssize.width )              \
                    x = load_macro(_src[sx]);                                   \
                sum += x*icvCubicCoeffs[ifx*2];                                 \
                if( (unsigned)(sx = sx0 + cn) < (unsigned)ssize.width )         \
                    x = load_macro(_src[sx]);                                   \
                sum += x*icvCubicCoeffs[(ICV_CUBIC_TAB_SIZE - ifx)*2];          \
                if( (unsigned)(sx = sx0 + cn*2) < (unsigned)ssize.width )       \
                    x = load_macro(_src[sx]);                                   \
                row[dx] = sum + x*icvCubicCoeffs[(ICV_CUBIC_TAB_SIZE-ifx)*2+1]; \
            }                                                                   \
                                                                                \
            if( sy == 0 )                                                       \
                for( k1 = 0; k1 < k; k1++ )                                     \
                    memcpy( buf[k1], row, dsize.width*sizeof(row[0]));          \
        }                                                                       \
                                                                                \
        prev_sy2 = sy2;                                                         \
                                                                                \
        row0 = buf[0]; row1 = buf[1];                                           \
        row2 = buf[2]; row3 = buf[3];                                           \
                                                                                \
        w0 = icvCubicCoeffs[ify*2+1];                                           \
        w1 = icvCubicCoeffs[ify*2];                                             \
        w2 = icvCubicCoeffs[(ICV_CUBIC_TAB_SIZE - ify)*2];                      \
        w3 = icvCubicCoeffs[(ICV_CUBIC_TAB_SIZE - ify)*2 + 1];                  \
                                                                                \
        for( dx = 0; dx < dsize.width; dx++ )                                   \
        {                                                                       \
            worktype val = cast_macro1( row0[dx]*w0 + row1[dx]*w1 +             \
                                        row2[dx]*w2 + row3[dx]*w3 );            \
            dst[dx] = cast_macro2(val);                                         \
        }                                                                       \
    }                                                                           \
                                                                                \
    return CV_OK;                                                               \
}


ICV_DEF_RESIZE_BILINEAR_FUNC( 8u, uchar, int, ialpha,
                              ICV_WARP_MUL_ONE_8U, ICV_WARP_DESCALE_8U )
ICV_DEF_RESIZE_BILINEAR_FUNC( 16u, ushort, float, alpha, CV_NOP, cvRound )
ICV_DEF_RESIZE_BILINEAR_FUNC( 32f, float, float, alpha, CV_NOP, CV_NOP )

ICV_DEF_RESIZE_BICUBIC_FUNC( 8u, uchar, int, CV_8TO32F, cvRound, CV_CAST_8U )
ICV_DEF_RESIZE_BICUBIC_FUNC( 16u, ushort, int, CV_NOP, cvRound, CV_CAST_16U )
ICV_DEF_RESIZE_BICUBIC_FUNC( 32f, float, float, CV_NOP, CV_NOP, CV_NOP )

ICV_DEF_RESIZE_AREA_FAST_FUNC( 8u, uchar, int, cvRound )
ICV_DEF_RESIZE_AREA_FAST_FUNC( 16u, ushort, int, cvRound )
ICV_DEF_RESIZE_AREA_FAST_FUNC( 32f, float, float, CV_NOP )

ICV_DEF_RESIZE_AREA_FUNC( 8u, uchar, CV_8TO32F, cvRound )
ICV_DEF_RESIZE_AREA_FUNC( 16u, ushort, CV_NOP, cvRound )
ICV_DEF_RESIZE_AREA_FUNC( 32f, float, CV_NOP, CV_NOP )


static void icvInitResizeTab( CvFuncTable* bilin_tab,
                              CvFuncTable* bicube_tab,
                              CvFuncTable* areafast_tab,
                              CvFuncTable* area_tab )
{
    bilin_tab->fn_2d[CV_8U] = (void*)icvResize_Bilinear_8u_CnR;
    bilin_tab->fn_2d[CV_16U] = (void*)icvResize_Bilinear_16u_CnR;
    bilin_tab->fn_2d[CV_32F] = (void*)icvResize_Bilinear_32f_CnR;

    bicube_tab->fn_2d[CV_8U] = (void*)icvResize_Bicubic_8u_CnR;
    bicube_tab->fn_2d[CV_16U] = (void*)icvResize_Bicubic_16u_CnR;
    bicube_tab->fn_2d[CV_32F] = (void*)icvResize_Bicubic_32f_CnR;

    areafast_tab->fn_2d[CV_8U] = (void*)icvResize_AreaFast_8u_CnR;
    areafast_tab->fn_2d[CV_16U] = (void*)icvResize_AreaFast_16u_CnR;
    areafast_tab->fn_2d[CV_32F] = (void*)icvResize_AreaFast_32f_CnR;

    area_tab->fn_2d[CV_8U] = (void*)icvResize_Area_8u_CnR;
    area_tab->fn_2d[CV_16U] = (void*)icvResize_Area_16u_CnR;
    area_tab->fn_2d[CV_32F] = (void*)icvResize_Area_32f_CnR;
}


typedef CvStatus (CV_STDCALL * CvResizeBilinearFunc)
                    ( const void* src, int srcstep, CvSize ssize,
                      void* dst, int dststep, CvSize dsize,
                      int cn, int xmax, const CvResizeAlpha* xofs,
                      const CvResizeAlpha* yofs, float* buf0, float* buf1 );

typedef CvStatus (CV_STDCALL * CvResizeBicubicFunc)
                    ( const void* src, int srcstep, CvSize ssize,
                      void* dst, int dststep, CvSize dsize,
                      int cn, int xmin, int xmax,
                      const CvResizeAlpha* xofs, float** buf );

typedef CvStatus (CV_STDCALL * CvResizeAreaFastFunc)
                    ( const void* src, int srcstep, CvSize ssize,
                      void* dst, int dststep, CvSize dsize,
                      int cn, const int* ofs, const int *xofs );

typedef CvStatus (CV_STDCALL * CvResizeAreaFunc)
                    ( const void* src, int srcstep, CvSize ssize,
                      void* dst, int dststep, CvSize dsize,
                      int cn, const CvDecimateAlpha* xofs,
                      int xofs_count, float* buf, float* sum );


////////////////////////////////// IPP resize functions //////////////////////////////////

icvResize_8u_C1R_t icvResize_8u_C1R_p = 0;
icvResize_8u_C3R_t icvResize_8u_C3R_p = 0;
icvResize_8u_C4R_t icvResize_8u_C4R_p = 0;
icvResize_16u_C1R_t icvResize_16u_C1R_p = 0;
icvResize_16u_C3R_t icvResize_16u_C3R_p = 0;
icvResize_16u_C4R_t icvResize_16u_C4R_p = 0;
icvResize_32f_C1R_t icvResize_32f_C1R_p = 0;
icvResize_32f_C3R_t icvResize_32f_C3R_p = 0;
icvResize_32f_C4R_t icvResize_32f_C4R_p = 0;

typedef CvStatus (CV_STDCALL * CvResizeIPPFunc)
( const void* src, CvSize srcsize, int srcstep, CvRect srcroi,
  void* dst, int dststep, CvSize dstroi,
  double xfactor, double yfactor, int interpolation );

//////////////////////////////////////////////////////////////////////////////////////////

CV_IMPL void
cvResize( const CvArr* srcarr, CvArr* dstarr, int method )
{
    static CvFuncTable bilin_tab, bicube_tab, areafast_tab, area_tab;
    static int inittab = 0;
    void* temp_buf = 0;

    CV_FUNCNAME( "cvResize" );

    __BEGIN__;
    
    CvMat srcstub, *src = (CvMat*)srcarr;
    CvMat dststub, *dst = (CvMat*)dstarr;
    CvSize ssize, dsize;
    float scale_x, scale_y;
    int k, sx, sy, dx, dy;
    int type, depth, cn;
    
    CV_CALL( src = cvGetMat( srcarr, &srcstub ));
    CV_CALL( dst = cvGetMat( dstarr, &dststub ));
    
    if( CV_ARE_SIZES_EQ( src, dst ))
        CV_CALL( cvCopy( src, dst ));

    if( !CV_ARE_TYPES_EQ( src, dst ))
        CV_ERROR( CV_StsUnmatchedFormats, "" );

    if( !inittab )
    {
        icvInitResizeTab( &bilin_tab, &bicube_tab, &areafast_tab, &area_tab );
        inittab = 1;
    }

    ssize = cvGetMatSize( src );
    dsize = cvGetMatSize( dst );
    type = CV_MAT_TYPE(src->type);
    depth = CV_MAT_DEPTH(type);
    cn = CV_MAT_CN(type);
    scale_x = (float)ssize.width/dsize.width;
    scale_y = (float)ssize.height/dsize.height;

    if( method == CV_INTER_CUBIC &&
        (MIN(ssize.width, dsize.width) <= 4 ||
        MIN(ssize.height, dsize.height) <= 4) )
        method = CV_INTER_LINEAR;

    if( icvResize_8u_C1R_p &&
        MIN(ssize.width, dsize.width) > 4 &&
        MIN(ssize.height, dsize.height) > 4 )
    {
        CvResizeIPPFunc ipp_func =
            type == CV_8UC1 ? icvResize_8u_C1R_p :
            type == CV_8UC3 ? icvResize_8u_C3R_p :
            type == CV_8UC4 ? icvResize_8u_C4R_p :
            type == CV_16UC1 ? icvResize_16u_C1R_p :
            type == CV_16UC3 ? icvResize_16u_C3R_p :
            type == CV_16UC4 ? icvResize_16u_C4R_p :
            type == CV_32FC1 ? icvResize_32f_C1R_p :
            type == CV_32FC3 ? icvResize_32f_C3R_p :
            type == CV_32FC4 ? icvResize_32f_C4R_p : 0;
        if( ipp_func && (CV_INTER_NN < method && method < CV_INTER_AREA))
        {
            int srcstep = src->step ? src->step : CV_STUB_STEP;
            int dststep = dst->step ? dst->step : CV_STUB_STEP;
            IPPI_CALL( ipp_func( src->data.ptr, ssize, srcstep,
                                 cvRect(0,0,ssize.width,ssize.height),
                                 dst->data.ptr, dststep, dsize,
                                 (double)dsize.width/ssize.width,
                                 (double)dsize.height/ssize.height, 1 << method ));
            EXIT;
        }
    }

    if( method == CV_INTER_NN )
    {
        IPPI_CALL( icvResize_NN_8u_C1R( src->data.ptr, src->step, ssize,
                                        dst->data.ptr, dst->step, dsize,
                                        CV_ELEM_SIZE(src->type)));
    }
    else if( method == CV_INTER_LINEAR || method == CV_INTER_AREA )
    {
        if( method == CV_INTER_AREA &&
            ssize.width >= dsize.width && ssize.height >= dsize.height )
        {
            // "area" method for (scale_x > 1 & scale_y > 1)
            int iscale_x = cvRound(scale_x);
            int iscale_y = cvRound(scale_y);

            if( fabs(scale_x - iscale_x) < DBL_EPSILON &&
                fabs(scale_y - iscale_y) < DBL_EPSILON )
            {
                int area = iscale_x*iscale_y;
                int srcstep = src->step / CV_ELEM_SIZE(depth);
                int* ofs = (int*)cvAlloc( (area + dsize.width*cn)*sizeof(int) );
                int* xofs = ofs + area;
                CvResizeAreaFastFunc func = (CvResizeAreaFastFunc)areafast_tab.fn_2d[depth];

                if( !func )
				{
					cvFree(&ofs);
                    CV_ERROR( CV_StsUnsupportedFormat, "" );
				}
                
                for( sy = 0, k = 0; sy < iscale_y; sy++ )
                    for( sx = 0; sx < iscale_x; sx++ )
                        ofs[k++] = sy*srcstep + sx*cn;

                for( dx = 0; dx < dsize.width; dx++ )
                {
                    sx = dx*iscale_x*cn;
                    for( k = 0; k < cn; k++ )
                        xofs[dx*cn + k] = sx + k;
                }

                IPPI_CALL( func( src->data.ptr, src->step, ssize, dst->data.ptr,
                                 dst->step, dsize, cn, ofs, xofs ));

				cvFree(&ofs);
            }
            else
            {
                int buf_len = dsize.width*cn + 4, buf_size, xofs_count = 0;
                float scale = 1.f/(scale_x*scale_y);
                float *buf, *sum;
                CvDecimateAlpha* xofs;
                CvResizeAreaFunc func = (CvResizeAreaFunc)area_tab.fn_2d[depth];

                if( !func || cn > 4 )
                    CV_ERROR( CV_StsUnsupportedFormat, "" );

                buf_size = buf_len*2*sizeof(float) + ssize.width*2*sizeof(CvDecimateAlpha);
                //if( buf_size < CV_MAX_LOCAL_SIZE )
                //    buf = (float*)cvStackAlloc(buf_size);
                //else
                    CV_CALL( temp_buf = buf = (float*)cvAlloc(buf_size));
                sum = buf + buf_len;
                xofs = (CvDecimateAlpha*)(sum + buf_len);

                for( dx = 0, k = 0; dx < dsize.width; dx++ )
                {
                    float fsx1 = dx*scale_x, fsx2 = fsx1 + scale_x;
                    int sx1 = cvCeil(fsx1), sx2 = cvFloor(fsx2);

                    assert( (unsigned)sx1 < (unsigned)ssize.width );

                    if( sx1 > fsx1 )
                    {
                        assert( k < ssize.width*2 );            
                        xofs[k].di = dx*cn;
                        xofs[k].si = (sx1-1)*cn;
                        xofs[k++].alpha = (sx1 - fsx1)*scale;
                    }

                    for( sx = sx1; sx < sx2; sx++ )
                    {
                        assert( k < ssize.width*2 );
                        xofs[k].di = dx*cn;
                        xofs[k].si = sx*cn;
                        xofs[k++].alpha = scale;
                    }

                    if( fsx2 - sx2 > 1e-3 )
                    {
                        assert( k < ssize.width*2 );
                        assert((unsigned)sx2 < (unsigned)ssize.width );
                        xofs[k].di = dx*cn;
                        xofs[k].si = sx2*cn;
                        xofs[k++].alpha = (fsx2 - sx2)*scale;
                    }
                }

                xofs_count = k;
                memset( sum, 0, buf_len*sizeof(float) );
                memset( buf, 0, buf_len*sizeof(float) );

                IPPI_CALL( func( src->data.ptr, src->step, ssize, dst->data.ptr,
                                 dst->step, dsize, cn, xofs, xofs_count, buf, sum ));
            }
        }
        else // true "area" method for the cases (scale_x > 1 & scale_y < 1) and
             // (scale_x < 1 & scale_y > 1) is not implemented.
             // instead, it is emulated via some variant of bilinear interpolation.
        {
            float inv_scale_x = (float)dsize.width/ssize.width;
            float inv_scale_y = (float)dsize.height/ssize.height;
            int xmax = dsize.width, width = dsize.width*cn, buf_size;
            float *buf0, *buf1;
            CvResizeAlpha *xofs, *yofs;
            int area_mode = method == CV_INTER_AREA;
            float fx, fy;
            CvResizeBilinearFunc func = (CvResizeBilinearFunc)bilin_tab.fn_2d[depth];

            if( !func )
                CV_ERROR( CV_StsUnsupportedFormat, "" );

            buf_size = width*2*sizeof(float) + (width + dsize.height)*sizeof(CvResizeAlpha);
            //if( buf_size < CV_MAX_LOCAL_SIZE )
            //    buf0 = (float*)cvStackAlloc(buf_size);
            //else
                CV_CALL( temp_buf = buf0 = (float*)cvAlloc(buf_size));
            buf1 = buf0 + width;
            xofs = (CvResizeAlpha*)(buf1 + width);
            yofs = xofs + width;

            for( dx = 0; dx < dsize.width; dx++ )
            {
                if( !area_mode )
                {
                    fx = (float)((dx+0.5)*scale_x - 0.5);
                    sx = cvFloor(fx);
                    fx -= sx;
                }
                else
                {
                    sx = cvFloor(dx*scale_x);
                    fx = (dx+1) - (sx+1)*inv_scale_x;
                    fx = fx <= 0 ? 0.f : fx - cvFloor(fx);
                }

                if( sx < 0 )
                    fx = 0, sx = 0;

                if( sx >= ssize.width-1 )
                {
                    fx = 0, sx = ssize.width-1;
                    if( xmax >= dsize.width )
                        xmax = dx;
                }

                if( depth != CV_8U )
                    for( k = 0, sx *= cn; k < cn; k++ )
                        xofs[dx*cn + k].idx = sx + k, xofs[dx*cn + k].alpha = fx;
                else
                    for( k = 0, sx *= cn; k < cn; k++ )
                        xofs[dx*cn + k].idx = sx + k,
                        xofs[dx*cn + k].ialpha = CV_FLT_TO_FIX(fx, ICV_WARP_SHIFT);
            }

            for( dy = 0; dy < dsize.height; dy++ )
            {
                if( !area_mode )
                {
                    fy = (float)((dy+0.5)*scale_y - 0.5);
                    sy = cvFloor(fy);
                    fy -= sy;
                    if( sy < 0 )
                        sy = 0, fy = 0;
                }
                else
                {
                    sy = cvFloor(dy*scale_y);
                    fy = (dy+1) - (sy+1)*inv_scale_y;
                    fy = fy <= 0 ? 0.f : fy - cvFloor(fy);
                }

                yofs[dy].idx = sy;
                if( depth != CV_8U )
                    yofs[dy].alpha = fy;
                else
                    yofs[dy].ialpha = CV_FLT_TO_FIX(fy, ICV_WARP_SHIFT);
            }

            IPPI_CALL( func( src->data.ptr, src->step, ssize, dst->data.ptr,
                             dst->step, dsize, cn, xmax, xofs, yofs, buf0, buf1 ));
        }
    }
    else if( method == CV_INTER_CUBIC )
    {
        int width = dsize.width*cn, buf_size;
        int xmin = dsize.width, xmax = -1;
        CvResizeAlpha* xofs;
        float* buf[4];
        CvResizeBicubicFunc func = (CvResizeBicubicFunc)bicube_tab.fn_2d[depth];

        if( !func )
            CV_ERROR( CV_StsUnsupportedFormat, "" );
        
        buf_size = width*(4*sizeof(float) + sizeof(xofs[0]));
        //if( buf_size < CV_MAX_LOCAL_SIZE )
        //    buf[0] = (float*)cvStackAlloc(buf_size);
        //else
            CV_CALL( temp_buf = buf[0] = (float*)cvAlloc(buf_size));

        for( k = 1; k < 4; k++ )
            buf[k] = buf[k-1] + width;
        xofs = (CvResizeAlpha*)(buf[3] + width);

        icvInitCubicCoeffTab();

        for( dx = 0; dx < dsize.width; dx++ )
        {
            float fx = dx*scale_x;
            sx = cvFloor(fx);
            fx -= sx;
            int ifx = cvRound(fx*ICV_CUBIC_TAB_SIZE);
            if( sx-1 >= 0 && xmin > dx )
                xmin = dx;
            if( sx+2 < ssize.width )
                xmax = dx + 1;
        
            // at least one of 4 points should be within the image - to
            // be able to set other points to the same value. see the loops
            // for( dx = 0; dx < xmin; dx++ ) ... and for( ; dx < width; dx++ ) ...
            if( sx < -2 )
                sx = -2;
            else if( sx > ssize.width )
                sx = ssize.width;

            for( k = 0; k < cn; k++ )
            {
                xofs[dx*cn + k].idx = sx*cn + k;
                xofs[dx*cn + k].ialpha = ifx;
            }
        }
    
        IPPI_CALL( func( src->data.ptr, src->step, ssize, dst->data.ptr,
                         dst->step, dsize, cn, xmin, xmax, xofs, buf ));
    }
    else
        CV_ERROR( CV_StsBadFlag, "Unknown/unsupported interpolation method" );

    __END__;

    cvFree( &temp_buf );
}


/* End of file. */
