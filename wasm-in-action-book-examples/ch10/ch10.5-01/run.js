const fs = require('fs');

(async () => {
	const wasm = await WebAssembly.compile(fs.readFileSync('./main.wasm'));

	var custSec = WebAssembly.Module.imports(wasm);
	console.log("ModuleSec:", custSec);

	const instance = await WebAssembly.instantiate(wasm, {
		"env": {
			console_log: (x) => {
				console.log(x);
			}
		},
		"number.wasm": {
			add: (a, b) => {
				return a+b+1000;
			}
		}
	});
	instance.exports._start();
})();
