default:
	rustc --target wasm32-wasi -o a.out.wasm main.rs
	wasmer a.out.wasm "hello wasm"

clean:
