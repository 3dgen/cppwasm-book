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


#include "_cv.h"



CvStatus cvIntegralImage_8u32s64s_C1R( const uchar* src, int srcstep,
                                 int* sum, int sumstep,     
                                 int64* sqsum, int sqsumstep,
                                 int* tilted, int tiltedstep,
                                 CvSize size )                  
{                                                               
    int x, y;                                                   
    int s;                                                  
    int64 sq;                                               
    int* buf = 0;                                           
                                                                
    srcstep /= sizeof(src[0]);                                  
                                                                
    memset( sum, 0, (size.width+1)*sizeof(sum[0]));             
    sumstep /= sizeof(sum[0]);                                  
    sum += sumstep + 1;                                         
                                                                
    if( sqsum )                                                 
    {                                                           
        memset( sqsum, 0, (size.width+1)*sizeof(sqsum[0]));     
        sqsumstep /= sizeof(sqsum[0]);                          
        sqsum += sqsumstep + 1;                                 
    }                                                           
                                                                
    if( tilted )                                                
    {                                                           
        memset( tilted, 0, (size.width+1)*sizeof(tilted[0]));   
        tiltedstep /= sizeof(tilted[0]);                        
        tilted += tiltedstep + 1;                               
    }                                                           
                                                                
    if( sqsum == 0 && tilted == 0 )                             
    {                                                           
        for( y = 0; y < size.height; y++, src += srcstep,       
                                          sum += sumstep )      
        {                                                       
            sum[-1] = 0;                                        
            for( x = 0, s = 0; x < size.width; x++ )            
            {                                                   
                int t = (src[x]);                 
                s += t;                                         
                sum[x] = sum[x - sumstep] + s;                  
            }                                                   
        }                                                       
    }                                                           
    else if( tilted == 0 )                                      
    {                                                           
        for( y = 0; y < size.height; y++, src += srcstep,       
                        sum += sumstep, sqsum += sqsumstep )    
        {                                                       
            sum[-1] = 0;                                        
            sqsum[-1] = 0;                                      
                                                                
            for( x = 0, s = 0, sq = 0; x < size.width; x++ )    
            {                                                   
                int it = src[x];                           
                int t = (it);                     
                int64 tq =  CV_8TO16U_SQR(it);              
                s += t;                                         
                sq += tq;                                       
                t = sum[x - sumstep] + s;                       
                tq = sqsum[x - sqsumstep] + sq;                 
                sum[x] = t;                                     
                sqsum[x] = tq;                                  
            }                                                   
        }                                                       
    }                                                           
    else                                                        
    {                                                           
        if( sqsum == 0 )                                        
        {                                                       
            assert(0);                                          
            return CV_NULLPTR_ERR;                              
        }                                                       
                                                                
        buf = (int*)cvAlloc((size.width + 1 )* sizeof(buf[0]));
        sum[-1] = tilted[-1] = 0;                               
        sqsum[-1] = 0;                                          
                                                                
        for( x = 0, s = 0, sq = 0; x < size.width; x++ )        
        {                                                       
            int it = src[x];                               
            int t = (it);                         
            int64 tq =  CV_8TO16U_SQR(it);                  
            buf[x] = tilted[x] = t;                             
            s += t;                                             
            sq += tq;                                           
            sum[x] = s;                                         
            sqsum[x] = sq;                                      
        }                                                       
                                                                
        if( size.width == 1 )                                   
            buf[1] = 0;                                         
                                                                
        for( y = 1; y < size.height; y++ )                      
        {                                                       
            int it;                                        
            int t0;                                         
            int64 tq0;                                      
                                                                
            src += srcstep;                                     
            sum += sumstep;                                     
            sqsum += sqsumstep;                                 
            tilted += tiltedstep;                               
                                                                
            it = src[0/*x*/];                                   
            s = t0 = (it);                            
            sq = tq0 =  CV_8TO16U_SQR(it);                      
                                                                
            sum[-1] = 0;                                        
            sqsum[-1] = 0;                                      
            /*tilted[-1] = buf[0];*/                            
            tilted[-1] = tilted[-tiltedstep];                   
                                                                
            sum[0] = sum[-sumstep] + t0;                        
            sqsum[0] = sqsum[-sqsumstep] + tq0;                 
            tilted[0] = tilted[-tiltedstep] + t0 + buf[1];      
                                                                
            for( x = 1; x < size.width - 1; x++ )               
            {                                                   
                int t1 = buf[x];                            
                buf[x-1] = t1 + t0;                             
                it = src[x];                                    
                t0 = (it);                            
                tq0 =  CV_8TO16U_SQR(it);                       
                s += t0;                                        
                sq += tq0;                                      
                sum[x] = sum[x - sumstep] + s;                  
                sqsum[x] = sqsum[x - sqsumstep] + sq;           
                t1 += buf[x+1] + t0 + tilted[x - tiltedstep - 1];
                tilted[x] = t1;                                 
            }                                                   
                                                                
            if( size.width > 1 )                                
            {                                                   
                int t1 = buf[x];                            
                buf[x-1] = t1 + t0;                             
                it = src[x];    /*+*/                           
                t0 = (it);                            
                tq0 =  CV_8TO16U_SQR(it);                       
                s += t0;                                        
                sq += tq0;                                      
                sum[x] = sum[x - sumstep] + s;                  
                sqsum[x] = sqsum[x - sqsumstep] + sq;           
                tilted[x] = t0 + t1 + tilted[x - tiltedstep - 1];
                buf[x] = t0;                                    
            }                                                   
        }
        cvFree(&buf);
    }                                                           
                                                                
    return CV_OK;                                               
}




CV_IMPL void
cvIntegral( const CvArr* image, CvArr* sumImage,
            CvArr* sumSqImage, CvArr* tiltedSumImage )
{
    
    CV_FUNCNAME( "cvIntegralImage" );

    __BEGIN__;

    CvMat src_stub, *src = (CvMat*)image;
    CvMat sum_stub, *sum = (CvMat*)sumImage;
    CvMat sqsum_stub, *sqsum = (CvMat*)sumSqImage;
    CvMat tilted_stub, *tilted = (CvMat*)tiltedSumImage;
    int coi0 = 0, coi1 = 0, coi2 = 0, coi3 = 0;
    //int depth;
    int cn;
    int src_step, sum_step, sqsum_step, tilted_step;
    CvSize size;


    CV_CALL( src = cvGetMat( src, &src_stub, &coi0 ));
    CV_CALL( sum = cvGetMat( sum, &sum_stub, &coi1 ));
    
    if( sum->width != src->width + 1 ||
        sum->height != src->height + 1 )
        CV_ERROR( CV_StsUnmatchedSizes, "" );

	if(CV_MAT_DEPTH(src->type)!=CV_8U || CV_MAT_CN(src->type)!=1)
		CV_ERROR( CV_StsUnsupportedFormat, "the source array must be 8UC1");

    if( CV_MAT_DEPTH( sum->type ) != CV_32S ||
        !CV_ARE_CNS_EQ( src, sum ))
        CV_ERROR( CV_StsUnsupportedFormat,
        "Sum array must have 32s type in case of 8u source array"
        "and the same number of channels as the source array" );

    if( sqsum )
    {
        CV_CALL( sqsum = cvGetMat( sqsum, &sqsum_stub, &coi2 ));
        if( !CV_ARE_SIZES_EQ( sum, sqsum ) )
            CV_ERROR( CV_StsUnmatchedSizes, "" );
        if( CV_MAT_DEPTH( sqsum->type ) != CV_64S || !CV_ARE_CNS_EQ( src, sqsum ))
            CV_ERROR( CV_StsUnsupportedFormat,
                      "Squares sum array must be 64s "
                      "and the same number of channels as the source array" );
    }

    if( tilted )
    {
        if( !sqsum )
            CV_ERROR( CV_StsNullPtr,
            "Squared sum array must be passed if tilted sum array is passed" );

        CV_CALL( tilted = cvGetMat( tilted, &tilted_stub, &coi3 ));
        if( !CV_ARE_SIZES_EQ( sum, tilted ) )
            CV_ERROR( CV_StsUnmatchedSizes, "" );
        if( !CV_ARE_TYPES_EQ( sum, tilted ) )
            CV_ERROR( CV_StsUnmatchedFormats,
                      "Sum and tilted sum must have the same types" );
        if( CV_MAT_CN(tilted->type) != 1 )
            CV_ERROR( CV_StsNotImplemented,
                      "Tilted sum can not be computed for multi-channel arrays" );
    }

    if( coi0 || coi1 || coi2 || coi3 )
        CV_ERROR( CV_BadCOI, "COI is not supported by the function" );

    //depth = CV_MAT_DEPTH(src->type);
    cn = CV_MAT_CN(src->type);


    size = cvGetMatSize(src);
    src_step = src->step ? src->step : CV_STUB_STEP;
    sum_step = sum->step ? sum->step : CV_STUB_STEP;
    sqsum_step = !sqsum ? 0 : sqsum->step ? sqsum->step : CV_STUB_STEP;
    tilted_step = !tilted ? 0 : tilted->step ? tilted->step : CV_STUB_STEP;

    if( cn == 1 )
    {
        
        cvIntegralImage_8u32s64s_C1R( src->data.ptr, src_step, (int*)(sum->data.ptr), sum_step,
                        sqsum ? (int64*)(sqsum->data.ptr) : 0, sqsum_step,
                        tilted ? (int*)(tilted->data.ptr) : 0, tilted_step, size );
    }

    __END__;
}


/* End of file. */
