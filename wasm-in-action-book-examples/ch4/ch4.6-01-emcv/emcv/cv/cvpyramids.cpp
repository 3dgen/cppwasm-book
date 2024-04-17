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
// Copyright (c) 2008-2012, EMCV Project,
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


#include "_cv.h"

/****************************************************************************************\
                         Down-sampling pyramids core functions
\****************************************************************************************/

//////////// Filtering macros /////////////

/* COMMON CASE */
/* 1/16[1    4    6    4    1]       */
/* ...| x0 | x1 | x2 | x3 | x4 |...  */
#define  PD_FILTER( x0, x1, x2, x3, x4 ) ((x2)*6+((x1)+(x3))*4+(x0)+(x4))

/* MACROS FOR BORDERS */

/* | b I a | b | reflection used ("I" denotes the image boundary) */

/* LEFT/TOP */
/* 1/16[1    4    6    4    1]       */
/*    | x2 | x1 I x0 | x1 | x2 |...  */
#define  PD_LT(x0,x1,x2)                 ((x0)*6 + (x1)*8 + (x2)*2)

/* RIGHT/BOTTOM */
/* 1/16[1    4    6    4    1]       */
/* ...| x0 | x1 | x2 | x3 I x2 |     */
#define  PD_RB(x0,x1,x2,x3)              ((x0) + ((x1) + (x3))*4 + (x2)*7)

/* SINGULAR CASE ( width == 2 || height == 2 ) */
/* 1/16[1    4    6    4    1]       */
/*    | x0 | x1 I x0 | x1 I x0 |     */
#define  PD_SINGULAR(x0,x1)    (((x0) + (x1))*8)

#define  PD_SCALE_INT(x)       (((x) + (1<<7)) >> 8)
#define  PD_SCALE_FLT(x)       ((x)*0.00390625f)

#define  PD_SZ  5

////////// generic macro ////////////

#define ICV_DEF_PYR_DOWN_FUNC( flavor, type, worktype, _pd_scale_ )                     \
static CvStatus CV_STDCALL                                                              \
icvPyrDownG5x5_##flavor##_CnR( const type* src, int srcstep, type* dst,                 \
                               int dststep, CvSize size, void *buf, int Cs )            \
{                                                                                       \
    worktype*  buffer = (worktype*)buf;  /* pointer to temporary buffer */              \
    worktype*  rows[PD_SZ]; /* array of rows pointers. dim(rows) is PD_SZ */            \
    int  y, top_row = 0;                                                                \
    int  Wd = size.width/2, Wdn = Wd*Cs;                                                \
    int  buffer_step = Wdn;                                                             \
    int  pd_sz = (PD_SZ + 1)*buffer_step;                                               \
    int  fst = 0, lst = size.height <= PD_SZ/2 ? size.height : PD_SZ/2 + 1;             \
                                                                                        \
    assert( Cs == 1 || Cs == 3 );                                                       \
    srcstep /= sizeof(src[0]); dststep /= sizeof(dst[0]);                               \
                                                                                        \
    /* main loop */                                                                     \
    for( y = 0; y < size.height; y += 2, dst += dststep )                               \
    {                                                                                   \
        /* set first and last indices of buffer rows which are need to be filled */     \
        int x, y1, k = top_row;                                                         \
        int x1 = buffer_step;                                                           \
        worktype *row01, *row23, *row4;                                                 \
                                                                                        \
        /* assign rows pointers */                                                      \
        for( y1 = 0; y1 < PD_SZ; y1++ )                                                 \
        {                                                                               \
            rows[y1] = buffer + k;                                                      \
            k += buffer_step;                                                           \
            k &= k < pd_sz ? -1 : 0;                                                    \
        }                                                                               \
                                                                                        \
        row01 = rows[0];                                                                \
        row23 = rows[2];                                                                \
        row4  = rows[4];                                                                \
                                                                                        \
        /* fill new buffer rows with filtered source (horizontal conv) */               \
        if( Cs == 1 )                                                                   \
        {                                                                               \
            if( size.width > PD_SZ/2 )                                                  \
                for( y1 = fst; y1 < lst; y1++, src += srcstep )                         \
                {                                                                       \
                    worktype *row = rows[y1];                                           \
                                                                                        \
                    /* process left & right bounds */                                   \
                    row[0]    = PD_LT( src[0], src[1], src[2] );                        \
                    row[Wd-1] = PD_RB( src[Wd*2-4], src[Wd*2-3],                        \
                                       src[Wd*2-2], src[Wd*2-1]);                       \
                    /* other points (even) */                                           \
                    for( x = 1; x < Wd - 1; x++ )                                       \
                    {                                                                   \
                        row[x] = PD_FILTER( src[2*x-2], src[2*x-1], src[2*x],           \
                                            src[2*x+1], src[2*x+2] );                   \
                    }                                                                   \
                }                                                                       \
            else                                                                        \
                for( y1 = fst; y1 < lst; y1++, src += srcstep )                         \
                {                                                                       \
                    rows[y1][0] = PD_SINGULAR( src[0], src[1] );                        \
                }                                                                       \
        }                                                                               \
        else /* Cs == 3 */                                                              \
        {                                                                               \
            for( y1 = fst; y1 < lst; y1++, src += srcstep )                             \
            {                                                                           \
                worktype *row = rows[y1];                                               \
                                                                                        \
                if( size.width > PD_SZ/2 )                                              \
                {                                                                       \
                    int c;                                                              \
                    for( c = 0; c < 3; c++ )                                            \
                    {                                                                   \
                        /* process left & right bounds  */                              \
                        row[c] = PD_LT( src[c], src[3+c], src[6+c] );                   \
                        row[Wdn-3+c] = PD_RB( src[Wdn*2-12+c], src[Wdn*2-9+c],          \
                                              src[Wdn*2-6+c], src[Wdn*2-3+c] );         \
                    }                                                                   \
                    /* other points (even) */                                           \
                    for( x = 3; x < Wdn - 3; x += 3 )                                   \
                    {                                                                   \
                        row[x]   = PD_FILTER( src[2*x-6], src[2*x-3], src[2*x],         \
                                              src[2*x+3], src[2*x+6] );                 \
                        row[x+1] = PD_FILTER( src[2*x-5], src[2*x-2], src[2*x+1],       \
                                              src[2*x+4], src[2*x+7] );                 \
                        row[x+2] = PD_FILTER( src[2*x-4], src[2*x-1], src[2*x+2],       \
                                              src[2*x+5], src[2*x+8] );                 \
                    }                                                                   \
                }                                                                       \
                else /* size.width <= PD_SZ/2 */                                        \
                {                                                                       \
                    row[0] = PD_SINGULAR( src[0], src[3] );                             \
                    row[1] = PD_SINGULAR( src[1], src[4] );                             \
                    row[2] = PD_SINGULAR( src[2], src[5] );                             \
                }                                                                       \
            }                                                                           \
        }                                                                               \
                                                                                        \
        /* second pass. Do vertical conv and write results do destination image */      \
        if( y > 0 )                                                                     \
        {                                                                               \
            if( y < size.height - PD_SZ/2 )                                             \
            {                                                                           \
                for( x = 0; x < Wdn; x++, x1++ )                                        \
                {                                                                       \
                    dst[x] = (type)_pd_scale_( PD_FILTER( row01[x],  row01[x1],         \
                                               row23[x], row23[x1], row4[x] ));         \
                }                                                                       \
                top_row += 2*buffer_step;                                               \
                top_row &= top_row < pd_sz ? -1 : 0;                                    \
            }                                                                           \
            else /* bottom */                                                           \
                for( x = 0; x < Wdn; x++, x1++ )                                        \
                    dst[x] = (type)_pd_scale_( PD_RB( row01[x], row01[x1],              \
                                                      row23[x], row23[x1]));            \
        }                                                                               \
        else                                                                            \
        {                                                                               \
            if( size.height > PD_SZ/2 ) /* top */                                       \
            {                                                                           \
                for( x = 0; x < Wdn; x++, x1++ )                                        \
                    dst[x] = (type)_pd_scale_( PD_LT( row01[x], row01[x1], row23[x] )); \
            }                                                                           \
            else /* size.height <= PD_SZ/2 */                                           \
            {                                                                           \
                for( x = 0; x < Wdn; x++, x1++ )                                        \
                    dst[x] = (type)_pd_scale_( PD_SINGULAR( row01[x], row01[x1] ));     \
            }                                                                           \
            fst = PD_SZ - 2;                                                            \
        }                                                                               \
                                                                                        \
        lst = y + 2 + PD_SZ/2 < size.height ? PD_SZ : size.height - y;                  \
    }                                                                                   \
                                                                                        \
    return CV_OK;                                                                       \
}


ICV_DEF_PYR_DOWN_FUNC( 8u, uchar, int, PD_SCALE_INT )
ICV_DEF_PYR_DOWN_FUNC( 16s, short, int, PD_SCALE_INT )
ICV_DEF_PYR_DOWN_FUNC( 16u, ushort, int, PD_SCALE_INT )
ICV_DEF_PYR_DOWN_FUNC( 32f, float, float, PD_SCALE_FLT )
ICV_DEF_PYR_DOWN_FUNC( 64f, double, double, PD_SCALE_FLT )


/****************************************************************************************\
                           Up-sampling pyramids core functions
\****************************************************************************************/

/////////// filtering macros //////////////

/* COMMON CASE: NON ZERO */
/* 1/16[1    4   6    4   1]       */
/* ...| x0 | 0 | x1 | 0 | x2 |...  */
#define  PU_FILTER( x0, x1, x2 )         ((x1)*6 + (x0) + (x2))

/* ZERO POINT AT CENTER */
/* 1/16[1   4    6   4    1]      */
/* ...| 0 | x0 | 0 | x1 | 0 |...  */
#define  PU_FILTER_ZI( x0, x1 )          (((x0) + (x1))*4)

/* MACROS FOR BORDERS */

/* | b I a | b | reflection */

/* LEFT/TOP */
/* 1/16[1    4   6    4   1]       */
/*    | x1 | 0 I x0 | 0 | x1 |...  */
#define  PU_LT( x0, x1 )                 ((x0)*6 + (x1)*2)

/* 1/16[1   4    6   4    1]       */
/*    | 0 I x0 | 0 | x1 | 0 |...   */
#define  PU_LT_ZI( x0, x1 )              PU_FILTER_ZI((x0),(x1))

/* RIGHT/BOTTOM: NON ZERO */
/* 1/16[1    4   6    4   1]       */
/* ...| x0 | 0 | x1 | 0 I x1 |     */
#define  PU_RB( x0, x1 )                 ((x0) + (x1)*7)

/* RIGHT/BOTTOM: ZERO POINT AT CENTER */
/* 1/16[1   4    6   4    1]       */
/* ...| 0 | x0 | 0 I x0 | 0 |      */
#define  PU_RB_ZI( x0 )                  ((x0)*8)

/* SINGULAR CASE */
/* 1/16[1    4   6    4   1]       */
/*    | x0 | 0 I x0 | 0 I x0 |     */
#define  PU_SINGULAR( x0 )               PU_RB_ZI((x0)) /* <--| the same formulas */
#define  PU_SINGULAR_ZI( x0 )            PU_RB_ZI((x0)) /* <--| */

/* x/64  - scaling in up-sampling functions */
#define  PU_SCALE_INT(x)                 (((x) + (1<<5)) >> 6)
#define  PU_SCALE_FLT(x)                 ((x)*0.015625f)

#define  PU_SZ  3

//////////// generic macro /////////////


