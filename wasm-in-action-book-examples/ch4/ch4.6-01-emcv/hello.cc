// Copyright 2016 ChaiShushan (chaishushan{AT}gmail.com).
// License: https://creativecommons.org/licenses/by-nc-sa/4.0/

#include "libfacedetection_capi.h"

#include <highgui.h>

#include <SDL/SDL.h>

#ifdef __EMSCRIPTEN__
#	include <emscripten.h>
#endif

SDL_Surface *g_pSurface = NULL;
IplImage    *g_pImgLena = NULL;
libfacedetection_capi_result_t* g_faceDB = NULL;

void detectFace(IplImage *pImg) {
	if(g_faceDB != NULL) return;

	g_faceDB = libfacedetection_capi_facedetect_rgb(
		(uint8_t*)(pImg->imageData),
		pImg->width, pImg->height,
		pImg->widthStep
	);

	int faceCount = libfacedetection_capi_result_len(g_faceDB);
	for(int i = 0; i < faceCount; i++) {
		libfacedetection_capi_face_t face;
		if(!libfacedetection_capi_result_get(g_faceDB, 0, &face)) {
			break;
		}

		printf("face: x = %d, y = %d, w = %d, h = %d\n",
			face.x, face.y, face.w, face.h
		);
		cvRectangle(pImg,
			cvPoint(face.x, face.y),
			cvPoint(face.x+face.w, face.y+face.h),
			CV_RGB(0, 255,0),
			3, 1, 0
		);
	}
}

void renderIplImage(SDL_Surface* screen, IplImage *pImg) {
	for (int i = 0; i < screen->h; i++) {
		for (int j = 0; j < screen->w; j++) {
			uchar R = CV_IMAGE_ELEM(pImg, uchar, i, j*3+0);
			uchar G = CV_IMAGE_ELEM(pImg, uchar, i, j*3+1);
			uchar B = CV_IMAGE_ELEM(pImg, uchar, i, j*3+2);

			*((Uint32*)screen->pixels+i*screen->w+j) = SDL_MapRGBA(
				screen->format, B, G, R, 255
			);
		}
	}
}

void renderloop() {
	SDL_LockSurface(g_pSurface);
	{
		detectFace(g_pImgLena);
		renderIplImage(g_pSurface, g_pImgLena);
	}
	SDL_UnlockSurface(g_pSurface);

	SDL_Flip(g_pSurface);
}

int main(int argc, char* argv[]) {
	SDL_Init(SDL_INIT_VIDEO);

	g_pImgLena = cvLoadImage("/lena512color.bmp", 0);
	if(g_pImgLena == NULL) {
		printf("load '%s' failed!\n", "lena512color.bmp");
		abort();
	}

	g_pSurface = SDL_SetVideoMode(
		g_pImgLena->width, g_pImgLena->height, 32,
		SDL_ANYFORMAT
	);

	emscripten_set_main_loop(renderloop,0,0);
	return 0;
}
