# 版权 @2020 柴树杉。

build:
	cargo build --target wasm32-unknown-unknown
	node run.js

myclib:
	@echo ===build myclib begin===

	/usr/local/opt/llvm/bin/clang --target=wasm32 \
		-nostdlib \
		-Wl,--no-entry \
		-Wl,--export-all \
		-c -o main_c.o ./src/main.c
	/usr/local/opt/llvm/bin/llvm-ar rcs libmyclib.a main_c.o

	@echo cargo:rustc-link-search=$(shell pwd)

	@echo ===build myclib done===

build-wasm-macos:
	# brew install llvm
	/usr/local/opt/llvm/bin/clang --target=wasm32 \
		-nostdlib \
		-Wl,--no-entry \
		-Wl,--export-all \
		-c -o main_c.o ./src/main.c
	/usr/local/opt/llvm/bin/llvm-ar rcs libmyclib.a main_c.o
	rm *.o

	rustc -L . -l myclib --target wasm32-unknown-unknown ./src/main.rs -o a.out.wasm
	wasm2wat a.out.wasm -o a.out.wasm.wat
	node run.js

wasm:
	cargo build --target wasm32-unknown-unknown
	cp ../target/wasm32-unknown-unknown/debug/hello-c-v2.wasm ./a.out.wasm
	wasm2wat a.out.wasm -o a.out.wasm.wat
	node run.js

clean:
	-rm a.out*
