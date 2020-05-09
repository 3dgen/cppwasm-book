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

#include <emscripten.h>
#include <stdio.h>
#include <time.h>

EM_PORT_API(void) step() {
	static int count = 0;
	static long cb = clock();	

	long t = clock();
	if (t - cb >= CLOCKS_PER_SEC) {
		cb += CLOCKS_PER_SEC;
		printf("current clock:%ld, current fps:%d\n", t, count);
		count = 0;
	}
	count++;
}
