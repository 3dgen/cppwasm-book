DOCKER_RUN:=docker run --rm -it -v `pwd`:/src apiaryio/emcc

default:
	gcc -o a.out main.cc
	./a.out

wasm:
	${DOCKER_RUN} emcc -O2 -s USE_PTHREADS=1 -s PTHREAD_POOL_SIZE=2 -o a.out.js main.cc

clean:
