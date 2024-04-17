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


/* ////////////////////////////////////////////////////////////////////
//
//  CvMat, CvMatND, CvSparceMat and IplImage support functions
//  (creation, deletion, copying, retrieving and setting elements etc.)
//
// */

#include "_cxcore.h"

/*
// Makes the library use native IPL image allocators
CV_IMPL void
cvSetIPLAllocators( Cv_iplCreateImageHeader createHeader,
                    Cv_iplAllocateImageData allocateData,
                    Cv_iplDeallocate deallocate,
                    Cv_iplCreateROI createROI,
                    Cv_iplCloneImage cloneImage )
{
    CV_FUNCNAME( "cvSetIPLAllocators" );

    __BEGIN__;

    if( !createHeader || !allocateData || !deallocate || !createROI || !cloneImage )
    {
        if( createHeader || allocateData || deallocate || createROI || cloneImage )
            CV_ERROR( CV_StsBadArg, "Either all the pointers should be null or "
                                    "they all should be non-null" );
    }

    CvIPL.createHeader = createHeader;
    CvIPL.allocateData = allocateData;
    CvIPL.deallocate = deallocate;
    CvIPL.createROI = createROI;
    CvIPL.cloneImage = cloneImage;

    __END__;
}
*/

/****************************************************************************************\
*                               CvMat creation and basic operations                      *
\****************************************************************************************/

// Creates CvMat and underlying data
CV_IMPL CvMat*
cvCreateMat( int height, int width, int type )
{
    CvMat* arr = 0;

    CV_FUNCNAME( "cvCreateMat" );
    
    __BEGIN__;

    CV_CALL( arr = cvCreateMatHeader( height, width, type ));
    CV_CALL( cvCreateData( arr ));

    __END__;

    if( cvGetErrStatus() < 0 )
        cvReleaseMat( &arr );

    return arr;
}


static void icvCheckHuge( CvMat* arr )
{
    if( (int64)arr->step*arr->rows > INT_MAX )
        arr->type &= ~CV_MAT_CONT_FLAG;
}

// Creates CvMat header only
CV_IMPL CvMat*
cvCreateMatHeader( int rows, int cols, int type )
{
    CvMat* arr = 0;
    
    CV_FUNCNAME( "cvCreateMatHeader" );

    __BEGIN__;

    int min_step;
    type = CV_MAT_TYPE(type);

    if( rows <= 0 || cols <= 0 )
        CV_ERROR( CV_StsBadSize, "Non-positive width or height" );

    min_step = CV_ELEM_SIZE(type)*cols;
    if( min_step <= 0 )
        CV_ERROR( CV_StsUnsupportedFormat, "Invalid matrix type" );

    CV_CALL( arr = (CvMat*)cvAlloc( sizeof(*arr)));

    arr->step = rows == 1 ? 0 : cvAlign(min_step, CV_DEFAULT_MAT_ROW_ALIGN);
    arr->type = CV_MAT_MAGIC_VAL | type |
                (arr->step == 0 || arr->step == min_step ? CV_MAT_CONT_FLAG : 0);
    arr->rows = rows;
    arr->cols = cols;
    arr->data.ptr = 0;
    arr->refcount = 0;
    arr->hdr_refcount = 1;

    icvCheckHuge( arr );

    __END__;

    if( cvGetErrStatus() < 0 )
        cvReleaseMat( &arr );

    return arr;
}


// Initializes CvMat header, allocated by the user
CV_IMPL CvMat*
cvInitMatHeader( CvMat* arr, int rows, int cols,
                 int type, void* data, int step )
{
    CV_FUNCNAME( "cvInitMatHeader" );
    
    __BEGIN__;

    int mask, pix_size, min_step;

    if( !arr )
        CV_ERROR_FROM_CODE( CV_StsNullPtr );

    if( (unsigned)CV_MAT_DEPTH(type) > CV_DEPTH_MAX )
        CV_ERROR_FROM_CODE( CV_BadNumChannels );

    if( rows <= 0 || cols <= 0 )
        CV_ERROR( CV_StsBadSize, "Non-positive cols or rows" );
 
    type = CV_MAT_TYPE( type );
    arr->type = type | CV_MAT_MAGIC_VAL;
    arr->rows = rows;
    arr->cols = cols;
    arr->data.ptr = (uchar*)data;
    arr->refcount = 0;
    arr->hdr_refcount = 0;

    mask = (arr->rows <= 1) - 1;
    pix_size = CV_ELEM_SIZE(type);
    min_step = arr->cols*pix_size & mask;

    if( step != CV_AUTOSTEP && step != 0 )
    {
        if( step < min_step )
            CV_ERROR_FROM_CODE( CV_BadStep );
        arr->step = step & mask;
    }
    else
    {
        arr->step = min_step;
    }

    arr->type = CV_MAT_MAGIC_VAL | type |
                (arr->step == min_step ? CV_MAT_CONT_FLAG : 0);

    icvCheckHuge( arr );

    __END__;

    return arr;
}


// Deallocates the CvMat structure and underlying data
CV_IMPL void
cvReleaseMat( CvMat** array )
{
    CV_FUNCNAME( "cvReleaseMat" );
    
    __BEGIN__;

    if( !array )
        CV_ERROR_FROM_CODE( CV_HeaderIsNull );

    if( *array )
    {
        CvMat* arr = *array;
        
        if( !CV_IS_MAT_HDR(arr) && !CV_IS_MATND_HDR(arr) )
            CV_ERROR_FROM_CODE( CV_StsBadFlag );

        *array = 0;

        cvDecRefData( arr );
        cvFree( &arr );
    }

    __END__;
}


// Creates a copy of matrix
CV_IMPL CvMat*
cvCloneMat( const CvMat* src )
{
    CvMat* dst = 0;
    CV_FUNCNAME( "cvCloneMat" );

    __BEGIN__;

    if( !CV_IS_MAT_HDR( src ))
        CV_ERROR( CV_StsBadArg, "Bad CvMat header" );

    CV_CALL( dst = cvCreateMatHeader( src->rows, src->cols, src->type ));

    if( src->data.ptr )
    {
        CV_CALL( cvCreateData( dst ));
        CV_CALL( cvCopy( src, dst ));
    }

    __END__;

    return dst;
}


/****************************************************************************************\
*                               CvMatND creation and basic operations                    *
\****************************************************************************************/

CV_IMPL CvMatND*
cvInitMatNDHeader( CvMatND* mat, int dims, const int* sizes,
                    int type, void* data )
{
    CvMatND* result = 0;

    CV_FUNCNAME( "cvInitMatNDHeader" );

    __BEGIN__;

    type = CV_MAT_TYPE(type);
    int i;
    int64 step = CV_ELEM_SIZE(type);

    if( !mat )
        CV_ERROR( CV_StsNullPtr, "NULL matrix header pointer" );

    if( step == 0 )
        CV_ERROR( CV_StsUnsupportedFormat, "invalid array data type" );

    if( !sizes )
        CV_ERROR( CV_StsNullPtr, "NULL <sizes> pointer" );

    if( dims <= 0 || dims > CV_MAX_DIM )
        CV_ERROR( CV_StsOutOfRange,
        "non-positive or too large number of dimensions" );

    for( i = dims - 1; i >= 0; i-- )
    {
        if( sizes[i] <= 0 )
            CV_ERROR( CV_StsBadSize, "one of dimesion sizes is non-positive" );
        mat->dim[i].size = sizes[i];
        if( step > INT_MAX )
            CV_ERROR( CV_StsOutOfRange, "The array is too big" );
        mat->dim[i].step = (int)step;
        step *= sizes[i];
    }

    mat->type = CV_MATND_MAGIC_VAL | (step <= INT_MAX ? CV_MAT_CONT_FLAG : 0) | type;
    mat->dims = dims;
    mat->data.ptr = (uchar*)data;
    mat->refcount = 0;
    mat->hdr_refcount = 0;
    result = mat;

    __END__;

    if( cvGetErrStatus() < 0 && mat )
    {
        mat->type = 0;
        mat->data.ptr = 0;
    }

    return result;
}


// Creates CvMatND and underlying data
CV_IMPL CvMatND*
cvCreateMatND( int dims, const int* sizes, int type )
{
    CvMatND* arr = 0;

    CV_FUNCNAME( "cvCreateMatND" );
    
    __BEGIN__;

    CV_CALL( arr = cvCreateMatNDHeader( dims, sizes, type ));
    CV_CALL( cvCreateData( arr ));

    __END__;

    if( cvGetErrStatus() < 0 )
        cvReleaseMatND( &arr );

    return arr;
}


// Creates CvMatND header only
CV_IMPL CvMatND*
cvCreateMatNDHeader( int dims, const int* sizes, int type )
{
    CvMatND* arr = 0;
    
    CV_FUNCNAME( "cvCreateMatNDHeader" );

    __BEGIN__;

    if( dims <= 0 || dims > CV_MAX_DIM )
        CV_ERROR( CV_StsOutOfRange,
        "non-positive or too large number of dimensions" );

    CV_CALL( arr = (CvMatND*)cvAlloc( sizeof(*arr) ));
    
    CV_CALL( cvInitMatNDHeader( arr, dims, sizes, type, 0 ));
    arr->hdr_refcount = 1;

    __END__;

    if( cvGetErrStatus() < 0 )
        cvReleaseMatND( &arr );

    return arr;
}


// Creates a copy of nD array
CV_IMPL CvMatND*
cvCloneMatND( const CvMatND* src )
{
    CvMatND* dst = 0;
    CV_FUNCNAME( "cvCloneMatND" );

    __BEGIN__;

    int i, *sizes;

    if( !CV_IS_MATND_HDR( src ))
        CV_ERROR( CV_StsBadArg, "Bad CvMatND header" );

    sizes = (int*)cvAlloc( src->dims*sizeof(sizes[0]) );

    for( i = 0; i < src->dims; i++ )
        sizes[i] = src->dim[i].size;

    CV_CALL( dst = cvCreateMatNDHeader( src->dims, sizes, src->type ));

    if( src->data.ptr )
    {
        CV_CALL( cvCreateData( dst ));
        CV_CALL( cvCopy( src, dst ));
    }

	cvFree(&sizes);
    __END__;

    return dst;
}


static CvMatND*
cvGetMatND( const CvArr* arr, CvMatND* matnd, int* coi )
{
    CvMatND* result = 0;
    
    CV_FUNCNAME( "cvGetMatND" );

    __BEGIN__;

    if( coi )
        *coi = 0;

    if( !matnd || !arr )
        CV_ERROR( CV_StsNullPtr, "NULL array pointer is passed" );

    if( CV_IS_MATND_HDR(arr))
    {
        if( !((CvMatND*)arr)->data.ptr )
            CV_ERROR( CV_StsNullPtr, "The matrix has NULL data pointer" );
        
        result = (CvMatND*)arr;
    }
    else
    {
        CvMat stub, *mat = (CvMat*)arr;
        
        if( CV_IS_IMAGE_HDR( mat ))
            CV_CALL( mat = cvGetMat( mat, &stub, coi ));

        if( !CV_IS_MAT_HDR( mat ))
            CV_ERROR( CV_StsBadArg, "Unrecognized or unsupported array type" );
        
        if( !mat->data.ptr )
            CV_ERROR( CV_StsNullPtr, "Input array has NULL data pointer" );

        matnd->data.ptr = mat->data.ptr;
        matnd->refcount = 0;
        matnd->hdr_refcount = 0;
        matnd->type = mat->type;
        matnd->dims = 2;
        matnd->dim[0].size = mat->rows;
        matnd->dim[0].step = mat->step;
        matnd->dim[1].size = mat->cols;
        matnd->dim[1].step = CV_ELEM_SIZE(mat->type);
        result = matnd;
    }

    __END__;

    return result;
}


// returns number of dimensions to iterate.
/*
Checks whether <count> arrays have equal type, sizes (mask is optional array
that needs to have the same size, but 8uC1 or 8sC1 type).
Returns number of dimensions to iterate through:
0 means that all arrays are continuous,
1 means that all arrays are vectors of continuous arrays etc.
and the size of largest common continuous part of the arrays 
*/
CV_IMPL int
cvInitNArrayIterator( int count, CvArr** arrs,
                      const CvArr* mask, CvMatND* stubs,
                      CvNArrayIterator* iterator, int flags )
{
    int dims = -1;

    CV_FUNCNAME( "cvInitArrayOp" );
    
    __BEGIN__;

    int i, j, size, dim0 = -1;
    int64 step;
    CvMatND* hdr0 = 0;

    if( count < 1 || count > CV_MAX_ARR )
        CV_ERROR( CV_StsOutOfRange, "Incorrect number of arrays" );

    if( !arrs || !stubs )
        CV_ERROR( CV_StsNullPtr, "Some of required array pointers is NULL" );

    if( !iterator )
        CV_ERROR( CV_StsNullPtr, "Iterator pointer is NULL" );

    for( i = 0; i <= count; i++ )
    {
        const CvArr* arr = i < count ? arrs[i] : mask;
        CvMatND* hdr;
        
        if( !arr )
        {
            if( i < count )
                CV_ERROR( CV_StsNullPtr, "Some of required array pointers is NULL" );
            break;
        }

        if( CV_IS_MATND( arr ))
            hdr = (CvMatND*)arr;
        else
        {
            int coi = 0;
            CV_CALL( hdr = cvGetMatND( arr, stubs + i, &coi ));
            if( coi != 0 )
                CV_ERROR( CV_BadCOI, "COI set is not allowed here" );
        }

        iterator->hdr[i] = hdr;

        if( i > 0 )
        {
            if( hdr->dims != hdr0->dims )
                CV_ERROR( CV_StsUnmatchedSizes,
                          "Number of dimensions is the same for all arrays" );
            
            if( i < count )
            {
                switch( flags & (CV_NO_DEPTH_CHECK|CV_NO_CN_CHECK))
                {
                case 0:
                    if( !CV_ARE_TYPES_EQ( hdr, hdr0 ))
                        CV_ERROR( CV_StsUnmatchedFormats,
                                  "Data type is not the same for all arrays" );
                    break;
                case CV_NO_DEPTH_CHECK:
                    if( !CV_ARE_CNS_EQ( hdr, hdr0 ))
                        CV_ERROR( CV_StsUnmatchedFormats,
                                  "Number of channels is not the same for all arrays" );
                    break;
                case CV_NO_CN_CHECK:
                    if( !CV_ARE_CNS_EQ( hdr, hdr0 ))
                        CV_ERROR( CV_StsUnmatchedFormats,
                                  "Depth is not the same for all arrays" );
                    break;
                }
            }
            else
            {
                if( !CV_IS_MASK_ARR( hdr ))
                    CV_ERROR( CV_StsBadMask, "Mask should have 8uC1 or 8sC1 data type" );
            }

            if( !(flags & CV_NO_SIZE_CHECK) )
            {
                for( j = 0; j < hdr->dims; j++ )
                    if( hdr->dim[j].size != hdr0->dim[j].size )
                        CV_ERROR( CV_StsUnmatchedSizes,
                                  "Dimension sizes are the same for all arrays" );
            }
        }
        else
            hdr0 = hdr;

        step = CV_ELEM_SIZE(hdr->type);
        for( j = hdr->dims - 1; j > dim0; j-- )
        {
            if( step != hdr->dim[j].step )
                break;
            step *= hdr->dim[j].size;
        }

        if( j == dim0 && step > INT_MAX )
            j++;

        if( j > dim0 )
            dim0 = j;

        iterator->hdr[i] = (CvMatND*)hdr;
        iterator->ptr[i] = (uchar*)hdr->data.ptr;
    }

    size = 1;
    for( j = hdr0->dims - 1; j > dim0; j-- )
        size *= hdr0->dim[j].size;

    dims = dim0 + 1;
    iterator->dims = dims;
    iterator->count = count;
    iterator->size = cvSize(size,1);

    for( i = 0; i < dims; i++ )
        iterator->stack[i] = hdr0->dim[i].size;

    __END__;

    return dims;
}


// returns zero value if iteration is finished, non-zero otherwise
CV_IMPL  int  cvNextNArraySlice( CvNArrayIterator* iterator )
{
    assert( iterator != 0 );
    int i, dims, size = 0;

    for( dims = iterator->dims; dims > 0; dims-- )
    {
        for( i = 0; i < iterator->count; i++ )
            iterator->ptr[i] += iterator->hdr[i]->dim[dims-1].step;

        if( --iterator->stack[dims-1] > 0 )
            break;

        size = iterator->hdr[0]->dim[dims-1].size;

        for( i = 0; i < iterator->count; i++ )
            iterator->ptr[i] -= (size_t)size*iterator->hdr[i]->dim[dims-1].step;

        iterator->stack[dims-1] = size;
    }

    return dims > 0;
}


/****************************************************************************************\
*                            CvSparseMat creation and basic operations                   *
\****************************************************************************************/


// Creates CvMatND and underlying data
CV_IMPL CvSparseMat*
cvCreateSparseMat( int dims, const int* sizes, int type )
{
    CvSparseMat* arr = 0;

    CV_FUNCNAME( "cvCreateSparseMat" );
    
    __BEGIN__;

    type = CV_MAT_TYPE( type );
    int pix_size1 = CV_ELEM_SIZE1(type);
    int pix_size = pix_size1*CV_MAT_CN(type);
    int i, size;
    CvMemStorage* storage;

    if( pix_size == 0 )
        CV_ERROR( CV_StsUnsupportedFormat, "invalid array data type" );

    if( dims <= 0 || dims > CV_MAX_DIM_HEAP )
        CV_ERROR( CV_StsOutOfRange, "bad number of dimensions" );

    if( !sizes )
        CV_ERROR( CV_StsNullPtr, "NULL <sizes> pointer" );

    for( i = 0; i < dims; i++ )
    {
        if( sizes[i] <= 0 )
            CV_ERROR( CV_StsBadSize, "one of dimesion sizes is non-positive" );
    }

    CV_CALL( arr = (CvSparseMat*)cvAlloc(sizeof(*arr)+MAX(0,dims-CV_MAX_DIM)*sizeof(arr->size[0])));

    arr->type = CV_SPARSE_MAT_MAGIC_VAL | type;
    arr->dims = dims;
    arr->refcount = 0;
    arr->hdr_refcount = 1;
    memcpy( arr->size, sizes, dims*sizeof(sizes[0]));

    arr->valoffset = (int)cvAlign(sizeof(CvSparseNode), pix_size1);
    arr->idxoffset = (int)cvAlign(arr->valoffset + pix_size, sizeof(int));
    size = (int)cvAlign(arr->idxoffset + dims*sizeof(int), sizeof(CvSetElem));

    CV_CALL( storage = cvCreateMemStorage( CV_SPARSE_MAT_BLOCK ));
    CV_CALL( arr->heap = cvCreateSet( 0, sizeof(CvSet), size, storage ));

    arr->hashsize = CV_SPARSE_HASH_SIZE0;
    size = arr->hashsize*sizeof(arr->hashtable[0]);
    
    CV_CALL( arr->hashtable = (void**)cvAlloc( size ));
    memset( arr->hashtable, 0, size );

    __END__;

    if( cvGetErrStatus() < 0 )
        cvReleaseSparseMat( &arr );

    return arr;
}


// Creates CvMatND and underlying data
CV_IMPL void
cvReleaseSparseMat( CvSparseMat** array )
{
    CV_FUNCNAME( "cvReleaseSparseMat" );
    
    __BEGIN__;

    if( !array )
        CV_ERROR_FROM_CODE( CV_HeaderIsNull );

    if( *array )
    {
        CvSparseMat* arr = *array;
        
        if( !CV_IS_SPARSE_MAT_HDR(arr) )
            CV_ERROR_FROM_CODE( CV_StsBadFlag );

        *array = 0;

        cvReleaseMemStorage( &arr->heap->storage );
        cvFree( &arr->hashtable );
        cvFree( &arr );
    }

    __END__;
}


// Creates CvMatND and underlying data
CV_IMPL CvSparseMat*
cvCloneSparseMat( const CvSparseMat* src )
{
    CvSparseMat* dst = 0;
    
    CV_FUNCNAME( "cvCloneSparseMat" );
    
    __BEGIN__;

    if( !CV_IS_SPARSE_MAT_HDR(src) )
        CV_ERROR( CV_StsBadArg, "Invalid sparse array header" );

    CV_CALL( dst = cvCreateSparseMat( src->dims, src->size, src->type ));
    CV_CALL( cvCopy( src, dst )); 

    __END__;

    if( cvGetErrStatus() < 0 )
        cvReleaseSparseMat( &dst );
    
    return dst;
}


CvSparseNode*
cvInitSparseMatIterator( const CvSparseMat* mat, CvSparseMatIterator* iterator )
{
    CvSparseNode* node = 0;
    
    CV_FUNCNAME( "cvInitSparseMatIterator" );

    __BEGIN__;

    int idx;

    if( !CV_IS_SPARSE_MAT( mat ))
        CV_ERROR( CV_StsBadArg, "Invalid sparse matrix header" );

    if( !iterator )
        CV_ERROR( CV_StsNullPtr, "NULL iterator pointer" );

    iterator->mat = (CvSparseMat*)mat;
    iterator->node = 0;

    for( idx = 0; idx < mat->hashsize; idx++ )
        if( mat->hashtable[idx] )
        {
            node = iterator->node = (CvSparseNode*)mat->hashtable[idx];
            break;
        }

    iterator->curidx = idx;

    __END__;

    return node;
}

#define ICV_SPARSE_MAT_HASH_MULTIPLIER  33

static uchar*
icvGetNodePtr( CvSparseMat* mat, const int* idx, int* _type,
               int create_node, unsigned* precalc_hashval )
{
    uchar* ptr = 0;
    
    CV_FUNCNAME( "icvGetNodePtr" );

    __BEGIN__;

    int i, tabidx;
    unsigned hashval = 0;
    CvSparseNode *node;
    assert( CV_IS_SPARSE_MAT( mat ));

    if( !precalc_hashval )
    {
        for( i = 0; i < mat->dims; i++ )
        {
            int t = idx[i];
            if( (unsigned)t >= (unsigned)mat->size[i] )
                CV_ERROR( CV_StsOutOfRange, "One of indices is out of range" );
            hashval = hashval*ICV_SPARSE_MAT_HASH_MULTIPLIER + t;
        }
    }
    else
    {
        hashval = *precalc_hashval;
    }

    tabidx = hashval & (mat->hashsize - 1);
    hashval &= INT_MAX;

    for( node = (CvSparseNode*)mat->hashtable[tabidx];
         node != 0; node = node->next )
    {
        if( node->hashval == hashval )
        {
            int* nodeidx = CV_NODE_IDX(mat,node);
            for( i = 0; i < mat->dims; i++ )
                if( idx[i] != nodeidx[i] )
                    break;
            if( i == mat->dims )
            {
                ptr = (uchar*)CV_NODE_VAL(mat,node);
                break;
            }
        }
    }

    if( !ptr && create_node )
    {
        if( mat->heap->active_count >= mat->hashsize*CV_SPARSE_HASH_RATIO )
        {
            void** newtable;
            int newsize = MAX( mat->hashsize*2, CV_SPARSE_HASH_SIZE0);
            int newrawsize = newsize*sizeof(newtable[0]);
            
            CvSparseMatIterator iterator;
            assert( (newsize & (newsize - 1)) == 0 );

            // resize hash table
            CV_CALL( newtable = (void**)cvAlloc( newrawsize ));
            memset( newtable, 0, newrawsize );

            node = cvInitSparseMatIterator( mat, &iterator );
            while( node )
            {
                CvSparseNode* next = cvGetNextSparseNode( &iterator );
                int newidx = node->hashval & (newsize - 1);
                node->next = (CvSparseNode*)newtable[newidx];
                newtable[newidx] = node;
                node = next;
            }

            cvFree( &mat->hashtable );
            mat->hashtable = newtable;
            mat->hashsize = newsize;
            tabidx = hashval & (newsize - 1);
        }

        node = (CvSparseNode*)cvSetNew( mat->heap );
        node->hashval = hashval;
        node->next = (CvSparseNode*)mat->hashtable[tabidx];
        mat->hashtable[tabidx] = node;
        CV_MEMCPY_INT( CV_NODE_IDX(mat,node), idx, mat->dims );
        ptr = (uchar*)CV_NODE_VAL(mat,node);
        if( create_node > 0 )
            CV_ZERO_CHAR( ptr, CV_ELEM_SIZE(mat->type));
    }

    if( _type )
        *_type = CV_MAT_TYPE(mat->type);

    __END__;

    return ptr;
}


static void
icvDeleteNode( CvSparseMat* mat, const int* idx, unsigned* precalc_hashval )
{
    CV_FUNCNAME( "icvDeleteNode" );

    __BEGIN__;

    int i, tabidx;
    unsigned hashval = 0;
    CvSparseNode *node, *prev = 0;
    assert( CV_IS_SPARSE_MAT( mat ));

    if( !precalc_hashval )
    {
        for( i = 0; i < mat->dims; i++ )
        {
            int t = idx[i];
            if( (unsigned)t >= (unsigned)mat->size[i] )
                CV_ERROR( CV_StsOutOfRange, "One of indices is out of range" );
            hashval = hashval*ICV_SPARSE_MAT_HASH_MULTIPLIER + t;
        }
    }
    else
    {
        hashval = *precalc_hashval;
    }

    tabidx = hashval & (mat->hashsize - 1);
    hashval &= INT_MAX;

    for( node = (CvSparseNode*)mat->hashtable[tabidx];
         node != 0; prev = node, node = node->next )
    {
        if( node->hashval == hashval )
        {
            int* nodeidx = CV_NODE_IDX(mat,node);
            for( i = 0; i < mat->dims; i++ )
                if( idx[i] != nodeidx[i] )
                    break;
            if( i == mat->dims )
                break;
        }
    }

    if( node )
    {
        if( prev )
            prev->next = node->next;
        else
            mat->hashtable[tabidx] = node->next;
        cvSetRemoveByPtr( mat->heap, node );
    }

    __END__;
}



/****************************************************************************************\
*                          Common for multiple array types operations                    *
\****************************************************************************************/

// Allocates underlying array data
CV_IMPL void
cvCreateData( CvArr* arr )
{
    CV_FUNCNAME( "cvCreateData" );
    
    __BEGIN__;

    if( CV_IS_MAT_HDR( arr ))
    {
        size_t step, total_size;
        CvMat* mat = (CvMat*)arr;
        step = mat->step;

        if( mat->data.ptr != 0 )
            CV_ERROR( CV_StsError, "Data is already allocated" );

        if( step == 0 )
            step = CV_ELEM_SIZE(mat->type)*mat->cols;

        total_size = step*mat->rows + sizeof(int) + CV_MALLOC_ALIGN;
        CV_CALL( mat->refcount = (int*)cvAlloc( (size_t)total_size ));
        mat->data.ptr = (uchar*)cvAlignPtr( mat->refcount + 1, CV_MALLOC_ALIGN );
        *mat->refcount = 1;
    }
    else if( CV_IS_IMAGE_HDR(arr))
    {
        IplImage* img = (IplImage*)arr;

        if( img->imageData != 0 )
            CV_ERROR( CV_StsError, "Data is already allocated" );

        //if( !CvIPL.allocateData )
        {
            CV_CALL( img->imageData = img->imageDataOrigin = 
                        (char*)cvAlloc( (size_t)img->imageSize ));
        }
/*        else
        {
            int depth = img->depth;
            int width = img->width;

            if( img->depth == IPL_DEPTH_32F || img->nChannels == 64 )
            {
                img->width *= img->depth == IPL_DEPTH_32F ? sizeof(float) : sizeof(double);
                img->depth = IPL_DEPTH_8U;
            }

            CvIPL.allocateData( img, 0, 0 );

            img->width = width;
            img->depth = depth;
        }
*/
    }
    else if( CV_IS_MATND_HDR( arr ))
    {
        CvMatND* mat = (CvMatND*)arr;
        int i;
        size_t total_size = CV_ELEM_SIZE(mat->type);

        if( mat->data.ptr != 0 )
            CV_ERROR( CV_StsError, "Data is already allocated" );

        if( CV_IS_MAT_CONT( mat->type ))
        {
            total_size = (size_t)mat->dim[0].size*(mat->dim[0].step != 0 ?
                         mat->dim[0].step : total_size);
        }
        else
        {
            for( i = mat->dims - 1; i >= 0; i-- )
            {
                size_t size = (size_t)mat->dim[i].step*mat->dim[i].size;

                if( total_size < size )
                    total_size = size;
            }
        }
        
        CV_CALL( mat->refcount = (int*)cvAlloc( total_size +
                                        sizeof(int) + CV_MALLOC_ALIGN ));
        mat->data.ptr = (uchar*)cvAlignPtr( mat->refcount + 1, CV_MALLOC_ALIGN );
        *mat->refcount = 1;
    }
    else
    {
        CV_ERROR( CV_StsBadArg, "unrecognized or unsupported array type" );
    }

    __END__;
}


// Assigns external data to array
CV_IMPL void
cvSetData( CvArr* arr, void* data, int step )
{
    CV_FUNCNAME( "cvSetData" );

    __BEGIN__;

    int pix_size, min_step;

    if( CV_IS_MAT_HDR(arr) || CV_IS_MATND_HDR(arr) )
        cvReleaseData( arr );

    if( CV_IS_MAT_HDR( arr ))
    {
        CvMat* mat = (CvMat*)arr;
    
        int type = CV_MAT_TYPE(mat->type);
        pix_size = CV_ELEM_SIZE(type);
        min_step = mat->cols*pix_size & ((mat->rows <= 1) - 1);

        if( step != CV_AUTOSTEP )
        {
            if( step < min_step && data != 0 )
                CV_ERROR_FROM_CODE( CV_BadStep );
            mat->step = step & ((mat->rows <= 1) - 1);
        }
        else
        {
            mat->step = min_step;
        }

        mat->data.ptr = (uchar*)data;
        mat->type = CV_MAT_MAGIC_VAL | type |
                    (mat->step==min_step ? CV_MAT_CONT_FLAG : 0);
        icvCheckHuge( mat );
    }
    else if( CV_IS_IMAGE_HDR( arr ))
    {
        IplImage* img = (IplImage*)arr;
    
        pix_size = ((img->depth & 255) >> 3)*img->nChannels;
        min_step = img->width*pix_size;

        if( step != CV_AUTOSTEP && img->height > 1 )
        {
            if( step < min_step && data != 0 )
                CV_ERROR_FROM_CODE( CV_BadStep );
            img->widthStep = step;
        }
        else
        {
            img->widthStep = min_step;
        }

        img->imageSize = img->widthStep * img->height;
        img->imageData = img->imageDataOrigin = (char*)data;

        if( (((int)(size_t)data | step) & 7) == 0 &&
            cvAlign(img->width * pix_size, 8) == step )
        {
            img->align = 8;
        }
        else
        {
            img->align = 4;
        }
    }
    else if( CV_IS_MATND_HDR( arr ))
    {
        CvMatND* mat = (CvMatND*)arr;
        int i;
        int64 cur_step;
    
        if( step != CV_AUTOSTEP )
            CV_ERROR( CV_BadStep,
            "For multidimensional array only CV_AUTOSTEP is allowed here" );

        mat->data.ptr = (uchar*)data;
        cur_step = CV_ELEM_SIZE(mat->type);

        for( i = mat->dims - 1; i >= 0; i-- )
        {
            if( cur_step > INT_MAX )
                CV_ERROR( CV_StsOutOfRange, "The array is too big" );
            mat->dim[i].step = (int)cur_step;
            cur_step *= mat->dim[i].size;
        }
    }
    else
    {
        CV_ERROR( CV_StsBadArg, "unrecognized or unsupported array type" );
    }

    __END__;
}


// Deallocates array's data
CV_IMPL void
cvReleaseData( CvArr* arr )
{
    CV_FUNCNAME( "cvReleaseData" );
    
    __BEGIN__;

    if( CV_IS_MAT_HDR( arr ) || CV_IS_MATND_HDR( arr ))
    {
        CvMat* mat = (CvMat*)arr;
        cvDecRefData( mat );
    }
    else if( CV_IS_IMAGE_HDR( arr ))
    {
        IplImage* img = (IplImage*)arr;

        //if( !CvIPL.deallocate )
        {
            char* ptr = img->imageDataOrigin;
            img->imageData = img->imageDataOrigin = 0;
            cvFree( &ptr );
        }
        //else
        //{
        //    CvIPL.deallocate( img, IPL_IMAGE_DATA );
        //}
    }
    else
    {
        CV_ERROR( CV_StsBadArg, "unrecognized or unsupported array type" );
    }

    __END__;
}


// Retrieves essential information about image ROI or CvMat data
CV_IMPL void
cvGetRawData( const CvArr* arr, uchar** data, int* step, CvSize* roi_size )
{
    CV_FUNCNAME( "cvGetRawData" );

    __BEGIN__;

    if( CV_IS_MAT( arr ))
    {
        CvMat *mat = (CvMat*)arr;

        if( step )
            *step = mat->step;

        if( data )
            *data = mat->data.ptr;

        if( roi_size )
            *roi_size = cvGetMatSize( mat );
    }
    else if( CV_IS_IMAGE( arr ))
    {
        IplImage* img = (IplImage*)arr;

        if( step )
            *step = img->widthStep;

        if( data )
            CV_CALL( *data = cvPtr2D( img, 0, 0 ));

        if( roi_size )
        {
            if( img->roi )
            {
                *roi_size = cvSize( img->roi->width, img->roi->height );
            }
            else
            {
                *roi_size = cvSize( img->width, img->height );
            }
        }
    }
    else if( CV_IS_MATND( arr ))
    {
        CvMatND* mat = (CvMatND*)arr;

        if( !CV_IS_MAT_CONT( mat->type ))
            CV_ERROR( CV_StsBadArg, "Only continuous nD arrays are supported here" );

        if( data )
            *data = mat->data.ptr;

        if( roi_size || step )
        {
            int i, size1 = mat->dim[0].size, size2 = 1;

            if( mat->dims > 2 )
                for( i = 1; i < mat->dims; i++ )
                    size1 *= mat->dim[i].size;
            else
                size2 = mat->dim[1].size;

            if( roi_size )
            {
                roi_size->width = size2;
                roi_size->height = size1;
            }

            if( step )
                *step = size1 == 1 ? 0 : mat->dim[0].step;
        }
    }
    else
    {
        CV_ERROR( CV_StsBadArg, "unrecognized or unsupported array type" );
    }

    __END__;
}


CV_IMPL int
cvGetElemType( const CvArr* arr )
{
    int type = -1;

    CV_FUNCNAME( "cvGetElemType" );

    __BEGIN__;

    if( CV_IS_MAT_HDR(arr) || CV_IS_MATND_HDR(arr) || CV_IS_SPARSE_MAT_HDR(arr))
    {
        type = CV_MAT_TYPE( ((CvMat*)arr)->type );
    }
    else if( CV_IS_IMAGE(arr))
    {
        IplImage* img = (IplImage*)arr;
        type = CV_MAKETYPE( icvIplToCvDepth(img->depth), img->nChannels );
    }
    else
        CV_ERROR( CV_StsBadArg, "unrecognized or unsupported array type" );

    __END__;

    return type;
}


// Returns a number of array dimensions
CV_IMPL int
cvGetDims( const CvArr* arr, int* sizes )
{
    int dims = -1;
    CV_FUNCNAME( "cvGetDims" );

    __BEGIN__;

    if( CV_IS_MAT_HDR( arr ))
    {
        CvMat* mat = (CvMat*)arr;
        
        dims = 2;
        if( sizes )
        {
            sizes[0] = mat->rows;
            sizes[1] = mat->cols;
        }
    }
    else if( CV_IS_IMAGE( arr ))
    {
        IplImage* img = (IplImage*)arr;
        dims = 2;

        if( sizes )
        {
            sizes[0] = img->height;
            sizes[1] = img->width;
        }
    }
    else if( CV_IS_MATND_HDR( arr ))
    {
        CvMatND* mat = (CvMatND*)arr;
        dims = mat->dims;
        
        if( sizes )
        {
            int i;
            for( i = 0; i < dims; i++ )
                sizes[i] = mat->dim[i].size;
        }
    }
    else if( CV_IS_SPARSE_MAT_HDR( arr ))
    {
        CvSparseMat* mat = (CvSparseMat*)arr;
        dims = mat->dims;
        
        if( sizes )
            memcpy( sizes, mat->size, dims*sizeof(sizes[0]));
    }
    else
    {
        CV_ERROR( CV_StsBadArg, "unrecognized or unsupported array type" );
    }

    __END__;

    return dims;
}


// Returns the size of particular array dimension
CV_IMPL int
cvGetDimSize( const CvArr* arr, int index )
{
    int size = -1;
    CV_FUNCNAME( "cvGetDimSize" );

    __BEGIN__;

    if( CV_IS_MAT( arr ))
    {
        CvMat *mat = (CvMat*)arr;

        switch( index )
        {
        case 0:
            size = mat->rows;
            break;
        case 1:
            size = mat->cols;
            break;
        default:
            CV_ERROR( CV_StsOutOfRange, "bad dimension index" );
        }
    }
    else if( CV_IS_IMAGE( arr ))
    {
        IplImage* img = (IplImage*)arr;

        switch( index )
        {
        case 0:
            size = !img->roi ? img->height : img->roi->height;
            break;
        case 1:
            size = !img->roi ? img->width : img->roi->width;
            break;
        default:
            CV_ERROR( CV_StsOutOfRange, "bad dimension index" );
        }
    }
    else if( CV_IS_MATND_HDR( arr ))
    {
        CvMatND* mat = (CvMatND*)arr;
        
        if( (unsigned)index >= (unsigned)mat->dims )
            CV_ERROR( CV_StsOutOfRange, "bad dimension index" );

        size = mat->dim[index].size;
    }
    else if( CV_IS_SPARSE_MAT_HDR( arr ))
    {
        CvSparseMat* mat = (CvSparseMat*)arr;
        
        if( (unsigned)index >= (unsigned)mat->dims )
            CV_ERROR( CV_StsOutOfRange, "bad dimension index" );

        size = mat->size[index];
    }
    else
    {
        CV_ERROR( CV_StsBadArg, "unrecognized or unsupported array type" );
    }

    __END__;

    return size;
}


// Returns the size of CvMat or IplImage
CV_IMPL CvSize
cvGetSize( const CvArr* arr )
{
    CvSize size = { 0, 0 };

    CV_FUNCNAME( "cvGetSize" );

    __BEGIN__;

    if( CV_IS_MAT_HDR( arr ))
    {
        CvMat *mat = (CvMat*)arr;

        size.width = mat->cols;
        size.height = mat->rows;
    }
    else if( CV_IS_IMAGE_HDR( arr ))
    {
        IplImage* img = (IplImage*)arr;

        if( img->roi )
        {
            size.width = img->roi->width;
            size.height = img->roi->height;
        }
        else
        {
            size.width = img->width;
            size.height = img->height;
        }
    }
    else
    {
        CV_ERROR( CV_StsBadArg, "Array should be CvMat or IplImage" );
    }

    __END__;

    return size;
}


// Selects sub-array (no data is copied)
CV_IMPL  CvMat*
cvGetSubRect( const CvArr* arr, CvMat* submat, CvRect rect )
{
    CvMat* res = 0;
    
    CV_FUNCNAME( "cvGetRect" );

    __BEGIN__;

    CvMat stub, *mat = (CvMat*)arr;

    if( !CV_IS_MAT( mat ))
        CV_CALL( mat = cvGetMat( mat, &stub ));

    if( !submat )
        CV_ERROR( CV_StsNullPtr, "" );

    if( (rect.x|rect.y|rect.width|rect.height) < 0 )
        CV_ERROR( CV_StsBadSize, "" );

    if( rect.x + rect.width > mat->cols ||
        rect.y + rect.height > mat->rows )
        CV_ERROR( CV_StsBadSize, "" );

    {
    /*
    int* refcount = mat->refcount;

    if( refcount )
        ++*refcount;

    cvDecRefData( submat );
    */
    submat->data.ptr = mat->data.ptr + (size_t)rect.y*mat->step +
                       rect.x*CV_ELEM_SIZE(mat->type);
    submat->step = mat->step & (rect.height > 1 ? -1 : 0);
    submat->type = (mat->type & (rect.width < mat->cols ? ~CV_MAT_CONT_FLAG : -1)) |
                   (submat->step == 0 ? CV_MAT_CONT_FLAG : 0);
    submat->rows = rect.height;
    submat->cols = rect.width;
    submat->refcount = 0;
    res = submat;
    }
    
    __END__;

    return res;
}


// Selects array's row span.
CV_IMPL  CvMat*
cvGetRows( const CvArr* arr, CvMat* submat,
           int start_row, int end_row, int delta_row )
{
    CvMat* res = 0;
    
    CV_FUNCNAME( "cvGetRows" );

    __BEGIN__;

    CvMat stub, *mat = (CvMat*)arr;

    if( !CV_IS_MAT( mat ))
        CV_CALL( mat = cvGetMat( mat, &stub ));

    if( !submat )
        CV_ERROR( CV_StsNullPtr, "" );

    if( (unsigned)start_row >= (unsigned)mat->rows ||
        (unsigned)end_row > (unsigned)mat->rows || delta_row <= 0 )
        CV_ERROR( CV_StsOutOfRange, "" );

    {
    /*
    int* refcount = mat->refcount;

    if( refcount )
        ++*refcount;

    cvDecRefData( submat );
    */
    if( delta_row == 1 )
    {
        submat->rows = end_row - start_row;
        submat->step = mat->step & (submat->rows > 1 ? -1 : 0);
    }
    else
    {
        submat->rows = (end_row - start_row + delta_row - 1)/delta_row;
        submat->step = mat->step * delta_row;
    }

    submat->cols = mat->cols;
    submat->step &= submat->rows > 1 ? -1 : 0;
    submat->data.ptr = mat->data.ptr + (size_t)start_row*mat->step;
    submat->type = (mat->type | (submat->step == 0 ? CV_MAT_CONT_FLAG : 0)) &
                   (delta_row != 1 ? ~CV_MAT_CONT_FLAG : -1);
    submat->refcount = 0;
    submat->hdr_refcount = 0;
    res = submat;
    }
    
    __END__;

    return res;
}


// Selects array's column span.
CV_IMPL  CvMat*
cvGetCols( const CvArr* arr, CvMat* submat, int start_col, int end_col )
{
    CvMat* res = 0;
    
    CV_FUNCNAME( "cvGetCols" );

    __BEGIN__;

    CvMat stub, *mat = (CvMat*)arr;

    if( !CV_IS_MAT( mat ))
        CV_CALL( mat = cvGetMat( mat, &stub ));

    if( !submat )
        CV_ERROR( CV_StsNullPtr, "" );

    if( (unsigned)start_col >= (unsigned)mat->cols ||
        (unsigned)end_col > (unsigned)mat->cols )
        CV_ERROR( CV_StsOutOfRange, "" );

    {
    /*
    int* refcount = mat->refcount;

    if( refcount )
        ++*refcount;

    cvDecRefData( submat );
    */
    submat->rows = mat->rows;
    submat->cols = end_col - start_col;
    submat->step = mat->step & (submat->rows > 1 ? -1 : 0);
    submat->data.ptr = mat->data.ptr + (size_t)start_col*CV_ELEM_SIZE(mat->type);
    submat->type = mat->type & (submat->step && submat->cols < mat->cols ?
                                ~CV_MAT_CONT_FLAG : -1);
    submat->refcount = 0;
    submat->hdr_refcount = 0;
    res = submat;
    }
    
    __END__;

    return res;
}


// Selects array diagonal
CV_IMPL  CvMat*
cvGetDiag( const CvArr* arr, CvMat* submat, int diag )
{
    CvMat* res = 0;
    
    CV_FUNCNAME( "cvGetDiag" );

    __BEGIN__;

    CvMat stub, *mat = (CvMat*)arr;
    int len, pix_size; 

    if( !CV_IS_MAT( mat ))
        CV_CALL( mat = cvGetMat( mat, &stub ));

    if( !submat )
        CV_ERROR( CV_StsNullPtr, "" );

    pix_size = CV_ELEM_SIZE(mat->type);

    /*{
    int* refcount = mat->refcount;

    if( refcount )
        ++*refcount;

    cvDecRefData( submat );
    }*/

    if( diag >= 0 )
    {
        len = mat->cols - diag;
        
        if( len <= 0 )
            CV_ERROR( CV_StsOutOfRange, "" );

        len = CV_IMIN( len, mat->rows );
        submat->data.ptr = mat->data.ptr + diag*pix_size;
    }
    else
    {
        len = mat->rows + diag;
        
        if( len <= 0 )
            CV_ERROR( CV_StsOutOfRange, "" );

        len = CV_IMIN( len, mat->cols );
        submat->data.ptr = mat->data.ptr - diag*mat->step;
    }

    submat->rows = len;
    submat->cols = 1;
    submat->step = (mat->step + pix_size) & (submat->rows > 1 ? -1 : 0);
    submat->type = mat->type;
    if( submat->step )
        submat->type &= ~CV_MAT_CONT_FLAG;
    else
        submat->type |= CV_MAT_CONT_FLAG;
    submat->refcount = 0;
    submat->hdr_refcount = 0;
    res = submat;
    
    __END__;

    return res;
}


/****************************************************************************************\
*                      Operations on CvScalar and accessing array elements               *
\****************************************************************************************/

// Converts CvScalar to specified type
CV_IMPL void
cvScalarToRawData( const CvScalar* scalar, void* data, int type, int extend_to_12 )
{
    CV_FUNCNAME( "cvScalarToRawData" );

    type = CV_MAT_TYPE(type);
    
    __BEGIN__;

    int cn = CV_MAT_CN( type );
    int depth = type & CV_MAT_DEPTH_MASK;

    assert( scalar && data );
    if( (unsigned)(cn - 1) >= 4 )
        CV_ERROR( CV_StsOutOfRange, "The number of channels must be 1, 2, 3 or 4" );

    switch( depth )
    {
    case CV_8UC1:
        while( cn-- )
        {
            int t = cvRound( scalar->val[cn] );
            ((uchar*)data)[cn] = CV_CAST_8U(t);
        }
        break;
    case CV_8SC1:
        while( cn-- )
        {
            int t = cvRound( scalar->val[cn] );
            ((char*)data)[cn] = CV_CAST_8S(t);
        }
        break;
    case CV_16UC1:
        while( cn-- )
        {
            int t = cvRound( scalar->val[cn] );
            ((ushort*)data)[cn] = CV_CAST_16U(t);
        }
        break;
    case CV_16SC1:
        while( cn-- )
        {
            int t = cvRound( scalar->val[cn] );
            ((short*)data)[cn] = CV_CAST_16S(t);
        }
        break;
    case CV_32SC1:
        while( cn-- )
            ((int*)data)[cn] = cvRound( scalar->val[cn] );
        break;
    case CV_32FC1:
        while( cn-- )
            ((float*)data)[cn] = (float)(scalar->val[cn]);
        break;
    case CV_64FC1:
        while( cn-- )
            ((double*)data)[cn] = (double)(scalar->val[cn]);
        break;
    default:
        assert(0);
        CV_ERROR_FROM_CODE( CV_BadDepth );
    }

    if( extend_to_12 )
    {
        int pix_size = CV_ELEM_SIZE(type);
        int offset = CV_ELEM_SIZE1(depth)*12;

        do
        {
            offset -= pix_size;
            CV_MEMCPY_AUTO( (char*)data + offset, data, pix_size );
        }
        while( offset > pix_size );
    }

    __END__;
}


// Converts data of specified type to CvScalar
CV_IMPL void
cvRawDataToScalar( const void* data, int flags, CvScalar* scalar )
{
    CV_FUNCNAME( "cvRawDataToScalar" );
    
    __BEGIN__;

    int cn = CV_MAT_CN( flags );

    assert( scalar && data );
    
    if( (unsigned)(cn - 1) >= 4 )
        CV_ERROR( CV_StsOutOfRange, "The number of channels must be 1, 2, 3 or 4" );

    memset( scalar->val, 0, sizeof(scalar->val));

    switch( CV_MAT_DEPTH( flags ))
    {
    case CV_8U:
        while( cn-- )
            scalar->val[cn] = CV_8TO32F(((uchar*)data)[cn]);
        break;
    case CV_8S:
        while( cn-- )
            scalar->val[cn] = CV_8TO32F(((char*)data)[cn]);
        break;
    case CV_16U:
        while( cn-- )
            scalar->val[cn] = ((ushort*)data)[cn];
        break;
    case CV_16S:
        while( cn-- )
            scalar->val[cn] = ((short*)data)[cn];
        break;
    case CV_32S:
        while( cn-- )
            scalar->val[cn] = ((int*)data)[cn];
        break;
    case CV_32F:
        while( cn-- )
            scalar->val[cn] = ((float*)data)[cn];
        break;
    case CV_64F:
        while( cn-- )
            scalar->val[cn] = ((double*)data)[cn];
        break;
    default:
        assert(0);
        CV_ERROR_FROM_CODE( CV_BadDepth );
    }

    __END__;
}


static double icvGetReal( const void* data, int type )
{
    switch( type )
    {
    case CV_8U:
        return *(uchar*)data;
    case CV_8S:
        return *(char*)data;
    case CV_16U:
        return *(ushort*)data;
    case CV_16S:
        return *(short*)data;
    case CV_32S:
        return *(int*)data;
    case CV_32F:
        return *(float*)data;
    case CV_64F:
        return *(double*)data;
    }

    return 0;
}


static void icvSetReal( double value, const void* data, int type )
{
    if( type < CV_32F )
    {
        int ivalue = cvRound(value);
        switch( type )
        {
        case CV_8U:
            *(uchar*)data = CV_CAST_8U(ivalue);
            break;
        case CV_8S:
            *(char*)data = CV_CAST_8S(ivalue);
            break;
        case CV_16U:
            *(ushort*)data = CV_CAST_16U(ivalue);
            break;
        case CV_16S:
            *(short*)data = CV_CAST_16S(ivalue);
            break;
        case CV_32S:
            *(int*)data = CV_CAST_32S(ivalue);
            break;
        }
    }
    else
    {
        switch( type )
        {
        case CV_32F:
            *(float*)data = (float)value;
            break;
        case CV_64F:
            *(double*)data = value;
            break;
        }
    }
}


// Returns pointer to specified element of array (linear index is used)
CV_IMPL  uchar*
cvPtr1D( const CvArr* arr, int idx, int* _type )
{
    uchar* ptr = 0;
    
    CV_FUNCNAME( "cvPtr1D" );

    __BEGIN__;

    if( CV_IS_MAT( arr ))
    {
        CvMat* mat = (CvMat*)arr;

        int type = CV_MAT_TYPE(mat->type);
        int pix_size = CV_ELEM_SIZE(type);

        if( _type )
            *_type = type;
        
        // the first part is mul-free sufficient check
        // that the index is within the matrix
        if( (unsigned)idx >= (unsigned)(mat->rows + mat->cols - 1) &&
            (unsigned)idx >= (unsigned)(mat->rows*mat->cols))
            CV_ERROR( CV_StsOutOfRange, "index is out of range" );

        if( CV_IS_MAT_CONT(mat->type))
        {
            ptr = mat->data.ptr + (size_t)idx*pix_size;
        }
        else
        {
            int row, col;
            if( mat->cols == 1 )
                row = idx, col = 0;
            else
                row = idx/mat->cols, col = idx - row*mat->cols;
            ptr = mat->data.ptr + (size_t)row*mat->step + col*pix_size;
        }
    }
    else if( CV_IS_IMAGE_HDR( arr ))
    {
        IplImage* img = (IplImage*)arr;
        int width = !img->roi ? img->width : img->roi->width;
        int y = idx/width, x = idx - y*width;

        ptr = cvPtr2D( arr, y, x, _type );
    }
    else if( CV_IS_MATND( arr ))
    {
        CvMatND* mat = (CvMatND*)arr;
        int j, type = CV_MAT_TYPE(mat->type);
        size_t size = mat->dim[0].size;

        if( _type )
            *_type = type;

        for( j = 1; j < mat->dims; j++ )
            size *= mat->dim[j].size;

        if((unsigned)idx >= (unsigned)size )
            CV_ERROR( CV_StsOutOfRange, "index is out of range" );

        if( CV_IS_MAT_CONT(mat->type))
        {
            int pix_size = CV_ELEM_SIZE(type);
            ptr = mat->data.ptr + (size_t)idx*pix_size;
        }
        else
        {
            ptr = mat->data.ptr;
            for( j = mat->dims - 1; j >= 0; j-- )
            {
                int sz = mat->dim[j].size;
                if( sz )
                {
                    int t = idx/sz;
                    ptr += (idx - t*sz)*mat->dim[j].step;
                    idx = t;
                }
            }
        }
    }
    else if( CV_IS_SPARSE_MAT( arr ))
    {
        CvSparseMat* m = (CvSparseMat*)arr;
        if( m->dims == 1 )
            ptr = icvGetNodePtr( (CvSparseMat*)arr, &idx, _type, 1, 0 );
        else
        {
            int i, n = m->dims;
            int* _idx = (int*)cvAlloc(n*sizeof(_idx[0]));
            
            for( i = n - 1; i >= 0; i-- )
            {
                int t = idx / m->size[i];
                _idx[i] = idx - t*m->size[i];
                idx = t;
            }
            ptr = icvGetNodePtr( (CvSparseMat*)arr, _idx, _type, 1, 0 );
			cvFree(&_idx);
        }
    }
    else
    {
        CV_ERROR( CV_StsBadArg, "unrecognized or unsupported array type" );
    }

    __END__;

    return ptr;
}


// Returns pointer to specified element of 2d array
CV_IMPL  uchar*
cvPtr2D( const CvArr* arr, int y, int x, int* _type )
{
    uchar* ptr = 0;
    
    CV_FUNCNAME( "cvPtr2D" );

    __BEGIN__;

    if( CV_IS_MAT( arr ))
    {
        CvMat* mat = (CvMat*)arr;
        int type;

        if( (unsigned)y >= (unsigned)(mat->rows) ||
            (unsigned)x >= (unsigned)(mat->cols) )
            CV_ERROR( CV_StsOutOfRange, "index is out of range" );

        type = CV_MAT_TYPE(mat->type);
        if( _type )
            *_type = type;

        ptr = mat->data.ptr + (size_t)y*mat->step + x*CV_ELEM_SIZE(type);
    }
    else if( CV_IS_IMAGE( arr ))
    {
        IplImage* img = (IplImage*)arr;
        int pix_size = (img->depth & 255) >> 3;
        int width, height;
        ptr = (uchar*)img->imageData;

        if( img->dataOrder == 0 )
            pix_size *= img->nChannels;

        if( img->roi )
        {
            width = img->roi->width;
            height = img->roi->height;

            ptr += img->roi->yOffset*img->widthStep +
                   img->roi->xOffset*pix_size;

            if( img->dataOrder )
            {
                int coi = img->roi->coi;
                if( !coi )
                    CV_ERROR( CV_BadCOI,
                        "COI must be non-null in case of planar images" );
                ptr += (coi - 1)*img->imageSize;
            }
        }
        else
        {
            width = img->width;
            height = img->height;
        }

        if( (unsigned)y >= (unsigned)height ||
            (unsigned)x >= (unsigned)width )
            CV_ERROR( CV_StsOutOfRange, "index is out of range" );

        ptr += y*img->widthStep + x*pix_size;

        if( _type )
        {
            int type = icvIplToCvDepth(img->depth);
            if( type < 0 || (unsigned)(img->nChannels - 1) > 3 )
                CV_ERROR( CV_StsUnsupportedFormat, "" );

            *_type = CV_MAKETYPE( type, img->nChannels );
        }
    }
    else if( CV_IS_MATND( arr ))
    {
        CvMatND* mat = (CvMatND*)arr;

        if( mat->dims != 2 || 
            (unsigned)y >= (unsigned)(mat->dim[0].size) ||
            (unsigned)x >= (unsigned)(mat->dim[1].size) )
            CV_ERROR( CV_StsOutOfRange, "index is out of range" );

        ptr = mat->data.ptr + (size_t)y*mat->dim[0].step + x*mat->dim[1].step;
        if( _type )
            *_type = CV_MAT_TYPE(mat->type);
    }
    else if( CV_IS_SPARSE_MAT( arr ))
    {
        int idx[] = { y, x };
        ptr = icvGetNodePtr( (CvSparseMat*)arr, idx, _type, 1, 0 );
    }
    else
    {
        CV_ERROR( CV_StsBadArg, "unrecognized or unsupported array type" );
    }

    __END__;

    return ptr;
}


// Returns pointer to specified element of 3d array
CV_IMPL  uchar*
cvPtr3D( const CvArr* arr, int z, int y, int x, int* _type )
{
    uchar* ptr = 0;
    
    CV_FUNCNAME( "cvPtr3D" );

    __BEGIN__;

    if( CV_IS_MATND( arr ))
    {
        CvMatND* mat = (CvMatND*)arr;

        if( mat->dims != 3 || 
            (unsigned)z >= (unsigned)(mat->dim[0].size) ||
            (unsigned)y >= (unsigned)(mat->dim[1].size) ||
            (unsigned)x >= (unsigned)(mat->dim[2].size) )
            CV_ERROR( CV_StsOutOfRange, "index is out of range" );

        ptr = mat->data.ptr + (size_t)z*mat->dim[0].step +
              (size_t)y*mat->dim[1].step + x*mat->dim[2].step;

        if( _type )
            *_type = CV_MAT_TYPE(mat->type);
    }
    else if( CV_IS_SPARSE_MAT( arr ))
    {
        int idx[] = { z, y, x };
        ptr = icvGetNodePtr( (CvSparseMat*)arr, idx, _type, 1, 0 );
    }
    else
    {
        CV_ERROR( CV_StsBadArg, "unrecognized or unsupported array type" );
    }

    __END__;

    return ptr;
}


// Returns pointer to specified element of n-d array
CV_IMPL  uchar*
cvPtrND( const CvArr* arr, const int* idx, int* _type,
         int create_node, unsigned* precalc_hashval )
{
    uchar* ptr = 0;
    CV_FUNCNAME( "cvPtrND" );

    __BEGIN__;

    if( !idx )
        CV_ERROR( CV_StsNullPtr, "NULL pointer to indices" );

    if( CV_IS_SPARSE_MAT( arr ))
        ptr = icvGetNodePtr( (CvSparseMat*)arr, idx, 
                             _type, create_node, precalc_hashval );
    else if( CV_IS_MATND( arr ))
    {
        CvMatND* mat = (CvMatND*)arr;
        int i;
        ptr = mat->data.ptr;

        for( i = 0; i < mat->dims; i++ )
        {
            if( (unsigned)idx[i] >= (unsigned)(mat->dim[i].size) )
                CV_ERROR( CV_StsOutOfRange, "index is out of range" );
            ptr += (size_t)idx[i]*mat->dim[i].step;
        }

        if( _type )
            *_type = CV_MAT_TYPE(mat->type);
    }
    else if( CV_IS_MAT_HDR(arr) || CV_IS_IMAGE_HDR(arr) )
        ptr = cvPtr2D( arr, idx[0], idx[1], _type );
    else
        CV_ERROR( CV_StsBadArg, "unrecognized or unsupported array type" );

    __END__;

    return ptr;
}


// Returns specifed element of n-D array given linear index
CV_IMPL  CvScalar
cvGet1D( const CvArr* arr, int idx )
{
    CvScalar scalar = {{0,0,0,0}};

    CV_FUNCNAME( "cvGet1D" );

    __BEGIN__;

    int type = 0;
    uchar* ptr;
    
    if( CV_IS_MAT( arr ) && CV_IS_MAT_CONT( ((CvMat*)arr)->type ))
    {
        CvMat* mat = (CvMat*)arr;

        type = CV_MAT_TYPE(mat->type);
        int pix_size = CV_ELEM_SIZE(type);

        // the first part is mul-free sufficient check
        // that the index is within the matrix
        if( (unsigned)idx >= (unsigned)(mat->rows + mat->cols - 1) &&
            (unsigned)idx >= (unsigned)(mat->rows*mat->cols))
            CV_ERROR( CV_StsOutOfRange, "index is out of range" );

        ptr = mat->data.ptr + (size_t)idx*pix_size;
    }
    else if( !CV_IS_SPARSE_MAT( arr ) || ((CvSparseMat*)arr)->dims > 1 )
        ptr = cvPtr1D( arr, idx, &type );
    else
        ptr = icvGetNodePtr( (CvSparseMat*)arr, &idx, &type, 0, 0 );

    cvRawDataToScalar( ptr, type, &scalar );

    __END__;

    return scalar;
}


// Returns specifed element of 2D array
CV_IMPL  CvScalar
cvGet2D( const CvArr* arr, int y, int x )
{
    CvScalar scalar = {{0,0,0,0}};

    CV_FUNCNAME( "cvGet2D" );

    __BEGIN__;

    int type = 0;
    uchar* ptr;

    if( CV_IS_MAT( arr ))
    {
        CvMat* mat = (CvMat*)arr;

        if( (unsigned)y >= (unsigned)(mat->rows) ||
            (unsigned)x >= (unsigned)(mat->cols) )
            CV_ERROR( CV_StsOutOfRange, "index is out of range" );

        type = CV_MAT_TYPE(mat->type);
        ptr = mat->data.ptr + (size_t)y*mat->step + x*CV_ELEM_SIZE(type);
    }
    else if( !CV_IS_SPARSE_MAT( arr ))
        ptr = cvPtr2D( arr, y, x, &type );
    else
    {
        int idx[] = { y, x };
        ptr = icvGetNodePtr( (CvSparseMat*)arr, idx, &type, 0, 0 );
    }

    cvRawDataToScalar( ptr, type, &scalar );

    __END__;

    return scalar;
}


// Returns specifed element of 3D array
CV_IMPL  CvScalar
cvGet3D( const CvArr* arr, int z, int y, int x )
{
    CvScalar scalar = {{0,0,0,0}};

    /*CV_FUNCNAME( "cvGet3D" );*/

    __BEGIN__;

    int type = 0;
    uchar* ptr;

    if( !CV_IS_SPARSE_MAT( arr ))
        ptr = cvPtr3D( arr, z, y, x, &type );
    else
    {
        int idx[] = { z, y, x };
        ptr = icvGetNodePtr( (CvSparseMat*)arr, idx, &type, 0, 0 );
    }

    cvRawDataToScalar( ptr, type, &scalar );

    __END__;

    return scalar;
}


// Returns specifed element of nD array
CV_IMPL  CvScalar
cvGetND( const CvArr* arr, const int* idx )
{
    CvScalar scalar = {{0,0,0,0}};

    /*CV_FUNCNAME( "cvGetND" );*/

    __BEGIN__;

    int type = 0;
    uchar* ptr;

    if( !CV_IS_SPARSE_MAT( arr ))
        ptr = cvPtrND( arr, idx, &type );
    else
        ptr = icvGetNodePtr( (CvSparseMat*)arr, idx, &type, 0, 0 );

    cvRawDataToScalar( ptr, type, &scalar );

    __END__;

    return scalar;
}


// Returns specifed element of n-D array given linear index
CV_IMPL  double
cvGetReal1D( const CvArr* arr, int idx )
{
    double value = 0;

    CV_FUNCNAME( "cvGetReal1D" );

    __BEGIN__;

    int type = 0;
    uchar* ptr;

    if( CV_IS_MAT( arr ) && CV_IS_MAT_CONT( ((CvMat*)arr)->type ))
    {
        CvMat* mat = (CvMat*)arr;

        type = CV_MAT_TYPE(mat->type);
        int pix_size = CV_ELEM_SIZE(type);

        // the first part is mul-free sufficient check
        // that the index is within the matrix
        if( (unsigned)idx >= (unsigned)(mat->rows + mat->cols - 1) &&
            (unsigned)idx >= (unsigned)(mat->rows*mat->cols))
            CV_ERROR( CV_StsOutOfRange, "index is out of range" );

        ptr = mat->data.ptr + (size_t)idx*pix_size;
    }
    else if( !CV_IS_SPARSE_MAT( arr ) || ((CvSparseMat*)arr)->dims > 1 )
        ptr = cvPtr1D( arr, idx, &type );
    else
        ptr = icvGetNodePtr( (CvSparseMat*)arr, &idx, &type, 0, 0 );

    if( ptr )
    {
        if( CV_MAT_CN( type ) > 1 )
            CV_ERROR( CV_BadNumChannels, "cvGetReal* support only single-channel arrays" );

        value = icvGetReal( ptr, type );
    }

    __END__;

    return value;
}


// Returns specifed element of 2D array
CV_IMPL  double
cvGetReal2D( const CvArr* arr, int y, int x )
{
    double value = 0;

    CV_FUNCNAME( "cvGetReal2D" );

    __BEGIN__;

    int type = 0;
    uchar* ptr;
    
    if( CV_IS_MAT( arr ))
    {
        CvMat* mat = (CvMat*)arr;

        if( (unsigned)y >= (unsigned)(mat->rows) ||
            (unsigned)x >= (unsigned)(mat->cols) )
            CV_ERROR( CV_StsOutOfRange, "index is out of range" );

        type = CV_MAT_TYPE(mat->type);
        ptr = mat->data.ptr + (size_t)y*mat->step + x*CV_ELEM_SIZE(type);
    }
    else if( !CV_IS_SPARSE_MAT( arr ))
        ptr = cvPtr2D( arr, y, x, &type );
    else
    {
        int idx[] = { y, x };
        ptr = icvGetNodePtr( (CvSparseMat*)arr, idx, &type, 0, 0 );
    }

    if( ptr )
    {
        if( CV_MAT_CN( type ) > 1 )
            CV_ERROR( CV_BadNumChannels, "cvGetReal* support only single-channel arrays" );

        value = icvGetReal( ptr, type );
    }

    __END__;

    return value;
}


// Returns specifed element of 3D array
CV_IMPL  double
cvGetReal3D( const CvArr* arr, int z, int y, int x )
{
    double value = 0;

    CV_FUNCNAME( "cvGetReal3D" );

    __BEGIN__;

    int type = 0;
    uchar* ptr;

    if( !CV_IS_SPARSE_MAT( arr ))
        ptr = cvPtr3D( arr, z, y, x, &type );
    else
    {
        int idx[] = { z, y, x };
        ptr = icvGetNodePtr( (CvSparseMat*)arr, idx, &type, 0, 0 );
    }
    
    if( ptr )
    {
        if( CV_MAT_CN( type ) > 1 )
            CV_ERROR( CV_BadNumChannels, "cvGetReal* support only single-channel arrays" );

        value = icvGetReal( ptr, type );
    }

    __END__;

    return value;
}


// Returns specifed element of nD array
CV_IMPL  double
cvGetRealND( const CvArr* arr, const int* idx )
{
    double value = 0;

    CV_FUNCNAME( "cvGetRealND" );

    __BEGIN__;

    int type = 0;
    uchar* ptr;
    
    if( !CV_IS_SPARSE_MAT( arr ))
        ptr = cvPtrND( arr, idx, &type );
    else
        ptr = icvGetNodePtr( (CvSparseMat*)arr, idx, &type, 0, 0 );

    if( ptr )
    {
        if( CV_MAT_CN( type ) > 1 )
            CV_ERROR( CV_BadNumChannels, "cvGetReal* support only single-channel arrays" );

        value = icvGetReal( ptr, type );
    }

    __END__;

    return value;
}


// Assigns new value to specifed element of nD array given linear index
CV_IMPL  void
cvSet1D( CvArr* arr, int idx, CvScalar scalar )
{
    CV_FUNCNAME( "cvSet1D" );

    __BEGIN__;

    int type = 0;
    uchar* ptr;
    
    if( CV_IS_MAT( arr ) && CV_IS_MAT_CONT( ((CvMat*)arr)->type ))
    {
        CvMat* mat = (CvMat*)arr;

        type = CV_MAT_TYPE(mat->type);
        int pix_size = CV_ELEM_SIZE(type);

        // the first part is mul-free sufficient check
        // that the index is within the matrix
        if( (unsigned)idx >= (unsigned)(mat->rows + mat->cols - 1) &&
            (unsigned)idx >= (unsigned)(mat->rows*mat->cols))
            CV_ERROR( CV_StsOutOfRange, "index is out of range" );

        ptr = mat->data.ptr + (size_t)idx*pix_size;
    }
    else if( !CV_IS_SPARSE_MAT( arr ) || ((CvSparseMat*)arr)->dims > 1 )
        ptr = cvPtr1D( arr, idx, &type );
    else
        ptr = icvGetNodePtr( (CvSparseMat*)arr, &idx, &type, -1, 0 );

    cvScalarToRawData( &scalar, ptr, type );

    __END__;
}


// Assigns new value to specifed element of 2D array
CV_IMPL  void
cvSet2D( CvArr* arr, int y, int x, CvScalar scalar )
{
    CV_FUNCNAME( "cvSet2D" );

    __BEGIN__;

    int type = 0;
    uchar* ptr;
    
    if( CV_IS_MAT( arr ))
    {
        CvMat* mat = (CvMat*)arr;

        if( (unsigned)y >= (unsigned)(mat->rows) ||
            (unsigned)x >= (unsigned)(mat->cols) )
            CV_ERROR( CV_StsOutOfRange, "index is out of range" );

        type = CV_MAT_TYPE(mat->type);
        ptr = mat->data.ptr + (size_t)y*mat->step + x*CV_ELEM_SIZE(type);
    }
    else if( !CV_IS_SPARSE_MAT( arr ))
        ptr = cvPtr2D( arr, y, x, &type );
    else
    {
        int idx[] = { y, x };
        ptr = icvGetNodePtr( (CvSparseMat*)arr, idx, &type, -1, 0 );
    }
    cvScalarToRawData( &scalar, ptr, type );

    __END__;
}


// Assigns new value to specifed element of 3D array
CV_IMPL  void
cvSet3D( CvArr* arr, int z, int y, int x, CvScalar scalar )
{
    /*CV_FUNCNAME( "cvSet3D" );*/

    __BEGIN__;

    int type = 0;
    uchar* ptr;
    
    if( !CV_IS_SPARSE_MAT( arr ))
        ptr = cvPtr3D( arr, z, y, x, &type );
    else
    {
        int idx[] = { z, y, x };
        ptr = icvGetNodePtr( (CvSparseMat*)arr, idx, &type, -1, 0 );
    }
    cvScalarToRawData( &scalar, ptr, type );

    __END__;
}


// Assigns new value to specifed element of nD array
CV_IMPL  void
cvSetND( CvArr* arr, const int* idx, CvScalar scalar )
{
    /*CV_FUNCNAME( "cvSetND" );*/

    __BEGIN__;

    int type = 0;
    uchar* ptr;
    
    if( !CV_IS_SPARSE_MAT( arr ))
        ptr = cvPtrND( arr, idx, &type );
    else
        ptr = icvGetNodePtr( (CvSparseMat*)arr, idx, &type, -1, 0 );
    cvScalarToRawData( &scalar, ptr, type );

    __END__;
}


CV_IMPL  void
cvSetReal1D( CvArr* arr, int idx, double value )
{
    CV_FUNCNAME( "cvSetReal1D" );

    __BEGIN__;

    int type = 0;
    uchar* ptr;
    
    if( CV_IS_MAT( arr ) && CV_IS_MAT_CONT( ((CvMat*)arr)->type ))
    {
        CvMat* mat = (CvMat*)arr;

        type = CV_MAT_TYPE(mat->type);
        int pix_size = CV_ELEM_SIZE(type);

        // the first part is mul-free sufficient check
        // that the index is within the matrix
        if( (unsigned)idx >= (unsigned)(mat->rows + mat->cols - 1) &&
            (unsigned)idx >= (unsigned)(mat->rows*mat->cols))
            CV_ERROR( CV_StsOutOfRange, "index is out of range" );

        ptr = mat->data.ptr + (size_t)idx*pix_size;
    }
    else if( !CV_IS_SPARSE_MAT( arr ) || ((CvSparseMat*)arr)->dims > 1 )
        ptr = cvPtr1D( arr, idx, &type );
    else
        ptr = icvGetNodePtr( (CvSparseMat*)arr, &idx, &type, -1, 0 );

    if( CV_MAT_CN( type ) > 1 )
        CV_ERROR( CV_BadNumChannels, "cvSetReal* support only single-channel arrays" );

    if( ptr )
        icvSetReal( value, ptr, type );

    __END__;
}


CV_IMPL  void
cvSetReal2D( CvArr* arr, int y, int x, double value )
{
    CV_FUNCNAME( "cvSetReal2D" );

    __BEGIN__;

    int type = 0;
    uchar* ptr;
    
    if( CV_IS_MAT( arr ))
    {
        CvMat* mat = (CvMat*)arr;

        if( (unsigned)y >= (unsigned)(mat->rows) ||
            (unsigned)x >= (unsigned)(mat->cols) )
            CV_ERROR( CV_StsOutOfRange, "index is out of range" );

        type = CV_MAT_TYPE(mat->type);
        ptr = mat->data.ptr + (size_t)y*mat->step + x*CV_ELEM_SIZE(type);
    }
    else if( !CV_IS_SPARSE_MAT( arr ))
    {
        ptr = cvPtr2D( arr, y, x, &type );
    }
    else
    {
        int idx[] = { y, x };
        ptr = icvGetNodePtr( (CvSparseMat*)arr, idx, &type, -1, 0 );
    }
    if( CV_MAT_CN( type ) > 1 )
        CV_ERROR( CV_BadNumChannels, "cvSetReal* support only single-channel arrays" );

    if( ptr )
        icvSetReal( value, ptr, type );

    __END__;
}


CV_IMPL  void
cvSetReal3D( CvArr* arr, int z, int y, int x, double value )
{
    CV_FUNCNAME( "cvSetReal3D" );

    __BEGIN__;

    int type = 0;
    uchar* ptr;
    
    if( !CV_IS_SPARSE_MAT( arr ))
        ptr = cvPtr3D( arr, z, y, x, &type );
    else
    {
        int idx[] = { z, y, x };
        ptr = icvGetNodePtr( (CvSparseMat*)arr, idx, &type, -1, 0 );
    }
    if( CV_MAT_CN( type ) > 1 )
        CV_ERROR( CV_BadNumChannels, "cvSetReal* support only single-channel arrays" );

    if( ptr )
        icvSetReal( value, ptr, type );

    __END__;
}


CV_IMPL  void
cvSetRealND( CvArr* arr, const int* idx, double value )
{
    CV_FUNCNAME( "cvSetRealND" );

    __BEGIN__;

    int type = 0;
    uchar* ptr;
    
    if( !CV_IS_SPARSE_MAT( arr ))
        ptr = cvPtrND( arr, idx, &type );
    else
        ptr = icvGetNodePtr( (CvSparseMat*)arr, idx, &type, -1, 0 );

    if( CV_MAT_CN( type ) > 1 )
        CV_ERROR( CV_BadNumChannels, "cvSetReal* support only single-channel arrays" );

    if( ptr )
        icvSetReal( value, ptr, type );

    __END__;
}


CV_IMPL void
cvClearND( CvArr* arr, const int* idx )
{
    /*CV_FUNCNAME( "cvClearND" );*/

    __BEGIN__;

    if( !CV_IS_SPARSE_MAT( arr ))
    {
        int type;
        uchar* ptr;
        ptr = cvPtrND( arr, idx, &type );
        if( ptr )
            CV_ZERO_CHAR( ptr, CV_ELEM_SIZE(type) );
    }
    else
    {
        icvDeleteNode( (CvSparseMat*)arr, idx, 0 );
    }

    __END__;
}


/****************************************************************************************\
*                             Conversion to CvMat or IplImage                            *
\****************************************************************************************/

// convert array (CvMat or IplImage) to CvMat
CV_IMPL CvMat*
cvGetMat( const CvArr* array, CvMat* mat,
          int* pCOI, int allowND )
{
    CvMat* result = 0;
    CvMat* src = (CvMat*)array;
    int coi = 0;
    
    CV_FUNCNAME( "cvGetMat" );

    __BEGIN__;

    if( !mat || !src )
        CV_ERROR( CV_StsNullPtr, "NULL array pointer is passed" );

    if( CV_IS_MAT_HDR(src))
    {
        if( !src->data.ptr )
            CV_ERROR( CV_StsNullPtr, "The matrix has NULL data pointer" );
        
        result = (CvMat*)src;
    }
    else if( CV_IS_IMAGE_HDR(src) )
    {
        const IplImage* img = (const IplImage*)src;
        int depth, order;

        if( img->imageData == 0 )
            CV_ERROR( CV_StsNullPtr, "The image has NULL data pointer" );

        depth = icvIplToCvDepth( img->depth );
        if( depth < 0 )
            CV_ERROR_FROM_CODE( CV_BadDepth );

        order = img->dataOrder & (img->nChannels > 1 ? -1 : 0);

        if( img->roi )
        {
            if( order == IPL_DATA_ORDER_PLANE )
            {
                int type = depth;

                if( img->roi->coi == 0 )
                    CV_ERROR( CV_StsBadFlag,
                    "Images with planar data layout should be used with COI selected" );

                CV_CALL( cvInitMatHeader( mat, img->roi->height,
                                   img->roi->width, type,
                                   img->imageData + (img->roi->coi-1)*img->imageSize +
                                   img->roi->yOffset*img->widthStep +
                                   img->roi->xOffset*CV_ELEM_SIZE(type),
                                   img->widthStep ));
            }
            else /* pixel order */
            {
                int type = CV_MAKETYPE( depth, img->nChannels );
                coi = img->roi->coi;

                if( img->nChannels > CV_CN_MAX )
                    CV_ERROR( CV_BadNumChannels,
                        "The image is interleaved and has over CV_CN_MAX channels" );

                CV_CALL( cvInitMatHeader( mat, img->roi->height, img->roi->width,
                                          type, img->imageData +
                                          img->roi->yOffset*img->widthStep +
                                          img->roi->xOffset*CV_ELEM_SIZE(type),
                                          img->widthStep ));
            }
        }
        else
        {
            int type = CV_MAKETYPE( depth, img->nChannels );

            if( order != IPL_DATA_ORDER_PIXEL )
                CV_ERROR( CV_StsBadFlag, "Pixel order should be used with coi == 0" );

            CV_CALL( cvInitMatHeader( mat, img->height, img->width, type,
                                      img->imageData, img->widthStep ));
        }

        result = mat;
    }
    else if( allowND && CV_IS_MATND_HDR(src) )
    {
        CvMatND* matnd = (CvMatND*)src;
        int i;
        int size1 = matnd->dim[0].size, size2 = 1;
        
        if( !src->data.ptr )
            CV_ERROR( CV_StsNullPtr, "Input array has NULL data pointer" );

        if( !CV_IS_MAT_CONT( matnd->type ))
            CV_ERROR( CV_StsBadArg, "Only continuous nD arrays are supported here" );

        if( matnd->dims > 2 )
            for( i = 1; i < matnd->dims; i++ )
                size2 *= matnd->dim[i].size;
        else
            size2 = matnd->dims == 1 ? 1 : matnd->dim[1].size;

        mat->refcount = 0;
        mat->hdr_refcount = 0;
        mat->data.ptr = matnd->data.ptr;
        mat->rows = size1;
        mat->cols = size2;
        mat->type = CV_MAT_TYPE(matnd->type) | CV_MAT_MAGIC_VAL | CV_MAT_CONT_FLAG;
        mat->step = size2*CV_ELEM_SIZE(matnd->type);
        mat->step &= size1 > 1 ? -1 : 0;

        icvCheckHuge( mat );
        result = mat;
    }
    else
    {
        CV_ERROR( CV_StsBadFlag, "Unrecognized or unsupported array type" );
    }

    __END__;

    if( pCOI )
        *pCOI = coi;

    return result;
}


CV_IMPL CvArr*
cvReshapeMatND( const CvArr* arr,
                int sizeof_header, CvArr* _header,
                int new_cn, int new_dims, int* new_sizes )
{
    CvArr* result = 0;
    CV_FUNCNAME( "cvReshapeMatND" );

    __BEGIN__;

    int dims, coi = 0;

    if( !arr || !_header )
        CV_ERROR( CV_StsNullPtr, "NULL pointer to array or destination header" );

    if( new_cn == 0 && new_dims == 0 )
        CV_ERROR( CV_StsBadArg, "None of array parameters is changed: dummy call?" );

    CV_CALL( dims = cvGetDims( arr ));

    if( new_dims == 0 )
    {
        new_sizes = 0;
        new_dims = dims;
    }
    else if( new_dims == 1 )
    {
        new_sizes = 0;
    }
    else
    {
        if( new_dims <= 0 || new_dims > CV_MAX_DIM )
            CV_ERROR( CV_StsOutOfRange, "Non-positive or too large number of dimensions" );
        if( !new_sizes )
            CV_ERROR( CV_StsNullPtr, "New dimension sizes are not specified" );
    }

    if( new_dims <= 2 )
    {
        CvMat* mat = (CvMat*)arr;
        CvMat* header = (CvMat*)_header;
        int* refcount = 0;
        int  hdr_refcount = 0;
        int  total_width, new_rows, cn;

        if( sizeof_header != sizeof(CvMat))
            CV_ERROR( CV_StsBadArg, "The header should be CvMat" );

        if( mat == header )
        {
            refcount = mat->refcount;
            hdr_refcount = mat->hdr_refcount;
        }
        else if( !CV_IS_MAT( mat ))
            CV_CALL( mat = cvGetMat( mat, header, &coi, 1 ));

        cn = CV_MAT_CN( mat->type );
        total_width = mat->cols * cn;

        if( new_cn == 0 )
            new_cn = cn;

        if( new_sizes )
            new_rows = new_sizes[0];
        else if( new_dims == 1 )
            new_rows = total_width*mat->rows/new_cn;
        else
        {
            new_rows = mat->rows;
            if( new_cn > total_width )
                new_rows = mat->rows * total_width / new_cn;
        }

        if( new_rows != mat->rows )
        {
            int total_size = total_width * mat->rows;

            if( !CV_IS_MAT_CONT( mat->type ))
                CV_ERROR( CV_BadStep,
                "The matrix is not continuous so the number of rows can not be changed" );

            total_width = total_size / new_rows;

            if( total_width * new_rows != total_size )
                CV_ERROR( CV_StsBadArg, "The total number of matrix elements "
                                        "is not divisible by the new number of rows" );
        }

        header->rows = new_rows;
        header->cols = total_width / new_cn;

        if( header->cols * new_cn != total_width ||
            new_sizes && header->cols != new_sizes[1] )
            CV_ERROR( CV_StsBadArg, "The total matrix width is not "
                            "divisible by the new number of columns" );

        header->type = CV_MAKETYPE( mat->type & ~CV_MAT_CN_MASK, new_cn );
        header->step = header->cols * CV_ELEM_SIZE(mat->type);
        header->step &= new_rows > 1 ? -1 : 0;
        header->refcount = refcount;
        header->hdr_refcount = hdr_refcount;
    }
    else
    {
        CvMatND* header = (CvMatND*)_header;

        if( sizeof_header != sizeof(CvMatND))
            CV_ERROR( CV_StsBadSize, "The header should be CvMatND" );
        
        if( !new_sizes )
        {
            if( !CV_IS_MATND( arr ))
                CV_ERROR( CV_StsBadArg, "The source array must be CvMatND" );

            {
            CvMatND* mat = (CvMatND*)arr;
            assert( new_cn > 0 );
            int last_dim_size = mat->dim[mat->dims-1].size*CV_MAT_CN(mat->type);
            int new_size = last_dim_size/new_cn;

            if( new_size*new_cn != last_dim_size )
                CV_ERROR( CV_StsBadArg,
                "The last dimension full size is not divisible by new number of channels");

            if( mat != header )
            {
                memcpy( header, mat, sizeof(*header));
                header->refcount = 0;
                header->hdr_refcount = 0;
            }

            header->dim[header->dims-1].size = new_size;
            header->type = CV_MAKETYPE( header->type & ~CV_MAT_CN_MASK, new_cn );
            }
        }
        else
        {
            CvMatND stub;
            CvMatND* mat = (CvMatND*)arr;
            int i, size1, size2;
            int step;
            
            if( new_cn != 0 )
                CV_ERROR( CV_StsBadArg,
                "Simultaneous change of shape and number of channels is not supported. "
                "Do it by 2 separate calls" );
            
            if( !CV_IS_MATND( mat ))
            {
                CV_CALL( cvGetMatND( mat, &stub, &coi ));
                mat = &stub;
            }

            if( CV_IS_MAT_CONT( mat->type ))
                CV_ERROR( CV_StsBadArg, "Non-continuous nD arrays are not supported" );

            size1 = mat->dim[0].size;
            for( i = 1; i < dims; i++ )
                size1 *= mat->dim[i].size;

            size2 = 1;
            for( i = 0; i < new_dims; i++ )
            {
                if( new_sizes[i] <= 0 )
                    CV_ERROR( CV_StsBadSize,
                    "One of new dimension sizes is non-positive" );
                size2 *= new_sizes[i];
            }

            if( size1 != size2 )
                CV_ERROR( CV_StsBadSize,
                "Number of elements in the original and reshaped array is different" );

            if( header != mat )
            {
                header->refcount = 0;
                header->hdr_refcount = 0;
            }

            header->dims = new_dims;
            header->type = mat->type;
            header->data.ptr = mat->data.ptr;
            step = CV_ELEM_SIZE(header->type);

            for( i = new_dims - 1; i >= 0; i-- )
            {
                header->dim[i].size = new_sizes[i];
                header->dim[i].step = step;
                step *= new_sizes[i];
            }
        }
    }

    if( !coi )
        CV_ERROR( CV_BadCOI, "COI is not supported by this operation" );

    result = _header;
    
    __END__;

    return result;
}


CV_IMPL CvMat*
cvReshape( const CvArr* array, CvMat* header,
           int new_cn, int new_rows )
{
    CvMat* result = 0;
    CV_FUNCNAME( "cvReshape" );

    __BEGIN__;

    CvMat *mat = (CvMat*)array;
    int total_width, new_width;

    if( !header )
        CV_ERROR( CV_StsNullPtr, "" );

    if( !CV_IS_MAT( mat ))
    {
        int coi = 0;
        CV_CALL( mat = cvGetMat( mat, header, &coi, 1 ));
        if( coi )
            CV_ERROR( CV_BadCOI, "COI is not supported" );
    }

    if( new_cn == 0 )
        new_cn = CV_MAT_CN(mat->type);
    else if( (unsigned)(new_cn - 1) > 3 )
        CV_ERROR( CV_BadNumChannels, "" );

    if( mat != header )
    {
        *header = *mat;
        header->refcount = 0;
        header->hdr_refcount = 0;
    }

    total_width = mat->cols * CV_MAT_CN( mat->type );

    if( (new_cn > total_width || total_width % new_cn != 0) && new_rows == 0 )
        new_rows = mat->rows * total_width / new_cn;

    if( new_rows == 0 || new_rows == mat->rows )
    {
        header->rows = mat->rows;
        header->step = mat->step;
    }
    else
    {
        int total_size = total_width * mat->rows;
        if( !CV_IS_MAT_CONT( mat->type ))
            CV_ERROR( CV_BadStep,
            "The matrix is not continuous, thus its number of rows can not be changed" );

        if( (unsigned)new_rows > (unsigned)total_size )
            CV_ERROR( CV_StsOutOfRange, "Bad new number of rows" );

        total_width = total_size / new_rows;

        if( total_width * new_rows != total_size )
            CV_ERROR( CV_StsBadArg, "The total number of matrix elements "
                                    "is not divisible by the new number of rows" );

        header->rows = new_rows;
        header->step = total_width * CV_ELEM_SIZE1(mat->type);
    }

    new_width = total_width / new_cn;

    if( new_width * new_cn != total_width )
        CV_ERROR( CV_BadNumChannels,
        "The total width is not divisible by the new number of channels" );

    header->cols = new_width;
    header->type = CV_MAKETYPE( mat->type & ~CV_MAT_CN_MASK, new_cn );

    result = header;

    __END__;

    return  result;
}


