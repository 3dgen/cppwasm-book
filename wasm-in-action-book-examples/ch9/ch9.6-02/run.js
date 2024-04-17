// 版权 @2020 柴树杉。

const fs = require('fs');
const buf = fs.readFileSync('./a.out.wasm');

WebAssembly.instantiate(new Uint8Array(buf), {
	env: {
		env_print_int: (x) => { console.log(x); }
	}
}).then(function(result) {
	result.instance.exports.main();
})
