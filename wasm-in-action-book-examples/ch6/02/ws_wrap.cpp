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

struct WS_WRAPPER;
struct WS_CB;

//imp by JavaScript, call by C:
EM_PORT_API(struct WS_WRAPPER*) WSNew(const char *url, struct WS_CB *cb);
EM_PORT_API(int) WSSend(struct WS_WRAPPER *ws, const char *data);
EM_PORT_API(void) WSDelete(struct WS_WRAPPER *ws);

//WebSocket callback:
class CWSCallback{
public:
	CWSCallback(const char *url){
		m_ws = WSNew(url, (struct WS_CB*)this);
	}
	virtual ~CWSCallback(){}
	
	void OnOpen(){
		printf("OnOpen\n");
		WSSend(m_ws, "I love wasm!");
	}
	
	void OnClose(){
		printf("OnClose\n");
	}
	
	void OnMessage(const char* data){
		printf("OnMessage: %s\n", data);
		WSDelete(m_ws);
		m_ws = NULL;
	}
	
	void OnError(){
		printf("OnError\n");
	}
	
	struct WS_WRAPPER *m_ws;
};

//imp by C, call by JavaScript:
EM_PORT_API(void) WSOnOpen(struct WS_CB *cb){
	if (cb == NULL) return;
	CWSCallback *pc = (CWSCallback*)cb;
	pc->OnOpen();
}

EM_PORT_API(void) WSOnClose(struct WS_CB *cb){
	if (cb == NULL) return;
	CWSCallback *pc = (CWSCallback*)cb;
	pc->OnClose();
}

EM_PORT_API(void) WSOnMessage(struct WS_CB *cb, const char* data){
	if (cb == NULL) return;
	CWSCallback *pc = (CWSCallback*)cb;
	pc->OnMessage(data);
}

EM_PORT_API(void) WSOnError(struct WS_CB *cb){
	if (cb == NULL) return;
	CWSCallback *pc = (CWSCallback*)cb;
	pc->OnError();
}

CWSCallback wscb("ws://localhost:40001/ws_echo");
