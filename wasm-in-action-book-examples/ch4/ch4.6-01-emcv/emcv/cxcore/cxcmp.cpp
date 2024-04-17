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

#include "cxcore.h"

CV_IMPL  void
cvAbsDiff( const void* srcarr1, const void* srcarr2, void* dstarr )
{

    CV_FUNCNAME( "cvAbsDiff" );

    __BEGIN__;

    int coi1 = 0, coi2 = 0, coi3 = 0;
    CvMat srcstub1, *src1 = (CvMat*)srcarr1;
    CvMat srcstub2, *src2 = (CvMat*)srcarr2;
    CvMat dststub,  *dst = (CvMat*)dstarr;
    CvSize size;
    int type, depth, pixel_size;

    CV_CALL( src1 = cvGetMat( src1, &srcstub1, &coi1 ));
    CV_CALL( src2 = cvGetMat( src2, &srcstub2, &coi2 ));
    CV_CALL( dst = cvGetMat( dst, &dststub, &coi3 ));

    if( coi1 != 0 || coi2 != 0 || coi3 != 0 )
        CV_ERROR( CV_BadCOI, "" );

    if( !CV_ARE_SIZES_EQ( src1, src2 ) )
        CV_ERROR_FROM_CODE( CV_StsUnmatchedSizes );

    type = CV_MAT_TYPE(src1->type);
    depth = CV_MAT_DEPTH(type);

    if( !CV_ARE_SIZES_EQ( src1, dst ))
        CV_ERROR_FROM_CODE( CV_StsUnmatchedSizes );

    if( !CV_ARE_TYPES_EQ( src1, src2 ))
        CV_ERROR_FROM_CODE( CV_StsUnmatchedFormats );

    if( !CV_ARE_TYPES_EQ( src1, dst ))
        CV_ERROR_FROM_CODE( CV_StsUnmatchedFormats );

    size.width = src1->step * src1->height;
    size.height = 1;
	pixel_size = CV_DEPTH_BYTES[depth];

	if(depth == CV_8U)
	{
		int idx;
		unsigned char * p1;
		unsigned char * p2;
		unsigned char * pdst; 
		p1 = src1->data.ptr ; 
		p2 = src2->data.ptr; 
		pdst = dst->data.ptr; 
		
#ifdef _TMS320C6X
       	for (idx = 0; idx < size.width/pixel_size; idx+=4)
       	{
       		_amem4(pdst) = _subabs4(_amem4_const(p1), _amem4_const(p2) );
       		p1 += 4;
       		p2 += 4;
       		pdst += 4;
       	}	
#else
       	for (idx = 0; idx < size.width/pixel_size; idx+=1)
       	{
			(*pdst) = abs((*p1)-(*p2));
			pdst++;
			p1++;
			p2++;
       	}	
#endif
    }
	else if(depth == CV_32S)        
	{
		int idx;
		int * p1;
		int * p2;
		int * pdst; 
		p1 = src1->data.i; 
		p2 = src2->data.i; 
		pdst = dst->data.i;

       	for (idx = 0; idx < size.width/pixel_size; idx++)
       	{
#ifdef _TMS320C6X
       		*pdst = _abs(_ssub(*p1, *p2));
#else
       		*pdst = abs((*p1)-(*p2));
#endif
       		p1 += 1;
       		p2 += 1;
       		pdst += 1;
       	}	
    }
    else
    {
		CV_ERROR( CV_StsUnsupportedFormat, "unsupported matrix type." );
	}

    __END__;
}