#define ICV_DEF_PYR_UP_FUNC( flavor, type, worktype, _pu_scale_ )                       \
static CvStatus CV_STDCALL                                                              \
icvPyrUpG5x5_##flavor##_CnR( const type* src, int srcstep, type* dst,                   \
                             int dststep, CvSize size, void *buf, int Cs )              \
{                                                                                       \
    worktype *buffer = (worktype*)buf;                                                  \
    worktype *rows[PU_SZ];                                                              \
    int y, top_row = 0;                                                                 \
    int Wd = size.width * 2, Wdn = Wd * Cs, Wn = size.width * Cs;                       \
    int buffer_step = Wdn;                                                              \
    int pu_sz = PU_SZ*buffer_step;                                                      \
    int fst = 0, lst = size.height <= PU_SZ/2 ? size.height : PU_SZ/2 + 1;              \
                                                                                        \
    assert( Cs == 1 || Cs == 3 );                                                       \
    srcstep /= sizeof(src[0]); dststep /= sizeof(dst[0]);                               \
                                                                                        \
    /* main loop */                                                                     \
    for( y = 0; y < size.height; y++, dst += 2 * dststep )                              \
    {                                                                                   \
        int x, y1, k = top_row;                                                         \
        worktype *row0, *row1, *row2;                                                   \
        type *dst1;                                                                     \
                                                                                        \
        /* assign rows pointers */                                                      \
        for( y1 = 0; y1 < PU_SZ; y1++ )                                                 \
        {                                                                               \
            rows[y1] = buffer + k;                                                      \
            k += buffer_step;                                                           \
            k &= k < pu_sz ? -1 : 0;                                                    \
        }                                                                               \
                                                                                        \
        row0 = rows[0];                                                                 \
        row1 = rows[1];                                                                 \
        row2 = rows[2];                                                                 \
        dst1 = dst + dststep;                                                           \
                                                                                        \
        /* fill new buffer rows with filtered source (horizontal conv) */               \
        if( Cs == 1 )                                                                   \
            if( size.width > PU_SZ / 2 )                                                \
                for( y1 = fst; y1 < lst; y1++, src += srcstep )                         \
                {                                                                       \
                    worktype *row = rows[y1];                                           \
                                                                                        \
                    /* process left & right bounds */                                   \
                    row[0] = PU_LT( src[0], src[1] );                                   \
                    row[1] = PU_LT_ZI( src[0], src[1] );                                \
                    row[size.width * 2 - 2] = PU_RB( src[size.width - 2],               \
                                                     src[size.width - 1] );             \
                    row[size.width * 2 - 1] = PU_RB_ZI( src[size.width - 1] );          \
                    /* other points */                                                  \
                    for( x = 1; x < size.width - 1; x++ )                               \
                    {                                                                   \
                        row[2 * x] = PU_FILTER( src[x - 1], src[x], src[x + 1] );       \
                        row[2 * x + 1] = PU_FILTER_ZI( src[x], src[x + 1] );            \
                    }                                                                   \
                }                                                                       \
            else                /* size.width <= PU_SZ/2 */                             \
                for( y1 = fst; y1 < lst; y1++, src += srcstep )                         \
                {                                                                       \
                    worktype *row = rows[y1];                                           \
                    worktype val = src[0];                                              \
                                                                                        \
                    row[0] = PU_SINGULAR( val );                                        \
                    row[1] = PU_SINGULAR_ZI( val );                                     \
                }                                                                       \
        else                    /* Cs == 3 */                                           \
            for( y1 = fst; y1 < lst; y1++, src += srcstep )                             \
            {                                                                           \
                worktype *row = rows[y1];                                               \
                                                                                        \
                if( size.width > PU_SZ / 2 )                                            \
                {                                                                       \
                    int c;                                                              \
                                                                                        \
                    for( c = 0; c < 3; c++ )                                            \
                    {                                                                   \
                        /* process left & right bounds  */                              \
                        row[c] = PU_LT( src[c], src[3 + c] );                           \
                        row[3 + c] = PU_LT_ZI( src[c], src[3 + c] );                    \
                        row[Wn * 2 - 6 + c] = PU_RB( src[Wn - 6 + c], src[Wn - 3 + c]); \
                        row[Wn * 2 - 3 + c] = PU_RB_ZI( src[Wn - 3 + c] );              \
                    }                                                                   \
                    /* other points */                                                  \
                    for( x = 3; x < Wn - 3; x += 3 )                                    \
                    {                                                                   \
                        row[2 * x] = PU_FILTER( src[x - 3], src[x], src[x + 3] );       \
                        row[2 * x + 3] = PU_FILTER_ZI( src[x], src[x + 3] );            \
                                                                                        \
                        row[2 * x + 1] = PU_FILTER( src[x - 2], src[x + 1], src[x + 4]);\
                        row[2 * x + 4] = PU_FILTER_ZI( src[x + 1], src[x + 4] );        \
                                                                                        \
                        row[2 * x + 2] = PU_FILTER( src[x - 1], src[x + 2], src[x + 5]);\
                        row[2 * x + 5] = PU_FILTER_ZI( src[x + 2], src[x + 5] );        \
                    }                                                                   \
                }                                                                       \
                else            /* size.width <= PU_SZ/2 */                             \
                {                                                                       \
                    int c;                                                              \
                                                                                        \
                    for( c = 0; c < 3; c++ )                                            \
                    {                                                                   \
                        row[c] = PU_SINGULAR( src[c] );                                 \
                        row[3 + c] = PU_SINGULAR_ZI( src[c] );                          \
                    }                                                                   \
                }                                                                       \
            }                                                                           \
                                                                                        \
        /* second pass. Do vertical conv and write results do destination image */      \
        if( y > 0 )                                                                     \
        {                                                                               \
            if( y < size.height - PU_SZ / 2 )                                           \
            {                                                                           \
                for( x = 0; x < Wdn; x++ )                                              \
                {                                                                       \
                    dst[x] = (type)_pu_scale_( PU_FILTER( row0[x], row1[x], row2[x] )); \
                    dst1[x] = (type)_pu_scale_( PU_FILTER_ZI( row1[x], row2[x] ));      \
                }                                                                       \
                top_row += buffer_step;                                                 \
                top_row &= top_row < pu_sz ? -1 : 0;                                    \
            }                                                                           \
            else                /* bottom */                                            \
                for( x = 0; x < Wdn; x++ )                                              \
                {                                                                       \
                    dst[x] = (type)_pu_scale_( PU_RB( row0[x], row1[x] ));              \
                    dst1[x] = (type)_pu_scale_( PU_RB_ZI( row1[x] ));                   \
                }                                                                       \
        }                                                                               \
        else                                                                            \
        {                                                                               \
            if( size.height > PU_SZ / 2 ) /* top */                                     \
                for( x = 0; x < Wdn; x++ )                                              \
                {                                                                       \
                    dst[x] = (type)_pu_scale_( PU_LT( row0[x], row1[x] ));              \
                    dst1[x] = (type)_pu_scale_( PU_LT_ZI( row0[x], row1[x] ));          \
                }                                                                       \
            else                /* size.height <= PU_SZ/2 */                            \
                for( x = 0; x < Wdn; x++ )                                              \
                {                                                                       \
                    dst[x] = (type)_pu_scale_( PU_SINGULAR( row0[x] ));                 \
                    dst1[x] = (type)_pu_scale_( PU_SINGULAR_ZI( row0[x] ));             \
                }                                                                       \
            fst = PU_SZ - 1;                                                            \
        }                                                                               \
                                                                                        \
        lst = y < size.height - PU_SZ/2 - 1 ? PU_SZ : size.height + PU_SZ/2 - y - 1;    \
    }                                                                                   \
                                                                                        \
    return CV_OK;                                                                       \
}


ICV_DEF_PYR_UP_FUNC( 8u, uchar, int, PU_SCALE_INT )
ICV_DEF_PYR_UP_FUNC( 16s, short, int, PU_SCALE_INT )
ICV_DEF_PYR_UP_FUNC( 16u, ushort, int, PU_SCALE_INT )
ICV_DEF_PYR_UP_FUNC( 32f, float, float, PU_SCALE_FLT )
ICV_DEF_PYR_UP_FUNC( 64f, double, double, PU_SCALE_FLT )


static CvStatus CV_STDCALL
icvPyrUpG5x5_GetBufSize( int roiWidth, CvDataType dataType,
                         int channels, int *bufSize )
{
    int bufStep;

    if( !bufSize )
        return CV_NULLPTR_ERR;
    *bufSize = 0;

    if( roiWidth < 0 )
        return CV_BADSIZE_ERR;
    if( channels != 1 && channels != 3 )
        return CV_UNSUPPORTED_CHANNELS_ERR;

    bufStep = 2*roiWidth*channels;

    if( dataType == cv64f )
        bufStep *= sizeof(double);
    else
        bufStep *= sizeof(int);

    *bufSize = bufStep * PU_SZ;
    return CV_OK;
}


static CvStatus CV_STDCALL
icvPyrDownG5x5_GetBufSize( int roiWidth, CvDataType dataType,
                           int channels, int *bufSize )
{
    int bufStep;

    if( !bufSize )
        return CV_NULLPTR_ERR;
    *bufSize = 0;

    if( roiWidth < 0 || (roiWidth & 1) != 0 )
        return CV_BADSIZE_ERR;
    if( channels != 1 && channels != 3 )
        return CV_UNSUPPORTED_CHANNELS_ERR;

    bufStep = 2*roiWidth*channels;

    if( dataType == cv64f )
        bufStep *= sizeof(double);
    else
        bufStep *= sizeof(int);

    *bufSize = bufStep * (PD_SZ + 1);
    return CV_OK;
}

/****************************************************************************************\
                        Downsampled image border completion
\****************************************************************************************/

#define ICV_DEF_PYR_BORDER_FUNC( flavor, arrtype, worktype, _pd_scale_ )                \
static CvStatus CV_STDCALL                                                              \
icvPyrDownBorder_##flavor##_CnR( const arrtype *src, int src_step, CvSize src_size,     \
                          arrtype *dst, int dst_step, CvSize dst_size, int channels )   \
{                                                                                       \
    int local_alloc = 0;                                                                \
    worktype *buf = 0, *buf0 = 0;                                                       \
    const arrtype* src2;                                                                \
    arrtype* dst2;                                                                      \
    int buf_size;                                                                       \
    int i, j;                                                                           \
    int W = src_size.width, H = src_size.height;                                        \
    int Wd = dst_size.width, Hd = dst_size.height;                                      \
    int Wd_, Hd_;                                                                       \
    int Wn = W*channels;                                                                \
    int bufW;                                                                           \
    int cols, rows; /* columns and rows to modify */                                    \
                                                                                        \
    assert( channels == 1 || channels == 3 );                                           \
                                                                                        \
    buf_size = MAX(src_size.width,src_size.height) * sizeof(buf[0]) * 2 * channels;     \
                                                                                        \
    {                                                                                   \
        buf = (worktype*)cvAlloc( buf_size );                                           \
        if( !buf )                                                                      \
            return CV_OUTOFMEM_ERR;                                                     \
    }                                                                                   \
                                                                                        \
    buf0 = buf;                                                                         \
                                                                                        \
    src_step /= sizeof(src[0]);                                                         \
    dst_step /= sizeof(dst[0]);                                                         \
                                                                                        \
    cols = (W & 1) + (Wd*2 > W);                                                        \
    rows = (H & 1) + (Hd*2 > H);                                                        \
                                                                                        \
    src2 = src + (H-1)*src_step;                                                        \
    dst2 = dst + (Hd - rows)*dst_step;                                                  \
    src += (W - 1)*channels;                                                            \
    dst += (Wd - cols)*channels;                                                        \
                                                                                        \
    /* part of row(column) from 1 to Wd_(Hd_) is processed using PD_FILTER macro */     \
    Wd_ = Wd - 1 + (cols == 1 && (W & 1) != 0);                                         \
    Hd_ = Hd - 1 + (rows == 1 && (H & 1) != 0);                                         \
                                                                                        \
    bufW = channels * cols;                                                             \
                                                                                        \
    /******************* STAGE 1. ******************/                                   \
                                                                                        \
    /* do horizontal convolution of the 1-2 right columns and write results to buffer */\
    if( cols > 0 )                                                                      \
    {                                                                                   \
        if( W <= 2 )                                                                    \
        {                                                                               \
            assert( Wd == 1 );                                                          \
            for( i = 0; i < H; i++, src += src_step, buf += channels )                  \
            {                                                                           \
                if( channels == 1 )                                                     \
                    buf[0] = PD_SINGULAR( src[1-Wn], src[0] );                          \
                else                                                                    \
                {                                                                       \
                    buf[0] = PD_SINGULAR( src[3-Wn], src[0] );                          \
                    buf[1] = PD_SINGULAR( src[4-Wn], src[1] );                          \
                    buf[2] = PD_SINGULAR( src[5-Wn], src[2] );                          \
                }                                                                       \
            }                                                                           \
        }                                                                               \
        else if( W == 3 && Wd == 1 || W > 3 && !(Wd & 1) )                              \
        {                                                                               \
            for( i = 0; i < H; i++, src += src_step, buf += channels )                  \
            {                                                                           \
                if( channels == 1 )                                                     \
                    buf[0] = PD_LT( src[-2], src[-1], src[0] );                         \
                else                                                                    \
                {                                                                       \
                    buf[0] = PD_LT( src[-6], src[-3], src[0] );                         \
                    buf[1] = PD_LT( src[-5], src[-2], src[1] );                         \
                    buf[2] = PD_LT( src[-4], src[-1], src[2] );                         \
                }                                                                       \
            }                                                                           \
        }                                                                               \
        else if( W == 3 )                                                               \
        {                                                                               \
            for( i = 0; i < H; i++, src += src_step, buf += channels*2 )                \
            {                                                                           \
                if( channels == 1 )                                                     \
                {                                                                       \
                    buf[0] = PD_LT( src[-2], src[-1], src[0] );                         \
                    buf[1] = PD_LT( src[0], src[-1], src[-2] );                         \
                }                                                                       \
                else                                                                    \
                {                                                                       \
                    buf[0] = PD_LT( src[-6], src[-3], src[0] );                         \
                    buf[1] = PD_LT( src[-5], src[-2], src[1] );                         \
                    buf[2] = PD_LT( src[-4], src[-1], src[2] );                         \
                    buf[3] = PD_LT( src[0], src[-3], src[-6] );                         \
                    buf[4] = PD_LT( src[1], src[-2], src[-5] );                         \
                    buf[5] = PD_LT( src[2], src[-1], src[-4] );                         \
                }                                                                       \
            }                                                                           \
        }                                                                               \
        else if( cols == 1 )                                                            \
        {                                                                               \
            for( i = 0; i < H; i++, src += src_step, buf += channels )                  \
            {                                                                           \
                if( channels == 1 )                                                     \
                    buf[0] = PD_FILTER( src[-4], src[-3], src[-2], src[-1], src[0]);    \
                else                                                                    \
                {                                                                       \
                    buf[0] = PD_FILTER( src[-12], src[-9], src[-6], src[-3], src[0]);   \
                    buf[1] = PD_FILTER( src[-11], src[-8], src[-5], src[-2], src[1]);   \
                    buf[2] = PD_FILTER( src[-10], src[-7], src[-4], src[-1], src[2]);   \
                }                                                                       \
            }                                                                           \
        }                                                                               \
        else                                                                            \
        {                                                                               \
            for( i = 0; i < H; i++, src += src_step, buf += channels*2 )                \
            {                                                                           \
                if( channels == 1 )                                                     \
                {                                                                       \
                    buf[0] = PD_FILTER( src[-4], src[-3], src[-2], src[-1], src[0] );   \
                    buf[1] = PD_LT( src[0], src[-1], src[-2] );                         \
                }                                                                       \
                else                                                                    \
                {                                                                       \
                    buf[0] = PD_FILTER( src[-12], src[-9], src[-6], src[-3], src[0] );  \
                    buf[1] = PD_FILTER( src[-11], src[-8], src[-5], src[-2], src[1] );  \
                    buf[2] = PD_FILTER( src[-10], src[-7], src[-4], src[-1], src[2] );  \
                    buf[3] = PD_LT( src[0], src[-3], src[-6] );                         \
                    buf[4] = PD_LT( src[1], src[-2], src[-5] );                         \
                    buf[5] = PD_LT( src[2], src[-1], src[-4] );                         \
                }                                                                       \
            }                                                                           \
        }                                                                               \
        buf = buf0;                                                                     \
    }                                                                                   \
                                                                                        \
    src = src2;                                                                         \
                                                                                        \
    /******************* STAGE 2. ******************/                                   \
                                                                                        \
    /* do vertical convolution of the pre-processed right columns, */                   \
    /* stored in buffer, and write results to the destination */                        \
    /* do vertical convolution of the 1-2 bottom rows */                                \
    /* and write results to the buffer */                                               \
    if( H <= 2 )                                                                        \
    {                                                                                   \
        if( cols > 0 )                                                                  \
        {                                                                               \
            assert( Hd == 1 );                                                          \
            for( j = 0; j < bufW; j++ )                                                 \
                dst[j] = (arrtype)_pd_scale_( PD_SINGULAR( buf[j], buf[j+(H-1)*bufW] ));\
        }                                                                               \
                                                                                        \
        if( rows > 0 )                                                                  \
        {                                                                               \
            for( j = 0; j < Wn; j++ )                                                   \
                buf[j] = PD_SINGULAR( src[j-src_step], src[j] );                        \
        }                                                                               \
    }                                                                                   \
    else if( H == 3 )                                                                   \
    {                                                                                   \
        if( cols > 0 )                                                                  \
        {                                                                               \
            for( j = 0; j < bufW; j++ )                                                 \
            {                                                                           \
                dst[j]= (arrtype)_pd_scale_(PD_LT( buf[j], buf[j+bufW], buf[j+bufW*2]));\
            }                                                                           \
            if( Hd == 2 )                                                               \
            {                                                                           \
                dst += dst_step;                                                        \
                for( j = 0; j < bufW; j++ )                                             \
                    dst[j] = (arrtype)_pd_scale_( PD_LT( buf[j+bufW*2],                 \
                                                  buf[j+bufW], buf[j] ));               \
            }                                                                           \
        }                                                                               \
                                                                                        \
        if( Hd == 1 )                                                                   \
        {                                                                               \
            for( j = 0; j < Wn; j++ )                                                   \
                buf[j] = PD_LT( src[j-src_step*2], src[j - src_step], src[j] );         \
        }                                                                               \
        else                                                                            \
        {                                                                               \
            for( j = 0; j < Wn; j++ )                                                   \
            {                                                                           \
                buf[j] = PD_LT( src[j-src_step*2], src[j - src_step], src[j] );         \
                buf[j+Wn] = PD_LT( src[j],src[j-src_step],src[j-src_step*2] );          \
            }                                                                           \
        }                                                                               \
    }                                                                                   \
    else                                                                                \
    {                                                                                   \
        if( cols > 0 )                                                                  \
        {                                                                               \
            /* top of the right border */                                               \
            for( j = 0; j < bufW; j++ )                                                 \
                dst[j]=(arrtype)_pd_scale_( PD_LT( buf[j], buf[j+bufW], buf[j+bufW*2]));\
                                                                                        \
            /* middle part of the right border */                                       \
            buf += bufW*2;                                                              \
            dst += dst_step;                                                            \
            for( i = 1; i < Hd_; i++, dst += dst_step, buf += bufW*2 )                  \
            {                                                                           \
                for( j = 0; j < bufW; j++ )                                             \
                    dst[j] = (arrtype)_pd_scale_( PD_FILTER( buf[j-bufW*2], buf[j-bufW],\
                                                  buf[j], buf[j+bufW], buf[j+bufW*2] ));\
            }                                                                           \
                                                                                        \
            /* bottom of the right border */                                            \
            if( !(H & 1) )                                                              \
            {                                                                           \
                for( j = 0; j < bufW; j++ )                                             \
                    dst[j] = (arrtype)_pd_scale_( PD_RB( buf[j-bufW*2], buf[j-bufW],    \
                                                         buf[j], buf[j+bufW] ));        \
            }                                                                           \
            else if( rows > 1 )                                                         \
            {                                                                           \
                for( j = 0; j < bufW; j++ )                                             \
                    dst[j]=(arrtype)_pd_scale_( PD_LT( buf[j-bufW*2],                   \
                                                buf[j-bufW], buf[j]));                  \
            }                                                                           \
                                                                                        \
            buf = buf0;                                                                 \
        }                                                                               \
                                                                                        \
        if( rows > 0 )                                                                  \
        {                                                                               \
            if( !(H & 1) )                                                              \
            {                                                                           \
                for( j = 0; j < Wn; j++ )                                               \
                    buf[j] = PD_LT( src[j], src[j-src_step], src[j-src_step*2] );       \
            }                                                                           \
            else if( cols == 1 )                                                        \
            {                                                                           \
                for( j = 0; j < Wn; j++ )                                               \
                    buf[j] = PD_FILTER( src[j-src_step*4], src[j-src_step*3],           \
                                        src[j-src_step*2], src[j-src_step], src[j] );   \
            }                                                                           \
            else                                                                        \
            {                                                                           \
                for( j = 0; j < Wn; j++ )                                               \
                {                                                                       \
                    buf[j] = PD_FILTER( src[j-src_step*4], src[j-src_step*3],           \
                                        src[j-src_step*2], src[j-src_step], src[j] );   \
                    buf[j+Wn] = PD_LT( src[j], src[j-src_step], src[j-src_step*2] );    \
                }                                                                       \
            }                                                                           \
        }                                                                               \
    }                                                                                   \
                                                                                        \
                                                                                        \
    /******************* STAGE 3. ******************/                                   \
                                                                                        \
    /* do horizontal convolution of the pre-processed bottom rows,*/                    \
    /* stored in buffer, and write results to the destination */                        \
    if( rows > 0 )                                                                      \
    {                                                                                   \
        dst = dst2;                                                                     \
                                                                                        \
        if( W <= 2 )                                                                    \
        {                                                                               \
            assert( Wd == 1 );                                                          \
            for( ; rows--; dst += dst_step, buf += Wn )                                 \
            {                                                                           \
                if( channels == 1 )                                                     \
                    dst[0] = (arrtype)_pd_scale_( PD_SINGULAR( buf[0], buf[Wn-1] ));    \
                else                                                                    \
                {                                                                       \
                    dst[0] = (arrtype)_pd_scale_( PD_SINGULAR( buf[0], buf[Wn-3] ));    \
                    dst[1] = (arrtype)_pd_scale_( PD_SINGULAR( buf[1], buf[Wn-2] ));    \
                    dst[2] = (arrtype)_pd_scale_( PD_SINGULAR( buf[2], buf[Wn-1] ));    \
                }                                                                       \
            }                                                                           \
        }                                                                               \
        else if( W == 3 )                                                               \
        {                                                                               \
            if( Wd == 1 )                                                               \
            {                                                                           \
                for( ; rows--; dst += dst_step, buf += Wn )                             \
                {                                                                       \
                    if( channels == 1 )                                                 \
                        dst[0] = (arrtype)_pd_scale_( PD_LT(buf[0], buf[1], buf[2] ));  \
                    else                                                                \
                    {                                                                   \
                        dst[0] = (arrtype)_pd_scale_( PD_LT(buf[0], buf[3], buf[6] ));  \
                        dst[1] = (arrtype)_pd_scale_( PD_LT(buf[1], buf[4], buf[7] ));  \
                        dst[2] = (arrtype)_pd_scale_( PD_LT(buf[2], buf[5], buf[8] ));  \
                    }                                                                   \
                }                                                                       \
            }                                                                           \
            else                                                                        \
            {                                                                           \
                for( ; rows--; dst += dst_step, buf += Wn )                             \
                {                                                                       \
                    if( channels == 1 )                                                 \
                    {                                                                   \
                        dst[0] = (arrtype)_pd_scale_( PD_LT(buf[0], buf[1], buf[2] ));  \
                        dst[1] = (arrtype)_pd_scale_( PD_LT(buf[2], buf[1], buf[0] ));  \
                    }                                                                   \
                    else                                                                \
                    {                                                                   \
                        dst[0] = (arrtype)_pd_scale_( PD_LT(buf[0], buf[3], buf[6] ));  \
                        dst[1] = (arrtype)_pd_scale_( PD_LT(buf[1], buf[4], buf[7] ));  \
                        dst[2] = (arrtype)_pd_scale_( PD_LT(buf[2], buf[5], buf[8] ));  \
                        dst[3] = (arrtype)_pd_scale_( PD_LT(buf[6], buf[3], buf[0] ));  \
                        dst[4] = (arrtype)_pd_scale_( PD_LT(buf[7], buf[4], buf[1] ));  \
                        dst[5] = (arrtype)_pd_scale_( PD_LT(buf[8], buf[5], buf[2] ));  \
                    }                                                                   \
                }                                                                       \
            }                                                                           \
        }                                                                               \
        else                                                                            \
        {                                                                               \
            for( ; rows--; dst += dst_step, buf += Wn )                                 \
            {                                                                           \
                if( channels == 1 )                                                     \
                {                                                                       \
                    /* left part of the bottom row */                                   \
                    dst[0] = (arrtype)_pd_scale_( PD_LT( buf[0], buf[1], buf[2] ));     \
                                                                                        \
                    /* middle part of the bottom row */                                 \
                    for( i = 1; i < Wd_; i++ )                                          \
                    {                                                                   \
                        dst[i] = (arrtype)_pd_scale_( PD_FILTER(buf[i*2-2], buf[i*2-1], \
                                                    buf[i*2],buf[i*2+1], buf[i*2+2] )); \
                    }                                                                   \
                                                                                        \
                    /* right part of the bottom row */                                  \
                    if( !(W & 1) )                                                      \
                        dst[i] = (arrtype)_pd_scale_( PD_RB( buf[i*2-2],buf[i*2-1],     \
                                                    buf[i*2], buf[i*2+1] ));            \
                    else if( cols > 1 )                                                 \
                        dst[i] = (arrtype)_pd_scale_( PD_LT( buf[i*2-2],                \
                                                    buf[i*2-1], buf[i*2] ));            \
                }                                                                       \
                else                                                                    \
                {                                                                       \
                    /* left part of the bottom row */                                   \
                    dst[0] = (arrtype)_pd_scale_( PD_LT( buf[0], buf[3], buf[6] ));     \
                    dst[1] = (arrtype)_pd_scale_( PD_LT( buf[1], buf[4], buf[7] ));     \
                    dst[2] = (arrtype)_pd_scale_( PD_LT( buf[2], buf[5], buf[8] ));     \
                                                                                        \
                    /* middle part of the bottom row */                                 \
                    for( i = 3; i < Wd_*3; i++ )                                        \
                    {                                                                   \
                        dst[i] = (arrtype)_pd_scale_( PD_FILTER(buf[i*2-6], buf[i*2-3], \
                                                      buf[i*2],buf[i*2+3], buf[i*2+6]));\
                    }                                                                   \
                                                                                        \
                    /* right part of the bottom row */                                  \
                    if( !(W & 1) )                                                      \
                    {                                                                   \
                        dst[i] = (arrtype)_pd_scale_( PD_RB( buf[i*2-6],buf[i*2-3],     \
                                                    buf[i*2], buf[i*2+3] ));            \
                        dst[i+1] = (arrtype)_pd_scale_( PD_RB( buf[i*2-5],buf[i*2-2],   \
                                                    buf[i*2+1], buf[i*2+4] ));          \
                        dst[i+2] = (arrtype)_pd_scale_( PD_RB( buf[i*2-4],buf[i*2-1],   \
                                                    buf[i*2+2], buf[i*2+5] ));          \
                    }                                                                   \
                    else if( cols > 1 )                                                 \
                    {                                                                   \
                        dst[i] = (arrtype)_pd_scale_( PD_LT( buf[i*2-6], buf[i*2-3], buf[i*2] ));   \
                        dst[i+1] = (arrtype)_pd_scale_( PD_LT( buf[i*2-5], buf[i*2-2], buf[i*2+1]));\
                        dst[i+2] = (arrtype)_pd_scale_( PD_LT( buf[i*2-4], buf[i*2-1], buf[i*2+2]));\
                    }                                                                   \
                }                                                                       \
            }                                                                           \
        }                                                                               \
    }                                                                                   \
                                                                                        \
    if( !local_alloc )                                                                  \
        cvFree( &buf0 );                                                                \
                                                                                        \
    return CV_OK;                                                                       \
}


#define ICV_DEF_INIT_PYR_TABLE( FUNCNAME )                          \
static void icvInit##FUNCNAME##Table( CvFuncTable* tab )            \
{                                                                   \
    tab->fn_2d[CV_8U] = (void*)icv##FUNCNAME##_8u_CnR;              \
    tab->fn_2d[CV_8S] = 0;                                          \
    tab->fn_2d[CV_16S] = (void*)icv##FUNCNAME##_16s_CnR;            \
    tab->fn_2d[CV_16U] = (void*)icv##FUNCNAME##_16u_CnR;            \
    tab->fn_2d[CV_32F] = (void*)icv##FUNCNAME##_32f_CnR;            \
    tab->fn_2d[CV_64F] = (void*)icv##FUNCNAME##_64f_CnR;            \
}

static void icvInitPyrDownBorderTable( CvFuncTable* tab );

ICV_DEF_INIT_PYR_TABLE( PyrUpG5x5 )
ICV_DEF_INIT_PYR_TABLE( PyrDownG5x5 )

typedef CvStatus (CV_STDCALL * CvPyrDownBorderFunc)( const void* src, int srcstep,
                                                     CvSize srcsize, void* dst,
                                                     int dststep, CvSize dstsize, int cn );

////////////////////////////// IPP pyramid functions /////////////////////////////////////

icvPyrDown_Gauss5x5_8u_C1R_t icvPyrDown_Gauss5x5_8u_C1R_p = 0; 
icvPyrDown_Gauss5x5_8u_C3R_t icvPyrDown_Gauss5x5_8u_C3R_p = 0; 
icvPyrDown_Gauss5x5_32f_C1R_t icvPyrDown_Gauss5x5_32f_C1R_p = 0; 
icvPyrDown_Gauss5x5_32f_C3R_t icvPyrDown_Gauss5x5_32f_C3R_p = 0; 

icvPyrUp_Gauss5x5_8u_C1R_t icvPyrUp_Gauss5x5_8u_C1R_p = 0; 
icvPyrUp_Gauss5x5_8u_C3R_t icvPyrUp_Gauss5x5_8u_C3R_p = 0; 
icvPyrUp_Gauss5x5_32f_C1R_t icvPyrUp_Gauss5x5_32f_C1R_p = 0; 
icvPyrUp_Gauss5x5_32f_C3R_t icvPyrUp_Gauss5x5_32f_C3R_p = 0; 

icvPyrUpGetBufSize_Gauss5x5_t icvPyrUpGetBufSize_Gauss5x5_p = 0;
icvPyrDownGetBufSize_Gauss5x5_t icvPyrDownGetBufSize_Gauss5x5_p = 0;

