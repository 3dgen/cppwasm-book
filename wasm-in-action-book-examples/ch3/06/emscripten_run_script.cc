#include <emscripten.h>
#include <math.h>

const char* get_js_code(){
	static char buf[1024];
	sprintf(buf, "console.log('你好，Emscripten！');");
	return buf;
}

int main(){
	emscripten_run_script("console.log(42);");
	
	emscripten_run_script(get_js_code());
	
	emscripten_run_script(R"(
		function my_print(s) {
			console.log("JS:my_print():", s);
		}
		my_print("Hello!");
	)");

	int num = emscripten_run_script_int(R"(
		function show_me_the_number() {
			return 13;
		}
		show_me_the_number();
	)");
	printf("num:%d\n", num);
	
	const char* str = emscripten_run_script_string(R"(
		function show_me_the_answer() {
			return "The answer is 42.";
		}
		show_me_the_answer();
	)");
	printf("%s\n", str);

	
	return 0;
}
