# 3.4 内存管理

第2章我们介绍了Emscripten使用的线性内存模型，以及C/C++代码和JavaScript代码通过Emscripten堆交换数据的方法。本节将介绍Emscripten堆（既内存）管理的相关内容。

## 3.4.1 内存容量/栈容量

Emscripten当前版本（v 1.38.11）默认的内存容量为16MB，栈容量为5MB。

在使用`emcc`编译时，可以使用`TOTAL_MEMORY`参数控制内存容量，例如：

```
emcc mem.cc -s TOTAL_MEMORY=67108864 -o mem.js
```

上述命令通过`-s TOTAL_MEMORY=67108864`将内存容量设置为64MB。

相应的，栈容量可以通过`TOTAL_STACK`参数控制，例如下列命令将栈容量设为3MB：

```
emcc mem.cc -s TOTAL_STACK=3145728 -o mem.js
```

栈空间消耗内存，并且在运行时栈空间不可调，实际程序可用的堆空间容量小等于内存容量减去栈容量，因此在设置编译参数时，`TOTAL_MEMORY`必须大于`TOTAL_STACK`。

由于WebAssembly内存单位为页，1页=64KB，因此当编译目标为`wasm`时，`TOTAL_MEMORY`必须为64KB的整数倍。

除了通过`TOTAL_MEMORY`参数在编译时设定内存容量外，还可以通过预设`Module`对象`TOTAL_MEMORY`属性值的方法设定内存容量，例如下列JavaScript脚本将内存容量设定为128MB：

```js
	<script>
	var Module = {
		TOTAL_MEMORY : 134217728
	}
	</script>
	<script src="mem.js"></script>
```

在上例中，Emscripten模块位于`mem.js`中。该方法设置内存容量会覆盖编译时参数，但需要特别注意的是，必须在Emscripten模块装载前预设`Module.TOTAL_MEMORY`的值，若在`mem.js`加载后修改`Module.TOTAL_MEMORY`，不会更改内存的实际容量。

## 3.4.2 可变内存

在默认设置下，Emscripten堆一经初始化，容量就固定了，无法再扩容。而某些程序在运行时需要的内存容量在不同工况下可能有很大的波动。为了满足某些极端工况的需求而将`TOTAL_MEMORY`设置得非常高无疑是非常浪费的，为此，Emscripten提供了可在运行时扩大内存容量的模式，欲开启该模式，需要在编译时增加`-s ALLOW_MEMORY_GROWTH=1`参数，例如：

```
emcc mem.cc -s ALLOW_MEMORY_GROWTH=1 -o mem.js
```

在可变内存模式下，使用`malloc`等函数分配内存时，若可用空间不足，将引发Emscripten堆扩容。扩容时，内存中原有的数据会被拷入扩容后的内存空间中，因此扩容并不会导致数据丢失或地址变更。

可变内存虽然提供了很多便利，但当编译目标为`asm.js`时，可变内存模式会影响性能。然而可扩容的内存是WebAssembly的自有特性，当编译目标为`wasm`时，使用可变内存模式非常高效，不会影响运行性能，因此在编译为WebAssembly时，可变内存是推荐用法。

> **tips** 即使采用了可变内存模式，内存容量仍然受32位地址空间的限制。

## 3.4.3 内存分配器

Emscripten提供了两种内存分配器：

- `dlmalloc` 默认值。由Doug Lea创建的内存分配器，其变种广泛使用于Linux等。
- `emmalloc` 专为Emscripten设计的内存分配器。

`emmalloc`的代码体积小于`dlmalloc`，但是如果程序中频繁申请大量的小片内存，使用`dlmalloc`性能较好。

编译时设置`MALLOC`参数可以指定内存分配器，比如下列命令指定使用`emmalloc`：

```
emcc mem.cc -s MALLOC="emmalloc" -o mem.js
```

> **tips** 除非对于代码体积极度敏感的场合，使用默认的`dlmalloc`内存分配器无疑是更优的选择。