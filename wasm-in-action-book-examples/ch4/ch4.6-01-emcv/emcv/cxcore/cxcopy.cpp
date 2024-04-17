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

/* ////////////////////////////////////////////////////////////////////
//
//  CvMat basic operations: cvCopy, cvSet
//
// */

#include "_cxcore.h"

/////////////////////////////////////////////////////////////////////////////////////////
//                                                                                     //
//                                  L/L COPY & SET FUNCTIONS                           //
//                                                                                     //
/////////////////////////////////////////////////////////////////////////////////////////
/*
IPCVAPI_IMPL( CvStatus, icvCopy_8u_C1R, ( const uchar* src, int srcstep,
                                          uchar* dst, int dststep, CvSize size ),
                                          (src, srcstep, dst, dststep, size) )
{
    for( ; size.height--; src += srcstep, dst += dststep )
        memcpy( dst, src, size.width );

    return  CV_OK;
}
*/
CV_IMPL CvStatus icvCopy_8u_C1R( const uchar* src, int srcstep, uchar* dst, int dststep, CvSize size )
{
    for( ; size.height--; src += srcstep, dst += dststep )
        memcpy( dst, src, size.width );

    return  CV_OK;
}

/* dst = src */
CV_IMPL void
cvCopy( const void* srcarr, void* dstarr, const void* maskarr )
{
    CV_FUNCNAME( "cvCopy" );
    
    __BEGIN__;

    int pix_size;
    CvMat srcstub, *src = (CvMat*)srcarr;
    CvMat dststub, *dst = (CvMat*)dstarr;
    CV_CALL( src = cvGetMat( srcarr, &srcstub ));
    CV_CALL( dst = cvGetMat( dstarr, &dststub ));    
    
    CvSize size;

    if( !CV_IS_MAT(src) || !CV_IS_MAT(dst) )
        CV_ERROR_FROM_CODE( CV_StsUnmatchedFormats );

    if( !CV_ARE_TYPES_EQ( src, dst ))
        CV_ERROR_FROM_CODE( CV_StsUnmatchedFormats );

    if( !CV_ARE_SIZES_EQ( src, dst ))
        CV_ERROR_FROM_CODE( CV_StsUnmatchedSizes );

    size = cvGetMatSize( src );
    pix_size = CV_ELEM_SIZE(src->type);

    if( !maskarr )
    {
        int src_step = src->step, dst_step = dst->step;
        size.width *= pix_size;
        if( CV_IS_MAT_CONT( src->type & dst->type ) && (src_step == dst_step) && (src_step == src->width * pix_size))
        {
            size.width *= size.height;

            if( size.width <= CV_MAX_INLINE_MAT_OP_SIZE*
                              CV_MAX_INLINE_MAT_OP_SIZE*(int)sizeof(double))
            {
                memcpy( dst->data.ptr, src->data.ptr, size.width );
                EXIT;
            }

            size.height = 1;
            src_step = dst_step = CV_STUB_STEP;
        }

        icvCopy_8u_C1R( src->data.ptr, src_step,
                        dst->data.ptr, dst_step, size );
    }
    else
    {
        CV_ERROR( CV_StsUnsupportedFormat, "Mask is not supported currently" );
    }

    __END__;
}

/****************************************************************************************\
*                                          Clearing                                      *
\****************************************************************************************/

CV_IMPL void
cvSetZero( CvArr* arr )
{
    CV_FUNCNAME( "cvSetZero" );
    
    __BEGIN__;
    
    CvMat mat, *pmat;
    int coi = 0;
    
    CV_CALL( pmat = cvGetMat( arr, &mat, &coi ));
    
    memset(pmat->data.ptr, 0, pmat->step*pmat->height);

	__END__;
}
/* End of file. */