typedef CvStatus (CV_STDCALL * CvPyramidFunc)
( const void* src, int srcstep, void* dst,
  int dststep, CvSize size, void* buffer, int cn );

typedef CvStatus (CV_STDCALL * CvPyramidIPPFunc)
( const void* src, int srcstep, void* dst, int dststep, CvSize size, void* buffer );

//////////////////////////////////////////////////////////////////////////////////////////

/****************************************************************************************\
*                                 External functions                                     *
\****************************************************************************************/

CV_IMPL void
cvPyrUp( const void* srcarr, void* dstarr, int _filter )
{
    static CvFuncTable pyrup_tab;
    static int inittab = 0;
    
    void *buffer = 0;
    int local_alloc = 0;

    CV_FUNCNAME( "cvPyrUp" );

    __BEGIN__;

    int coi1 = 0, coi2 = 0;
    int buffer_size = 0;
    int type, depth, cn;
    CvMat srcstub, *src = (CvMat*)srcarr;
    CvMat dststub, *dst = (CvMat*)dstarr;
    CvFilter filter = (CvFilter) _filter;
    CvPyramidFunc func;
    CvPyramidIPPFunc ipp_func = 0;
    int use_ipp = 0;
    CvSize size;

    if( !inittab )
    {
        icvInitPyrUpG5x5Table( &pyrup_tab );
        inittab = 1;
    }

    CV_CALL( src = cvGetMat( src, &srcstub, &coi1 ));
    CV_CALL( dst = cvGetMat( dst, &dststub, &coi2 ));

    if( coi1 != 0 || coi2 != 0 )
        CV_ERROR( CV_BadCOI, "" );

    if( filter != CV_GAUSSIAN_5x5 )
        CV_ERROR( CV_StsBadArg, "this filter type not supported" );

    if( !CV_ARE_TYPES_EQ( src, dst ))
        CV_ERROR( CV_StsUnmatchedFormats, "" );

    if( src->cols*2 != dst->cols || src->rows*2 != dst->rows )
        CV_ERROR( CV_StsUnmatchedSizes, "" );

    size = cvGetMatSize(src);
    type = CV_MAT_TYPE(src->type);
    depth = CV_MAT_DEPTH(type);
    cn = CV_MAT_CN(type);

    if( cn != 1 && cn != 3 )
        CV_ERROR( CV_StsUnsupportedFormat, "The images must have 1 or 3 channel" );

    func = (CvPyramidFunc)pyrup_tab.fn_2d[depth];

    if( !func )
        CV_ERROR( CV_StsUnsupportedFormat, "" );

    if( icvPyrUpGetBufSize_Gauss5x5_p )
    {
        ipp_func = type == CV_8UC1 ? icvPyrUp_Gauss5x5_8u_C1R_p :
                   type == CV_8UC3 ? icvPyrUp_Gauss5x5_8u_C3R_p :
                   type == CV_32FC1 ? icvPyrUp_Gauss5x5_32f_C1R_p :
                   type == CV_32FC3 ? icvPyrUp_Gauss5x5_32f_C3R_p : 0;

        use_ipp = ipp_func && icvPyrUpGetBufSize_Gauss5x5_p( size.width,
                    icvDepthToDataType(type), cn, &buffer_size ) >= 0;
    }

    if( !use_ipp )
        icvPyrUpG5x5_GetBufSize( size.width, icvDepthToDataType(type), cn, &buffer_size );

	/*
    if( buffer_size <= CV_MAX_LOCAL_SIZE )
    {
        buffer = cvStackAlloc( buffer_size );
        local_alloc = 1;
    }
    else*/
        CV_CALL( buffer = cvAlloc( buffer_size ));

    if( !use_ipp )
        func( src->data.ptr, src->step, dst->data.ptr, dst->step, size, buffer, cn );
    else
        IPPI_CALL( ipp_func( src->data.ptr, src->step ? src->step : CV_STUB_STEP,
            dst->data.ptr, dst->step ? dst->step : CV_STUB_STEP, size, buffer ));
    __END__;

    if( buffer && !local_alloc )
        cvFree( &buffer );
}


CV_IMPL void
cvPyrDown( const void* srcarr, void* dstarr, int _filter )
{
    static CvFuncTable pyrdown_tab;
    static CvFuncTable pyrdownborder_tab;
    static int inittab = 0;
    
    void *buffer = 0;
    int local_alloc = 0;

    CV_FUNCNAME( "cvPyrDown" );

    __BEGIN__;

    int coi1 = 0, coi2 = 0;
    int buffer_size = 0;
    int type, depth, cn;
    CvMat srcstub, *src = (CvMat*)srcarr;
    CvMat dststub, *dst = (CvMat*)dstarr;
    CvFilter filter = (CvFilter) _filter;
    CvPyramidFunc func;
    CvPyramidIPPFunc ipp_func = 0;
    int use_ipp = 0;
    CvSize src_size, src_size2, dst_size;

    if( !inittab )
    {
        icvInitPyrDownG5x5Table( &pyrdown_tab );
        icvInitPyrDownBorderTable( &pyrdownborder_tab );
        inittab = 1;
    }

    CV_CALL( src = cvGetMat( src, &srcstub, &coi1 ));
    CV_CALL( dst = cvGetMat( dst, &dststub, &coi2 ));

    if( coi1 != 0 || coi2 != 0 )
        CV_ERROR( CV_BadCOI, "" );

    if( filter != CV_GAUSSIAN_5x5 )
        CV_ERROR( CV_StsBadArg, "this filter type not supported" );

    if( !CV_ARE_TYPES_EQ( src, dst ))
        CV_ERROR( CV_StsUnmatchedFormats, "" );

    src_size = cvGetMatSize(src);
    dst_size = cvGetMatSize(dst);
    src_size2.width = src_size.width & -2;
    src_size2.height = src_size.height & -2;

    if( (unsigned)(dst_size.width - src_size.width/2) > 1 ||
        (unsigned)(dst_size.height - src_size.height/2) > 1 )
        CV_ERROR( CV_StsUnmatchedSizes, "" );

    // current restriction of PyrDownBorder* 
    if( src_size.width <= 2 && dst_size.width != 1 ||
        src_size.height <= 2 && dst_size.height != 1 )
        CV_ERROR( CV_StsUnmatchedSizes, "" );

    /*if( src->data.ptr == dst->data.ptr )
        CV_ERROR( CV_StsInplaceNotSupported, "" );*/

    type = CV_MAT_TYPE(src->type);
    depth = CV_MAT_DEPTH(type);
    cn = CV_MAT_CN(type);

    if( cn != 1 && cn != 3 )
        CV_ERROR( CV_StsUnsupportedFormat, "The images must have 1 or 3 channel" );

    func = (CvPyramidFunc)pyrdown_tab.fn_2d[depth];

    if( !func )
        CV_ERROR( CV_StsUnsupportedFormat, "" );

    if( icvPyrDownGetBufSize_Gauss5x5_p )
    {
        ipp_func = type == CV_8UC1 ? icvPyrDown_Gauss5x5_8u_C1R_p :
                   type == CV_8UC3 ? icvPyrDown_Gauss5x5_8u_C3R_p :
                   type == CV_32FC1 ? icvPyrDown_Gauss5x5_32f_C1R_p :
                   type == CV_32FC3 ? icvPyrDown_Gauss5x5_32f_C3R_p : 0;

        use_ipp = ipp_func && icvPyrDownGetBufSize_Gauss5x5_p( src_size2.width,
                    icvDepthToDataType(type), cn, &buffer_size ) >= 0;
    }

    if( !use_ipp )
        icvPyrDownG5x5_GetBufSize( src_size2.width,
            icvDepthToDataType(type), cn, &buffer_size );

	/*
    if( buffer_size <= CV_MAX_LOCAL_SIZE )
    {
        buffer = cvStackAlloc( buffer_size );
        local_alloc = 1;
    }
    else*/
        CV_CALL( buffer = cvAlloc( buffer_size ));

    if( !use_ipp )
        func( src->data.ptr, src->step, dst->data.ptr,
              dst->step, src_size2, buffer, cn );
    else
        IPPI_CALL( ipp_func( src->data.ptr, src->step ? src->step : CV_STUB_STEP,
            dst->data.ptr, dst->step ? dst->step : CV_STUB_STEP, src_size2, buffer ));

    if( src_size.width != dst_size.width*2 || src_size.height != dst_size.height*2 )
    {
        CvPyrDownBorderFunc border_func = (CvPyrDownBorderFunc)
                            pyrdownborder_tab.fn_2d[CV_MAT_DEPTH(type)];

        if( !border_func )
            CV_ERROR( CV_StsUnsupportedFormat, "" );

        IPPI_CALL( border_func( src->data.ptr, src->step, src_size,
                                dst->data.ptr, dst->step, dst_size, CV_MAT_CN(type) ));
    }

    __END__;

    if( buffer && !local_alloc )
        cvFree( &buffer );
}


/* MSVC .NET 2003 spends a long time building this, thus, as the code
   is not performance-critical, we turn off the optimization here */
#if defined _MSC_VER && _MSC_VER > 1300 && !defined CV_ICC
#pragma optimize("", off)
#endif

ICV_DEF_PYR_BORDER_FUNC( 8u, uchar, int, PD_SCALE_INT )
ICV_DEF_PYR_BORDER_FUNC( 16u, ushort, int, PD_SCALE_INT )
ICV_DEF_PYR_BORDER_FUNC( 16s, short, int, PD_SCALE_INT )
ICV_DEF_PYR_BORDER_FUNC( 32f, float, float, PD_SCALE_FLT )
ICV_DEF_PYR_BORDER_FUNC( 64f, double, double, PD_SCALE_FLT )

#define ICV_DEF_INIT_PYR_BORDER_TABLE( FUNCNAME )                   \
static void icvInit##FUNCNAME##Table( CvFuncTable* tab )            \
{                                                                   \
    tab->fn_2d[CV_8U] = (void*)icv##FUNCNAME##_8u_CnR;              \
    tab->fn_2d[CV_8S] = 0;                                          \
    tab->fn_2d[CV_16U] = (void*)icv##FUNCNAME##_16u_CnR;            \
    tab->fn_2d[CV_16S] = (void*)icv##FUNCNAME##_16s_CnR;            \
    tab->fn_2d[CV_32F] = (void*)icv##FUNCNAME##_32f_CnR;            \
    tab->fn_2d[CV_64F] = (void*)icv##FUNCNAME##_64f_CnR;            \
}

ICV_DEF_INIT_PYR_BORDER_TABLE( PyrDownBorder )

/* End of file. */
