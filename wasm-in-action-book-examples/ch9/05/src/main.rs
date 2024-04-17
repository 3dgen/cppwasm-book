
extern "C" {
	fn console_log(a: i32);
}

fn main() {
	unsafe{console_log(42);}
}
