#include <emscripten.h>
#include <math.h>

int main(){
	EM_ASM(console.log('你好，Emscripten！'));
	EM_ASM(var k = 42;console.log('The answer is:', k););
	EM_ASM(
		var k = 42;  //define k
		console.log('The answer is:', k);
	);

	int sum = EM_ASM_({return $0 + $1 + $2;}, 1, 2, 3);
	printf("sum(1, 2, 3): %d\n", sum);
	
	char buf[32];
	double pi = 3.14159;
	EM_ASM_(
		{
			console.log('addr of buf:', $0);
			console.log('sqrt(pi):', $1);
		},
		buf, sqrt(pi)
	);
		
	double pi2 = EM_ASM_DOUBLE(
		{
			return $0 * $1;
		},
		pi, 2.0
	);
	printf("pi2: %lf\n", pi2);
	
	EM_ASM_(
		{
            console.log('arguments count:', arguments.length);
			for(var i = 0; i < arguments.length; i++) {
				console.log('$', i, ':', arguments[i]);
			}
		},
		42, 13
	);
	
	int answer = EM_ASM_INT_V(return 42);
	printf("The answer is:%d\n", answer);
	double pi_js = EM_ASM_DOUBLE_V(return 3.14159);
	printf("PI:%lf\n", pi_js);

}
