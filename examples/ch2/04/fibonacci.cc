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
#include <malloc.h>

EM_PORT_API(int*) fibonacci(int count) {
	if (count <= 0) return NULL;

	int* re = (int*)malloc(count * 4);
	if (NULL == re) {
		printf("Not enough memory.\n");
		return NULL;
	}

	re[0] = 1;
	int i0 = 0, i1 = 1;
	for (int i = 1; i < count; i++){
		re[i] = i0 + i1;
		i0 = i1;
		i1 = re[i];
	}
	
	return re;
}

EM_PORT_API(void) free_buf(void* buf) {
	free(buf);
}