// convert array (CvMat or IplImage) to IplImage
CV_IMPL IplImage*
cvGetImage( const CvArr* array, IplImage* img )
{
    IplImage* result = 0;
    const IplImage* src = (const IplImage*)array;
    
    CV_FUNCNAME( "cvGetImage" );

    __BEGIN__;

    int depth;

    if( !img )
        CV_ERROR_FROM_CODE( CV_StsNullPtr );

    if( !CV_IS_IMAGE_HDR(src) )
    {
        const CvMat* mat = (const CvMat*)src;
        
        if( !CV_IS_MAT_HDR(mat))
            CV_ERROR_FROM_CODE( CV_StsBadFlag );

        if( mat->data.ptr == 0 )
            CV_ERROR_FROM_CODE( CV_StsNullPtr );

        depth = cvCvToIplDepth(mat->type);

        cvInitImageHeader( img, cvSize(mat->cols, mat->rows),
                           depth, CV_MAT_CN(mat->type) );
        cvSetData( img, mat->data.ptr, mat->step );

        result = img;
    }
    else
    {
        result = (IplImage*)src;
    }

    __END__;

    return result;
}


/****************************************************************************************\
*                               IplImage-specific functions                              *
\****************************************************************************************/

static IplROI* icvCreateROI( int coi, int xOffset, int yOffset, int width, int height )
{
    IplROI *roi = 0;

    CV_FUNCNAME( "icvCreateROI" );

    __BEGIN__;

    //if( !CvIPL.createROI )
    {
        CV_CALL( roi = (IplROI*)cvAlloc( sizeof(*roi)));

        roi->coi = coi;
        roi->xOffset = xOffset;
        roi->yOffset = yOffset;
        roi->width = width;
        roi->height = height;
    }
    //else
    //{
    //    roi = CvIPL.createROI( coi, xOffset, yOffset, width, height );
    //}

    __END__;

    return roi;
}

static  void
icvGetColorModel( int nchannels, char** colorModel, char** channelSeq )
{
    static char* tab[][2] =
    {
        {"GRAY", "GRAY"},
        {"",""},
        {"RGB","BGR"},
        {"RGB","BGRA"}
    };

    nchannels--;
    *colorModel = *channelSeq = "";

    if( (unsigned)nchannels <= 3 )
    {
        *colorModel = tab[nchannels][0];
        *channelSeq = tab[nchannels][1];
    }
}


// create IplImage header
CV_IMPL IplImage *
cvCreateImageHeader( CvSize size, int depth, int channels )
{
    IplImage *img = 0;

    CV_FUNCNAME( "cvCreateImageHeader" );

    __BEGIN__;

    //if( !CvIPL.createHeader )
    {
        CV_CALL( img = (IplImage *)cvAlloc( sizeof( *img )));
        CV_CALL( cvInitImageHeader( img, size, depth, channels, IPL_ORIGIN_TL,
                                    CV_DEFAULT_IMAGE_ROW_ALIGN ));
    }
/*
    else
    {
        char *colorModel;
        char *channelSeq;

        icvGetColorModel( channels, &colorModel, &channelSeq );

        img = CvIPL.createHeader( channels, 0, depth, colorModel, channelSeq,
                                  IPL_DATA_ORDER_PIXEL, IPL_ORIGIN_TL,
                                  CV_DEFAULT_IMAGE_ROW_ALIGN,
                                  size.width, size.height, 0, 0, 0, 0 );
    }
*/
    __END__;

    if( cvGetErrStatus() < 0 && img )
        cvReleaseImageHeader( &img );

    return img;
}


// create IplImage header and allocate underlying data
CV_IMPL IplImage *
cvCreateImage( CvSize size, int depth, int channels )
{
    IplImage *img = 0;

    CV_FUNCNAME( "cvCreateImage" );

    __BEGIN__;

    CV_CALL( img = cvCreateImageHeader( size, depth, channels ));
    assert( img );
    CV_CALL( cvCreateData( img ));

    __END__;

    if( cvGetErrStatus() < 0 )
        cvReleaseImage( &img );

    return img;
}


// initalize IplImage header, allocated by the user
CV_IMPL IplImage*
cvInitImageHeader( IplImage * image, CvSize size, int depth,
                   int channels, int origin, int align )
{
    IplImage* result = 0;

    CV_FUNCNAME( "cvInitImageHeader" );

    __BEGIN__;

    char *colorModel, *channelSeq;

    if( !image )
        CV_ERROR( CV_HeaderIsNull, "null pointer to header" );

    memset( image, 0, sizeof( *image ));
    image->nSize = sizeof( *image );

    CV_CALL( icvGetColorModel( channels, &colorModel, &channelSeq ));
    strncpy( image->colorModel, colorModel, 4 );
    strncpy( image->channelSeq, channelSeq, 4 );

    if( size.width < 0 || size.height < 0 )
        CV_ERROR( CV_BadROISize, "Bad input roi" );

    if( (depth != (int)IPL_DEPTH_1U && depth != (int)IPL_DEPTH_8U &&
         depth != (int)IPL_DEPTH_8S && depth != (int)IPL_DEPTH_16U &&
         depth != (int)IPL_DEPTH_16S && depth != (int)IPL_DEPTH_32S && depth != (int)IPL_DEPTH_64S &&
         depth != (int)IPL_DEPTH_32F && depth != (int)IPL_DEPTH_64F) ||
         channels < 0 )
        CV_ERROR( CV_BadDepth, "Unsupported format" );
    if( origin != CV_ORIGIN_BL && origin != CV_ORIGIN_TL )
        CV_ERROR( CV_BadOrigin, "Bad input origin" );

    if( align != 4 && align != 8 )
        CV_ERROR( CV_BadAlign, "Bad input align" );

    image->width = size.width;
    image->height = size.height;

    if( image->roi )
    {
        image->roi->coi = 0;
        image->roi->xOffset = image->roi->yOffset = 0;
        image->roi->width = size.width;
        image->roi->height = size.height;
    }

    image->nChannels = MAX( channels, 1 );
    image->depth = depth;
    image->align = align;
    image->widthStep = (((image->width * image->nChannels *
         (image->depth & ~IPL_DEPTH_SIGN) + 7)/8)+ align - 1) & (~(align - 1));
    image->origin = origin;
    image->imageSize = image->widthStep * image->height;

    result = image;

    __END__;

    return result;
}


CV_IMPL void
cvReleaseImageHeader( IplImage** image )
{
    CV_FUNCNAME( "cvReleaseImageHeader" );

    __BEGIN__;

    if( !image )
        CV_ERROR( CV_StsNullPtr, "" );

    if( *image )
    {
        IplImage* img = *image;
        *image = 0;
        
        //if( !CvIPL.deallocate )
        //{
            cvFree( &img->roi );
            cvFree( &img );
        //}
        //else
        //{
        //    CvIPL.deallocate( img, IPL_IMAGE_HEADER | IPL_IMAGE_ROI );
        //}
    }
    __END__;
}


CV_IMPL void
cvReleaseImage( IplImage ** image )
{
    CV_FUNCNAME( "cvReleaseImage" );

    __BEGIN__

    if( !image )
        CV_ERROR( CV_StsNullPtr, "" );

    if( *image )
    {
        IplImage* img = *image;
        *image = 0;
        
        cvReleaseData( img );
        cvReleaseImageHeader( &img );
    }

    __END__;
}


CV_IMPL void
cvSetImageROI( IplImage* image, CvRect rect )
{
    CV_FUNCNAME( "cvSetImageROI" );

    __BEGIN__;

    if( !image )
        CV_ERROR( CV_HeaderIsNull, "" );

    if( rect.x > image->width || rect.y > image->height )
        CV_ERROR( CV_BadROISize, "" );

    if( rect.x + rect.width < 0 || rect.y + rect.height < 0 )
        CV_ERROR( CV_BadROISize, "" );

    if( rect.x < 0 )
    {
        rect.width += rect.x;
        rect.x = 0;
    }

    if( rect.y < 0 )
    {
        rect.height += rect.y;
        rect.y = 0;
    }

    if( rect.x + rect.width > image->width )
        rect.width = image->width - rect.x;

    if( rect.y + rect.height > image->height )
        rect.height = image->height - rect.y;

    if( image->roi )
    {
        image->roi->xOffset = rect.x;
        image->roi->yOffset = rect.y;
        image->roi->width = rect.width;
        image->roi->height = rect.height;
    }
    else
    {
        CV_CALL( image->roi = icvCreateROI( 0, rect.x, rect.y, rect.width, rect.height ));
    }

    __END__;
}


CV_IMPL void
cvResetImageROI( IplImage* image )
{
    CV_FUNCNAME( "cvResetImageROI" );

    __BEGIN__;

    if( !image )
        CV_ERROR( CV_HeaderIsNull, "" );

    if( image->roi )
    {
        //if( !CvIPL.deallocate )
        {
            cvFree( &image->roi );
        }
        //else
        //{
        //    CvIPL.deallocate( image, IPL_IMAGE_ROI );
        //    image->roi = 0;
        //}
    }

    __END__;
}


CV_IMPL CvRect
cvGetImageROI( const IplImage* img )
{
    CvRect rect = { 0, 0, 0, 0 };
    
    CV_FUNCNAME( "cvGetImageROI" );

    __BEGIN__;

    if( !img )
        CV_ERROR( CV_StsNullPtr, "Null pointer to image" );

    if( img->roi )
        rect = cvRect( img->roi->xOffset, img->roi->yOffset,
                       img->roi->width, img->roi->height );
    else
        rect = cvRect( 0, 0, img->width, img->height );

    __END__;
    
    return rect;
}


CV_IMPL void
cvSetImageCOI( IplImage* image, int coi )
{
    CV_FUNCNAME( "cvSetImageCOI" );

    __BEGIN__;

    if( !image )
        CV_ERROR( CV_HeaderIsNull, "" );

    if( (unsigned)coi > (unsigned)(image->nChannels) )
        CV_ERROR( CV_BadCOI, "" );

    if( image->roi || coi != 0 )
    {
        if( image->roi )
        {
            image->roi->coi = coi;
        }
        else
        {
            CV_CALL( image->roi = icvCreateROI( coi, 0, 0, image->width, image->height ));
        }
    }

    __END__;
}


CV_IMPL int
cvGetImageCOI( const IplImage* image )
{
    int coi = -1;
    CV_FUNCNAME( "cvGetImageCOI" );

    __BEGIN__;

    if( !image )
        CV_ERROR( CV_HeaderIsNull, "" );

    coi = image->roi ? image->roi->coi : 0;

    __END__;

    return coi;
}


CV_IMPL IplImage*
cvCloneImage( const IplImage* src )
{
    IplImage* dst = 0;
    CV_FUNCNAME( "cvCloneImage" );

    __BEGIN__;

    if( !CV_IS_IMAGE_HDR( src ))
        CV_ERROR( CV_StsBadArg, "Bad image header" );

    //if( !CvIPL.cloneImage )
    {
        CV_CALL( dst = (IplImage*)cvAlloc( sizeof(*dst)));

        memcpy( dst, src, sizeof(*src));
        dst->imageData = dst->imageDataOrigin = 0;
        dst->roi = 0;

        if( src->roi )
        {
            dst->roi = icvCreateROI( src->roi->coi, src->roi->xOffset,
                          src->roi->yOffset, src->roi->width, src->roi->height );
        }

        if( src->imageData )
        {
            int size = src->imageSize;
            cvCreateData( dst );
            memcpy( dst->imageData, src->imageData, size );
        }
    }
    //else
    //{
    //    dst = CvIPL.cloneImage( src );
    //}

    __END__;

    return dst;
}


/****************************************************************************************\
*                            Additional operations on CvTermCriteria                     *
\****************************************************************************************/

CV_IMPL CvTermCriteria
cvCheckTermCriteria( CvTermCriteria criteria, double default_eps,
                     int default_max_iters )
{
    CV_FUNCNAME( "cvCheckTermCriteria" );

    CvTermCriteria crit;

    crit.type = CV_TERMCRIT_ITER|CV_TERMCRIT_EPS;
    crit.max_iter = default_max_iters;
    crit.epsilon = (float)default_eps;
    
    __BEGIN__;

    if( (criteria.type & ~(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER)) != 0 )
        CV_ERROR( CV_StsBadArg,
                  "Unknown type of term criteria" );

    if( (criteria.type & CV_TERMCRIT_ITER) != 0 )
    {
        if( criteria.max_iter <= 0 )
            CV_ERROR( CV_StsBadArg,
                  "Iterations flag is set and maximum number of iterations is <= 0" );
        crit.max_iter = criteria.max_iter;
    }
    
    if( (criteria.type & CV_TERMCRIT_EPS) != 0 )
    {
        if( criteria.epsilon < 0 )
            CV_ERROR( CV_StsBadArg, "Accuracy flag is set and epsilon is < 0" );

        crit.epsilon = criteria.epsilon;
    }

    if( (criteria.type & (CV_TERMCRIT_EPS | CV_TERMCRIT_ITER)) == 0 )
        CV_ERROR( CV_StsBadArg,
                  "Neither accuracy nor maximum iterations "
                  "number flags are set in criteria type" );

    __END__;

    crit.epsilon = (float)MAX( 0, crit.epsilon );
    crit.max_iter = MAX( 1, crit.max_iter );

    return crit;
}


/* End of file. */
