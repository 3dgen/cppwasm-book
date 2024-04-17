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
#include <memory>

EM_PORT_API(double) add(double a, int b) {
	return a + (double)b;
}

EM_PORT_API(void) print_string(const char* str) {
	printf("C:print_string(): %s\n", str);
}

EM_PORT_API(int) sum(uint8_t* ptr, int count) {
	int total = 0, temp;
	for (int i = 0; i < count; i++){
		memcpy(&temp, ptr + i * 4, 4);
		total += temp;
	}
	return total;
}

EM_PORT_API(const char*) get_string() {
	const static char str[] = "This is a test.";
	return str;
}