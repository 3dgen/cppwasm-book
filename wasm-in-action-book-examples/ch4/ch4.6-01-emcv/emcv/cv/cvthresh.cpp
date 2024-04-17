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

CV_IMPL void
cvThreshold( const void* srcarr, void* dstarr, double thresh, double maxval, int type )
{
    CV_FUNCNAME( "cvThreshold" );

    __BEGIN__;

    CvMat src_stub, *src = (CvMat*)srcarr;
    CvMat dst_stub, *dst = (CvMat*)dstarr;
    int coi1 = 0, coi2 = 0;

    CV_CALL( src = cvGetMat( src, &src_stub, &coi1 ));
    CV_CALL( dst = cvGetMat( dst, &dst_stub, &coi2 ));

    if( coi1 + coi2 )
        CV_ERROR( CV_BadCOI, "COI is not supported by the function" );

    if( !CV_ARE_CNS_EQ( src, dst ) )
        CV_ERROR( CV_StsUnmatchedFormats, "Both arrays must have equal number of channels" );

	if( type != CV_THRESH_BINARY)
		CV_ERROR( CV_StsBadFlag, "Only binary threshold is supported now. Please use type=CV_THRESH_BINARY." );
	
    if( CV_MAT_TYPE(src->type) != CV_8UC1 || CV_MAT_TYPE(dst->type) != CV_8UC1)
        CV_ERROR( CV_StsUnsupportedFormat, "Types should be 8uC1" );

    switch( CV_MAT_DEPTH(src->type) )
    {
    case CV_8U:
        register unsigned char ithreshs[4];
        register unsigned char imaxvals[4];
	    int ithresh, imaxval;
	    int idx;
        
        ithresh = cvFloor(thresh);
        ithresh = (ithresh>255) ? 255 : ithresh;
        ithresh = (ithresh<0) ? 0 : ithresh;
        ithreshs[0]=ithreshs[1]=ithreshs[2]=ithreshs[3]=ithresh;
        
        imaxval = cvRound(maxval);
        imaxval = (imaxval>255) ? 255 : imaxval;
		imaxval = (imaxval<0) ? 0 : imaxval;
		imaxvals[0]=imaxvals[1]=imaxvals[2]=imaxvals[3]=imaxval;
		
#ifdef _TMS320C6X
        register unsigned int binvals;
		for(idx=0; idx < (src->step*src->height)/4; idx++)
		{
			binvals = _cmpgtu4( _mem4_const(src->data.i+idx), _mem4_const(ithreshs) );
			binvals = _xpnd4( binvals  );
			_mem4(dst->data.i+idx) = binvals & _mem4_const(imaxvals);
		}
#else
		for(idx=0; idx < (src->step*src->height); idx++)
		{
			dst->data.ptr[idx] = (src->data.ptr[idx]>ithresh)? imaxval: 0;
		}

#endif
        break;
    default:
        CV_ERROR( CV_BadDepth, cvUnsupportedFormat );
    }

    __END__;

}

/* End of file. */
