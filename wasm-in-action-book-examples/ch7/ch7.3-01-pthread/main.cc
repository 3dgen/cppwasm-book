#include <stdio.h>
#include <pthread.h>

int fib(int n) {
	return (n>1)? fib(n-1)+fib(n-2): 1;
}

void *fib_worker(void *arg) {
	int n = *((int*)arg);
	*((int*)arg) = fib(n);
	return arg;
}

// Foreground thread and main entry point
int main(int argc, char *argv[]) {
	int fib42 = 42;
	pthread_t bg_thread;

	if (pthread_create(&bg_thread, NULL, fib_worker, &fib42)) {
		printf("pthread_create failed\n");
		return 1;
	}

	int fib47 = fib(47);
	if (pthread_join(bg_thread, NULL)) {
		printf("pthread_join failed\n");
		return 2;
	}

	printf("Fib(42) = %d\n", fib42);
	printf("Fib(47) = %d\n", fib47);

	return 0;
}
