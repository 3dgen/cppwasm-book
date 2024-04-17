# 版权 @2020 柴树杉。

build:
	cargo run

wasm:
	cargo build --target wasm32-unknown-unknown
	cp ../target/wasm32-unknown-unknown/debug/hello-c-v2.wasm ./a.out.wasm
	wasm2wat a.out.wasm -o a.out.wasm.wat
	node run.js

clean:
	-rm a.out*
