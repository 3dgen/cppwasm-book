# 7.4 7.4 Conway's Game of Life

This section will use the techniques in sections 7.1 through 7.3 to write a complete interactive mini-game "Life".

## 7.4.1 Introduction to *Conway's Game of Life*

*Conway's Game of Life* is a cellular automaton that simulates a virtual two-dimensional grid life, each grid in the grids is called a cell, and each cell is in one of two possible states, *alive* or *dead*, and the cell state changes according to the following rules:

- Any live cell with fewer than two live neighbors dies.
- Any live cell with two or three live neighbors lives on to the next generation.
- Any live cell with more than three live neighbors dies.
- Any dead cell with exactly three live neighbors becomes a live cell.

> **info** For more information about *Life*, refer to [https://en.wikipedia.org/wiki/Conway%27s_Game_of_Life](https://en.wikipedia.org/wiki/Conway%27s_Game_of_Life)。

## 7.4.2 Design requirements

We will implement the *Life* game on the webpage, hopefully it has the following features:

1. The status of all cells can be set randomly through keyboard shortcuts;
1. Pause/resume game running at any time via keyboard shortcuts;
1. When the game is paused, you can click on a cell to reverse its state.

## 7.4.3 C code analysis

```c
//life.cc
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
```
- In *Life*, since the generations are isolated from each other, we allocated two buffers `cells0` and `cells1`;
- `img_buf` is the image buffer that is finally output to `Canvas`. Since 1 pixel is very small on the screen, we set the stretch factor `scale`, and 1 cell will occupy `scale * scale` pixels in the image.
- The export function `init_env()` is used to initialize each buffer used internally and save parameters such as grid length and width;
- The `create_seeds()` function is used for random initialization of all cell states.

```c
//life.cc
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
```

- The `evolve()` function evolves according to the rules of *Life*. Each time it evolves, the state of `cells1` is calculated according to the state of `cells0`, and then the two are swapped. Note that we have set up a circular two-dimensional space here. Logically, the leftmost and rightmost sides of the mesh are connected together, and the uppermost and lowermost sides are connected together.

```c
//life.cc
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
```

- The export function `step()` determines whether evolving is required according to the pause flag `pausing`, and then converts the current generation state (`cells0`) into image data and returns;
- The export function `on_mouse_click()` is used to handle the mouse click event of the `Canvas`;
- The export function `on_key_up()` is used to handle keyboard events.

## 7.4.4 Web page code analysis

```html
//life.html
    <canvas id="myCanvas" tabindex="0"></canvas>
    <p id = 'tip'>Loading WebAssembly...</p>
    <script>
    Module = {};
    Module.onRuntimeInitialized = function() {
      var canvas = document.getElementById('myCanvas');
      var ctx = canvas.getContext("2d");
      canvas.width = 512;
      canvas.height = 512;
      Module._init_env(256, 256, 2);

      canvas.addEventListener("click", onMouseClick, true);
      canvas.addEventListener("keyup", onKeyUp, true);
      canvas.focus();

      window.requestAnimationFrame(update);
      var tip = document.getElementById('tip');
      tip.innerHTML = "Press 'p' to pause/resume, 'r' to reset. Click cell to invert it's state while pausing.";
    }
```

- When `Module.onRuntimeInitialized` callback, the size of the *Life* is initialized to 256 * 256 (the stretch factor is 2, so the `Canvas` size is 512 * 512), and the event handler of the keyboard and mouse is set.

```js
//life.html
    function update() {
      var buf_addr = Module._step();
      var u8o = new Uint8ClampedArray(Module.HEAPU8.subarray(buf_addr,
        buf_addr + 512 * 512 * 4));
      var imgData = new ImageData(u8o, 512, 512);

      var canvas = document.getElementById('myCanvas');
      var ctx = canvas.getContext('2d');
      ctx.putImageData(imgData, 0, 0);

      window.requestAnimationFrame(update);
    }
```

- The `update()` function calls the C function `step()` to evolve and update the resulting image to `Canvas`.

```js
//life.html
    function getPointOnCanvas(canvas, x, y) {
      var bbox = canvas.getBoundingClientRect();
      return {
        x: x - bbox.left * (canvas.width / bbox.width),
        y: y - bbox.top * (canvas.height / bbox.height)
      };    
    }

    function onMouseClick(event) {
      var canvas = document.getElementById('myCanvas');
      var loc = getPointOnCanvas(canvas, event.clientX, event.clientY);
      Module._on_mouse_click(loc.x, loc.y);
    }
	
	  function onKeyUp(event) {
      Module.ccall('on_key_up', 'null', ['string'], [event.key]);
    }
```

- The `onMouseClick()` function handles mouse click events;
- The `onKeyUp()` function handles keyboard events.

## 7.4.5 Running *Life*

Compile with the following command:

```
emcc life.cc -s "EXTRA_EXPORTED_RUNTIME_METHODS=['ccall']" -o life.js
```

After browsing the page, the evolving of the cells will be displayed. Below is a screenshot:

![](images/04-life.png)

The keyboard button 'p' is used to switch the *pause*/*run* state, and 'r' is used for reset. In the *pause* state, clicking on a cell with the mouse can toggle its state.