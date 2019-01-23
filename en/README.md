#  WebAssembly friendly programming with C/C++

This book introduces the use of C/C++ for WebAssembly development. For the basics of WebAssembly, please refer to the "WebAssembly Primer"(Simplified Chinese), the sale address:

- EPUB：[https://www.epubit.com/book/detail/40619](https://www.epubit.com/book/detail/40619)
- JD：[https://item.jd.com/12499372.html](https://item.jd.com/12499372.html)

----

![](cover.png)

- Author: Ending，Github [@3dgen](https://github.com/3dgen)
- Author: ChaiShushan，Github [@chai2010](https://github.com/chai2010)，Twitter [@chaishushan](https://twitter.com/chaishushan)
- Translator: Ending，Github [@3dgen](https://github.com/3dgen)
- Translator: yushih, Github [@yushih](https://github.com/yushih)
- Project: https://github.com/3dgen/cppwasm-book

## Preface

> *Ending's law: "Any application that can be compiled to WebAssembly, will be compiled to WebAssembly eventually."*

WebAssembly is the newest virtual machine standard for the web. C/C++ source code can be compiled into the WebAssembly binary format(.wasm) through the Emscripten toolchain, and then imported into web pages for JavaScript calls - this means that programs written in C/C++ can run in the web page.

This book introduces how to develope WebAssembly modules by C/C++ from the basic use of Emscripten; some general design principles and technical frameworks are proposed based on the first-hand experience gained by the author in the actual project.

We believe that an ideal Web-oriented C/C++ project should be insensitive to the compilation target - can be compiled to native code, or can be compiled to WebAssembly and run in the web page, the switch between each other only need to change the target configuration. So, we can make full use of the powerful development, debugging, analysis, testing and other functions of the existing IDE environment to improve project quality and reduce development costs.

However, the operating environment of WebAssembly is very different from the native code. Therefore, in order to achieve the above ideal goal, the characteristics (or limitations) of the Web environment must be fully considered, from the overall framework to the interface design and even to the data exchange between functions. This is the connotation of "WebAssembly friendly" implemented in this book.

## Read online

- [SUMMARY.md](SUMMARY.md)
- https://3dgen.cn/cppwasm-book

## Reference

- ["WebAssembly Primer"](https://www.epubit.com/book/detail/40619)
- [https://github.com/chai2010/awesome-wasm-zh](https://github.com/chai2010/awesome-wasm-zh)

----

## Progress

* [Chapter 1 Getting started with Emscripten](ch1-quick-guide/readme.md)
  * [x] [1.1 Installing Emscripten](ch1-quick-guide/ch1-01-install.md)
  * [x] [1.2 Hello, world!](ch1-quick-guide/ch1-02-helloworld.md)
  * [x] [1.3 Taking a look at the Emscripten glue code](ch1-quick-guide/ch1-03-glue-code.md)
  * [x] [1.4 Selecting compilation target](ch1-quick-guide/ch1-04-compile.md)

* [Chapter 2 Connecting C and JavaScript](ch2-c-js/readme.md)
  * [x] [2.1 Calling compiled C functions from JavaScript](ch2-c-js/ch2-01-js-call-c.md)
  * [x] [2.2 Implement C API in JavaScript](ch2-c-js/ch2-02-implement-c-api-in-js.md)
  * [x] [2.3 Memory model](ch2-c-js/ch2-03-mem-model.md)
  * [x] [2.4 Exchange data between C and JavaScript](ch2-c-js/ch2-04-data-exchange.md)
  * [x] [2.5 Using `EM_ASM`](ch2-c-js/ch2-05-em-asm.md)
  * [x] [2.6 Using `emscripten_run_script`](ch2-c-js/ch2-06-run-script.md)
  * [x] [2.7 Using `ccall`/`cwrap`](ch2-c-js/ch2-07-ccall-cwrap.md)
  * [x] [2.8 Supplement](ch2-c-js/ch2-08-ext.md)

* [Chapter 3 Emscripten runtime](ch3-runtime/readme.md)
  * [x] [3.1 Runtime lifecycle](ch3-runtime/ch3-01-main.md)
  * [x] [3.2 Message loop](ch3-runtime/ch3-02-message-loop.md)
  * [x] [3.3 File system](ch3-runtime/ch3-03-fs.md)
  * [x] [3.4 Memory management](ch3-runtime/ch3-04-mem.md)
  * [x] [3.5 Customize Module object](ch3-runtime/ch3-05-module.md)
  * [x] [3.6 Summary](ch3-runtime/ch3-06-summary.md)

* [Chapter 4 General techniques that WebAssembly friendly](ch4-techniques/readme.md)
  * [x] [4.1 Message loop detaching](ch4-techniques/ch4-01-msg-loop-detach.md)
  * [x] [4.2 Memory alignment](ch4-techniques/ch4-02-align.md)
  * [x] [4.3 Exporting C++ objects using the C interface](ch4-techniques/ch4-03-export-obj.md)
  * [x] [4.4 Lifecycle control for C++ object](ch4-techniques/ch4-04-obj-life-cycle.md)
  * [x] [4.5 Importing JavaScript object using C interface](ch4-techniques/ch4-05-import-js-obj.md)
  * [x] [4.6 Be careful with `int64`](ch4-techniques/ch4-06-int64-issue.md)
  * [x] [4.7 Forget about filesystem](ch4-techniques/ch4-07-forget-about-fs.md)

* [Chapter 5 Network IO](ch5-net/readme.md)
  * [x] [5.1 XMLHttpRequest](ch5-net/ch5-01-http.md)
  * [x] [5.2 WebSocket](ch5-net/ch5-02-websocket.md)
  <!--* [ ] 5.3 fetch-->

* [Chapter 6 Multithreading](ch6-threads/readme.md)
  * [x] [6.1 Multithreading in JavaScript](ch6-threads/ch6-01-worker.md)
  * [x] [6.2 Using Emscripten in Web Worker](ch6-threads/ch6-02-sample.md)
  <!--* [ ] 6.3 A simple framework for multithreading-->

* [Chapter 7 GUI](ch7-gui/readme.md)
  * [x] [7.1 Canvas](ch7-gui/ch7-01-canvas.md)
  * [x] [7.2 Mouse events](ch7-gui/ch7-02-mouse.md)
  * [x] [7.3 Keyboard events](ch7-gui/ch7-03-keyboard.md)
  * [x] [7.4 Conway's Game of Life](ch7-gui/ch7-04-life.md)

<!--* Chapter 8 Project management
  * [ ] 8.1 Using Makefile
  * [ ] 8.2 Using static library-->

----

## Copyright


