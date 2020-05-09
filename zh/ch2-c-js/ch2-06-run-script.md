# 2.6 `emscripten_run_script`系列函数

2.5介绍的`EM_ASM`系列宏只能接受硬编码常量字符串，而本节将要介绍的`emscripten_run_script`系列函数可以接受动态输入的字符串，该系列辅助函数可以类比于JavaScript中的`eval()`方法。

## 2.6.1 `emscripten_run_script()`

函数声明：

`void emscripten_run_script(const char *script)`

参数：

- `script`：包含JavaScript脚本语句的字符串。

返回值：

- 无

函数使用很简单，例如：

```c
int main(){
	emscripten_run_script("console.log(42);");
	return 0;
}
```

上述代码中`emscripten_run_script("console.log(42);");`执行效果等价于在JavaScript中执行`console.log(42);`。

`emscripten_run_script()`可以接受动态生成的字符串，例如：

```c
const char* get_js_code(){
	static char buf[1024];
	sprintf(buf, "console.log('你好，Emscripten！');");
	return buf;
}

int main(){
	emscripten_run_script(get_js_code());
	return 0;
}
```

由于传入的脚本最终会通过JavaScript的`eval()`方法执行，因此传入的脚本可以是任意的JavaScript代码，比如：

```c
	emscripten_run_script(R"(
		function my_print(s) {
			console.log("JS:my_print():", s);
		}
		my_print("Hello!");
	)");
```

上述代码先定义了一个方法`my_print()`，然后调用它输出了“Hello!”

> **tips** 为了避免使用转义符方便阅读，上述例子定义字符串时，使用了C++11标准的raw字符串定义方法（`R`前缀），在使用`emcc`命令编译时，必须增加`-std=c++11`参数，完整的命令行为：
```
emcc emscripten_run_script.cc -std=c++11  -o emscripten_run_script.js
```

## 2.6.2 `emscripten_run_script_int()`

函数声明：

`int emscripten_run_script_int(const char *script)`

参数：

- `script`：包含JavaScript脚本语句的字符串。

返回值：
- int

该函数与`emscripten_run_script()`类似，区别是它会将输入的脚本的执行结果作为整型数返回，例如：

```c
	int num = emscripten_run_script_int(R"(
		function show_me_the_number() {
			return 13;
		}
		show_me_the_number();
	)");
	printf("num:%d\n", num);
```

上述程序将输出：

```
num:13
```

## 2.6.3 `emscripten_run_script_string()`

函数声明：

`char *emscripten_run_script_string(const char *script)`

参数：

- `script`：包含JavaScript脚本语句的字符串。

返回值：
- char *

该函数与`emscripten_run_script_int()`类似，区别是返回值为字符串，例如：

```c
	const char* str = emscripten_run_script_string(R"(
		function show_me_the_answer() {
			return "The answer is 42.";
		}
		show_me_the_answer();
	)");
	printf("%s\n", str);
```

上述程序将输出：

```
The answer is 42.
```

我们在胶水代码中可以看到辅助函数`emscripten_run_script_string()`实现代码如下：

```js
  function _emscripten_run_script_string(ptr) {
      var s = eval(Pointer_stringify(ptr)) + '';
      var me = _emscripten_run_script_string;
      var len = lengthBytesUTF8(s);
      if (!me.bufferSize || me.bufferSize < len+1) {
        if (me.bufferSize) _free(me.buffer);
        me.bufferSize = len+1;
        me.buffer = _malloc(me.bufferSize);
      }
      stringToUTF8(s, me.buffer, me.bufferSize);
      return me.buffer;
    }
```

该函数在C/C++内存中分配了空间，用于保存传入脚本执行后返回的字符串。并且从代码中不难发现，多次调用`emscripten_run_script_string()`时，后面调用的结果有可能覆盖前面调用的结果——因为`me.buffer`是重复使用的。

本节例子的输出如下：

![](images/06-run-script.png)