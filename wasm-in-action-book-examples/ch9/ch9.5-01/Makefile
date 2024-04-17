wasm-debug:
	cargo build --target wasm32-unknown-unknown
	cp ./target/wasm32-unknown-unknown/debug/hello-main.wasm a.out.wasm
	node run.js

wasm-release:
	cargo build --target wasm32-unknown-unknown --release
	cp ./target/wasm32-unknown-unknown/debug/hello-main.wasm a.out.wasm
	node run.js

clean:
	-rm a.out
