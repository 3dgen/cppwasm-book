(module $hello_wasi
	;; fn console_log(v: i32)
	(import "env" "console_log" (func $console_log (param i32)))

	;; fn add(a: i32, b: i32) -> i32
	;;(import "number.wasm" "add" (func $add (param i32 i32) (result i32)))

	;; _start 类似 main 函数, 自动执行
	(func $main (export "_start")
		i32.const 42
		;;i32.const 2
		;;call $add

		call $console_log
	)
)
