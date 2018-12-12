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
struct WS_CONNECTOR;

//imp by JavaScript, call by C:
EM_PORT_API(struct WS_WRAPPER*) WSNew(const char *url, struct WS_CONNECTOR *conn);
EM_PORT_API(int) WSSend(struct WS_WRAPPER *ws, const char *data);
EM_PORT_API(void) WSDelete(struct WS_WRAPPER *ws);

class CWSConnector{
public:
	CWSConnector(const char *url){
		m_ws = WSNew(url, (struct WS_CONNECTOR*)this);
	}
	virtual ~CWSConnector(){}
	
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
EM_PORT_API(void) WSOnOpen(struct WS_CONNECTOR *conn){
	if (conn == NULL) return;
	CWSConnector *pc = (CWSConnector*)conn;
	pc->OnOpen();
}

EM_PORT_API(void) WSOnClose(struct WS_CONNECTOR *conn){
	if (conn == NULL) return;
	CWSConnector *pc = (CWSConnector*)conn;
	pc->OnClose();
}

EM_PORT_API(void) WSOnMessage(struct WS_CONNECTOR *conn, const char* data){
	if (conn == NULL) return;
	CWSConnector *pc = (CWSConnector*)conn;
	pc->OnMessage(data);
}

EM_PORT_API(void) WSOnError(struct WS_CONNECTOR *conn){
	if (conn == NULL) return;
	CWSConnector *pc = (CWSConnector*)conn;
	pc->OnError();
}

int main(){
	new CWSConnector("ws://localhost:40001/ws_echo");
}