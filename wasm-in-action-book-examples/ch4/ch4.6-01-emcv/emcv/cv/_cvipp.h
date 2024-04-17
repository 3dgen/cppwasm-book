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


#ifndef _CV_IPP_H_
#define _CV_IPP_H_

/****************************************************************************************\
*                                  Creating Borders                                      *
\****************************************************************************************/

#define IPCV_COPY_BORDER( bordertype, flavor )                                      \
IPCVAPI_EX( CvStatus, icvCopy##bordertype##Border_##flavor##R,                      \
    "ippiCopy" #bordertype "Border_" #flavor "R", CV_PLUGINS1(CV_PLUGIN_IPPI),      \
    ( const void* pSrc,  int srcStep, CvSize srcRoiSize, void* pDst,  int dstStep,  \
      CvSize dstRoiSize, int topBorderHeight, int leftBorderWidth ))                \
                                                                                    \
IPCVAPI_EX( CvStatus, icvCopy##bordertype##Border_##flavor##IR,                     \
    "ippiCopy" #bordertype "Border_" #flavor "IR", CV_PLUGINS1(CV_PLUGIN_IPPI),     \
    ( const void* pSrc,  int srcDstStep, CvSize srcRoiSize,                         \
      CvSize dstRoiSize, int topBorderHeight, int leftBorderWidth ))

IPCV_COPY_BORDER( Replicate, 8u_C1 )
IPCV_COPY_BORDER( Replicate, 16s_C1 )
IPCV_COPY_BORDER( Replicate, 8u_C3 )
IPCV_COPY_BORDER( Replicate, 32s_C1 )
IPCV_COPY_BORDER( Replicate, 16s_C3 )
IPCV_COPY_BORDER( Replicate, 16s_C4 )
IPCV_COPY_BORDER( Replicate, 32s_C3 )
IPCV_COPY_BORDER( Replicate, 32s_C4 )

/****************************************************************************************\
*                                        Moments                                         *
\****************************************************************************************/

#define IPCV_MOMENTS( suffix, ipp_suffix, cn )                      \
IPCVAPI_EX( CvStatus, icvMoments##suffix##_C##cn##R,                \
"ippiMoments" #ipp_suffix "_C" #cn "R", CV_PLUGINS1(CV_PLUGIN_IPPI),\
( const void* img, int step, CvSize size, void* momentstate ))

IPCV_MOMENTS( _8u, 64f_8u, 1 )
IPCV_MOMENTS( _32f, 64f_32f, 1 )

#undef IPCV_MOMENTS

IPCVAPI_EX( CvStatus, icvMomentInitAlloc_64f,
            "ippiMomentInitAlloc_64f", CV_PLUGINS1(CV_PLUGIN_IPPI),
            (void** momentstate, CvHintAlgorithm hint ))

IPCVAPI_EX( CvStatus, icvMomentFree_64f,
            "ippiMomentFree_64f", CV_PLUGINS1(CV_PLUGIN_IPPI),
            (void* momentstate ))

IPCVAPI_EX( CvStatus, icvGetSpatialMoment_64f,
            "ippiGetSpatialMoment_64f", CV_PLUGINS1(CV_PLUGIN_IPPI),
            (const void* momentstate, int mOrd, int nOrd,
             int nChannel, CvPoint roiOffset, double* value ))

/****************************************************************************************\
*                                  Background differencing                               *
\****************************************************************************************/

/////////////////////////////////// Accumulation /////////////////////////////////////////

#define IPCV_ACCUM( flavor, arrtype, acctype )                                      \
IPCVAPI_EX( CvStatus, icvAdd_##flavor##_C1IR,                                       \
    "ippiAdd_" #flavor "_C1IR", CV_PLUGINS1(CV_PLUGIN_IPPCV),                       \
    ( const arrtype* src, int srcstep, acctype* dst, int dststep, CvSize size ))    \
IPCVAPI_EX( CvStatus, icvAddSquare_##flavor##_C1IR,                                 \
    "ippiAddSquare_" #flavor "_C1IR", CV_PLUGINS1(CV_PLUGIN_IPPCV),                 \
    ( const arrtype* src, int srcstep, acctype* dst, int dststep, CvSize size ))    \
IPCVAPI_EX( CvStatus, icvAddProduct_##flavor##_C1IR,                                \
    "ippiAddProduct_" #flavor "_C1IR", CV_PLUGINS1(CV_PLUGIN_IPPCV),                \
    ( const arrtype* src1, int srcstep1, const arrtype* src2, int srcstep2,         \
      acctype* dst, int dststep, CvSize size ))                                     \
IPCVAPI_EX( CvStatus, icvAddWeighted_##flavor##_C1IR,                               \
    "ippiAddWeighted_" #flavor "_C1IR", CV_PLUGINS1(CV_PLUGIN_IPPCV),               \
    ( const arrtype* src, int srcstep, acctype* dst, int dststep,                   \
      CvSize size, acctype alpha ))                                                 \
                                                                                    \
IPCVAPI_EX( CvStatus, icvAdd_##flavor##_C1IMR,                                      \
    "ippiAdd_" #flavor "_C1IMR", CV_PLUGINS1(CV_PLUGIN_IPPCV),                      \
    ( const arrtype* src, int srcstep, const uchar* mask, int maskstep,             \
      acctype* dst, int dststep, CvSize size ))                                     \
