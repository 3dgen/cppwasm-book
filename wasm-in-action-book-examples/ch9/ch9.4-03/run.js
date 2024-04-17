// 版权 @2020 柴树杉。

const fs = require('fs');
const buf = fs.readFileSync('./a.out.wasm');

class RustApp {
	constructor() {
		this._inst = null;
	}

	Init(instance) {
		this._inst = instance;
	}
	Exports() {
		return this._inst.exports;
	}

	async Run(instance = null) {
		if(instance != null) { this._inst = instance; }
		this._inst.exports.main();
	}

	Mem() {
		return this._inst.exports.memory;
	}
	MemView(addr, len) {
		return new DataView(this._inst.exports.memory.buffer, addr, len);
	}
	MemUint8Array(addr, len) {
		return new Uint8Array(this.Mem().buffer, addr, len);
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

// ----------------------------------------------------------------------------

// string_new
// string_data_ptr
function RustString_new(rustApp, jsString) {
	const bytes = new TextEncoder("utf-8").encode(jsString);
	let rustString = rustApp.Exports().string_new(bytes.length);
	let addr = rustApp.Exports().string_data_ptr(rustString);
	rustApp.MemUint8Array(addr, bytes.length).set(bytes);
	return rustString;
}

// string_delete
function RustString_delete(rustApp, rustString) {
	rustApp.Exports().string_delete(rustString);
}

function RustARGS_init(rustApp, ...args) {
	args.forEach((v, i) => {
		let rustString = RustString_new(rustApp, v);
		rustApp.Exports().ARGS_init(i, rustString);
		RustString_delete(rustApp, rustString);
	})
}

// ----------------------------------------------------------------------------

const theRustApp = new RustApp();

WebAssembly.instantiate(new Uint8Array(buf), {
	env: {
		env_print_int: (x) => {
			console.log(x);
		},
		env_print_str: (s, len) => {
			console.log(theRustApp.GetString(s, len));
		}
	}
}).then((result) => {
	theRustApp.Init(result.instance);

	// 测试传入字符串
	let rustString = RustString_new(theRustApp, "你好, Rust字符串");
	theRustApp.Exports().string_print(rustString);

	// 初始化 argv
	RustARGS_init(theRustApp, ...process.argv);

	return theRustApp.Run();
}).catch((err) => {
	console.error(err);
});
