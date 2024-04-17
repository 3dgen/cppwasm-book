const fs = require('fs');

// https://nodejs.org/api/wasi.html
// v13.7.0: --experimental-wasi-unstable-prev
const { WASI } = require('wasi');

const wasi = new WASI({args: process.argv, env: process.env});
const importObject = { wasi_snapshot_preview1: wasi.wasiImport };

(async () => {
	const wasm = await WebAssembly.compile(fs.readFileSync('./hello.wasm'));

	var custSec = WebAssembly.Module.imports(wasm);
	console.log("ModuleSec:", custSec);

	const instance = await WebAssembly.instantiate(wasm, importObject);
	wasi.start(instance);
})();
