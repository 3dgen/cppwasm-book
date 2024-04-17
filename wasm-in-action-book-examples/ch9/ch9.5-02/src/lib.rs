
#![no_std]

#[panic_handler]
fn panic(_info: &core::panic::PanicInfo) -> ! {
	loop {}
}

extern "C" {
	fn env_print_str(s: *const u8, len: usize);
}

#[no_mangle]
pub fn main() {
	let s = "你好, WebAssembly!";
	unsafe {
		env_print_str(s.as_ptr(), s.len());
	}
}
