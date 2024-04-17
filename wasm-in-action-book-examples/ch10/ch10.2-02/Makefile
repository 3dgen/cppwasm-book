default:
	rustc --crate-type cdylib --target wasm32-unknown-unknown -o a.out.wasm hello.rs
	wasm2wat a.out.wasm -o a.out.wasm.wat

	wasmer run ./a.out.wasm 

clean:
