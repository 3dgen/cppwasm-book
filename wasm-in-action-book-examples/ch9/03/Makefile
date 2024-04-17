
wasi-run:
	cargo build --target=wasm32-wasi

	cp ./target/wasm32-wasi/debug/hello.wasm a.out.wasm
	wasm2wat a.out.wasm -o a.out.wasm.wat

	# https://github.com/wasmerio/wasmer
	wasmer run ./a.out.wasm

clean:
	-rm a.out
