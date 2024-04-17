// 版权 @2020 柴树杉。

// 1. &u8 和 *const u8 几乎是等价的
// 2. String 和 C++ 的 std::string 类似
// 3. &str 和 Go 的 reflect.StringHeader 类似, 带有长度信息

extern "C" {
    fn env_print_int(x: i32);
}

#[link(name = "myclib")]
extern "C" {
    fn strlen(s: *const u8) -> i32;
    fn cMain();
}

fn main() {
    unsafe {
        let s = b"hello\0".as_ptr();
        env_print_int(strlen(s));
        cMain();
    }
}
