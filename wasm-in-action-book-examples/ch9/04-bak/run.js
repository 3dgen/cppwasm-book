// 版权 @2020 柴树杉。

const fs = require('fs');
const buf = fs.readFileSync('./a.out.wasm');

WebAssembly.instantiate(new Uint8Array(buf), {
	env: {
		"console_log": (x)=>{console.log(x);}
	}
}).then(function(result) {
	const fib = (i) => {
		return result.instance.exports.fib(i);
	}
	result.instance.exports.main();
	for(let i = 1; i <= 5; i++) {
		console.log(fib(i));
	}
})
