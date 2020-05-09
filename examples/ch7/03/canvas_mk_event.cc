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
#include <stdio.h>

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

EM_PORT_API(void) on_mouse_move(int x, int y) {
	if (img_buf == NULL) {
		printf("img_buf not ready!\n");
		return;
	}
	if (x >= img_width || x < 0 || y >= img_height || y <0) {
		printf("out of range!\n");
		return;
	}
	
	printf("on_mouse_move(); x:%d; y:%d; RGBA:(%d, %d, %d, %d)\n", x, y,
		img_buf[(y * img_width + x) * 4],
		img_buf[(y * img_width + x) * 4 + 1],
		img_buf[(y * img_width + x) * 4 + 2],
		img_buf[(y * img_width + x) * 4 + 3]);
}

EM_PORT_API(void) on_key_down(const char* key) {
	printf("on_key_down(); key:%s\n", key);
}