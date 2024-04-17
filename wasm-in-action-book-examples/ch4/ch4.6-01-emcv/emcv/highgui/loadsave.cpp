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
//    * Shushan Chai (Shenzhen Institute of Advanced Technology, Chinese Academy of Sciences)

#include "highgui.h"
#include <stdio.h>

typedef struct
{
	char type1;
	char type2;
}BmpFileHead;
typedef struct
{
    unsigned int imageSize;
    unsigned int blank;
    unsigned int startPosition;
    unsigned int    length;
    unsigned int    width;
    unsigned int    height;
    unsigned short    colorPlane;
    unsigned short    bitColor;
    unsigned int    zipFormat;
    unsigned int    realSize;
    unsigned int    xPels;
    unsigned int    yPels;
    unsigned int    colorUse;
    unsigned int    colorImportant;
}BmpInfoHead;


typedef struct{
	unsigned char G;
	unsigned char B;
	unsigned char R;
	unsigned char A;
}RGBA;

CV_IMPL IplImage*
cvLoadImage( const char* filename, int flags)
{
	int channels;
	FILE * p;
	BmpFileHead bmpfilehead;
	BmpInfoHead bmpinfohead;
	int step, modbytes, idx;
    IplImage* image = 0;

    CV_FUNCNAME( "cvLoadImage" );

    __BEGIN__;

    if( !filename || strlen(filename) == 0 )
        CV_ERROR( CV_StsNullPtr, "null filename" );

 	p = fopen(filename, "rb");
 	if(!p)
 		CV_ERROR( CV_StsNullPtr, "Can not open file" );
	
	//fread(&bmpfilehead, sizeof(BmpFileHead), 1, p);
	fread(&bmpfilehead, 2, 1, p);//in some system, sizeof(BmpFileHead)=4
	fread(&bmpinfohead, sizeof(BmpInfoHead), 1, p);
	
	if( (bmpfilehead.type1 != 'B' && bmpfilehead.type1 != 'b') ||
		(bmpfilehead.type2 != 'M' && bmpfilehead.type2 != 'm') )
		CV_ERROR( CV_StsUnsupportedFormat, "The file is not a BMP file" );
		
	channels = bmpinfohead.bitColor / 8;
	if( channels != 1 && channels != 3)
		CV_ERROR( CV_StsUnsupportedFormat, "Only 1 or 3 channels images can be loaded." );
	
	if(bmpinfohead.width <= 0 || bmpinfohead.height <=0)
		CV_ERROR( CV_StsOutOfRange, "Bad image size");

 	modbytes = (bmpinfohead.width*channels)%4;
 	step = modbytes ? ((bmpinfohead.width*channels)-modbytes+4)  : (bmpinfohead.width*channels);

	image = cvCreateImage(cvSize(bmpinfohead.width, bmpinfohead.height), 8, channels);
	fseek(p, bmpinfohead.startPosition, SEEK_SET);
	for(idx=bmpinfohead.height-1; idx >= 0 ; idx--)
	{
		int n = fread(image->imageData+image->widthStep*idx, step, 1, p);
        assert(n==1);
	}

	fclose(p);
	__END__;
	return image;
}


CV_IMPL int
cvSaveImage( const char* filename, const IplImage * image )
{
	int channels;
	FILE * p;
	BmpFileHead bmpfilehead;
	BmpInfoHead bmpinfohead;
	RGBA rgba;
	int step, modbytes, idx;

	CV_FUNCNAME( "cvSaveImage" );
    __BEGIN__;

    channels = image->nChannels;
    if( channels != 3 && channels != 1)
        CV_ERROR( CV_BadNumChannels, "Only 1 or 3-channel image is supported" );

    if( !filename || strlen(filename) == 0 )
        CV_ERROR( CV_StsNullPtr, "null filename" );

 	p = fopen(filename, "wb");
 	if(!p)
 		CV_ERROR( CV_StsNullPtr, "Can not create file" );
 	
 	modbytes = (image->width*channels)%4;
 	step = modbytes ? ((image->width*channels)-modbytes+4)  : (image->width*channels);
 	
 	//bmpfilehead = (BmpFileHead *)cvAlloc(sizeof(BmpFileHead));
 	//bmpinfohead = (BmpInfoHead *)cvAlloc(sizeof(BmpInfoHead));
 	
 	memset(&bmpfilehead, 0, sizeof(BmpFileHead));
 	memset(&bmpinfohead, 0, sizeof(BmpInfoHead));
 	memset(&rgba, 0, sizeof(RGBA));
 	
 	bmpfilehead.type1='B';
 	bmpfilehead.type2='M';
 	
 	bmpinfohead.imageSize = ( 2+sizeof(BmpInfoHead)+image->height*step );
 	bmpinfohead.startPosition = (channels==3) ? (2+sizeof(BmpInfoHead)) : (2+sizeof(BmpInfoHead)+1024);
 	bmpinfohead.length = 40;
 	bmpinfohead.width = (image->width);
 	bmpinfohead.height = (image->height);
 	bmpinfohead.colorPlane = (1);
 	bmpinfohead.bitColor = (channels==3) ?(24) : 8;
 	bmpinfohead.realSize = (image->height*step);

	//write header
	fwrite(&bmpfilehead, sizeof(BmpFileHead), 1, p);
	fwrite(&bmpinfohead, sizeof(BmpInfoHead), 1, p);

	//write 
	if(channels==1)
	{
		for(idx =0; idx<256; idx++)
		{
			rgba.R=idx;
			rgba.G=idx;
			rgba.B=idx;
			rgba.A = 0;
			fwrite(&rgba, sizeof(RGBA), 1, p);
		}
	}
	
	for(idx = image->height-1; idx >=0 ; idx--)
 		fwrite(image->imageData+image->widthStep*idx, step, 1, p);
 	
 	fclose(p);
    __END__;
    
    return 0;
}

#ifdef DM64X_NETWORK
#include <netmain.h>

// save as Embedded file
CV_IMPL int
efs_cvSaveImage( const char* filename, const IplImage * image )
{
    int channels;
    EFS_FILE * p;
    BmpFileHead bmpfilehead;
    BmpInfoHead bmpinfohead;
    RGBA rgba;
    int step, modbytes, idx;
   
    int imageLength;
    char *pData;

    CV_FUNCNAME( "cvSaveImageEfs" );
    __BEGIN__;

    channels = image->nChannels;
    if( channels != 3 && channels != 1)
        CV_ERROR( CV_BadNumChannels, "Only 1 or 3-channel image is supported" );

    if( !filename || strlen(filename) == 0 ||
       strlen(filename) >= EFS_FILENAME_MAX)
    {
        CV_ERROR( CV_StsNullPtr, "null filename" );
    }
       
    //
   
    modbytes = (image->width*channels)%4;
    step = modbytes ? ((image->width*channels)-modbytes+4)  : (image->width*channels);
   
    // image size
   
    imageLength = 0;
    {
       // header
       
       imageLength += sizeof(BmpFileHead);
       imageLength += sizeof(BmpInfoHead);
   
       // color table

        if(channels==1)
        {
            imageLength += sizeof(RGBA)*256;
        }

        // data
      
        imageLength += step*image->height;
    }
   
    // alloc memory
   
    pData = (char*)cvAlloc(imageLength);
   
    // create embedded file
   
    efs_createfilecb((char*)filename, imageLength, (UINT8*)pData,
                             (EFSFUN)cvFree_, (UINT32)pData );
       
    // open file

    p = efs_fopen((char*)filename, "wb");
    if(!p)
        CV_ERROR( CV_StsNullPtr, "Can not create file" );
   
    //bmpfilehead = (BmpFileHead *)cvAlloc(sizeof(BmpFileHead));
    //bmpinfohead = (BmpInfoHead *)cvAlloc(sizeof(BmpInfoHead));
   
    memset(&bmpfilehead, 0, sizeof(BmpFileHead));
    memset(&bmpinfohead, 0, sizeof(BmpInfoHead));
    memset(&rgba, 0, sizeof(RGBA));
   
    bmpfilehead.type1='B';
    bmpfilehead.type2='M';
   
    bmpinfohead.imageSize = ( 2+sizeof(BmpInfoHead)+image->height*step );
    bmpinfohead.startPosition = (channels==3) ? (2+sizeof(BmpInfoHead)) : (2+sizeof(BmpInfoHead)+1024);
    bmpinfohead.length = 40;
    bmpinfohead.width = (image->width);
    bmpinfohead.height = (image->height);
    bmpinfohead.colorPlane = (1);
    bmpinfohead.bitColor = (channels==3) ?(24) : 8;
    bmpinfohead.realSize = (image->height*step);

    //write header
    efs_fwrite(&bmpfilehead, sizeof(BmpFileHead), 1, p);
    efs_fwrite(&bmpinfohead, sizeof(BmpInfoHead), 1, p);

    //write
    if(channels==1)
    {
        for(idx =0; idx<256; idx++)
        {
            rgba.R=idx;
            rgba.G=idx;
            rgba.B=idx;
            rgba.A = 0;
            efs_fwrite(&rgba, sizeof(RGBA), 1, p);
        }
    }
   
    for(idx = image->height-1; idx >=0 ; idx--)
        efs_fwrite(image->imageData+image->widthStep*idx, step, 1, p);
   
    efs_fclose(p);
    __END__;
   
    return 0;
}

#endif   // DM64X_NETWORK


