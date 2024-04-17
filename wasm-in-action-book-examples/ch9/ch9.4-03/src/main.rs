
extern "C" {
	fn env_print_str(s: *const u8, len: usize);
}

fn print_str(s: &str) {
	unsafe {
		env_print_str(s.as_ptr(), s.len());
	}
}

static mut _ARGS: Vec<String> = Vec::new();

#[no_mangle]
#[allow(dead_code)]
#[allow(non_snake_case)]
pub fn ARGS_init(i: usize, s: *mut String) {
	let me = unsafe {
		assert!(!s.is_null());
		&mut *s
	};
	unsafe {
		if _ARGS.len() < (i+1) {
			_ARGS.resize(i+1, String::from(""));
		}
		_ARGS[i] = me.clone();
	}
}

#[allow(dead_code)]
#[allow(non_snake_case)]
fn ARGS() -> Vec<String> {
	unsafe { _ARGS.clone() }
}

#[no_mangle]
pub fn string_new(size: usize) -> *mut String {
	Box::into_raw(Box::new(String::from("\0").repeat(size)))
}

#[no_mangle]
pub fn string_delete(ptr: *mut String) {
	if ptr.is_null() {
		return;
	}
	unsafe {
		Box::from_raw(ptr);
	}
}

#[no_mangle]
pub fn string_data_ptr(ptr: *mut String) -> *mut u8 {
	let me = unsafe {
		assert!(!ptr.is_null());
		&mut *ptr
	};

	return me.as_mut_ptr();
}

#[no_mangle]
pub fn string_data_size(ptr: *mut String) -> usize {
	let me = unsafe {
		assert!(!ptr.is_null());
		&mut *ptr
	};

	return me.len();
}

#[no_mangle]
pub fn string_print(ptr: *mut String) {
	let me = unsafe {
		assert!(!ptr.is_null());
		&mut *ptr
	};
	unsafe {
		env_print_str(me.as_ptr(), me.len());
	}
}

fn main() {
	for (i, x) in ARGS().iter().enumerate() {
		print_str(format!("{}: {}", i, x).as_str());
	}
}
