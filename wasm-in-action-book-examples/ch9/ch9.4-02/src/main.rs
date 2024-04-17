
extern "C" {
	fn env_print_str(s: *const u8, len: usize);
}

fn print_str(s: str) {
	unsafe {
		env_print_str(s.as_ptr(), s.len());
	}
}

pub struct Args {
	args: [str],
}

impl Args {
}

#[no_mangle]
pub fn args_init_argc(n: i32) {
	//
}

#[no_mangle]
pub fn args_init_argv(i: i32, s *const i8, len: usize) {
	//
}

fn main() {
	print_str("aaa")
}

