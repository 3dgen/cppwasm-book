(module
	;; fn add(a: i32, b: i32) -> i32
	(func $add (export "add")
		(param $a i32) (param $b i32)
		(result i32)

		get_local $a
		get_local $b
		i32.add
    )
)
