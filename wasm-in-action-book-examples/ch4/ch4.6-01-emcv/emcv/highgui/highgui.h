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


#ifndef _HIGH_GUI_
#define _HIGH_GUI_

#include "../cv/cv.h"

#ifndef SKIP_INCLUDES

  #include "../cxcore/cxcore.h"
  #if defined WIN32 || defined WIN64
    #include <windows.h>
  #endif

#else // SKIP_INCLUDES

  #if defined WIN32 || defined WIN64
    #define CV_CDECL __cdecl
    #define CV_STDCALL __stdcall
  #else
    #define CV_CDECL
    #define CV_STDCALL
  #endif

  #ifndef CV_EXTERN_C
    #ifdef __cplusplus
      #define CV_EXTERN_C extern "C"
      #define CV_DEFAULT(val) = val
    #else
      #define CV_EXTERN_C
      #define CV_DEFAULT(val)
    #endif
  #endif

  #ifndef CV_EXTERN_C_FUNCPTR
    #ifdef __cplusplus
      #define CV_EXTERN_C_FUNCPTR(x) extern "C" { typedef x; }
    #else
      #define CV_EXTERN_C_FUNCPTR(x) typedef x
    #endif
  #endif

  #ifndef CV_INLINE
    #if defined __cplusplus
      #define CV_INLINE inline
    #elif (defined WIN32 || defined WIN64) && !defined __GNUC__
      #define CV_INLINE __inline
    #else
      #define CV_INLINE static
    #endif
  #endif /* CV_INLINE */

  #if (defined WIN32 || defined WIN64) && defined CVAPI_EXPORTS
    #define CV_EXPORTS __declspec(dllexport)
  #else
    #define CV_EXPORTS
  #endif

  #ifndef CVAPI
    #define CVAPI(rettype) CV_EXTERN_C CV_EXPORTS rettype CV_CDECL
  #endif

#endif // SKIP_INCLUDES

#ifdef __cplusplus
  extern "C" {
#endif /* __cplusplus */

/* 8bit, color or not */
#define CV_LOAD_IMAGE_UNCHANGED  -1
/* 8bit, gray */
#define CV_LOAD_IMAGE_GRAYSCALE   0
/* ?, color */
#define CV_LOAD_IMAGE_COLOR       1
/* any depth, ? */ 
#define CV_LOAD_IMAGE_ANYDEPTH    2
/* ?, any color */
#define CV_LOAD_IMAGE_ANYCOLOR    4

/* load image from file 
  iscolor can be a combination of above flags where CV_LOAD_IMAGE_UNCHANGED
  overrides the other flags
  using CV_LOAD_IMAGE_ANYCOLOR alone is equivalent to CV_LOAD_IMAGE_UNCHANGED
  unless CV_LOAD_IMAGE_ANYDEPTH is specified images are converted to 8bit
*/
CVAPI(IplImage*) cvLoadImage( const char* filename, int iscolor CV_DEFAULT(CV_LOAD_IMAGE_COLOR));

/* save image to file */
CV_IMPL int cvSaveImage( const char* filename, const IplImage * image );

//#define CV_CVTIMG_FLIP      1
//#define CV_CVTIMG_SWAP_RB   2 
/* utility function: convert one image to another with optional vertical flip */
//CVAPI(void) cvConvertImage( const CvArr* src, CvArr* dst, int flags CV_DEFAULT(0));

// save as Embedded file
#ifdef DM64X_NETWORK
CVAPI(int) efs_cvSaveImage( const char* filename, const IplImage * image );
#endif

#ifndef _TMS320C6X
/****************************************************************************************\
*                         Working with Video Files and Cameras                           *
\****************************************************************************************/

/* "black box" capture structure */
typedef struct CvCapture CvCapture;

/* start capturing frames from video file */
CVAPI(CvCapture*) cvCreateFileCapture( const char* filename );

#define CV_CAP_ANY      0     // autodetect

#define CV_CAP_MIL      100   // MIL proprietary drivers

#define CV_CAP_VFW      200   // platform native
#define CV_CAP_V4L      200
#define CV_CAP_V4L2     200

#define CV_CAP_FIREWARE 300   // IEEE 1394 drivers
#define CV_CAP_FIREWIRE 300
#define CV_CAP_IEEE1394 300
#define CV_CAP_DC1394   300
#define CV_CAP_CMU1394  300

#define CV_CAP_STEREO   400   // TYZX proprietary drivers
#define CV_CAP_TYZX     400
#define CV_TYZX_LEFT    400
#define CV_TYZX_RIGHT   401
#define CV_TYZX_COLOR   402
#define CV_TYZX_Z       403

#define CV_CAP_QT       500   // QuickTime

#define CV_CAP_UNICAP   600   // Unicap drivers

#define CV_CAP_DSHOW    700   // DirectShow (via videoInput)

/* start capturing frames from camera: index = camera_index + domain_offset (CV_CAP_*) */
CVAPI(CvCapture*) cvCreateCameraCapture( int index );

/* grab a frame, return 1 on success, 0 on fail.
  this function is thought to be fast               */
CVAPI(int) cvGrabFrame( CvCapture* capture );

/* get the frame grabbed with cvGrabFrame(..)
  This function may apply some frame processing like
  frame decompression, flipping etc.
  !!!DO NOT RELEASE or MODIFY the retrieved frame!!! */
      CVAPI(IplImage*) cvRetrieveFrame( CvCapture* capture, int streamIdx CV_DEFAULT(0));

/* Just a combination of cvGrabFrame and cvRetrieveFrame
   !!!DO NOT RELEASE or MODIFY the retrieved frame!!!      */
CVAPI(IplImage*) cvQueryFrame( CvCapture* capture);

/* stop capturing/reading and free resources */
CVAPI(void) cvReleaseCapture( CvCapture** capture );

#define CV_CAP_PROP_POS_MSEC       0
#define CV_CAP_PROP_POS_FRAMES     1
#define CV_CAP_PROP_POS_AVI_RATIO  2
#define CV_CAP_PROP_FRAME_WIDTH    3
#define CV_CAP_PROP_FRAME_HEIGHT   4
#define CV_CAP_PROP_FPS            5
#define CV_CAP_PROP_FOURCC         6
#define CV_CAP_PROP_FRAME_COUNT    7
#define CV_CAP_PROP_FORMAT         8
#define CV_CAP_PROP_MODE           9
#define CV_CAP_PROP_BRIGHTNESS    10
#define CV_CAP_PROP_CONTRAST      11
#define CV_CAP_PROP_SATURATION    12
#define CV_CAP_PROP_HUE           13
#define CV_CAP_PROP_GAIN          14
#define CV_CAP_PROP_EXPOSURE      15
#define CV_CAP_PROP_CONVERT_RGB   16
#define CV_CAP_PROP_WHITE_BALANCE 17
#define CV_CAP_PROP_RECTIFICATION 18

/* retrieve or set capture properties */
CVAPI(double) cvGetCaptureProperty( CvCapture* capture, int property_id );
CVAPI(int)    cvSetCaptureProperty( CvCapture* capture, int property_id, double value );

#endif	// _TMS320C6X

#ifdef __cplusplus
}  /* end of extern "C" */
#endif /* __cplusplus */

#endif /* _HIGH_GUI_ */
