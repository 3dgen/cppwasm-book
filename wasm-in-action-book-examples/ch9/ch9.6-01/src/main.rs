// 版权 @2020 柴树杉。

// 1. &u8 和 *const u8 几乎是等价的
// 2. String 和 C++ 的 std::string 类似
// 3. &str 和 Go 的 reflect.StringHeader 类似, 带有长度信息

#[link(name = "c")]
extern "C" {
    fn strlen(s: *const u8) -> usize;
}

fn main() {
	unsafe {
		let s = b"hello\0".as_ptr();
		println!("{:?}", r_strlen(s));
		println!("{:?}", strlen(s));

	}
}
unsafe fn foo(s: &str)  {
	let s0: &str = "abc";
	let p0: *const u8 = s0.as_ptr();
	let mut p1: *mut u8 = p0 as *mut u8;
}

unsafe fn r_strlen(s: *const u8) -> usize {
    let mut p = s;
    while *p != b'\0' {
        p = p.add(1);
    }
    (p as usize) - (s as usize)
}
