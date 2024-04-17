// Copyright 2016 ChaiShushan (chaishushan{AT}gmail.com).
// License: https://creativecommons.org/licenses/by-nc-sa/4.0/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <SDL/SDL.h>

#ifdef __EMSCRIPTEN__
#	include <emscripten.h>
#endif

// Tweetable Mathematical Art
// http://codegolf.stackexchange.com/questions/35569/tweetable-mathematical-art
//
// Ray Tracing
// http://www.cnblogs.com/daishuo/p/3954422.html

#define DIM    400
#define DM1    (DIM-1)
#define _sq(x) ((x)*(x))                         // square
#define _cb(x) abs((x)*(x)*(x))                  // absolute value of cube
#define _cr(x) (unsigned char)(pow((x),1.0/3.0)) // cube root
 
unsigned char GR(int,int,int,int);
unsigned char BL(int,int,int,int);

unsigned char RD(int i, int j, int center_x, int center_y) {
	return BL(i, j, center_x, center_y);
}

unsigned char GR(int i, int j, int center_x, int center_y) {
	return BL(i, j, center_x, center_y);
}

unsigned char BL(int i, int j, int center_x, int center_y) {
	const int n = 3;

	float r = sqrt(0.f+_sq(i-center_y)+_sq(j-center_x));
	float s = r/((DIM/2)/2);

	if(s < 1) {
		i -= center_y;
		j -= center_x;

		float k = (
			sqrt(1.-_sq(s)) * sqrt(1.-_sq(s/n)) +
			_sq(s)/n
		) * n;

		i = i/k + center_y;
		j = j/k + center_x;
	}

	return (i/((DIM/2)/8)+j/((DIM/2)/8))%2*255;
}

SDL_Surface *screen = NULL;
int center_x = -1;
int center_y = -1;

void renderloop() {
	SDL_Flip(screen); 

	if (SDL_MUSTLOCK(screen)) {
		SDL_LockSurface(screen);
	}

	bool center_changed = false;
	if(center_x < 0 || center_y < 0) {
		center_x = DIM/2;
		center_y = DIM/2;
		center_changed = true;
	}

	SDL_Event event;
	while(SDL_PollEvent(&event)) {
		switch(event.type) {
		case SDL_QUIT:
			break;

		case SDL_MOUSEMOTION:
			if(center_x != event.button.x || center_y != event.button.y) {
				center_x = event.button.x;
				center_y = event.button.y;
				center_changed = true;
			}

			printf("mouse(x, y): (%d, %d)\n", center_x, center_y);
			fflush(stdout);
			break;
		}
	}

	if(center_changed) {
		for (int i = 0; i < DIM; i++) {
			for (int j = 0; j < DIM; j++) {
				*((Uint32*)screen->pixels + i * DIM + j) = SDL_MapRGBA(screen->format,
					RD(i, j, center_x, center_y) & 255,
					GR(i, j, center_x, center_y) & 255,
					BL(i, j, center_x, center_y) & 255,
					255
				);
			}
		}
	}

	if (SDL_MUSTLOCK(screen)) {
		SDL_UnlockSurface(screen);
	}
}

int main(int argc, char* argv[]) {
	printf("hello, world!\n");

	SDL_Init(SDL_INIT_VIDEO);
	screen = SDL_SetVideoMode(DIM, DIM, 32, SDL_ANYFORMAT);
	if(screen == NULL) {
		SDL_Quit();
		return 1;
	}

	emscripten_set_main_loop(renderloop,0,0);
	return 0;
}
