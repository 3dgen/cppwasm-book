
#if defined(__cplusplus)
#	define WASM_EXPORT extern "C" __attribute__((visibility("default")))
#	define WASM_IMPORT(module_name) extern "C" __attribute__((import_module(module_name)))
#else
#	define WASM_EXPORT extern __attribute__((visibility("default")))
#	define WASM_IMPORT(module_name) extern __attribute__((import_module(module_name)))
#endif

struct iov_t {
	const char* base;
	int len;
};

WASM_IMPORT("wasi_unstable") int fd_write(
	int fd, const iov_t* iovs, int iovs_len, int* nwritten
);

WASM_EXPORT void _start() {
	const char s0[] = "hello world\n";
	const char s1[] = "hello clang\n";

	iov_t iovs[] = {
		iov_t{s0, sizeof(s0)-1},
		iov_t{s1, sizeof(s1)-1},
	};

	int nwritten = 0;
	fd_write(1, &iovs[0], sizeof(iovs)/sizeof(iovs[0]), &nwritten);
}
