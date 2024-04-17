extern crate wasmer_runtime;
extern crate wasmer_wasi;

fn main() -> wasmer_runtime::error::Result<()> {
    let import_object = wasmer_wasi::generate_import_object_for_version(
        // Snapshot0: wasi_unstable
        // Snapshot1: wasi_snapshot_preview1
        // Latest: Latest version
        wasmer_wasi::WasiVersion::Snapshot1,
        vec![], // args: Vec<Vec<u8>>
        vec![], // envs: Vec<Vec<u8>>
        vec![], // preopened_files: Vec<PathBuf>
        vec![], // mapped_dirs: Vec<(String, PathBuf)>
    );

    let hello_wasm = include_bytes!(concat!(env!("CARGO_MANIFEST_DIR"), "/hello.wasm"));
    let instance = wasmer_runtime::instantiate(hello_wasm, &import_object)?;
    instance.call("_start", &[])?;
    Ok(())
}
