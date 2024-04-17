extern crate wasmer_runtime;

use std::str;

use wasmer_runtime::{error, func, imports, instantiate, Ctx, Value};

// Make sure that the compiled wasm-sample-app is accessible at this path.
static WASM: &'static [u8] =
    include_bytes!("../wasm-sample-app/target/wasm32-unknown-unknown/release/wasm_sample_app.wasm");

fn main() -> error::Result<()> {
    // Let's define the import object used to import our function
    // into our webassembly sample application.
    //
    // We've defined a macro that makes it super easy.
    //
    // The signature tells the runtime what the signature (the parameter
    // and return types) of the function we're defining here is.
    // The allowed types are `i32`, `u32`, `i64`, `u64`,
    // `f32`, and `f64`.
    //
    // Make sure to check this carefully!
    let import_object = imports! {
        // Define the "env" namespace that was implicitly used
        // by our sample application.
        "env" => {
            // name        // the func! macro autodetects the signature
            "print_str" => func!(print_str),
        },
    };

    // Compile our webassembly into an `Instance`.
    let mut instance = instantiate(WASM, &import_object)?;
    let memory = instance.context_mut().memory(0);

    let host_string = "from Rust!";

    // Write the string into the lineary memory
    for (byte, cell) in host_string
        .bytes()
        .zip(memory.view()[0 as usize..(host_string.len()) as usize].iter())
    {
        cell.set(byte);
    }

    // Call our exported function!
    instance.call(
        "hello_string_from_rust",
        &[Value::I32(0), Value::I32(host_string.len() as _)],
    )?;

    Ok(())
}

// Let's define our "print_str" function.
//
// The declaration must start with "extern" or "extern "C"".
fn print_str(ctx: &mut Ctx, ptr: u32, len: u32) {
    // Get a slice that maps to the memory currently used by the webassembly
    // instance.
    //
    // Webassembly only supports a single memory for now,
    // but in the near future, it'll support multiple.
    //
    // Therefore, we don't assume you always just want to access first
    // memory and force you to specify the first memory.
    let memory = ctx.memory(0);

    // Get a subslice that corresponds to the memory used by the string.
    let str_vec: Vec<_> = memory.view()[ptr as usize..(ptr + len) as usize]
        .iter()
        .map(|cell| cell.get())
        .collect();

    // Convert the subslice to a `&str`.
    let string = str::from_utf8(&str_vec).unwrap();

    // Print it!
    println!("{}", string);
}
