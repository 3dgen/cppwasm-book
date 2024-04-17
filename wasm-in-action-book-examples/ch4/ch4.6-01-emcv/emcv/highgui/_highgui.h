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

#ifndef __HIGHGUI_H_
#define __HIGHGUI_H_

#ifndef _TMS320C6X

#include "highgui.h"
#include "../cxcore/cxmisc.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#include <assert.h>

//#ifndef WIN32
//#include "cvconfig.h"
//#else
//void  FillBitmapInfo( BITMAPINFO* bmi, int width, int height, int bpp, int origin );
//#endif

#define HAVE_CAMV4L
#define HAVE_CAMV4L2 

/* Errors */
#define HG_OK          0 /* Don't bet on it! */
#define HG_BADNAME    -1 /* Bad window or file name */
#define HG_INITFAILED -2 /* Can't initialize HigHGUI. Possibly, can't find vlgrfmts.dll */
#define HG_WCFAILED   -3 /* Can't create a window */
#define HG_NULLPTR    -4 /* The null pointer where it should not appear */
#define HG_BADPARAM   -5

#define CV_WINDOW_MAGIC_VAL     0x00420042
#define CV_TRACKBAR_MAGIC_VAL   0x00420043

/***************************** CvCapture structure ******************************/

struct CvCapture
{
    virtual ~CvCapture() {}
    virtual double getProperty(int) { return 0; }
    virtual bool setProperty(int, double) { return 0; }
    virtual bool grabFrame() { return true; }
    virtual IplImage* retrieveFrame(int) { return 0; }
    virtual IplImage* queryFrame() { return grabFrame() ? retrieveFrame(0) : 0; }
};

/*************************** CvVideoWriter structure ****************************/

struct CvVideoWriter
{
    virtual ~CvVideoWriter() {}
    virtual bool writeFrame(const IplImage*) { return false; }
};

#ifdef WIN32
#define HAVE_VFW 1

#if _MSC_VER >= 1400 && !defined WIN64
#define HAVE_VIDEOINPUT 1
#endif

/* uncomment to enable OpenEXR codec (will not compile under MSVC6) */
//#define HAVE_ILMIMF 1

/* uncomment to enable CMUCamera1394 fireware camera module */
//#define HAVE_CMU1394 1
#endif


#if defined (HAVE_CAMV4L) || defined (HAVE_CAMV4L2)
CvCapture * cvCreateCameraCapture_V4L( int index );
#endif

#ifdef HAVE_DC1394
CvCapture * cvCreateCameraCapture_DC1394( int index );
#endif

#ifdef HAVE_DC1394_2
CvCapture * cvCreateCameraCapture_DC1394_2( int index );
#endif

#ifdef HAVE_MIL
CvCapture* cvCreateCameraCapture_MIL( int index );
#endif

#ifdef HAVE_CMU1394
CvCapture * cvCreateCameraCapture_CMU( int index );
#endif

#ifdef HAVE_TYZX
CV_IMPL CvCapture * cvCreateCameraCapture_TYZX( int index );
#endif

#ifdef WIN32
CvCapture* cvCreateFileCapture_Win32( const char* filename );

CvCapture* cvCreateCameraCapture_VFW( int index );
CvCapture* cvCreateFileCapture_VFW( const char* filename );

CvVideoWriter* cvCreateVideoWriter_Win32( const char* filename, int fourcc,
                                          double fps, CvSize frameSize, int is_color );
CvVideoWriter* cvCreateVideoWriter_VFW( const char* filename, int fourcc,
                                        double fps, CvSize frameSize, int is_color );

#ifdef HAVE_VIDEOINPUT
CvCapture* cvCreateCameraCapture_DShow( int index );
#endif

#endif

CVAPI(int) cvHaveImageReader(const char* filename);
CVAPI(int) cvHaveImageWriter(const char* filename);

CvCapture* cvCreateFileCapture_Images(const char* filename);
CvVideoWriter* cvCreateVideoWriter_Images(const char* filename);

#ifdef HAVE_XINE
CvCapture* cvCreateFileCapture_XINE (const char* filename);
#endif

#ifdef HAVE_GSTREAMER
#define CV_CAP_GSTREAMER_1394		0
#define CV_CAP_GSTREAMER_V4L		1
#define CV_CAP_GSTREAMER_V4L2		2
#define CV_CAP_GSTREAMER_FILE		3

CvCapture * cvCreateCapture_GStreamer(int type, const char *filename);
#endif

#ifdef HAVE_FFMPEG
CvCapture* cvCreateFileCapture_FFMPEG (const char* filename);

CvVideoWriter* cvCreateVideoWriter_FFMPEG ( const char* filename, int fourcc,
                                            double fps, CvSize frameSize, int is_color );
#endif

#ifdef HAVE_QUICKTIME
CvCapture * cvCreateFileCapture_QT (const char  * filename);
CvCapture * cvCreateCameraCapture_QT  (const int     index);

CvVideoWriter* cvCreateVideoWriter_QT ( const char* filename, int fourcc,
                                        double fps, CvSize frameSize, int is_color );
#endif

#ifdef HAVE_UNICAP
CvCapture * cvCreateCameraCapture_Unicap  (const int     index);

#endif

#endif	// _TMS320C6X

#endif /* __HIGHGUI_H_ */
