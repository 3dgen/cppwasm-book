# 目录

* [第0章 WebAssembly简介](ch0-intro/readme.md)

* [第1章 Emscripten快速入门](ch1-quick-guide/readme.md)
  * [1.1 安装Emscripten](ch1-quick-guide/ch1-01-install.md)
  * [1.2 你好，世界！](ch1-quick-guide/ch1-02-helloworld.md)
  * [1.3 胶水代码初探](ch1-quick-guide/ch1-03-glue-code.md)
  * [1.4 编译目标及编译流程](ch1-quick-guide/ch1-04-compile.md)

* [第2章 C与JavaScript互操作](ch2-c-js/readme.md)
  * [2.1 JavaScript调用C函数](ch2-c-js/ch2-01-js-call-c.md)
  * [2.2 JavaScript函数注入C](ch2-c-js/ch2-02-implement-c-api-in-js.md)
  * [2.3 单向透明的内存模型](ch2-c-js/ch2-03-mem-model.md)
  * 2.4 JavaScript与C交换数据
  * 2.5 EM_ASM宏
  * 2.6 emscripten_run_script函数

* 第3章 Emscripten运行时
  * 3.1 main函数与生命周期
  * 3.2 消息循环
  * 3.3 文件系统
  * 3.4 内存管理
  * 3.5 Module定制

* 第4章 WebAssembly友好的一般性方法
  * 4.1 消息循环分离
  * 4.2 数据对齐
  * 4.3 使用C接口导出C++对象
  * 4.4 C++对象生命周期控制
  * 4.5 使用C接口注入JavaScript对象
  * 4.6 小心int64
  * 4.7 忘掉文件系统

* 第5章 网络IO
  * 5.1 websocket
  * 5.2 ajax
  * 5.3 fetch
  * 5.4 一个通用的网络IO小框架

* 第6章 多线程
  * 6.1 JavaScript中的多线模型
  * 6.2 一个例子
  * 6.3 一个通用的多线程小框架

* 第7章 GUI及交互
  * 7.1 canvas
  * 7.2 鼠标事件
  * 7.3 键盘事件
  * 7.4 触屏事件

* 第8章 工程管理
  * 8.1 使用Makefile
  * 8.2 静态库
