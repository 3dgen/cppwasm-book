// 版权 @2020 柴树杉。

const fs = require('fs');
const buf = fs.readFileSync('./a.out.wasm');

class RustApp {
	constructor() {
		this._inst = null;
	}

	async Run(instance) {
		this._inst = instance;
		this._inst.exports.main();
	}

	Mem() {
		return this._inst.exports.memory;
	}
	MemView(addr, len) {
		return new DataView(this._inst.exports.memory.buffer, addr, len);
	}
	MemUint8Array(addr, len) {
		return new Uint8Array(this.Mem().buffer, addr, len)
	}

	GetString(addr, len) {
		return new TextDecoder("utf-8").decode(this.MemView(addr, len));
	}
	SetString(addr, len, s) {
		const bytes = new TextEncoder("utf-8").encode(s);
		if(len > bytes.length) { len = bytes.length; }
		this.MemUint8Array(addr, len).set(bytes);
	}
}

const rustApp = new RustApp();
rustApp.argv = process.argv.slice(2);

WebAssembly.instantiate(new Uint8Array(buf), {
	env: {
		env_print_str: (s, len) => {
			console.log(rustApp.GetString(s, len));
		}
	}
}).then((result) => {
	return rustApp.Run(result.instance);
}).catch((err) => {
	console.error(err);
});
