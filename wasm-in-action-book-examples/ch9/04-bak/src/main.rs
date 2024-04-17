fn main() {

	unsafe{
		for i in 0..10 {
			console_log(fib(i+1))
		}
		console_log(122);
	}
}

#[cfg(target_arch="wasm32")]
extern "C" {
    pub fn console_log(a: i32);
}

#[cfg(not(target_arch="wasm32"))]
unsafe fn console_log(a: i32) {
	println!("{}", a)
}

#[no_mangle]
#[cfg(target_arch="wasm32")]
pub fn fib(n: i32) -> i32 {
	if n >= 2 {
		return fib(n - 1) + fib( n - 2);
	}
	return n;
}

#[no_mangle]
#[cfg(not(target_arch="wasm32"))]
pub fn fib(n: i32) -> i32 {
	if n >= 2 {
		return fib(n - 1) + fib( n - 2);
	}
	return n;
}
