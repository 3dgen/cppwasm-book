# 3.6 小结

Emscripten工具链的功能/自定义参数非常繁多，前面章节介绍的是其中最常使用到的部分。

从个人经验来说，有两个较常查阅的源文件，分别为：
- `emsdk/emscripten/<sdk_ver>/src/settings.js`
- `emsdk/emscripten/<sdk_ver>/system/include/emscripten.h`

前者包含了所有的编译选项及解释；后者包含了`emscripten_set_main_loop()`/`emscripten_run_script()`等Emscripten特有函数的声明。

当然，对于希望深入了解Emscripten的读者来说，官网[https://kripken.github.io/emscripten-site/](https://kripken.github.io/emscripten-site/)是必不可少的资料来源。


本书关于Emscripten的基本介绍到此告一段落。后续章节将着重于实际工程实践中遇到的常见问题和解决途径。