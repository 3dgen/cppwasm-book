default:
	rustc --target wasm32-wasi -o a.out.wasm hello.rs
	wasm2wat a.out.wasm -o a.out.wasm.wat

	wasmer run ./a.out.wasm 

clean:
