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
#include <atomic>

#ifndef SAFE_RELEASE
	#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }
#endif

class CRefCount {
public:
	CRefCount() : m_ref_count(1) {}
	virtual ~CRefCount() { printf("CRefCount:~CRefCount()\n"); }

	void AddRef() {
		m_ref_count++;
	}
	
	int Release() {
		int t = --m_ref_count;
		printf("refcount now:%d\n", t);
		if (t == 0) delete this;
		return t;
	}

protected:
	std::atomic<int> m_ref_count;
};

struct RefCount;

EM_PORT_API(void) CObj_AddRef(struct RefCount* obj) {
	CRefCount *ro = (CRefCount*)obj;
	ro->AddRef();
}

EM_PORT_API(int) CObj_Release(struct RefCount* obj) {
	CRefCount *ro = (CRefCount*)obj;
	if (ro) {
		return ro->Release();
	}
	else return 0;
}

//-----------------------------------

class CShape : public CRefCount{
public:
	CShape() {}
	virtual ~CShape() { printf("CShape:~CShape()\n"); }

	virtual void WhatAreYou() = 0;
};

struct Shape;

EM_PORT_API(void) Shape_WhatAreYou(struct Shape* shape) {
	CShape *obj = (CShape*)shape;
	obj->WhatAreYou();
}

//-----------------------------------

class CTriangle : public CShape {
public:
	CTriangle() {}
	virtual ~CTriangle() { printf("CTriangle:~CTriangle()\n"); }
	
	void WhatAreYou(){ printf("I'm a triangle.\n"); }
};

EM_PORT_API(struct Shape*) Shape_Create_Triangle() {
	CTriangle *obj = new CTriangle();
	return (struct Shape*)obj;
}
