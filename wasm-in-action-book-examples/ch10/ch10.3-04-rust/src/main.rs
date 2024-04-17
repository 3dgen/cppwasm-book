extern crate wasmer_runtime;

fn main() -> wasmer_runtime::error::Result<()> {
    let import_object = wasmer_runtime::imports! {
        "wasi_snapshot_preview1" => {
            "fd_write" => wasmer_runtime::func!(fd_write),
        },
    };

    let hello_wasm = include_bytes!(concat!(env!("CARGO_MANIFEST_DIR"), "/hello.wasm"));
    let instance = wasmer_runtime::instantiate(hello_wasm, &import_object)?;
    instance.call("_start", &[])?;
    Ok(())
}

fn fd_write(
    ctx: &mut wasmer_runtime::Ctx,
    _fd: i32,
    iovs_ptr: u32,
    iovs_len: u32,
    p_nwritten: u32,
) -> i32 {
    let mem = ctx.memory(0).view::<u8>().as_ptr();

    unsafe {
        let mut nwritten: u32 = 0;
        let mut i: u32 = 0;

        while i < iovs_len {
            let buf = *(mem.add((iovs_ptr + i*8+0) as usize) as *const u32);
            let len = *(mem.add((iovs_ptr + i*8+4) as usize) as *const u32);

            let s = String::from_raw_parts(
				mem.add(buf as usize) as *mut u8,
                len as usize,
                len as usize,
            );
			print!("{}", s);
            std::mem::forget(s);

            nwritten = nwritten + len;
            i = i + 1;
        }

        let p_nwritten = mem.add(p_nwritten as usize) as *mut u32;
        *p_nwritten = nwritten;
        return 0;
    }
}
