# 7.3 Keyboard events

Handling `Canvas` keyboard events is not as straightforward as handling mouse events, because `Canvas` itself does not support keyboard event. We need to do a little trick - add another object on the `Canvas` that supports keyboard events.

The example in this section will add keyboard events based on the excample in Section 7.2.

html:

```html
//canvas_mk_event.html
    <canvas id="myCanvas" tabindex="0"></canvas>
    <script>
    Module = {};
    Module.onRuntimeInitialized = function() {
        //... ...
        canvas.addEventListener("mousemove", onMouseMove, true);
		canvas.addEventListener("keydown", onKeyDown, true);
		canvas.focus();
	  }
    }

    //... ...
	
	function onKeyDown(event) {
	  Module.ccall('on_key_down', 'null', ['string'], [event.key]);
	}
    </script>
    <script src="canvas_mk_event.js"></script>

```

`<canvas id="myCanvas" tabindex="0"></canvas>` A `tabindex` element is attached to the `Canvas` to support keyboard events. The method of adding a keyboard event handler is similar to 7.2.

The corresponding handler is added to the C code:

```c
//canvas_mk_event.cc
EM_PORT_API(void) on_key_down(const char* key) {
	printf("on_key_down(); key:%s\n", key);
}
```

Compile with the following command:

```
emcc canvas_mk_event.cc -s "EXTRA_EXPORTED_RUNTIME_METHODS=['ccall']" -o canvas_mk_event.js
```

Browse the page, the console outputs:

![](images/03-mk.png)