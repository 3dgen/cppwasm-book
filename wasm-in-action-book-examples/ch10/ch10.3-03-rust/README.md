# Rust Embedder App Example

**NOTE:** Please see the [Wasmer Rust Integration documentation](https://docs.wasmer.io/runtime/rust-integration/runtime-rust-integration-installation) for an up-to-date example of user the Wasmer Rust Integration / Embedding.

This repo showcases how to use the [wasmer-runtime](https://crates.io/crates/wasmer-runtime/) from Rust, based on the blogpost: https://medium.com/wasmer/executing-webassembly-in-your-rust-application-d5cd32e8ce46

See [`src/main.rs`](./src/main.rs) for the example implementation.

The `wasm-sample-app` directory contains an example rust wasm app to run in the embedder app.

## Requirements
- Rust target `wasm32-unknown-unknown` - install using `rustup target add wasm32-unknown-unknown`

## Running

```bash
# Building the wasm-sample-app
cd wasm-sample-app && cargo build --release && cd ..

# Running the wasm sample from Rust
cargo run
```
