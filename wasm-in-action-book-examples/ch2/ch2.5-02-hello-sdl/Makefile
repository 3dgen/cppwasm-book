# Copyright 2016 ChaiShushan (chaishushan{AT}gmail.com).
# License: https://creativecommons.org/licenses/by-nc-sa/4.0/

EMCC:=docker run --rm -it -v `pwd`:/src apiaryio/emcc emcc

default:
	${EMCC} hello.cc -o index.html -O1

clean:
