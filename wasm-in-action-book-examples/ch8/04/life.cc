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
#include <time.h>
#include <stdlib.h>

bool *cells0 = NULL, *cells1 = NULL;
uint32_t *img_buf = NULL;
int width = 0, height = 0;
bool pausing = false;
int scale = 2;

void create_seeds() {
	if (cells0 == NULL) return;

	srand(clock());
	for (int i = 0; i < width * height; i++){
		cells0[i] = (rand() % 2) != 1;
	}
}

EM_PORT_API(void) init_env(int w, int h, int s) {
	if (cells0) free(cells0);
	if (cells1) free(cells1);
	if (img_buf) free(img_buf);

	width = w;
	height = h;
	scale = s;

	cells0 = (bool*)malloc(w * h);
	cells1 = (bool*)malloc(w * h);
	img_buf = (uint32_t*)malloc(w * h * scale * scale * 4);

	create_seeds();
}

struct DIR{
	int x, y;
};
void evolve(){
	static DIR dirs[] = {{-1, -1}, {0, -1}, {1, -1}, {-1, 0}, {1, 0}, {-1, 1}, {0, 1}, {1, 1}};

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			int live_count = 0;
			for (int i = 0; i < 8; i++) {
				int nx = (x + dirs[i].x + width) % width;
				int ny = (y + dirs[i].y + height) % height;
				if (cells0[ny * width + nx]) {
					live_count++;
				}
			}

			if (cells0[y * width + x]) {
				switch (live_count) {
					case 2:
					case 3:
						cells1[y * width + x] = true;
					break;

					default:
						cells1[y * width + x] = false;
					break;
				}
			}
			else {
				switch (live_count) {
					case 3:
						cells1[y * width + x] = true;
					break;

					default:
						cells1[y * width + x] = false;
					break;
				}
			}
		}
	}

	bool *temp = cells0;
	cells0 = cells1;
	cells1 = temp;
}

EM_PORT_API(uint8_t*) step() {
	if (img_buf == NULL) return NULL;

	if (!pausing) {
		evolve();
	}

	for (int x = 0; x < width; x++){
		for (int y = 0; y < height; y++){
			uint32_t color = cells0[y * width + x] ? 0xFF0000FF : 0xFFFFFFFF;
			for (int i = 0; i < scale; i++){
				for (int j = 0; j < scale; j++){
					int d = ((y * scale + j) * width * scale + x * scale + i);
					img_buf[d] = color;
				}
			}
		}
	}		

	return (uint8_t*)img_buf;
}

EM_PORT_API(void) on_mouse_click(int x, int y){
	if (!pausing) return;

	x /= scale;
	y /= scale;

	if (x < 0 || x >= width || y < 0 || y >= height) return;

	cells0[y * width + x] = !cells0[y * width + x];
}

EM_PORT_API(void) on_key_up(const char* key) {
	if (!key) return;

	switch(*key) {
	case 'p':
		pausing = !pausing;
		break;
	case 'r':
		create_seeds();
		break;
	}
}