// 版权 @2020 柴树杉。

const fs = require('fs');
const buf = fs.readFileSync('./a.out.wasm');

function console_log(x) {
	console.log(x);
}

WebAssembly.instantiate(new Uint8Array(buf), {
	env: { "console_log": console_log }
}).then(function(result) {
	result.instance.exports.main();
});
