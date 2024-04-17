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
//    * Shushan Chai (chaishushan@gmail.com)


#include "_cxcore.h"

//====================================================================
// icvSumIplImage
// icvSumMat
//====================================================================

// nChannels: 1
// depth:
//   IPL_DEPTH_8U/IPL_DEPTH_8S
//   IPL_DEPTH_16U/IPL_DEPTH_16S
//   IPL_DEPTH_32S

#define		ICV_SUM_C1(img,sum,type)								\
	do{																\
		const IplImage *_img = (const IplImage*)(img);				\
		int row, col, sum_val0 = 0;									\
																	\
		for(row = 0; row < _img->height; ++row)						\
			for(col = 0; col < _img->width; ++col)					\
				sum_val0 += CV_IMAGE_ELEM(_img, type, row, col);	\
																	\
		sum.val[0] = sum_val0;										\
	}while(0)

// nChannels: 1
// depth: IPL_DEPTH_32F/IPL_DEPTH_64F

#define		ICV_SUM_C1F(img,sum,type)								\
	do{																\
		const IplImage *_img = (const IplImage*)(img);				\
		double sum_val0 = 0;										\
																	\
		int row, col;												\
		for(row = 0; row < _img->height; ++row)						\
			for(col = 0; col < _img->width; ++col)					\
				sum_val0 += CV_IMAGE_ELEM(_img, type, row, col);	\
																	\
		sum.val[0] = sum_val0;										\
	}while(0)
	
// nChannels: 3
// depth:
//   IPL_DEPTH_8U/IPL_DEPTH_8S
//   IPL_DEPTH_16U/IPL_DEPTH_16S
//   IPL_DEPTH_32S

#define		ICV_SUM_C3(img,sum,type)								\
	do{																\
		const IplImage *_img = (const IplImage*)(img);				\
		int sum_val0 = 0;											\
		int sum_val1 = 0;											\
		int sum_val2 = 0;											\
																	\
		int row, col;												\
		for(row = 0; row < _img->height; ++row)						\
			for(col = 0; col < _img->width; ++col)					\
			{														\
				sum_val0 += CV_IMAGE_ELEM(_img, type, row, col*3+0);\
				sum_val1 += CV_IMAGE_ELEM(_img, type, row, col*3+1);\
				sum_val2 += CV_IMAGE_ELEM(_img, type, row, col*3+2);\
			}														\
																	\
		sum.val[0] = sum_val0;										\
	}while(0)

// nChannels: 1
// depth: IPL_DEPTH_32F/IPL_DEPTH_64F

#define		ICV_SUM_C3F(img,sum,type)								\
	do{																\
		const IplImage *_img = (const IplImage*)(img);				\
		double sum_val0 = 0;										\
		double sum_val1 = 0;										\
		double sum_val2 = 0;										\
																	\
		int row, col;												\
		for(row = 0; row < _img->height; ++row)						\
			for(col = 0; col < _img->width; ++col)					\
			{														\
				sum_val0 += CV_IMAGE_ELEM(_img, type, row, col*3+0);\
				sum_val1 += CV_IMAGE_ELEM(_img, type, row, col*3+1);\
				sum_val2 += CV_IMAGE_ELEM(_img, type, row, col*3+2);\
			}														\
																	\
		sum.val[0] = sum_val0;										\
	}while(0)

//====================================================================
//====================================================================
	
static
CvScalar icvSumIplImage(const IplImage *img)
{
	assert(CV_IS_IMAGE(img));
	
    CvScalar sum = {{0,0,0,0}};
	
	if(img->nChannels == 1)
	{
		switch(img->depth)
		{
		case IPL_DEPTH_8U : ICV_SUM_C1(img, sum, uchar); break;
		case IPL_DEPTH_8S : ICV_SUM_C1(img, sum, char); break;
		case IPL_DEPTH_16U: ICV_SUM_C1(img, sum, unsigned short); break;
		case IPL_DEPTH_16S: ICV_SUM_C1(img, sum, short); break;
		case IPL_DEPTH_32S: ICV_SUM_C1(img, sum, int); break;
		
		case IPL_DEPTH_32F: ICV_SUM_C1F(img, sum, float); break;
		case IPL_DEPTH_64F: ICV_SUM_C1F(img, sum, double); break;
		
		default: break;
		}
	}
	else if(img->nChannels == 3)
	{
		switch(img->depth)
		{
		case IPL_DEPTH_8U : ICV_SUM_C3(img, sum, uchar); break;
		case IPL_DEPTH_8S : ICV_SUM_C3(img, sum, char); break;
		case IPL_DEPTH_16U: ICV_SUM_C3(img, sum, unsigned short); break;
		case IPL_DEPTH_16S: ICV_SUM_C3(img, sum, short); break;
		case IPL_DEPTH_32S: ICV_SUM_C3(img, sum, int); break;
		
		case IPL_DEPTH_32F: ICV_SUM_C3F(img, sum, float); break;
		case IPL_DEPTH_64F: ICV_SUM_C3F(img, sum, double); break;
		
		default: break;
		}
	}
	return sum;
}

