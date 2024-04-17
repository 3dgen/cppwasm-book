#ifndef EM_PORT_API
#	if defined(__EMSCRIPTEN__)
#		include <emscripten.h>
#		if defined(__cplusplus)
#			define EM_PORT_API(rettype) extern "C" rettype EMSCRIPTEN_KEEPALIVE
#		else
#			define EM_PORT_API(rettype) rettype EMSCRIPTEN_KEEPALIVE
#		endif
#	else
#		if defined(__cplusplus)
#			define EM_PORT_API(rettype) extern "C" rettype
#		else
#			define EM_PORT_API(rettype) rettype
#		endif
#	endif
#endif

#include <stdint.h>
#include <malloc.h>

uint8_t *img_buf = NULL;
int img_width = 0, img_height = 0;

EM_PORT_API(uint8_t*) get_img_buf(int w, int h) {
	if (img_buf == NULL || w != img_width || h != img_height) {
		if (img_buf) {
            free(img_buf);
        }
		img_buf = (uint8_t*)malloc(w * h * 4);
		img_width = w;
		img_height = h;
	}

	return img_buf;
}

EM_PORT_API(void) draw_circle(int cx, int cy, int radii) {
	int sq = radii * radii;
	for (int y = 0; y < img_height; y++) {
		for (int x = 0; x < img_width; x++) {
			int d = (y - cy) * (y - cy) + (x - cx) * (x - cx);
			if (d < sq) {
				img_buf[(y * img_width + x) * 4] = 255;		//r
				img_buf[(y * img_width + x) * 4 + 1] = 0;	//g
				img_buf[(y * img_width + x) * 4 + 2] = 0;	//b
				img_buf[(y * img_width + x) * 4 + 3] = 255;	//a
			}
			else {
				img_buf[(y * img_width + x) * 4] = 0;		//r
				img_buf[(y * img_width + x) * 4 + 1] = 255;	//g
				img_buf[(y * img_width + x) * 4 + 2] = 255;	//b
				img_buf[(y * img_width + x) * 4 + 3] = 255;	//a
			}
		}
	}
}