IPCVAPI_EX( CvStatus, icvAddSquare_##flavor##_C1IMR,                                \
    "ippiAddSquare_" #flavor "_C1IMR", CV_PLUGINS1(CV_PLUGIN_IPPCV),                \
    ( const arrtype* src, int srcstep, const uchar* mask, int maskstep,             \
      acctype* dst, int dststep, CvSize size ))                                     \
IPCVAPI_EX( CvStatus, icvAddProduct_##flavor##_C1IMR,                               \
    "ippiAddProduct_" #flavor "_C1IMR", CV_PLUGINS1(CV_PLUGIN_IPPCV),               \
    ( const arrtype* src1, int srcstep1, const arrtype* src2, int srcstep2,         \
      const uchar* mask, int maskstep, acctype* dst, int dststep, CvSize size ))    \
IPCVAPI_EX( CvStatus, icvAddWeighted_##flavor##_C1IMR,                              \
    "ippiAddWeighted_" #flavor "_C1IMR", CV_PLUGINS1(CV_PLUGIN_IPPCV),              \
    ( const arrtype* src, int srcstep, const uchar* mask, int maskstep,             \
      acctype* dst, int dststep, CvSize size, acctype alpha ))                      \
                                                                                    \
IPCVAPI_EX( CvStatus, icvAdd_##flavor##_C3IMR,                                      \
    "ippiAdd_" #flavor "_C3IMR", CV_PLUGINS1(CV_PLUGIN_IPPCV),                      \
    ( const arrtype* src, int srcstep, const uchar* mask, int maskstep,             \
      acctype* dst, int dststep, CvSize size ))                                     \
IPCVAPI_EX( CvStatus, icvAddSquare_##flavor##_C3IMR,                                \
    "ippiAddSquare_" #flavor "_C3IMR", CV_PLUGINS1(CV_PLUGIN_IPPCV),                \
    ( const arrtype* src, int srcstep, const uchar* mask, int maskstep,             \
      acctype* dst, int dststep, CvSize size ))                                     \
IPCVAPI_EX( CvStatus, icvAddProduct_##flavor##_C3IMR,                               \
    "ippiAddProduct_" #flavor "_C3IMR", CV_PLUGINS1(CV_PLUGIN_IPPCV),               \
    ( const arrtype* src1, int srcstep1, const arrtype* src2, int srcstep2,         \
      const uchar* mask, int maskstep, acctype* dst, int dststep, CvSize size ))    \
IPCVAPI_EX( CvStatus, icvAddWeighted_##flavor##_C3IMR,                              \
    "ippiAddWeighted_" #flavor "_C3IMR", CV_PLUGINS1(CV_PLUGIN_IPPCV),              \
    ( const arrtype* src, int srcstep, const uchar* mask, int maskstep,             \
      acctype* dst, int dststep, CvSize size, acctype alpha ))

IPCV_ACCUM( 8u32f, uchar, float )
IPCV_ACCUM( 32f, float, float )

#undef IPCV_ACCUM

/****************************************************************************************\
*                                       Pyramids                                         *
\****************************************************************************************/

IPCVAPI_EX( CvStatus, icvPyrDownGetBufSize_Gauss5x5,
           "ippiPyrDownGetBufSize_Gauss5x5", CV_PLUGINS1(CV_PLUGIN_IPPCV),
            ( int roiWidth, CvDataType dataType, int channels, int* bufSize ))

IPCVAPI_EX( CvStatus, icvPyrUpGetBufSize_Gauss5x5,
           "ippiPyrUpGetBufSize_Gauss5x5", CV_PLUGINS1(CV_PLUGIN_IPPCV),
            ( int roiWidth, CvDataType dataType, int channels, int* bufSize ))

#define ICV_PYRDOWN( flavor, cn )                                           \
IPCVAPI_EX( CvStatus, icvPyrDown_Gauss5x5_##flavor##_C##cn##R,              \
"ippiPyrDown_Gauss5x5_" #flavor "_C" #cn "R", CV_PLUGINS1(CV_PLUGIN_IPPCV), \
( const void* pSrc, int srcStep, void* pDst, int dstStep,                   \
  CvSize roiSize, void* pBuffer ))

#define ICV_PYRUP( flavor, cn )                                             \
IPCVAPI_EX( CvStatus, icvPyrUp_Gauss5x5_##flavor##_C##cn##R,                \
"ippiPyrUp_Gauss5x5_" #flavor "_C" #cn "R", CV_PLUGINS1(CV_PLUGIN_IPPCV),   \
( const void* pSrc, int srcStep, void* pDst, int dstStep,                   \
  CvSize roiSize, void* pBuffer ))

ICV_PYRDOWN( 8u, 1 )
ICV_PYRDOWN( 8u, 3 )
ICV_PYRDOWN( 32f, 1 )
ICV_PYRDOWN( 32f, 3 )

ICV_PYRUP( 8u, 1 )
ICV_PYRUP( 8u, 3 )
ICV_PYRUP( 32f, 1 )
ICV_PYRUP( 32f, 3 )

#undef ICV_PYRDOWN
#undef ICV_PYRUP

/****************************************************************************************\
*                                Geometric Transformations                               *
\****************************************************************************************/

#define IPCV_RESIZE( flavor, cn )                                           \
IPCVAPI_EX( CvStatus, icvResize_##flavor##_C##cn##R,                        \
            "ippiResize_" #flavor "_C" #cn "R", CV_PLUGINS1(CV_PLUGIN_IPPI),\
           (const void* src, CvSize srcsize, int srcstep, CvRect srcroi,    \
            void* dst, int dststep, CvSize dstroi,                          \
            double xfactor, double yfactor, int interpolation ))

IPCV_RESIZE( 8u, 1 )
IPCV_RESIZE( 8u, 3 )
IPCV_RESIZE( 8u, 4 )

IPCV_RESIZE( 16u, 1 )
IPCV_RESIZE( 16u, 3 )
IPCV_RESIZE( 16u, 4 )

IPCV_RESIZE( 32f, 1 )
IPCV_RESIZE( 32f, 3 )
IPCV_RESIZE( 32f, 4 )

#undef IPCV_RESIZE

#define IPCV_WARPAFFINE_BACK( flavor, cn )                                  \
IPCVAPI_EX( CvStatus, icvWarpAffineBack_##flavor##_C##cn##R,                \
    "ippiWarpAffineBack_" #flavor "_C" #cn "R", CV_PLUGINS1(CV_PLUGIN_IPPI),\
    (const void* src, CvSize srcsize, int srcstep, CvRect srcroi,           \
    void* dst, int dststep, CvRect dstroi,                                  \
    const double* coeffs, int interpolate ))

IPCV_WARPAFFINE_BACK( 8u, 1 )
IPCV_WARPAFFINE_BACK( 8u, 3 )
IPCV_WARPAFFINE_BACK( 8u, 4 )

IPCV_WARPAFFINE_BACK( 32f, 1 )
IPCV_WARPAFFINE_BACK( 32f, 3 )
IPCV_WARPAFFINE_BACK( 32f, 4 )

#undef IPCV_WARPAFFINE_BACK

#define IPCV_WARPPERSPECTIVE_BACK( flavor, cn )                             \
IPCVAPI_EX( CvStatus, icvWarpPerspectiveBack_##flavor##_C##cn##R,           \
    "ippiWarpPerspectiveBack_" #flavor "_C" #cn "R", CV_PLUGINS1(CV_PLUGIN_IPPI),\
    (const void* src, CvSize srcsize, int srcstep, CvRect srcroi,           \
    void* dst, int dststep, CvRect dstroi,                                  \
    const double* coeffs, int interpolate ))

IPCV_WARPPERSPECTIVE_BACK( 8u, 1 )
IPCV_WARPPERSPECTIVE_BACK( 8u, 3 )
IPCV_WARPPERSPECTIVE_BACK( 8u, 4 )

IPCV_WARPPERSPECTIVE_BACK( 32f, 1 )
IPCV_WARPPERSPECTIVE_BACK( 32f, 3 )
IPCV_WARPPERSPECTIVE_BACK( 32f, 4 )

#undef IPCV_WARPPERSPECTIVE_BACK


#define IPCV_WARPPERSPECTIVE( flavor, cn )                                  \
IPCVAPI_EX( CvStatus, icvWarpPerspective_##flavor##_C##cn##R,               \
    "ippiWarpPerspective_" #flavor "_C" #cn "R", CV_PLUGINS1(CV_PLUGIN_IPPI),\
    (const void* src, CvSize srcsize, int srcstep, CvRect srcroi,           \
    void* dst, int dststep, CvRect dstroi,                                  \
    const double* coeffs, int interpolate ))

IPCV_WARPPERSPECTIVE( 8u, 1 )
IPCV_WARPPERSPECTIVE( 8u, 3 )
IPCV_WARPPERSPECTIVE( 8u, 4 )

IPCV_WARPPERSPECTIVE( 32f, 1 )
IPCV_WARPPERSPECTIVE( 32f, 3 )
IPCV_WARPPERSPECTIVE( 32f, 4 )

#undef IPCV_WARPPERSPECTIVE

#define IPCV_REMAP( flavor, cn )                                        \
IPCVAPI_EX( CvStatus, icvRemap_##flavor##_C##cn##R,                     \
    "ippiRemap_" #flavor "_C" #cn "R", CV_PLUGINS1(CV_PLUGIN_IPPI),     \
    ( const void* src, CvSize srcsize, int srcstep, CvRect srcroi,      \
      const float* xmap, int xmapstep, const float* ymap, int ymapstep, \
      void* dst, int dststep, CvSize dstsize, int interpolation )) 

IPCV_REMAP( 8u, 1 )
IPCV_REMAP( 8u, 3 )
IPCV_REMAP( 8u, 4 )

IPCV_REMAP( 32f, 1 )
IPCV_REMAP( 32f, 3 )
IPCV_REMAP( 32f, 4 )

#undef IPCV_REMAP

/****************************************************************************************\
*                                      Morphology                                        *
\****************************************************************************************/

#define IPCV_MORPHOLOGY( minmaxtype, morphtype, flavor, cn )                    \
IPCVAPI_EX( CvStatus, icv##morphtype##Rect_##flavor##_C##cn##R,                 \
    "ippiFilter" #minmaxtype "BorderReplicate_" #flavor "_C" #cn "R",           \
    CV_PLUGINS1(CV_PLUGIN_IPPCV), ( const void* src, int srcstep, void* dst,    \
    int dststep, CvSize roi, CvSize esize, CvPoint anchor, void* buffer ))      \
IPCVAPI_EX( CvStatus, icv##morphtype##Rect_GetBufSize_##flavor##_C##cn##R,      \
    "ippiFilter" #minmaxtype "GetBufferSize_" #flavor "_C" #cn "R",             \
    CV_PLUGINS1(CV_PLUGIN_IPPCV), ( int width, CvSize esize, int* bufsize ))    \
                                                                                \
IPCVAPI_EX( CvStatus, icv##morphtype##_##flavor##_C##cn##R,                     \
    "ippi" #morphtype "BorderReplicate_" #flavor "_C" #cn "R",                  \
    CV_PLUGINS1(CV_PLUGIN_IPPCV), ( const void* src, int srcstep,               \
    void* dst, int dststep, CvSize roi, int bordertype, void* morphstate ))

IPCV_MORPHOLOGY( Min, Erode, 8u, 1 )
IPCV_MORPHOLOGY( Min, Erode, 8u, 3 )
IPCV_MORPHOLOGY( Min, Erode, 8u, 4 )
IPCV_MORPHOLOGY( Min, Erode, 32f, 1 )
IPCV_MORPHOLOGY( Min, Erode, 32f, 3 )
IPCV_MORPHOLOGY( Min, Erode, 32f, 4 )
IPCV_MORPHOLOGY( Max, Dilate, 8u, 1 )
IPCV_MORPHOLOGY( Max, Dilate, 8u, 3 )
IPCV_MORPHOLOGY( Max, Dilate, 8u, 4 )
IPCV_MORPHOLOGY( Max, Dilate, 32f, 1 )
IPCV_MORPHOLOGY( Max, Dilate, 32f, 3 )
IPCV_MORPHOLOGY( Max, Dilate, 32f, 4 )

#undef IPCV_MORPHOLOGY

#define IPCV_MORPHOLOGY_INIT_ALLOC( flavor, cn )                            \
IPCVAPI_EX( CvStatus, icvMorphInitAlloc_##flavor##_C##cn##R,                \
    "ippiMorphologyInitAlloc_" #flavor "_C" #cn "R",                        \
    CV_PLUGINS1(CV_PLUGIN_IPPCV), ( int width, const uchar* element,        \
    CvSize esize, CvPoint anchor, void** morphstate ))

IPCV_MORPHOLOGY_INIT_ALLOC( 8u, 1 )
IPCV_MORPHOLOGY_INIT_ALLOC( 8u, 3 )
IPCV_MORPHOLOGY_INIT_ALLOC( 8u, 4 )
IPCV_MORPHOLOGY_INIT_ALLOC( 32f, 1 )
IPCV_MORPHOLOGY_INIT_ALLOC( 32f, 3 )
IPCV_MORPHOLOGY_INIT_ALLOC( 32f, 4 )

#undef IPCV_MORPHOLOGY_INIT_ALLOC

IPCVAPI_EX( CvStatus, icvMorphFree, "ippiMorphologyFree",
    CV_PLUGINS1(CV_PLUGIN_IPPCV), ( void* morphstate ))


/****************************************************************************************\
*                                 Smoothing Filters                                      *
\****************************************************************************************/

#define IPCV_FILTER_MEDIAN( flavor, cn )                                            \
IPCVAPI_EX( CvStatus, icvFilterMedian_##flavor##_C##cn##R,                          \
            "ippiFilterMedian_" #flavor "_C" #cn "R", CV_PLUGINS1(CV_PLUGIN_IPPI),  \
            ( const void* src, int srcstep, void* dst, int dststep,                 \
              CvSize roi, CvSize ksize, CvPoint anchor ))

IPCV_FILTER_MEDIAN( 8u, 1 )
IPCV_FILTER_MEDIAN( 8u, 3 )
IPCV_FILTER_MEDIAN( 8u, 4 )

#define IPCV_FILTER_BOX( flavor, cn )                                               \
IPCVAPI_EX( CvStatus, icvFilterBox_##flavor##_C##cn##R,                             \
            "ippiFilterBox_" #flavor "_C" #cn "R", 0/*CV_PLUGINS1(CV_PLUGIN_IPPI)*/,\
            ( const void* src, int srcstep, void* dst, int dststep,                 \
              CvSize roi, CvSize ksize, CvPoint anchor ))

IPCV_FILTER_BOX( 8u, 1 )
IPCV_FILTER_BOX( 8u, 3 )
IPCV_FILTER_BOX( 8u, 4 )
IPCV_FILTER_BOX( 32f, 1 )
IPCV_FILTER_BOX( 32f, 3 )
IPCV_FILTER_BOX( 32f, 4 )

#undef IPCV_FILTER_BOX

/****************************************************************************************\
*                                 Derivative Filters                                     *
\****************************************************************************************/

#define IPCV_FILTER_SOBEL_BORDER( suffix, flavor, srctype )                             \
IPCVAPI_EX( CvStatus, icvFilterSobel##suffix##GetBufSize_##flavor##_C1R,                \
    "ippiFilterSobel" #suffix "GetBufferSize_" #flavor "_C1R",                          \
    CV_PLUGINS1(CV_PLUGIN_IPPCV), ( CvSize roi, int masksize, int* buffersize ))        \
IPCVAPI_EX( CvStatus, icvFilterSobel##suffix##Border_##flavor##_C1R,                    \
    "ippiFilterSobel" #suffix "Border_" #flavor "_C1R", CV_PLUGINS1(CV_PLUGIN_IPPCV),   \
    ( const void* src, int srcstep, void* dst, int dststep, CvSize roi, int masksize,   \
      int bordertype, srctype bordervalue, void* buffer ))

IPCV_FILTER_SOBEL_BORDER( NegVert, 8u16s, uchar )
IPCV_FILTER_SOBEL_BORDER( Horiz, 8u16s, uchar )
IPCV_FILTER_SOBEL_BORDER( VertSecond, 8u16s, uchar )
IPCV_FILTER_SOBEL_BORDER( HorizSecond, 8u16s, uchar )
IPCV_FILTER_SOBEL_BORDER( Cross, 8u16s, uchar )

