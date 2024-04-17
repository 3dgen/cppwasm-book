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

class CSum {
public:
	CSum() {
		printf("CSum::CSum()\n");
		m_nSum = 13;
	}
	virtual ~CSum() {
		printf("CSum::~CSum()\n");
	}

	int Inc(int i){
		printf("CSum::Inc()\n");
		m_nSum += i;
		return m_nSum;
	}
private:
	int m_nSum;	
};

//-----------------------------------

struct Sum;

EM_PORT_API(struct Sum*) Sum_New() {
	CSum *obj = new CSum();
	return (struct Sum*)obj;
}

EM_PORT_API(void) Sum_Delete(struct Sum* sum) {
	CSum *obj = (CSum*)sum;
	delete obj;
}

EM_PORT_API(int) Sum_Inc(struct Sum* sum, int i) {
	CSum *obj = (CSum*)sum;
	return obj->Inc(i);
}
