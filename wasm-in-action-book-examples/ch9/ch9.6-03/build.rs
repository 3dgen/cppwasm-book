extern crate cc;

fn main() {
	let host = std::env::var("HOST").unwrap();
	let target = std::env::var("TARGET").unwrap();

	if target.contains("wasm32") {
		if host.contains("darwin") {
			// macOS平台需要单独安装LLVM
			// 安装的路径: /usr/local/opt/llvm
			// brew install llvm

			std::env::set_var("CC", "/usr/local/opt/llvm/bin/clang");
			std::env::set_var("AR", "/usr/local/opt/llvm/bin/llvm-ar");
		}
	}

	cc::Build::new()
		.file("src/main.c")
		.compile("myclib")
}