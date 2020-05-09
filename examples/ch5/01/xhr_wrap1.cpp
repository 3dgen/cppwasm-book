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

//imp by JavaScript, call by C:
EM_PORT_API(void) XHRGet(const char* url);

//imp by C, call by JavaScript:
EM_PORT_API(void) XHROnMessage(const char* url, const char* data){
	printf("http request succeeded. URL: %s  contex: %s\n", url, data);
}

EM_PORT_API(void) XHROnError(const char* url, const int error_code){
	printf("http request failed. URL: %s  error code:%d\n", url, error_code);
}

int main(){
	XHRGet("t1.txt");
	XHRGet("t2.txt");
	XHRGet("t3.txt");
}