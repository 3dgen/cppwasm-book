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


/* Haar features calculation */

#include "_cv.h"
#include <stdio.h>


#define STAGE_THRESHOLD_SHIFT 22
#define NODE_THRESHOLD_SHIFT 22

typedef int sumtype;
typedef int64 sqsumtype;

typedef struct CvHidHaarFeature
{
    struct
    {
        sumtype *p0, *p1, *p2, *p3;
        int weight;
    }
    rect[CV_HAAR_FEATURE_MAX];
}
CvHidHaarFeature;


typedef struct CvHidHaarTreeNode
{
    CvHidHaarFeature feature;
    int threshold;
    int left;
    int right;
}
CvHidHaarTreeNode;


typedef struct CvHidHaarClassifier
{
    int count;
    //CvHaarFeature* orig_feature;
    CvHidHaarTreeNode* node;
    int* alpha;
}
CvHidHaarClassifier;


typedef struct CvHidHaarStageClassifier
{
    int  count;
    int threshold;
    CvHidHaarClassifier* classifier;
    int two_rects;
    
    struct CvHidHaarStageClassifier* next;
    struct CvHidHaarStageClassifier* child;
    struct CvHidHaarStageClassifier* parent;
}
CvHidHaarStageClassifier;


struct CvHidHaarClassifierCascade
{
    int  count;
    int  is_stump_based;
    int  has_tilted_features;
    int  is_tree;
    int window_area;
    CvMat sum, sqsum, tilted;
    CvHidHaarStageClassifier* stage_classifier;
    sqsumtype *pq0, *pq1, *pq2, *pq3;
    sumtype *p0, *p1, *p2, *p3;

    void** ipp_stages;
};


//const int icv_object_win_border = 1;
//const float icv_stage_threshold_bias = 0.0001f;
const int icv_stage_threshold_bias = 419; //0.0001*(2^22)=419.4304


static void
icvReleaseHidHaarClassifierCascade( CvHidHaarClassifierCascade** _cascade )
{
    if( _cascade && *_cascade )
    {
        CvHidHaarClassifierCascade* cascade = *_cascade;
        cvFree( &cascade->ipp_stages );
        cvFree( _cascade );
    }
}

/* create more efficient internal representation of haar classifier cascade */
static CvHidHaarClassifierCascade*
icvCreateHidHaarClassifierCascade( CvHaarClassifierCascade* cascade )
{

    CvHidHaarClassifierCascade* out = 0;

    CV_FUNCNAME( "icvCreateHidHaarClassifierCascade" );

    __BEGIN__;

    int i, j, k, l;
    int datasize;
    int total_classifiers = 0;
    int total_nodes = 0;
    char errorstr[100];
    CvHidHaarClassifier* haar_classifier_ptr;
    CvHidHaarTreeNode* haar_node_ptr;
    CvSize orig_window_size;
    int has_tilted_features = 0;
    int max_count = 0;

    if( !CV_IS_HAAR_CLASSIFIER(cascade) )
        CV_ERROR( !cascade ? CV_StsNullPtr : CV_StsBadArg, "Invalid classifier pointer" );

    if( cascade->hid_cascade )
        CV_ERROR( CV_StsError, "hid_cascade has been already created" );

    if( !cascade->stage_classifier )
        CV_ERROR( CV_StsNullPtr, "" );

    if( cascade->count <= 0 )
        CV_ERROR( CV_StsOutOfRange, "Negative number of cascade stages" );

    orig_window_size = cascade->orig_window_size;
    
    /* check input structure correctness and calculate total memory size needed for
       internal representation of the classifier cascade */
    for( i = 0; i < cascade->count; i++ )
    {
        CvHaarStageClassifier* stage_classifier = cascade->stage_classifier + i;

        if( !stage_classifier->classifier ||
            stage_classifier->count <= 0 )
        {
            sprintf( errorstr, "header of the stage classifier #%d is invalid "
                     "(has null pointers or non-positive classfier count)", i );
            CV_ERROR( CV_StsError, errorstr );
        }

        max_count = MAX( max_count, stage_classifier->count );
        total_classifiers += stage_classifier->count;

        for( j = 0; j < stage_classifier->count; j++ )
        {
            CvHaarClassifier* classifier = stage_classifier->classifier + j;

            total_nodes += classifier->count;
            for( l = 0; l < classifier->count; l++ )
            {
                for( k = 0; k < CV_HAAR_FEATURE_MAX; k++ )
                {
                    if( classifier->haar_feature[l].rect[k].r.width )
                    {
                        CvRect r = classifier->haar_feature[l].rect[k].r;
                        int tilted = classifier->haar_feature[l].tilted;
                        has_tilted_features |= tilted != 0;
                        if( r.width < 0 || r.height < 0 || r.y < 0 ||
                            r.x + r.width > orig_window_size.width
                            ||
                            (!tilted &&
                            (r.x < 0 || r.y + r.height > orig_window_size.height))
                            ||
                            (tilted && (r.x - r.height < 0 ||
                            r.y + r.width + r.height > orig_window_size.height)))
                        {
                            sprintf( errorstr, "rectangle #%d of the classifier #%d of "
                                     "the stage classifier #%d is not inside "
                                     "the reference (original) cascade window", k, j, i );
                            CV_ERROR( CV_StsNullPtr, errorstr );
                        }
                    }
                }
            }
        }
    }

    // this is an upper boundary for the whole hidden cascade size
    datasize = sizeof(CvHidHaarClassifierCascade) +
               sizeof(CvHidHaarStageClassifier)*cascade->count +
               sizeof(CvHidHaarClassifier) * total_classifiers +
               sizeof(CvHidHaarTreeNode) * total_nodes +
               sizeof(void*)*(total_nodes + total_classifiers);

    CV_CALL( out = (CvHidHaarClassifierCascade*)cvAlloc( datasize ));
    memset( out, 0, sizeof(*out) );

    /* init header */
    out->count = cascade->count;
    out->stage_classifier = (CvHidHaarStageClassifier*)(out + 1);
    haar_classifier_ptr = (CvHidHaarClassifier*)(out->stage_classifier + cascade->count);
    haar_node_ptr = (CvHidHaarTreeNode*)(haar_classifier_ptr + total_classifiers);

    out->is_stump_based = 1;
    out->has_tilted_features = has_tilted_features;
    out->is_tree = 0;

    /* initialize internal representation */
    for( i = 0; i < cascade->count; i++ )
    {
        CvHaarStageClassifier* stage_classifier = cascade->stage_classifier + i;
        CvHidHaarStageClassifier* hid_stage_classifier = out->stage_classifier + i;

        hid_stage_classifier->count = stage_classifier->count;
        hid_stage_classifier->threshold = stage_classifier->threshold - icv_stage_threshold_bias;
        hid_stage_classifier->classifier = haar_classifier_ptr;
        hid_stage_classifier->two_rects = 1;
        haar_classifier_ptr += stage_classifier->count;

        hid_stage_classifier->parent = (stage_classifier->parent == -1)
            ? NULL : out->stage_classifier + stage_classifier->parent;
        hid_stage_classifier->next = (stage_classifier->next == -1)
            ? NULL : out->stage_classifier + stage_classifier->next;
        hid_stage_classifier->child = (stage_classifier->child == -1)
            ? NULL : out->stage_classifier + stage_classifier->child;
        
        out->is_tree |= hid_stage_classifier->next != NULL;

        for( j = 0; j < stage_classifier->count; j++ )
        {
            CvHaarClassifier* classifier = stage_classifier->classifier + j;
            CvHidHaarClassifier* hid_classifier = hid_stage_classifier->classifier + j;
            int node_count = classifier->count;
            int* alpha_ptr = (int*)(haar_node_ptr + node_count);

            hid_classifier->count = node_count;
            hid_classifier->node = haar_node_ptr;
            hid_classifier->alpha = alpha_ptr;
            
            for( l = 0; l < node_count; l++ )
            {
                CvHidHaarTreeNode* node = hid_classifier->node + l;
                CvHaarFeature* feature = classifier->haar_feature + l;
                memset( node, -1, sizeof(*node) );
                node->threshold = classifier->threshold[l];
                node->left = classifier->left[l];
                node->right = classifier->right[l];

                if( (feature->rect[2].weight) == 0 ||
                    feature->rect[2].r.width == 0 ||
                    feature->rect[2].r.height == 0 )
                    memset( &(node->feature.rect[2]), 0, sizeof(node->feature.rect[2]) );
                else
                    hid_stage_classifier->two_rects = 0;
            }

            memcpy( alpha_ptr, classifier->alpha, (node_count+1)*sizeof(alpha_ptr[0]));
            haar_node_ptr =
                (CvHidHaarTreeNode*)cvAlignPtr(alpha_ptr+node_count+1, sizeof(void*));

            out->is_stump_based &= node_count == 1;
        }
    }


    cascade->hid_cascade = out;
    assert( (char*)haar_node_ptr - (char*)out <= datasize );

    __END__;

    if( cvGetErrStatus() < 0 )
        icvReleaseHidHaarClassifierCascade( &out );


    return out;
}


#define sum_elem_ptr(sum,row,col)  \
    ((sumtype*)CV_MAT_ELEM_PTR_FAST((sum),(row),(col),sizeof(sumtype)))

#define sqsum_elem_ptr(sqsum,row,col)  \
    ((sqsumtype*)CV_MAT_ELEM_PTR_FAST((sqsum),(row),(col),sizeof(sqsumtype)))

#define calc_sum(rect,offset) \
    ((rect).p0[offset] - (rect).p1[offset] - (rect).p2[offset] + (rect).p3[offset])


CV_IMPL void
cvSetImagesForHaarClassifierCascade( CvHaarClassifierCascade* _cascade,
                                     const CvArr* _sum,
                                     const CvArr* _sqsum,
                                     const CvArr* _tilted_sum,
                                     int scale32x )
{
    CV_FUNCNAME("cvSetImagesForHaarClassifierCascade");

    __BEGIN__;

    CvMat sum_stub, *sum = (CvMat*)_sum;
    CvMat sqsum_stub, *sqsum = (CvMat*)_sqsum;
    CvMat tilted_stub, *tilted = (CvMat*)_tilted_sum;
    CvHidHaarClassifierCascade* cascade;
    int coi0 = 0, coi1 = 0;
    int i;
    CvRect equ_rect;
    int weight_scale;

    if( !CV_IS_HAAR_CLASSIFIER(_cascade) )
        CV_ERROR( !_cascade ? CV_StsNullPtr : CV_StsBadArg, "Invalid classifier pointer" );

    if( scale32x <= 0 )
        CV_ERROR( CV_StsOutOfRange, "Scale must be positive" );

    CV_CALL( sum = cvGetMat( sum, &sum_stub, &coi0 ));
    CV_CALL( sqsum = cvGetMat( sqsum, &sqsum_stub, &coi1 ));

    if( coi0 || coi1 )
        CV_ERROR( CV_BadCOI, "COI is not supported" );

    if( !CV_ARE_SIZES_EQ( sum, sqsum ))
        CV_ERROR( CV_StsUnmatchedSizes, "All integral images must have the same size" );

    if( CV_MAT_TYPE(sqsum->type) != CV_64FC1 ||
        CV_MAT_TYPE(sum->type) != CV_32SC1 )
        CV_ERROR( CV_StsUnsupportedFormat,
        "Only (32s, 64f, 32s) combination of (sum,sqsum,tilted_sum) formats is allowed" );

    if( !_cascade->hid_cascade )
        CV_CALL( icvCreateHidHaarClassifierCascade(_cascade) );

    cascade = _cascade->hid_cascade;

    if( cascade->has_tilted_features )
    {
        CV_CALL( tilted = cvGetMat( tilted, &tilted_stub, &coi1 ));

        if( CV_MAT_TYPE(tilted->type) != CV_32SC1 )
            CV_ERROR( CV_StsUnsupportedFormat,
            "Only (32s, 64s, 32s) combination of (sum,sqsum,tilted_sum) formats is allowed" );

        if( sum->step != tilted->step )
            CV_ERROR( CV_StsUnmatchedSizes,
            "Sum and tilted_sum must have the same stride (step, widthStep)" );

        if( !CV_ARE_SIZES_EQ( sum, tilted ))
            CV_ERROR( CV_StsUnmatchedSizes, "All integral images must have the same size" );
        cascade->tilted = *tilted;
    }
    
    _cascade->scale32x = scale32x;
    _cascade->real_window_size.width = (_cascade->orig_window_size.width * scale32x + 16)>>5 ;
    _cascade->real_window_size.height = (_cascade->orig_window_size.height * scale32x +16) >> 5;

    cascade->sum = *sum;
    cascade->sqsum = *sqsum;
    
    equ_rect.x = equ_rect.y = (scale32x+16)>>5;
    equ_rect.width = ((_cascade->orig_window_size.width-2)*scale32x + 16 ) >> 5;
    equ_rect.height = ((_cascade->orig_window_size.height-2)*scale32x + 16 ) >> 5;
    weight_scale = equ_rect.width*equ_rect.height;
    cascade->window_area = weight_scale;

    cascade->p0 = sum_elem_ptr(*sum, equ_rect.y, equ_rect.x);
    cascade->p1 = sum_elem_ptr(*sum, equ_rect.y, equ_rect.x + equ_rect.width );
    cascade->p2 = sum_elem_ptr(*sum, equ_rect.y + equ_rect.height, equ_rect.x );
    cascade->p3 = sum_elem_ptr(*sum, equ_rect.y + equ_rect.height,
                                     equ_rect.x + equ_rect.width );

    cascade->pq0 = sqsum_elem_ptr(*sqsum, equ_rect.y, equ_rect.x);
    cascade->pq1 = sqsum_elem_ptr(*sqsum, equ_rect.y, equ_rect.x + equ_rect.width );
    cascade->pq2 = sqsum_elem_ptr(*sqsum, equ_rect.y + equ_rect.height, equ_rect.x );
    cascade->pq3 = sqsum_elem_ptr(*sqsum, equ_rect.y + equ_rect.height,
                                          equ_rect.x + equ_rect.width );

    /* init pointers in haar features according to real window size and
       given image pointers */
    {
#ifdef _OPENMP
    int max_threads = cvGetNumThreads();
    #pragma omp parallel for num_threads(max_threads), schedule(dynamic) 
#endif // _OPENMP
    for( i = 0; i < _cascade->count; i++ )
    {
        int j, k, l;
        for( j = 0; j < cascade->stage_classifier[i].count; j++ )
        {
            for( l = 0; l < cascade->stage_classifier[i].classifier[j].count; l++ )
            {
                CvHaarFeature* feature = 
                    &_cascade->stage_classifier[i].classifier[j].haar_feature[l];
                /* CvHidHaarClassifier* classifier =
                    cascade->stage_classifier[i].classifier + j; */
                CvHidHaarFeature* hidfeature = 
                    &cascade->stage_classifier[i].classifier[j].node[l].feature;
                int sum0 = 0, area0 = 0;
                CvRect r[3];

                int base_w = -1, base_h = -1;
                int new_base_w = 0, new_base_h = 0;
                int kx, ky;
                int flagx = 0, flagy = 0;
                int x0 = 0, y0 = 0;

                int nr;

                /* align blocks */
                for( k = 0; k < CV_HAAR_FEATURE_MAX; k++ )
                {
                    if( !hidfeature->rect[k].p0 )
                        break;

                    r[k] = feature->rect[k].r;
                    base_w = (int)CV_IMIN( (unsigned)base_w, (unsigned)(r[k].width-1) );
                    base_w = (int)CV_IMIN( (unsigned)base_w, (unsigned)(r[k].x - r[0].x-1) );
                    base_h = (int)CV_IMIN( (unsigned)base_h, (unsigned)(r[k].height-1) );
                    base_h = (int)CV_IMIN( (unsigned)base_h, (unsigned)(r[k].y - r[0].y-1) );

                }

                nr = k;


                base_w += 1;
                base_h += 1;
                kx = r[0].width / base_w;
                ky = r[0].height / base_h;

                if( kx <= 0 )
                {
                    flagx = 1;
                    new_base_w = (( r[0].width * scale32x+16)>>5 ) / kx;
                    x0 = ( r[0].x * scale32x+16 )>>5;
                }

                if( ky <= 0 )
                {
                    flagy = 1;
                    new_base_h = (( r[0].height * scale32x + 16)>>5) / ky;
                    y0 = ( r[0].y * scale32x + 16 ) >>5;
                }
        
                for( k = 0; k < nr; k++ )
                {
                    CvRect tr;
                    int correction_ratio;
            

                    if( flagx )
                    {
                        tr.x = (r[k].x - r[0].x) * new_base_w / base_w + x0;
                        tr.width = r[k].width * new_base_w / base_w;
                    }
                    else

                    {
                        tr.x = ( r[k].x * scale32x +16)>>5;
                        tr.width = ( r[k].width * scale32x+16 )>>5;
                    }


                    if( flagy )
                    {
                        tr.y = (r[k].y - r[0].y) * new_base_h / base_h + y0;
                        tr.height = r[k].height * new_base_h / base_h;
                    }
                    else

                    {
                        tr.y = ( r[k].y * scale32x + 16 )>>5;
                        tr.height = ( r[k].height * scale32x +16 )>>5;
                    }

                    correction_ratio = weight_scale * (!feature->tilted ? 1 : 2);

                    if( !feature->tilted )
                    {
                        hidfeature->rect[k].p0 = sum_elem_ptr(*sum, tr.y, tr.x);
                        hidfeature->rect[k].p1 = sum_elem_ptr(*sum, tr.y, tr.x + tr.width);
                        hidfeature->rect[k].p2 = sum_elem_ptr(*sum, tr.y + tr.height, tr.x);
                        hidfeature->rect[k].p3 = sum_elem_ptr(*sum, tr.y + tr.height, tr.x + tr.width);
                    }
                    else
                    {
                        hidfeature->rect[k].p2 = sum_elem_ptr(*tilted, tr.y + tr.width, tr.x + tr.width);
                        hidfeature->rect[k].p3 = sum_elem_ptr(*tilted, tr.y + tr.width + tr.height,
                                                              tr.x + tr.width - tr.height);
                        hidfeature->rect[k].p0 = sum_elem_ptr(*tilted, tr.y, tr.x);
                        hidfeature->rect[k].p1 = sum_elem_ptr(*tilted, tr.y + tr.height, tr.x - tr.height);
                    }

                    hidfeature->rect[k].weight = ((feature->rect[k].weight)<< NODE_THRESHOLD_SHIFT)/(correction_ratio);

                    if( k == 0 )
                        area0 = tr.width * tr.height;
                    else
                        sum0 += hidfeature->rect[k].weight * tr.width * tr.height;
                }

                hidfeature->rect[0].weight = (int)(-sum0/area0);
            } /* l */
        } /* j */
    }
    }

    __END__;
}


#if 0 // 64bits version, for node 28bits shift

CV_INLINE
int icvEvalHidHaarClassifier( CvHidHaarClassifier* classifier,
                                 int64 variance_norm_factor,
                                 size_t p_offset )
{
    int idx = 0;
    do 
    {
        CvHidHaarTreeNode* node = classifier->node + idx;
        int64 t = node->threshold * variance_norm_factor;

        int64 sum = ((int64)calc_sum(node->feature.rect[0],p_offset)) * (int64)(node->feature.rect[0].weight);
        sum += ((int64)calc_sum(node->feature.rect[1],p_offset)) * (int64)(node->feature.rect[1].weight);

        if( node->feature.rect[2].p0 )
            sum +=  ((int64)calc_sum(node->feature.rect[2],p_offset)) *  (int64)(node->feature.rect[2].weight);

        idx = (sum < t )? node->left : node->right; 
    }
    while( idx > 0 );
    return classifier->alpha[-idx];
}

#endif

#if 1 // 32bits version, for node 22bits shift
CV_INLINE 
int icvEvalHidHaarClassifier( CvHidHaarClassifier* classifier,
                                 int variance_norm_factor,
                                 size_t p_offset )
{
    int idx = 0;
    do 
    {
        CvHidHaarTreeNode* node = classifier->node + idx;
        int t = node->threshold * variance_norm_factor;

        int sum = (calc_sum(node->feature.rect[0],p_offset)) * (node->feature.rect[0].weight);
        sum += (calc_sum(node->feature.rect[1],p_offset)) * (node->feature.rect[1].weight);

        if( node->feature.rect[2].p0 )
            sum +=  (calc_sum(node->feature.rect[2],p_offset)) *  (node->feature.rect[2].weight);

        idx = (sum < t )? node->left : node->right; 
    }
    while( idx > 0 );
    return classifier->alpha[-idx];
}

#endif

CV_IMPL int
cvRunHaarClassifierCascade( CvHaarClassifierCascade* _cascade,
                            CvPoint pt, int start_stage )
{
    int result = -1;
    CV_FUNCNAME("cvRunHaarClassifierCascade");

    __BEGIN__;

    int p_offset, pq_offset;
    int i, j;
    int64 rectsum, variance_factor;
    int variance_norm_factor;
    CvHidHaarClassifierCascade* cascade;

    if( !CV_IS_HAAR_CLASSIFIER(_cascade) )
        CV_ERROR( !_cascade ? CV_StsNullPtr : CV_StsBadArg, "Invalid cascade pointer" );

    cascade = _cascade->hid_cascade;
    if( !cascade )
        CV_ERROR( CV_StsNullPtr, "Hidden cascade has not been created.\n"
            "Use cvSetImagesForHaarClassifierCascade" );

    if( pt.x < 0 || pt.y < 0 ||
        pt.x + _cascade->real_window_size.width >= cascade->sum.width-2 ||
        pt.y + _cascade->real_window_size.height >= cascade->sum.height-2 )
        EXIT;

    p_offset = pt.y * (cascade->sum.step/sizeof(sumtype)) + pt.x;
    pq_offset = pt.y * (cascade->sqsum.step/sizeof(sqsumtype)) + pt.x;
    rectsum = calc_sum(*cascade,p_offset);//*cascade->inv_window_area;
    variance_factor = cascade->pq0[pq_offset] - cascade->pq1[pq_offset] -
                           cascade->pq2[pq_offset] + cascade->pq3[pq_offset];
    variance_factor = (variance_factor - (rectsum*rectsum + (cascade->window_area>>1))/cascade->window_area + (cascade->window_area>>1))/cascade->window_area;
    variance_norm_factor = int(sqrt(float(variance_factor))+0.5f);

    if( variance_norm_factor < 0 )
        variance_norm_factor = 1;

    if( cascade->is_tree )
    {
        CvHidHaarStageClassifier* ptr;
        assert( start_stage == 0 );

        result = 1;
        ptr = cascade->stage_classifier;

        while( ptr )
        {
            int stage_sum = 0;

            for( j = 0; j < ptr->count; j++ )
            {
                stage_sum += icvEvalHidHaarClassifier( ptr->classifier + j,
                    variance_norm_factor, p_offset );
            }

            if( stage_sum >= ptr->threshold )
            {
                ptr = ptr->child;
            }
            else
            {
                while( ptr && ptr->next == NULL ) ptr = ptr->parent;
                if( ptr == NULL )
                {
                    result = 0;
                    EXIT;
                }
                ptr = ptr->next;
            }
        }
    }
    else if( cascade->is_stump_based )
    {
        for( i = start_stage; i < cascade->count; i++ )
        {
            double stage_sum = 0;

            if( cascade->stage_classifier[i].two_rects )
            {
                for( j = 0; j < cascade->stage_classifier[i].count; j++ )
                {
                    CvHidHaarClassifier* classifier = cascade->stage_classifier[i].classifier + j;
                    CvHidHaarTreeNode* node = classifier->node;
                    double sum, t = node->threshold*variance_norm_factor, a, b;

                    sum = calc_sum(node->feature.rect[0],p_offset) * node->feature.rect[0].weight;
                    sum += calc_sum(node->feature.rect[1],p_offset) * node->feature.rect[1].weight;

                    a = classifier->alpha[0];
                    b = classifier->alpha[1];
                    stage_sum += sum < t ? a : b;
                }
            }
            else
            {
                for( j = 0; j < cascade->stage_classifier[i].count; j++ )
                {
                    CvHidHaarClassifier* classifier = cascade->stage_classifier[i].classifier + j;
                    CvHidHaarTreeNode* node = classifier->node;
                    double sum, t = node->threshold*variance_norm_factor, a, b;

                    sum = calc_sum(node->feature.rect[0],p_offset) * node->feature.rect[0].weight;
                    sum += calc_sum(node->feature.rect[1],p_offset) * node->feature.rect[1].weight;

                    if( node->feature.rect[2].p0 )
                        sum += calc_sum(node->feature.rect[2],p_offset) * node->feature.rect[2].weight;

                    a = classifier->alpha[0];
                    b = classifier->alpha[1];
                    stage_sum += sum < t ? a : b;
                }
            }

            if( stage_sum < cascade->stage_classifier[i].threshold )
            {
                result = -i;
                EXIT;
            }
        }
    }
    else
    {
        for( i = start_stage; i < cascade->count; i++ )
        {
            int stage_sum = 0;

            for( j = 0; j < cascade->stage_classifier[i].count; j++ )
            {
                stage_sum += icvEvalHidHaarClassifier(
                    cascade->stage_classifier[i].classifier + j,
                    variance_norm_factor, p_offset );
            }

            if( stage_sum < cascade->stage_classifier[i].threshold )
            {
                result = -i;
                EXIT;
            }
        }
    }

    result = 1;

    __END__;

    return result;
}


