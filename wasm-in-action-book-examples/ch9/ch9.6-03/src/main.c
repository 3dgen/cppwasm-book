// 版权 @2020 柴树杉。

extern void env_print_int(int x);

int strlen(const char* s) {
	const char* p = s;
	while(*p != '\0') {
		p = p+1;
	}
	return (int)(p-s);
}

void cMain() {
	const char* s = "abc";
	int len = strlen(s);
	env_print_int(len);
}
