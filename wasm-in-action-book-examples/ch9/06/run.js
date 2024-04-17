// 版权 @2020 柴树杉。

const fs = require('fs');
const buf = fs.readFileSync('./a.out.wasm');

WebAssembly.instantiate(new Uint8Array(buf)).then(function(result) {
	const double = (i) => {
		return result.instance.exports.double(i);
	}
	for(let i = 0; i < 5; i++) {
		console.log(double(i));
	}
});
