const fs = require('fs');
const buf = fs.readFileSync('./hello.wasm');

class WasiApp {
	constructor() {
		this._inst = null;
	}

	Init(instance) {
		this._inst = instance;
		return this;
	}
	async Run(instance=null) {
		if(instance != null) { this._inst = instance; }
		this._inst.exports._start();
	}

	Exports() {
		return this._inst.exports;
	}

	MemView(addr, len) {
		return new DataView(this._inst.exports.memory.buffer, addr, len);
	}

	GetString(addr, len) {
		return new TextDecoder("utf-8").decode(this.MemView(addr, len));
	}
}

const wasiApp = new WasiApp();

WebAssembly.instantiate(new Uint8Array(buf), {
	wasi_snapshot_preview1: { fd_write: fd_write }
}).then((result) => {
	return wasiApp.Init(result.instance).Run();
}).catch((err) => {
	console.error(err);
});

function fd_write(fd, iovs, iovs_len, p_nwritten) {
	let nwritten = 0;
	for(let i = 0; i < iovs_len; i++) {
		let iov = wasiApp.MemView(iovs + i*8, 8);

		let iov_base = iov.getInt32(0, true);
		let iov_size = iov.getInt32(4, true);
		let data = wasiApp.GetString(iov_base, iov_size);

		process.stdout.write(data);
		nwritten += iov_size;
	}
	wasiApp.MemView(p_nwritten, 4).setInt32(0, nwritten, true);
	return 0;
}
