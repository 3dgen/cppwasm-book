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
#include <stdint.h>

EM_PORT_API(int64_t) i64_add(int64_t a, int64_t b) {
	int64_t c = a + b;
	printf("a:%lld, b:%lld:, a+b: %lld\n", a, b, c);
	return c;
}

int main() {
	printf("main():");
	printf("%lld\n", i64_add(9223372036854775806, 1));
}