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

struct XHR_CB;

//imp by JavaScript, call by C:
EM_PORT_API(void) XHRGet(const char* url, XHR_CB* cb);

//XHR callback interface:
class CXHRCallbackInterface{
public:
	CXHRCallbackInterface(){}
	virtual ~CXHRCallbackInterface(){}
	
	virtual void OnMessage(const char* url, const char* data) = 0;
	virtual void OnError(const char* url, const int code) = 0;
};

//XHR callback1:
class CXHRCallback1 : public CXHRCallbackInterface{
public:
	CXHRCallback1(){}
	virtual ~CXHRCallback1(){}
	
	void OnMessage(const char* url, const char* data) {
		printf("CXHRCallback1::OnMessage(); URL: %s  contex: %s\n", url, data);
	}
	void OnError(const char* url, const int code) {
		printf("CXHRCallback1::OnError(); URL: %s  error code:%d\n", url, code);
	}
};

//XHR callback2:
class CXHRCallback2 : public CXHRCallbackInterface{
public:
	CXHRCallback2(){}
	virtual ~CXHRCallback2(){}
	
	void OnMessage(const char* url, const char* data) {
		printf("CXHRCallback2::OnMessage(); URL: %s  contex: %s\n", url, data);
	}
	void OnError(const char* url, const int code) {
		printf("CXHRCallback2::OnError(); URL: %s  error code:%d\n", url, code);
	}
};

//imp by C, call by JavaScript:
EM_PORT_API(void) XHROnMessage(const char* url, const char* data, XHR_CB* cb){
	CXHRCallbackInterface* ci = (CXHRCallbackInterface*)cb;
	ci->OnMessage(url, data);
}

EM_PORT_API(void) XHROnError(const char* url, const int code, XHR_CB* cb){
	CXHRCallbackInterface* ci = (CXHRCallbackInterface*)cb;
	ci->OnError(url, code);
}

CXHRCallback1 cb1;
CXHRCallback2 cb2;

int main(){
	XHRGet("t1.txt", (XHR_CB*)&cb1);
	XHRGet("t2.txt", (XHR_CB*)&cb2);
	XHRGet("t3.txt", (XHR_CB*)&cb2);
}