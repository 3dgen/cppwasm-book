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


#include "cxcore.h"
#include <stdio.h>

typedef struct
{
    const char* file;
    int         line;
}
CvStackRecord;

typedef struct CvContext
{
    int  err_code;
    int  err_mode;
    CvErrorCallback error_callback;
    void*  userdata;
    char  err_msg[4096];
    CvStackRecord  err_ctx;
} CvContext;

#if defined WIN32 || defined WIN64
#define CV_DEFAULT_ERROR_CALLBACK  cvGuiBoxReport
#else
#define CV_DEFAULT_ERROR_CALLBACK  cvStdErrReport
#endif

static CvContext*
icvCreateContext(void)
{
    CvContext* context = (CvContext*)malloc( sizeof(*context) );

    context->err_mode = CV_ErrModeLeaf;
    context->err_code = CV_StsOk;

    context->error_callback = CV_DEFAULT_ERROR_CALLBACK;
    context->userdata = 0;

    return context;
}

static void
icvDestroyContext(CvContext* context)
{
    free(context);
}


static CvContext*
icvGetContext(void)
{
#ifdef CV_DLL
#if defined WIN32 || defined WIN64
    CvContext* context;

    //assert(g_TlsIndex != TLS_OUT_OF_INDEXES);
    if( g_TlsIndex == TLS_OUT_OF_INDEXES )
    {
        g_TlsIndex = TlsAlloc();
        if( g_TlsIndex == TLS_OUT_OF_INDEXES )
            FatalAppExit( 0, "Only set CV_DLL for DLL usage" );
    }

    context = (CvContext*)TlsGetValue( g_TlsIndex );
    if( !context )
    {
        context = icvCreateContext();
        if( !context )
            FatalAppExit( 0, "OpenCV. Problem to allocate memory for TLS OpenCV context." );

        TlsSetValue( g_TlsIndex, context );
    }
    return context;
#else
    CvContext* context = (CvContext*)pthread_getspecific( g_TlsIndex );
    if( !context )
    {
    context = icvCreateContext();
    if( !context )
    {
            fprintf(stderr,"OpenCV. Problem to allocate memory for OpenCV context.");
        exit(1);
    }
    pthread_setspecific( g_TlsIndex, context );
    }
    return context;
#endif
#else /* static single-thread library case */
    static CvContext* context = 0;

    if( !context )
        context = icvCreateContext();

    return context;
#endif
}


CV_IMPL int
cvStdErrReport( int code, const char *func_name, const char *err_msg,
                const char *file, int line, void* )
{
    if( code == CV_StsBackTrace || code == CV_StsAutoTrace )
        fprintf( stderr, "\tcalled from " );
    else
        fprintf( stderr, "OpenCV ERROR: %s (%s)\n\tin function ",
                 cvErrorStr(code), err_msg ? err_msg : "no description" );

    fprintf( stderr, "%s, %s(%d)\n", func_name ? func_name : "<unknown>",
             file != NULL ? file : "", line );

    if( cvGetErrMode() == CV_ErrModeLeaf )
    {
        fprintf( stderr, "Terminating the application...\n" );
        return 1;
    }
    else
        return 0;
}


CV_IMPL int
cvGuiBoxReport( int code, const char *func_name, const char *err_msg,
                const char *file, int line, void* )
{
#if 1 //!defined WIN32 && !defined WIN64
    return cvStdErrReport( code, func_name, err_msg, file, line, 0 );
#else
    if( code != CV_StsBackTrace && code != CV_StsAutoTrace )
    {
        size_t msg_len = strlen(err_msg ? err_msg : "") + 1024;
        char* message = (char*)cvAlloc(msg_len);
        char title[100];

        wsprintf( message, "%s (%s)\nin function %s, %s(%d)\n\n"
                  "Press \"Abort\" to terminate application.\n"
                  "Press \"Retry\" to debug (if the app is running under debugger).\n"
                  "Press \"Ignore\" to continue (this is not safe).\n",
                  cvErrorStr(code), err_msg ? err_msg : "no description",
                  func_name, file, line );

        wsprintf( title, "OpenCV GUI Error Handler" );

        int answer = MessageBox( NULL, message, title, MB_ICONERROR|MB_ABORTRETRYIGNORE|MB_SYSTEMMODAL );

		cvFree(&message);
        if( answer == IDRETRY )
        {
            CV_DBG_BREAK();
        }
        return answer != IDIGNORE;
    }
    return 0;
#endif
}


CV_IMPL int cvNulDevReport( int /*code*/, const char* /*func_name*/,
    const char* /*err_msg*/, const char* /*file*/, int /*line*/, void* )
{
    return cvGetErrMode() == CV_ErrModeLeaf;
}


CV_IMPL CvErrorCallback
cvRedirectError( CvErrorCallback func, void* userdata, void** prev_userdata )
{
    CvContext* context = icvGetContext();

    CvErrorCallback old = context->error_callback;
    if( prev_userdata )
        *prev_userdata = context->userdata;
    if( func )
    {
        context->error_callback = func;
        context->userdata = userdata;
    }
    else
    {
        context->error_callback = CV_DEFAULT_ERROR_CALLBACK;
        context->userdata = 0;
    }

    return old;
}


CV_IMPL int cvGetErrInfo( const char** errorcode_desc, const char** description,
                          const char** filename, int* line )
{
    int code = cvGetErrStatus();

    if( errorcode_desc )
        *errorcode_desc = cvErrorStr( code );

    if( code >= 0 )
    {
        if( description )
            *description = 0;
        if( filename )
            *filename = 0;
        if( line )
            *line = 0;
    }
    else
    {
        CvContext* ctx = icvGetContext();

        if( description )
            *description = ctx->err_msg;
        if( filename )
            *filename = ctx->err_ctx.file;
        if( line )
            *line = ctx->err_ctx.line;
    }

    return code;
}


CV_IMPL const char* cvErrorStr( int status )
{
    static char buf[256];

    switch (status)
    {
    case CV_StsOk :        return "No Error";
    case CV_StsBackTrace : return "Backtrace";
    case CV_StsError :     return "Unspecified error";
    case CV_StsInternal :  return "Internal error";
    case CV_StsNoMem :     return "Insufficient memory";
    case CV_StsBadArg :    return "Bad argument";
    case CV_StsNoConv :    return "Iterations do not converge";
    case CV_StsAutoTrace : return "Autotrace call";
    case CV_StsBadSize :   return "Incorrect size of input array";
    case CV_StsNullPtr :   return "Null pointer";
    case CV_StsDivByZero : return "Divizion by zero occured";
    case CV_BadStep :      return "Image step is wrong";
    case CV_StsInplaceNotSupported : return "Inplace operation is not supported";
    case CV_StsObjectNotFound :      return "Requested object was not found";
    case CV_BadDepth :     return "Input image depth is not supported by function";
    case CV_StsUnmatchedFormats : return "Formats of input arguments do not match";
    case CV_StsUnmatchedSizes :  return "Sizes of input arguments do not match";
    case CV_StsOutOfRange : return "One of arguments\' values is out of range";
    case CV_StsUnsupportedFormat : return "Unsupported format or combination of formats";
    case CV_BadCOI :      return "Input COI is not supported";
    case CV_BadNumChannels : return "Bad number of channels";
    case CV_StsBadFlag :   return "Bad flag (parameter or structure field)";
    case CV_StsBadPoint :  return "Bad parameter of type CvPoint";
    case CV_StsBadMask : return "Bad type of mask argument";
    case CV_StsParseError : return "Parsing error";
    case CV_StsNotImplemented : return "The function/feature is not implemented";
    case CV_StsBadMemBlock :  return "Memory block has been corrupted";
    };

    sprintf(buf, "Unknown %s code %d", status >= 0 ? "status":"error", status);
    return buf;
}

CV_IMPL int cvGetErrMode(void)
{
    return icvGetContext()->err_mode;
}

CV_IMPL int cvSetErrMode( int mode )
{
    CvContext* context = icvGetContext();
    int prev_mode = context->err_mode;
    context->err_mode = mode;
    return prev_mode;
}

CV_IMPL int cvGetErrStatus()
{
    return icvGetContext()->err_code;
}

CV_IMPL void cvSetErrStatus( int code )
{
    icvGetContext()->err_code = code;
}


CV_IMPL void cvError( int code, const char* func_name,
                      const char* err_msg,
                      const char* file_name, int line )
{
    if( code == CV_StsOk )
        cvSetErrStatus( code );
    else
    {
        CvContext* context = icvGetContext();

        if( code != CV_StsBackTrace && code != CV_StsAutoTrace )
        {
            char* message = context->err_msg;
            context->err_code = code;

            strcpy( message, err_msg );
            context->err_ctx.file = file_name;
            context->err_ctx.line = line;
        }

        if( context->err_mode != CV_ErrModeSilent )
        {
            int terminate = context->error_callback( code, func_name, err_msg,
                                                    file_name, line, context->userdata );
            if( terminate )
            {
#if !defined WIN32 && !defined WIN64
                assert(0); // for post-mortem analysis with GDB
#endif
                exit(-abs(terminate));
            }
        }
    }
}


/******************** End of implementation of profiling stuff *********************/



/* function - destructor of thread */
void icvPthreadDestructor(void* key_val)
{
    CvContext* context = (CvContext*) key_val;
    icvDestroyContext( context );
}



/* function, which converts int to int */
CV_IMPL int
cvErrorFromIppStatus( int status )
{
    switch (status)
    {
    case CV_BADSIZE_ERR: return CV_StsBadSize;
    case CV_BADMEMBLOCK_ERR: return CV_StsBadMemBlock;
    case CV_NULLPTR_ERR: return CV_StsNullPtr;
    case CV_DIV_BY_ZERO_ERR: return CV_StsDivByZero;
    case CV_BADSTEP_ERR: return CV_BadStep ;
    case CV_OUTOFMEM_ERR: return CV_StsNoMem;
    case CV_BADARG_ERR: return CV_StsBadArg;
    case CV_NOTDEFINED_ERR: return CV_StsError;
    case CV_INPLACE_NOT_SUPPORTED_ERR: return CV_StsInplaceNotSupported;
    case CV_NOTFOUND_ERR: return CV_StsObjectNotFound;
    case CV_BADCONVERGENCE_ERR: return CV_StsNoConv;
    case CV_BADDEPTH_ERR: return CV_BadDepth;
    case CV_UNMATCHED_FORMATS_ERR: return CV_StsUnmatchedFormats;
    case CV_UNSUPPORTED_COI_ERR: return CV_BadCOI;
    case CV_UNSUPPORTED_CHANNELS_ERR: return CV_BadNumChannels;
    case CV_BADFLAG_ERR: return CV_StsBadFlag;
    case CV_BADRANGE_ERR: return CV_StsBadArg;
    case CV_BADCOEF_ERR: return CV_StsBadArg;
    case CV_BADFACTOR_ERR: return CV_StsBadArg;
    case CV_BADPOINT_ERR: return CV_StsBadPoint;

    default: return CV_StsError;
    }
}
/* End of file */