static
CvScalar icvSumMat(const CvMat *mat)
{
	assert(CV_IS_MAT(mat));
    CvScalar sum = {{0,0,0,0}};
	
	// donot supported
	
	return sum;
}

//

CV_IMPL CvScalar
cvSum( const CvArr* arr )
{
    CvScalar sum = {{0,0,0,0}};

    CV_FUNCNAME("cvSum");
	assert(false);

    __BEGIN__;

    //int type, coi = 0;
    //int mat_step;
    //CvSize size;
    //CvMat stub, ;
    CvMat *mat = (CvMat*)arr;
	
	if(CV_IS_IMAGE(arr))
	{
		sum = icvSumIplImage((const IplImage*)arr);
		EXIT;
	}
	else if(CV_IS_MAT(mat))
	{
        if( CV_IS_MATND(mat) )
        {
            CV_ERROR( CV_StsBadArg, "Only mat are supported here" );
            EXIT;
        }
		
		sum = icvSumMat(mat);
		EXIT;
	}

    __END__;

    return  sum;
}

#if 0
CV_IMPL int
cvCountNonZero( const CvArr* arr )
{
    static CvFuncTable nz_tab;
    static CvFuncTable nzcoi_tab;
    static int inittab = 0;

    int count = 0;

    CV_FUNCNAME("cvCountNonZero");

    __BEGIN__;

    int type, coi = 0;
    int mat_step;
    CvSize size;
    CvMat stub, *mat = (CvMat*)arr;

    if( !inittab )
    {
        icvInitCountNonZeroC1RTable( &nz_tab );
        icvInitCountNonZeroCnCRTable( &nzcoi_tab );
        inittab = 1;
    }

    if( !CV_IS_MAT(mat) )
    {
        if( CV_IS_MATND(mat) )
        {
            void* matnd = (void*)arr;
            CvMatND nstub;
            CvNArrayIterator iterator;
            CvFunc2D_1A1P func;

            CV_CALL( cvInitNArrayIterator( 1, &matnd, 0, &nstub, &iterator ));

            type = CV_MAT_TYPE(iterator.hdr[0]->type);

            if( CV_MAT_CN(type) != 1 )
                CV_ERROR( CV_BadNumChannels,
                    "Only single-channel array are supported here" );

            func = (CvFunc2D_1A1P)(nz_tab.fn_2d[CV_MAT_DEPTH(type)]);
            if( !func )
                CV_ERROR( CV_StsUnsupportedFormat, "" );
       
            do
            {
                int temp;
                IPPI_CALL( func( iterator.ptr[0], CV_STUB_STEP,
                                 iterator.size, &temp ));
                count += temp;
            }
            while( cvNextNArraySlice( &iterator ));
            EXIT;
        }
        else
            CV_CALL( mat = cvGetMat( mat, &stub, &coi ));
    }

    type = CV_MAT_TYPE(mat->type);
    size = cvGetMatSize( mat );

    mat_step = mat->step;

    if( CV_IS_MAT_CONT( mat->type ))
    {
        size.width *= size.height;
        size.height = 1;
        mat_step = CV_STUB_STEP;
    }

    if( CV_MAT_CN(type) == 1 || coi == 0 )
    {
        CvFunc2D_1A1P func = (CvFunc2D_1A1P)(nz_tab.fn_2d[CV_MAT_DEPTH(type)]);

        if( CV_MAT_CN(type) != 1 )
            CV_ERROR( CV_BadNumChannels,
            "The function can handle only a single channel at a time (use COI)");

        if( !func )
            CV_ERROR( CV_StsBadArg, cvUnsupportedFormat );

        IPPI_CALL( func( mat->data.ptr, mat_step, size, &count ));
    }
    else
    {
        CvFunc2DnC_1A1P func = (CvFunc2DnC_1A1P)(nzcoi_tab.fn_2d[CV_MAT_DEPTH(type)]);

        if( !func )
            CV_ERROR( CV_StsBadArg, cvUnsupportedFormat );

        IPPI_CALL( func( mat->data.ptr, mat_step, size, CV_MAT_CN(type), coi, &count ));
    }

    __END__;

    return  count;
}
#endif

