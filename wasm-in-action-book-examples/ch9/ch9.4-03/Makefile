wasm:
	cargo build --target wasm32-unknown-unknown
	cp ./target/wasm32-unknown-unknown/debug/hello-main.wasm a.out.wasm
	node run.js 11 22 aa bb

clean:
	-rm a.out