IPCV_FILTER_SOBEL_BORDER( NegVert, 32f, float )
IPCV_FILTER_SOBEL_BORDER( Horiz, 32f, float )
IPCV_FILTER_SOBEL_BORDER( VertSecond, 32f, float )
IPCV_FILTER_SOBEL_BORDER( HorizSecond, 32f, float )
IPCV_FILTER_SOBEL_BORDER( Cross, 32f, float )

#undef IPCV_FILTER_SOBEL_BORDER

#define IPCV_FILTER_SCHARR_BORDER( suffix, flavor, srctype )                            \
IPCVAPI_EX( CvStatus, icvFilterScharr##suffix##GetBufSize_##flavor##_C1R,               \
    "ippiFilterScharr" #suffix "GetBufferSize_" #flavor "_C1R",                         \
    CV_PLUGINS1(CV_PLUGIN_IPPCV), ( CvSize roi, int* buffersize ))                      \
IPCVAPI_EX( CvStatus, icvFilterScharr##suffix##Border_##flavor##_C1R,                   \
    "ippiFilterScharr" #suffix "Border_" #flavor "_C1R", CV_PLUGINS1(CV_PLUGIN_IPPCV),  \
    ( const void* src, int srcstep, void* dst, int dststep, CvSize roi,                 \
      int bordertype, srctype bordervalue, void* buffer ))

IPCV_FILTER_SCHARR_BORDER( Vert, 8u16s, uchar )
IPCV_FILTER_SCHARR_BORDER( Horiz, 8u16s, uchar )

IPCV_FILTER_SCHARR_BORDER( Vert, 32f, float )
IPCV_FILTER_SCHARR_BORDER( Horiz, 32f, float )

#undef IPCV_FILTER_SCHARR_BORDER


#define IPCV_FILTER_LAPLACIAN_BORDER( flavor, srctype )                                 \
IPCVAPI_EX( CvStatus, icvFilterLaplacianGetBufSize_##flavor##_C1R,                      \
    "ippiFilterLaplacianGetBufferSize_" #flavor "_C1R",                                 \
    CV_PLUGINS1(CV_PLUGIN_IPPCV), ( CvSize roi, int masksize, int* buffersize ))        \
IPCVAPI_EX( CvStatus, icvFilterLaplacianBorder_##flavor##_C1R,                          \
    "ippiFilterLaplacianBorder_" #flavor "_C1R", CV_PLUGINS1(CV_PLUGIN_IPPCV),          \
    ( const void* src, int srcstep, void* dst, int dststep, CvSize roi, int masksize,   \
      int bordertype, srctype bordervalue, void* buffer ))

IPCV_FILTER_LAPLACIAN_BORDER( 8u16s, uchar )
IPCV_FILTER_LAPLACIAN_BORDER( 32f, float )

#undef IPCV_FILTER_LAPLACIAN_BORDER


/////////////////////////////////////////////////////////////////////////////////////////

#define IPCV_FILTER_SOBEL( suffix, ipp_suffix, flavor )                             \
IPCVAPI_EX( CvStatus, icvFilterSobel##suffix##_##flavor##_C1R,                      \
    "ippiFilterSobel" #ipp_suffix "_" #flavor "_C1R", CV_PLUGINS1(CV_PLUGIN_IPPI),  \
    ( const void* src, int srcstep, void* dst, int dststep, CvSize roi, int aperture ))

IPCV_FILTER_SOBEL( Vert, Vert, 8u16s )
IPCV_FILTER_SOBEL( Horiz, Horiz, 8u16s )
IPCV_FILTER_SOBEL( VertSecond, VertSecond, 8u16s )
IPCV_FILTER_SOBEL( HorizSecond, HorizSecond, 8u16s )
IPCV_FILTER_SOBEL( Cross, Cross, 8u16s )

IPCV_FILTER_SOBEL( Vert, VertMask, 32f )
IPCV_FILTER_SOBEL( Horiz, HorizMask, 32f )
IPCV_FILTER_SOBEL( VertSecond, VertSecond, 32f )
IPCV_FILTER_SOBEL( HorizSecond, HorizSecond, 32f )
IPCV_FILTER_SOBEL( Cross, Cross, 32f )

#undef IPCV_FILTER_SOBEL

#define IPCV_FILTER_SCHARR( suffix, ipp_suffix, flavor )                            \
IPCVAPI_EX( CvStatus, icvFilterScharr##suffix##_##flavor##_C1R,                     \
    "ippiFilterScharr" #ipp_suffix "_" #flavor "_C1R", CV_PLUGINS1(CV_PLUGIN_IPPI), \
    ( const void* src, int srcstep, void* dst, int dststep, CvSize roi ))

IPCV_FILTER_SCHARR( Vert, Vert, 8u16s )
IPCV_FILTER_SCHARR( Horiz, Horiz, 8u16s )
IPCV_FILTER_SCHARR( Vert, Vert, 32f )
IPCV_FILTER_SCHARR( Horiz, Horiz, 32f )

#undef IPCV_FILTER_SCHARR

/****************************************************************************************\
*                                   Generic Filters                                      *
\****************************************************************************************/

#define IPCV_FILTER( suffix, ipp_suffix, cn, ksizetype, anchortype )                    \
IPCVAPI_EX( CvStatus, icvFilter##suffix##_C##cn##R,                                     \
            "ippiFilter" #ipp_suffix "_C" #cn "R", CV_PLUGINS1(CV_PLUGIN_IPPI),         \
            ( const void* src, int srcstep, void* dst, int dststep, CvSize size,        \
              const float* kernel, ksizetype ksize, anchortype anchor ))

IPCV_FILTER( _8u, 32f_8u, 1, CvSize, CvPoint )
IPCV_FILTER( _8u, 32f_8u, 3, CvSize, CvPoint )
IPCV_FILTER( _8u, 32f_8u, 4, CvSize, CvPoint )

IPCV_FILTER( _16s, 32f_16s, 1, CvSize, CvPoint )
IPCV_FILTER( _16s, 32f_16s, 3, CvSize, CvPoint )
IPCV_FILTER( _16s, 32f_16s, 4, CvSize, CvPoint )

IPCV_FILTER( _32f, _32f, 1, CvSize, CvPoint )
IPCV_FILTER( _32f, _32f, 3, CvSize, CvPoint )
IPCV_FILTER( _32f, _32f, 4, CvSize, CvPoint )

IPCV_FILTER( Column_8u, Column32f_8u, 1, int, int )
IPCV_FILTER( Column_8u, Column32f_8u, 3, int, int )
IPCV_FILTER( Column_8u, Column32f_8u, 4, int, int )

IPCV_FILTER( Column_16s, Column32f_16s, 1, int, int )
IPCV_FILTER( Column_16s, Column32f_16s, 3, int, int )
IPCV_FILTER( Column_16s, Column32f_16s, 4, int, int )

IPCV_FILTER( Column_32f, Column_32f, 1, int, int )
IPCV_FILTER( Column_32f, Column_32f, 3, int, int )
IPCV_FILTER( Column_32f, Column_32f, 4, int, int )

IPCV_FILTER( Row_8u, Row32f_8u, 1, int, int )
IPCV_FILTER( Row_8u, Row32f_8u, 3, int, int )
IPCV_FILTER( Row_8u, Row32f_8u, 4, int, int )

IPCV_FILTER( Row_16s, Row32f_16s, 1, int, int )
IPCV_FILTER( Row_16s, Row32f_16s, 3, int, int )
IPCV_FILTER( Row_16s, Row32f_16s, 4, int, int )

IPCV_FILTER( Row_32f, Row_32f, 1, int, int )
IPCV_FILTER( Row_32f, Row_32f, 3, int, int )
IPCV_FILTER( Row_32f, Row_32f, 4, int, int )

#undef IPCV_FILTER


/****************************************************************************************\
*                                  Color Transformations                                 *
\****************************************************************************************/

#define IPCV_COLOR( funcname, ipp_funcname, flavor )                            \
IPCVAPI_EX( CvStatus, icv##funcname##_##flavor##_C3R,                           \
        "ippi" #ipp_funcname "_" #flavor "_C3R,"                                \
        "ippi" #ipp_funcname "_" #flavor "_C3R",                                \
        CV_PLUGINS2(CV_PLUGIN_IPPI,CV_PLUGIN_IPPCC),                            \
        ( const void* src, int srcstep, void* dst, int dststep, CvSize size ))

IPCV_COLOR( RGB2XYZ, RGBToXYZ, 8u )
IPCV_COLOR( RGB2XYZ, RGBToXYZ, 16u )
IPCV_COLOR( RGB2XYZ, RGBToXYZ, 32f )
IPCV_COLOR( XYZ2RGB, XYZToRGB, 8u )
IPCV_COLOR( XYZ2RGB, XYZToRGB, 16u )
IPCV_COLOR( XYZ2RGB, XYZToRGB, 32f )

IPCV_COLOR( RGB2HSV, RGBToHSV, 8u )
IPCV_COLOR( HSV2RGB, HSVToRGB, 8u )

IPCV_COLOR( RGB2HLS, RGBToHLS, 8u )
IPCV_COLOR( RGB2HLS, RGBToHLS, 32f )
IPCV_COLOR( HLS2RGB, HLSToRGB, 8u )
IPCV_COLOR( HLS2RGB, HLSToRGB, 32f )

IPCV_COLOR( BGR2Lab, BGRToLab, 8u )
IPCV_COLOR( Lab2BGR, LabToBGR, 8u )

IPCV_COLOR( RGB2Luv, RGBToLUV, 8u )
/*IPCV_COLOR( RGB2Luv, RGBToLUV, 32f )*/
IPCV_COLOR( Luv2RGB, LUVToRGB, 8u )
/*IPCV_COLOR( Luv2RGB, LUVToRGB, 32f )*/

/****************************************************************************************\
*                                  Motion Templates                                      *
\****************************************************************************************/

IPCVAPI_EX( CvStatus, icvUpdateMotionHistory_8u32f_C1IR,
    "ippiUpdateMotionHistory_8u32f_C1IR", CV_PLUGINS1(CV_PLUGIN_IPPCV),
    ( const uchar* silIm, int silStep, float* mhiIm, int mhiStep,
      CvSize size,float  timestamp, float  mhi_duration ))

/****************************************************************************************\
*                                 Template Matching                                      *
\****************************************************************************************/

#define ICV_MATCHTEMPLATE( flavor, arrtype )                        \
IPCVAPI_EX( CvStatus, icvCrossCorrValid_Norm_##flavor##_C1R,        \
        "ippiCrossCorrValid_Norm_" #flavor "_C1R",                  \
        CV_PLUGINS1(CV_PLUGIN_IPPI),                                \
        ( const arrtype* pSrc, int srcStep, CvSize srcRoiSize,      \
        const arrtype* pTpl, int tplStep, CvSize tplRoiSize,        \
        float* pDst, int dstStep ))                                 \
IPCVAPI_EX( CvStatus, icvCrossCorrValid_NormLevel_##flavor##_C1R,   \
        "ippiCrossCorrValid_NormLevel_" #flavor "_C1R",             \
        CV_PLUGINS1(CV_PLUGIN_IPPI),                                \
        ( const arrtype* pSrc, int srcStep, CvSize srcRoiSize,      \
        const arrtype* pTpl, int tplStep, CvSize tplRoiSize,        \
        float* pDst, int dstStep ))                                 \
IPCVAPI_EX( CvStatus, icvSqrDistanceValid_Norm_##flavor##_C1R,      \
        "ippiSqrDistanceValid_Norm_" #flavor "_C1R",                \
        CV_PLUGINS1(CV_PLUGIN_IPPI),                                \
        ( const arrtype* pSrc, int srcStep, CvSize srcRoiSize,      \
        const arrtype* pTpl, int tplStep, CvSize tplRoiSize,        \
        float* pDst, int dstStep ))

ICV_MATCHTEMPLATE( 8u32f, uchar )
ICV_MATCHTEMPLATE( 32f, float )

/****************************************************************************************/
/*                                Distance Transform                                    */
/****************************************************************************************/

IPCVAPI_EX(CvStatus, icvDistanceTransform_3x3_8u32f_C1R,
    "ippiDistanceTransform_3x3_8u32f_C1R", CV_PLUGINS1(CV_PLUGIN_IPPCV),
    ( const uchar* pSrc, int srcStep, float* pDst,
      int dstStep, CvSize roiSize, const float* pMetrics ))

IPCVAPI_EX(CvStatus, icvDistanceTransform_5x5_8u32f_C1R,
    "ippiDistanceTransform_5x5_8u32f_C1R", CV_PLUGINS1(CV_PLUGIN_IPPCV),
    ( const uchar* pSrc, int srcStep, float* pDst,
      int dstStep, CvSize roiSize, const float* pMetrics ))

/****************************************************************************************\
*                               Thresholding functions                                   *
\****************************************************************************************/

IPCVAPI_EX( CvStatus, icvCompareC_8u_C1R_cv,
            "ippiCompareC_8u_C1R", CV_PLUGINS1(CV_PLUGIN_IPPI),
            ( const uchar* src1, int srcstep1, uchar scalar,
              uchar* dst, int dststep, CvSize size, int cmp_op ))
IPCVAPI_EX( CvStatus, icvAndC_8u_C1R,
            "ippiAndC_8u_C1R", CV_PLUGINS1(CV_PLUGIN_IPPI),
            ( const uchar* src1, int srcstep1, uchar scalar,
              uchar* dst, int dststep, CvSize size ))
IPCVAPI_EX( CvStatus, icvThreshold_GTVal_8u_C1R,
            "ippiThreshold_GTVal_8u_C1R", CV_PLUGINS1(CV_PLUGIN_IPPI),
            ( const uchar* pSrc, int srcstep, uchar* pDst, int dststep,
              CvSize size, uchar threshold, uchar value ))
IPCVAPI_EX( CvStatus, icvThreshold_GTVal_32f_C1R,
            "ippiThreshold_GTVal_32f_C1R", CV_PLUGINS1(CV_PLUGIN_IPPI),
            ( const float* pSrc, int srcstep, float* pDst, int dststep,
              CvSize size, float threshold, float value ))
IPCVAPI_EX( CvStatus, icvThreshold_LTVal_8u_C1R,
            "ippiThreshold_LTVal_8u_C1R", CV_PLUGINS1(CV_PLUGIN_IPPI),
            ( const uchar* pSrc, int srcstep, uchar* pDst, int dststep,
              CvSize size, uchar threshold, uchar value ))
IPCVAPI_EX( CvStatus, icvThreshold_LTVal_32f_C1R,
            "ippiThreshold_LTVal_32f_C1R", CV_PLUGINS1(CV_PLUGIN_IPPI),
            ( const float* pSrc, int srcstep, float* pDst, int dststep,
              CvSize size, float threshold, float value ))

/****************************************************************************************\
*                                 Canny Edge Detector                                    *
\****************************************************************************************/

IPCVAPI_EX( CvStatus, icvCannyGetSize, "ippiCannyGetSize", 0/*CV_PLUGINS1(CV_PLUGIN_IPPCV)*/,
           ( CvSize roiSize, int* bufferSize ))

IPCVAPI_EX( CvStatus, icvCanny_16s8u_C1R, "ippiCanny_16s8u_C1R", 0/*CV_PLUGINS1(CV_PLUGIN_IPPCV)*/,
    ( const short* pSrcDx, int srcDxStep, const short* pSrcDy, int srcDyStep,
      uchar*  pDstEdges, int dstEdgeStep, CvSize roiSize, float lowThresh,
      float  highThresh, void* pBuffer ))


/****************************************************************************************\
*                                 Radial Distortion Removal                              *
\****************************************************************************************/

IPCVAPI_EX( CvStatus, icvUndistortGetSize, "ippiUndistortGetSize",
            CV_PLUGINS1(CV_PLUGIN_IPPCV), ( CvSize roiSize, int *pBufsize ))

IPCVAPI_EX( CvStatus, icvCreateMapCameraUndistort_32f_C1R,
            "ippiCreateMapCameraUndistort_32f_C1R", CV_PLUGINS1(CV_PLUGIN_IPPCV),
            (float *pxMap, int xStep, float *pyMap, int yStep, CvSize roiSize,
            float fx, float fy, float cx, float cy, float k1, float k2,
            float p1, float p2, uchar *pBuffer ))

