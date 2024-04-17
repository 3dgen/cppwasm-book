default:
	/usr/local/opt/llvm/bin/clang --target=wasm32 \
		-nostdlib \
		-Wl,--no-entry \
		-Wl,--export-all \
		-c -o hello.o ./hello.cc
	/usr/local/opt/llvm/bin/wasm-ld \
		--allow-undefined \
		hello.o -o a.out.wasm

	wasm2wat a.out.wasm -o a.out.wasm.wat
	wasmer a.out.wasm

clean:
