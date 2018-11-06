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

class CShape {
public:
	CShape() {}
	virtual ~CShape() {}

	virtual void WhatAreYou() = 0;
};

class CTriangle : public CShape {
public:
	CTriangle() {}
	virtual ~CTriangle() {}
	
	void WhatAreYou(){ printf("I'm a triangle.\n"); }
};

class CCircle : public CShape {
public:
	CCircle() {}
	virtual ~CCircle() {}
	
	void WhatAreYou(){ printf("I'm a circle.\n"); }
};

//-----------------------------------

struct Shape;

EM_PORT_API(struct Shape*) Shape_New_Triangle() {
	CTriangle *obj = new CTriangle();
	return (struct Shape*)obj;
}

EM_PORT_API(struct Shape*) Shape_New_Circle() {
	CCircle *obj = new CCircle();
	return (struct Shape*)obj;
}

EM_PORT_API(void) Shape_Delete(struct Shape* shape) {
	CShape *obj = (CShape*)shape;
	delete obj;
}

EM_PORT_API(void) Shape_WhatAreYou(struct Shape* shape) {
	CShape *obj = (CShape*)shape;
	obj->WhatAreYou();
}
