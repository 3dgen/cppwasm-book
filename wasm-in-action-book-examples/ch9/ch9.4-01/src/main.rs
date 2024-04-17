
extern "C" {
	fn env_print_str(s: *const u8, len: usize);
}

fn main() {
	let s = "你好, WebAssembly!";
	unsafe {
		env_print_str(s.as_ptr(), s.len());
	}
}
