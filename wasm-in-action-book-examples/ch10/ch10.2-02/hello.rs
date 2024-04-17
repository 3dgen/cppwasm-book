
#![no_std]

#[panic_handler]
fn panic(_info: &core::panic::PanicInfo) -> ! {
	loop {}
}

#[repr(C)]
struct iov_t {
	base: *const u8,
	len:  usize,
}

#[link(wasm_import_module="wasi_unstable")]
extern "C" {
	fn fd_write(fd: i32, iovs: *const iov_t, iovs_len: usize, nwritten: *mut i32) -> i32;
}

#[no_mangle]
pub unsafe fn _start() {
	let s0 = b"hello ending!\n";
	let s1 = b"\xe4\xbd\xa0\xe5\xa5\xbd, chai2010!\n"; // 你好, chai2010!

	let iov = [
		iov_t { base: s0.as_ptr(), len: s0.len()},
		iov_t { base: s1.as_ptr(), len: s1.len()},
	];

	let mut nwritten: i32 = 0;
	fd_write(1, &iov[0], iov.len(), &mut nwritten as *mut i32);
}
