# 3.4 Memory management

In Chapter 2 we introduced the Emscripten's linear memory memory model and how to exchange data between C/C++ code and JavaScript using Emscripten's heap. This section introduces Emscripten's heap (i.e. memory) management.

## 3.4.1 Memory size/stack size

In the current version of Emscripten (v1.38.11), the default size of memory is 16MB, the default size of stack is 5MB.

When compiling with `emcc`, the `TOTAL_MEMORY` option controls memory size. For example:

```
emcc mem.cc -s TOTAL_MEMORY=67108864 -o mem.js
```

The above command sets memory size to 64MB with `-s TOTAL_MEMORY=67108864`.

Similarly, the stack size is controlled by the `TOTAL_STACK` option. This command set the stack size to 3MB:

```
emcc mem.cc -s TOTAL_STACK=3145728 -o mem.js
```

The stack space consumes memory and is not adjustable at run time. The usable heap space size is equal to or smaller than the  memory size subtracting the stack size, hence when specifying the options, `TOTAL_MEMORY` must be greater than `TOTAL_STACK`.

Because WebAssembly's memory unit is page, which is 64KB, `TOTAL_MEMORY` must be multiples of 64KB when compiling target is `wasm`.

Besides setting memory size at compile time with the `TOTAL_MEMORY` option, we can also set it using the `TOTAL_MEMORY` attribute of the `Module` object. The below JavaScript sets memory size to 138MB:

```js
	<script>
	var Module = {
		TOTAL_MEMORY : 134217728
	}
	</script>
	<script src="mem.js"></script>
```

In the example, Emscripten module is in `mem.js`. In this way the memory size specified by compiler option is overridden. Notice that `Module.TOTAL_MEMORY` must be set before loading the Emscripten module. Should it occur after `mem.js` is loaded, memory size would not change.

## 3.4.2 Growable memory

Under default settings, once the Emscripten heap is initialized, it size is fixed and can't grow. However some programs' memory consumption fluctuates wildly under different working conditions. It is no double wasteful to set `TOTAL_MEMORY` very large to satisfy the extreme case. Because of this Emscripten provides a mode to grow memory at run time, enabled by the `-s ALLOW_MEMORY_GROWTH=1` option. For example:

```
emcc mem.cc -s ALLOW_MEMORY_GROWTH=1 -o mem.js
```

Under growable memory mode, memory inefficiency when allocating with functions such as `malloc` triggers Emscripten heap growth. During growth, old data is copied into expanded memory space, thus no data loss or address change would happen.

Although very handy, when targeting `asm.js` growable memory impacts performance. However growable memory is a feature of WebAssembly, so when targeting `wasm`, it is very efficient and is recommended.

> **tips** Even under growable memory mode, memory size is bounded by 32 bit address space size.

## 3.4.3 Memory allocator

Emscripten provides two kinds of memory allocators:

- `dlmalloc` This is the default memory allocated created by Doug Lea. Its variations are wildly used on Linux and other places.
- `emmalloc` Memory allocator tailored for Emscripten.

`emmalloc` has smaller code size than `dlmalloc`. But if the program frequently allocates small fragments of memory, `dlmalloc` performs better.

The `MALLOC` compiler options specifies the allocator. For example, this command designates `emmalloc`:

```
emcc mem.cc -s MALLOC="emmalloc" -o mem.js
```

> **tips** Unless in scenarios where code size is critically sensitive, the default `dlmalloc` is the better choice.




