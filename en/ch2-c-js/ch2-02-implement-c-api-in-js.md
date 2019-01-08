# 2.2 Implement C API in JavaScript

Emscripten provides a variety of methods for calling JavaScript functions from C, including:
- `EM_JS`/`EM_ASM` macro inline JavaScript code.
- `emscripten_run_script` function.
- Implement C API in JavaScript, or another way to say "JavaScript function injection".

This section will focus on the last one.

## 2.2.1 C function declaration

In the C environment, we often encounter this situation: Module A calls the function implemented by module B(creating a function declaration in module A and a function body in module B). In Emscripten, the C code part is module A and the JavaScript code part is module B. For example, create `capi_js.cc` as follows:

```c
//capi_js.cc
EM_PORT_API(int) js_add(int a, int b);
EM_PORT_API(void) js_console_log_int(int param);

EM_PORT_API(void) print_the_answer() {
    int i = js_add(21, 21);
    js_console_log_int(i);
}
```

`print_the_answer()` calls the function `js_add()` to calculate 21+21, then calls `js_console_log_int()` to print the result, the latter only gives the declaration, and they will be implemented in JavaScript.

For the same reason as 2.1.1, the C function API declaration should use C-style name decoration. For the sake of simplicity, we used the `EM_PORT_API` macro when declaring `js_add()` and `js_console_log_int()` (this macro will add the `EMSCRIPTEN_KEEPALIVE` definition, since the function is not implemented in C code, at this time` EMSCRIPTEN_KEEPALIVE` has no practical effect).

## 2.2.2 Implement C functions in JavaScript

Create a JavaScript source file `pkg.js` as follows:

```js
//pkg.js
mergeInto(LibraryManager.library, {
    js_add: function (a, b) {
        console.log("js_add");
        return a + b;
    },

    js_console_log_int: function (param) {
        console.log("js_console_log_int:" + param);
    }
})
```

The code above defines two objects `js_add` and `js_console_log_int` according to the declarations of the two C functions, and merges them into `LibraryManager.library` - in JavaScript, methods (or functions) are also objects.

> **tips** `LibraryManager.library` can be simply understood as a library for injecting JavaScript into the C environment, as described in 2.2.1, "Module B". Although in fact it is far more complicated than this, this simple analogy is sufficient for most applications.

Now we execute the following command:

```
emcc capi_js.cc --js-library pkg.js -o capi_js.js
```

`--js-library pkg.js` means to use `pkg.js` as an additional library while linking. After the command is executed, we'll get `capi_js.js` and `capi_js.wasm`. Load it in the web page as in the previous chapter and call the exported function`print_the_answer()`:

```html
//capi_js.html
  <body>
    <script>
    Module = {};
    Module.onRuntimeInitialized = function() {
      Module._print_the_answer();
    }
    </script>
    <script src="capi_js.js"></script>
  </body>
```

After browsing the page, the console will output:

![](images/02-c-api-js.png)

Since then, we have implemented a C API in JavaScript and called it from C.

## 2.2.3 Closure restriction and solution

Closure can't be used directly in the "mergeInto(LibraryManager.library..." functions. Of course this can be solved by calling another JavaScript function in the injected one. For example, we create `closure.cc` as follows:

```c
//closure.cc
#include <stdio.h>

EM_PORT_API(int) show_me_the_answer();

EM_PORT_API(void) func() {
    printf("%d\n", show_me_the_answer());
}
```

The function `show_me_the_answer()` is implemented in `closure_pkg.js`

```js
//closure_pkg.js
mergeInto(LibraryManager.library, {
    show_me_the_answer: function () {
        return jsShowMeTheAnswer();
    }
})
```

`show_me_the_answer()` calls `jsShowMeTheAnswer()`, which will be implemented in the page `closure.html`:

```html
//closure.html
  <body>
    <script>
    function f1(){
      var answer = 42;
      function f2() {
        return answer;
      }
      return f2;
    }
    var jsShowMeTheAnswer = f1();

    Module = {};
    Module.onRuntimeInitialized = function() {
      Module._func();
    }
    </script>
    <script src="closure.js"></script>
  </body>
```

Readers familiar with JavaScript must have discovered that `jsShowMeTheAnswer()` uses closure.

Using this method, we can not only bypass the closure restriction, but also dynamically adjust the behavior of the injected functions - as in the above example, we can adjust the `jsShowMeTheAnswer()` object in the JavaScript to change the return value of the C function `show_me_the_answer()`.

## 2.2.4 Advantages and disadvantages

- Advantages: Using JavaScript function injection can keep the C code pure - the C code doesn't contain any JavaScript code;
- Disadvantages: This method requires an additional .js library file to be created, which is slightly more troublesome to maintain.