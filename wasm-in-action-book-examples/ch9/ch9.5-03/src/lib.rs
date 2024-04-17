
#![no_std]

#[panic_handler]
fn panic(_info: &core::panic::PanicInfo) -> ! {
	loop {}
}

extern crate wee_alloc;

#[global_allocator]
static ALLOC: wee_alloc::WeeAlloc = wee_alloc::WeeAlloc::INIT;

extern "C" {
	fn env_print_str(s: *const u8, len: usize);
}

// 1. src/main.rs => src/lib.rs
// 2. [lib] crate-type = ["cdylib"]
// 3. 名字中下连接符`-`变为下划线`_`
// 4. 手动导出 main 函数
// 5. wasm 文件 2MB => 8KB

// 6. [dependencies] wee_alloc = "0.4.5"

#[no_mangle]
pub fn main() {
	//use std::string::String;

	let s = "你好, WebAssembly!";
	//let s2 = String::from(s);
	unsafe {
		env_print_str(s.as_ptr(), s.len());
	}
}
