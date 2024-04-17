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


#ifndef _CV_GEOM_H_
#define _CV_GEOM_H_

/* Finds distance between two points */
CV_INLINE  float  icvDistanceL2_32f( CvPoint2D32f pt1, CvPoint2D32f pt2 )
{
    float dx = pt2.x - pt1.x;
    float dy = pt2.y - pt1.y;

    return cvSqrt( dx*dx + dy*dy );
}


int  icvIntersectLines( double x1, double dx1, double y1, double dy1,
                        double x2, double dx2, double y2, double dy2,
                        double* t2 );


void icvCreateCenterNormalLine( CvSubdiv2DEdge edge, double* a, double* b, double* c );

void icvIntersectLines3( double* a0, double* b0, double* c0,
                         double* a1, double* b1, double* c1,
                         CvPoint2D32f* point );


#define _CV_BINTREE_LIST()                                          \
   struct _CvTrianAttr* prev_v;   /* pointer to the parent  element on the previous level of the tree  */    \
   struct _CvTrianAttr* next_v1;   /* pointer to the child  element on the next level of the tree  */        \
   struct _CvTrianAttr* next_v2;   /* pointer to the child  element on the next level of the tree  */        

typedef struct _CvTrianAttr
{
   CvPoint pt;    /* Coordinates x and y of the vertex  which don't lie on the base line LMIAT  */
   char sign;             /*  sign of the triangle   */
   double area;       /*   area of the triangle    */
   double r1;   /*  The ratio of the height of triangle to the base of the triangle  */
   double r2;  /*   The ratio of the projection of the left side of the triangle on the base to the base */
   _CV_BINTREE_LIST()    /* structure double list   */
}
_CvTrianAttr;


/* curvature: 0 - 1-curvature, 1 - k-cosine curvature. */
CvStatus  icvApproximateChainTC89( CvChain*      chain,
                                   int header_size,
                                   CvMemStorage* storage,
                                   CvSeq**   contour,
                                   int method );

#endif /*_IPCVGEOM_H_*/

/* End of file. */
