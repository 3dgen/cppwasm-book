#ifndef EM_PORT_API
#	if defined(__EMSCRIPTEN__)
#		include <emscripten.h>
#		if defined(__cplusplus)
#			define EM_PORT_API(rettype) extern "C" rettype EMSCRIPTEN_KEEPALIVE
#		else
#			define EM_PORT_API(rettype) rettype EMSCRIPTEN_KEEPALIVE
#		endif
#	else
#		if defined(__cplusplus)
#			define EM_PORT_API(rettype) extern "C" rettype
#		else
#			define EM_PORT_API(rettype) rettype
#		endif
#	endif
#endif

#include <stdio.h>

void sync_idbfs() {
	EM_ASM(
		FS.syncfs(function (err) {});
	);
}

EM_PORT_API(void) test() {
	FILE* fp = fopen("/data/nodefs_data.txt", "r+t");
	if (fp == NULL) fp = fopen("/data/nodefs_data.txt", "w+t");
	int count = 0;
	if (fp) {
		fscanf(fp, "%d", &count);
		count++;
		fseek(fp, 0, SEEK_SET);
		fprintf(fp, "%d", count);
		fclose(fp);
		printf("count:%d\n", count);
		
		sync_idbfs();
	}
	else {
		printf("fopen failed.\n");
	}
}

int main() {
	EM_ASM(
		FS.mkdir('/data');
		FS.mount(IDBFS, {}, '/data');
		FS.syncfs(true, function (err) {
			assert(!err);
			ccall('test', 'v');
		});
	);
	
	return 0;
}