#define ICV_UNDISTORT_RADIAL( flavor, cn, arrtype )                                 \
IPCVAPI_EX( CvStatus, icvUndistortRadial_##flavor##_C##cn##R,                       \
    "ippiUndistortRadial_" #flavor "_C" #cn "R", CV_PLUGINS1(CV_PLUGIN_IPPCV),      \
    ( const arrtype* pSrc, int srcStep, uchar* pDst, int dstStep, CvSize roiSize,   \
      float fx, float fy, float cx, float cy, float k1, float k2, uchar *pBuffer ))

ICV_UNDISTORT_RADIAL( 8u, 1, uchar )
ICV_UNDISTORT_RADIAL( 8u, 3, uchar )

#undef ICV_UNDISTORT_RADIAL

/****************************************************************************************\
*                            Subpixel-accurate rectangle extraction                      *
\****************************************************************************************/

#define ICV_COPY_SUBPIX( flavor, cn, srctype, dsttype )                     \
IPCVAPI_EX( CvStatus, icvCopySubpix_##flavor##_C##cn##R,                    \
    "ippiCopySubpix_" #flavor "_C" #cn "R", CV_PLUGINS1(CV_PLUGIN_IPPCV),   \
    ( const srctype* pSrc, int srcStep, dsttype* pDst, int dstStep,         \
    CvSize size, float dx, float dy ))

ICV_COPY_SUBPIX( 8u, 1, uchar, uchar )
ICV_COPY_SUBPIX( 8u32f, 1, uchar, float )
//ICV_COPY_SUBPIX( 32f, 1, float, float )

IPCVAPI_EX( CvStatus, icvCopySubpix_32f_C1R,
    "ippiCopySubpix_32f_C1R", 0,
    ( const float* pSrc, int srcStep, float* pDst, int dstStep,
    CvSize size, float dx, float dy ))

#undef ICV_COPY_SUBPIX

/****************************************************************************************\
*                                Lucas-Kanade Optical Flow                               *
\****************************************************************************************/

IPCVAPI_EX( CvStatus, icvOpticalFlowPyrLKInitAlloc_8u_C1R,
            "ippiOpticalFlowPyrLKInitAlloc_8u_C1R", CV_PLUGINS1(CV_PLUGIN_IPPCV),
            ( void** ppState, CvSize roiSize, int winSize, int hint ))

IPCVAPI_EX( CvStatus, icvOpticalFlowPyrLKFree_8u_C1R,
            "ippiOpticalFlowPyrLKFree_8u_C1R", CV_PLUGINS1(CV_PLUGIN_IPPCV),
            ( void* pState ))

IPCVAPI_EX( CvStatus, icvOpticalFlowPyrLK_8u_C1R,
            "ippiOpticalFlowPyrLK_8u_C1R", CV_PLUGINS1(CV_PLUGIN_IPPCV),
            ( CvPyramid *pPyr1, CvPyramid *pPyr2,
            const float *pPrev, float* pNext, char *pStatus,
            float *pError, int numFeat, int winSize,
            int maxLev, int maxIter, float threshold, void* state ))


/****************************************************************************************\
*                                 Haar Object Detector                                   *
\****************************************************************************************/

IPCVAPI_EX( CvStatus, icvIntegral_8u32s_C1R,
            "ippiIntegral_8u32s_C1R", CV_PLUGINS1(CV_PLUGIN_IPPCV),
            ( const uchar* pSrc, int srcStep, int* pDst, int dstStep,
              CvSize roiSize, int val ))

IPCVAPI_EX( CvStatus, icvSqrIntegral_8u32s64f_C1R,
            "ippiSqrIntegral_8u32s64f_C1R", CV_PLUGINS1(CV_PLUGIN_IPPCV),
            ( const uchar* pSrc, int srcStep,
              int* pDst, int dstStep, double* pSqr, int sqrStep,
              CvSize roiSize, int val, double valSqr ))

IPCVAPI_EX( CvStatus, icvRectStdDev_32s32f_C1R,
            "ippiRectStdDev_32s32f_C1R", CV_PLUGINS1(CV_PLUGIN_IPPCV),
            ( const int* pSrc, int srcStep,
              const double* pSqr, int sqrStep, float* pDst, int dstStep,
              CvSize roiSize, CvRect rect ))

IPCVAPI_EX( CvStatus, icvHaarClassifierInitAlloc_32f,
            "ippiHaarClassifierInitAlloc_32f", CV_PLUGINS1(CV_PLUGIN_IPPCV),
            ( void **pState, const CvRect* pFeature, const float* pWeight,
              const float* pThreshold, const float* pVal1,
              const float* pVal2, const int* pNum, int length ))

IPCVAPI_EX( CvStatus, icvHaarClassifierFree_32f,
            "ippiHaarClassifierFree_32f", CV_PLUGINS1(CV_PLUGIN_IPPCV),
            ( void *pState ))

IPCVAPI_EX( CvStatus, icvApplyHaarClassifier_32s32f_C1R,
            "ippiApplyHaarClassifier_32s32f_C1R", CV_PLUGINS1(CV_PLUGIN_IPPCV),
            ( const int* pSrc, int srcStep, const float* pNorm,
              int normStep, uchar* pMask, int maskStep,
              CvSize roi, int *pPositive, float threshold,
              void *pState ))

#endif /*_CV_IPP_H_*/

