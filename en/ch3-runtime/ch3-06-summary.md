# 3.6 Summary

The Emscripten toolchain has extensive features/customizations, the most commonly used of which are introduced in previous sections.

From personal experiences, two source files are commonly consulted, which are:
- `emsdk/emscripten/<sdk_ver>/src/settings.js`
- `emsdk/emscripten/<sdk_ver>/system/include/emscripten.h`

The former one contains all compile options are their explanations; the later one contains the declarations of special functions such as `emscripten_set_main_loop()`/`emscripten_run_script()`.

Of course, for readers who hope to delve into Emscripten, the official website [https://kripken.github.io/emscripten-site/](https://kripken.github.io/emscripten-site/) is an indispensable information source.

This marks the end of basic introduction to Emscripten. Following chapters will focus on common problems and their solutions in real world projects.
