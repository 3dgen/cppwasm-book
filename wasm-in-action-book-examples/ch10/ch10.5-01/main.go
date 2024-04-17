package main

// #include <stdint.h>
//
// extern void console_log(void *context, int32_t x);
import "C"

import (
	"fmt"
	"unsafe"
	"log"

	wasm "github.com/wasmerio/go-ext-wasm/wasmer"
)

func main() {
	// Reads the WebAssembly module as bytes.
	bytes, err := wasm.ReadBytes("main.wasm")
	if err != nil {
		log.Fatal(err)
	}

	imports := wasm.NewImports().Namespace("env")
	_, err = imports.Append("console_log", console_log, C.console_log)
	if err != nil {
		log.Fatal(err)
	}

	// Instantiates the WebAssembly module.
	instance, err := wasm.NewInstanceWithImports(bytes, imports)
	if err != nil {
		log.Fatal(err)
	}
	defer instance.Close()

	// Gets the `sum` exported function from the WebAssembly instance.
	_start := instance.Exports["_start"]

	// Calls that exported function with Go standard values. The WebAssembly
	// types are inferred and values are casted automatically.
	_, err = _start()
	if err != nil {
		log.Fatal(err)
	}

	fmt.Println("done")
}

//export console_log
func console_log(ctx unsafe.Pointer, x int32) {
	fmt.Println("console_log:", x)
}