static int is_equal( const void* _r1, const void* _r2, void* )
{
    const CvRect* r1 = (const CvRect*)_r1;
    const CvRect* r2 = (const CvRect*)_r2;
	int distance5x = r1->width ;//int distance = cvRound(r1->width*0.2);

    return r2->x*5 <= r1->x*5 + distance5x &&
           r2->x*5 >= r1->x*5 - distance5x &&
           r2->y*5 <= r1->y*5 + distance5x &&
           r2->y*5 >= r1->y*5 - distance5x &&
           r2->width*5 <= r1->width * 6 &&
           r2->width * 6 >= r1->width*5;
}


CV_IMPL CvSeq*
cvHaarDetectObjects( const CvArr* _img,
                     CvHaarClassifierCascade* cascade,
                     CvMemStorage* storage, int scale_factor32x,
                     int min_neighbors, int flags, CvSize min_size )
{
    int split_stage = 2;

    CvMat stub, *img = (CvMat*)_img;
    CvMat *temp = 0, *sum = 0, *tilted = 0, *sqsum = 0, *norm_img = 0, *img_small = 0;
    CvSeq* seq = 0;
    CvSeq* seq2 = 0;
    CvSeq* idx_seq = 0;
    CvSeq* result_seq = 0;
    CvMemStorage* temp_storage = 0;
    CvAvgComp* comps = 0;
    int i;
    
#ifdef _OPENMP
    CvSeq* seq_thread[CV_MAX_THREADS] = {0};
    int max_threads = 0;
#endif
    
    CV_FUNCNAME( "cvHaarDetectObjects" );

    __BEGIN__;

    int factor32x;
    int npass = 2, coi;

    if( !CV_IS_HAAR_CLASSIFIER(cascade) )
        CV_ERROR( !cascade ? CV_StsNullPtr : CV_StsBadArg, "Invalid classifier cascade" );

    if( !storage )
        CV_ERROR( CV_StsNullPtr, "Null storage pointer" );

    CV_CALL( img = cvGetMat( img, &stub, &coi ));
    if( coi )
        CV_ERROR( CV_BadCOI, "COI is not supported" );

    if( CV_MAT_DEPTH(img->type) != CV_8U )
        CV_ERROR( CV_StsUnsupportedFormat, "Only 8-bit images are supported" );

	//added by Shiqi Yu
	//only gray images are supported.        
    if( CV_MAT_CN(img->type) > 1 )
    	CV_ERROR( CV_StsUnsupportedFormat, "Only single-channel images are supported" );
    	

    CV_CALL( temp = cvCreateMat( img->rows, img->cols, CV_8UC1 ));
    CV_CALL( sum = cvCreateMat( img->rows + 1, img->cols + 1, CV_32SC1 ));
    CV_CALL( sqsum = cvCreateMat( img->rows + 1, img->cols + 1, CV_64FC1 ));
    CV_CALL( temp_storage = cvCreateChildMemStorage( storage ));

#ifdef _OPENMP
    max_threads = cvGetNumThreads();
    for( i = 0; i < max_threads; i++ )
    {
        CvMemStorage* temp_storage_thread;
        CV_CALL( temp_storage_thread = cvCreateMemStorage(0));
        CV_CALL( seq_thread[i] = cvCreateSeq( 0, sizeof(CvSeq),
                        sizeof(CvRect), temp_storage_thread ));
    }
#endif

    if( !cascade->hid_cascade )
        CV_CALL( icvCreateHidHaarClassifierCascade(cascade) );

    if( cascade->hid_cascade->has_tilted_features )
        tilted = cvCreateMat( img->rows + 1, img->cols + 1, CV_32SC1 );

    seq = cvCreateSeq( 0, sizeof(CvSeq), sizeof(CvRect), temp_storage );
    seq2 = cvCreateSeq( 0, sizeof(CvSeq), sizeof(CvAvgComp), temp_storage );
    result_seq = cvCreateSeq( 0, sizeof(CvSeq), sizeof(CvAvgComp), storage );

    if( min_neighbors == 0 )
        seq = result_seq;

    {
        cvIntegral( img, sum, sqsum, tilted );
    
    
        if( (unsigned)split_stage >= (unsigned)cascade->count ||
            cascade->hid_cascade->is_tree )
        {
            split_stage = cascade->count;
            npass = 1;
        }

        for( factor32x = 32; factor32x*cascade->orig_window_size.width < (img->cols - 10)<<5 &&
                         factor32x*cascade->orig_window_size.height < (img->rows - 10)<<5;
             factor32x = (factor32x*scale_factor32x+16)>>5 )
        {
            const int ystep32x = MAX( 64, factor32x );
            CvSize win_size = { ( cascade->orig_window_size.width * factor32x + 16 )>>5,
                                ( cascade->orig_window_size.height * factor32x + 16)>>5 };
            //CvRect equ_rect = { 0, 0, 0, 0 };
            //int *p0 = 0, *p1 = 0, *p2 = 0, *p3 = 0;
            //int *pq0 = 0, *pq1 = 0, *pq2 = 0, *pq3 = 0;
            int pass, stage_offset = 0;
            int stop_height =  ( ((img->rows - win_size.height)<<5)+ (ystep32x>>1) ) / ystep32x;

            if( win_size.width < min_size.width || win_size.height < min_size.height )
                continue;

            cvSetImagesForHaarClassifierCascade( cascade, sum, sqsum, tilted, factor32x );
            cvZero( temp );

            cascade->hid_cascade->count = split_stage;

            for( pass = 0; pass < npass; pass++ )
            {
#ifdef _OPENMP
    #pragma omp parallel for num_threads(max_threads), schedule(dynamic)
#endif
                for( int _iy = 0; _iy < stop_height; _iy++ )
                {
                    int iy = (_iy*ystep32x+16)>>5;
                    int _ix, _xstep = 1;
                    int stop_width =( ((img->cols - win_size.width)<<5) +ystep32x/2) / ystep32x;
                    uchar* mask_row = temp->data.ptr + temp->step * iy;

                    for( _ix = 0; _ix < stop_width; _ix += _xstep )
                    {
                        int ix = (_ix*ystep32x+16)>>5; // it really should be ystep
                    
                        if( pass == 0 )
                        {
                            int result;
                            _xstep = 2;

                            result = cvRunHaarClassifierCascade( cascade, cvPoint(ix,iy), 0 );
                            if( result > 0 )
                            {
                                if( pass < npass - 1 )
                                    mask_row[ix] = 1;
                                else
                                {
                                    CvRect rect = cvRect(ix,iy,win_size.width,win_size.height);
#ifndef _OPENMP
                                    cvSeqPush( seq, &rect );
#else
                                    cvSeqPush( seq_thread[omp_get_thread_num()], &rect );
#endif
                                }
                            }
                            if( result < 0 )
                                _xstep = 1;
                        }
                        else if( mask_row[ix] )
                        {
                            int result = cvRunHaarClassifierCascade( cascade, cvPoint(ix,iy),
                                                                     stage_offset );
                            if( result > 0 )
                            {
                                if( pass == npass - 1 )
                                {
                                    CvRect rect = cvRect(ix,iy,win_size.width,win_size.height);
#ifndef _OPENMP
                                    cvSeqPush( seq, &rect );
#else
                                    cvSeqPush( seq_thread[omp_get_thread_num()], &rect );
#endif
                                }
                            }
                            else
                                mask_row[ix] = 0;
                        }
                    }
                }
                stage_offset = cascade->hid_cascade->count;
                cascade->hid_cascade->count = cascade->count;
            }
        }
    }

#ifdef _OPENMP
	// gather the results
	for( i = 0; i < max_threads; i++ )
	{
		CvSeq* s = seq_thread[i];
        int j, total = s->total;
        CvSeqBlock* b = s->first;
        for( j = 0; j < total; j += b->count, b = b->next )
            cvSeqPushMulti( seq, b->data, b->count );
	}
#endif

    if( min_neighbors != 0 )
    {
        // group retrieved rectangles in order to filter out noise 
        int ncomp = cvSeqPartition( seq, 0, &idx_seq, (CvCmpFunc)is_equal, 0 );
        CV_CALL( comps = (CvAvgComp*)cvAlloc( (ncomp+1)*sizeof(comps[0])));
        memset( comps, 0, (ncomp+1)*sizeof(comps[0]));

        // count number of neighbors
        for( i = 0; i < seq->total; i++ )
        {
            CvRect r1 = *(CvRect*)cvGetSeqElem( seq, i );
            int idx = *(int*)cvGetSeqElem( idx_seq, i );
            assert( (unsigned)idx < (unsigned)ncomp );

            comps[idx].neighbors++;
             
            comps[idx].rect.x += r1.x;
            comps[idx].rect.y += r1.y;
            comps[idx].rect.width += r1.width;
            comps[idx].rect.height += r1.height;
        }

        // calculate average bounding box
        for( i = 0; i < ncomp; i++ )
        {
            int n = comps[i].neighbors;
            if( n >= min_neighbors )
            {
                CvAvgComp comp;
                comp.rect.x = (comps[i].rect.x*2 + n)/(2*n);
                comp.rect.y = (comps[i].rect.y*2 + n)/(2*n);
                comp.rect.width = (comps[i].rect.width*2 + n)/(2*n);
                comp.rect.height = (comps[i].rect.height*2 + n)/(2*n);
                comp.neighbors = comps[i].neighbors;

                cvSeqPush( seq2, &comp );
            }
        }

        // filter out small face rectangles inside large face rectangles
        for( i = 0; i < seq2->total; i++ )
        {
            CvAvgComp r1 = *(CvAvgComp*)cvGetSeqElem( seq2, i );
            int j, flag = 1;

            for( j = 0; j < seq2->total; j++ )
            {
                CvAvgComp r2 = *(CvAvgComp*)cvGetSeqElem( seq2, j );
                int distance = (r2.rect.width *2+5)/10;//cvRound( r2.rect.width * 0.2 );
            
                if( i != j &&
                    r1.rect.x >= r2.rect.x - distance &&
                    r1.rect.y >= r2.rect.y - distance &&
                    r1.rect.x + r1.rect.width <= r2.rect.x + r2.rect.width + distance &&
                    r1.rect.y + r1.rect.height <= r2.rect.y + r2.rect.height + distance &&
                    (r2.neighbors > MAX( 3, r1.neighbors ) || r1.neighbors < 3) )
                {
                    flag = 0;
                    break;
                }
            }

            if( flag )
            {
                cvSeqPush( result_seq, &r1 );
                /* cvSeqPush( result_seq, &r1.rect ); */
            }
        }
    }

    __END__;

#ifdef _OPENMP
	for( i = 0; i < max_threads; i++ )
	{
		if( seq_thread[i] )
            cvReleaseMemStorage( &seq_thread[i]->storage );
	}
#endif

    cvReleaseMemStorage( &temp_storage );
    cvReleaseMat( &sum );
    cvReleaseMat( &sqsum );
    cvReleaseMat( &tilted );
    cvReleaseMat( &temp );
    cvReleaseMat( &norm_img );
    cvReleaseMat( &img_small );
    cvFree( &comps );

    return result_seq;
}


CV_IMPL void
cvReleaseHaarClassifierCascade( CvHaarClassifierCascade** _cascade )
{
    if( _cascade && *_cascade )
    {
        int i, j;
        CvHaarClassifierCascade* cascade = *_cascade;

        for( i = 0; i < cascade->count; i++ )
        {
            for( j = 0; j < cascade->stage_classifier[i].count; j++ )
                cvFree( &cascade->stage_classifier[i].classifier[j].haar_feature );
            cvFree( &cascade->stage_classifier[i].classifier );
        }
        icvReleaseHidHaarClassifierCascade( &cascade->hid_cascade );
        cvFree( _cascade );
    }
}



/* End of file. */
