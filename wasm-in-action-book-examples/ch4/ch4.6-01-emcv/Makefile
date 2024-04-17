# Copyright 2016 ChaiShushan (chaishushan{AT}gmail.com).
# License: https://creativecommons.org/licenses/by-nc-sa/4.0/

DOCKER_RUN:=docker run --rm -it -v `pwd`:/src apiaryio/emcc

EMCV_FILES:=$(wildcard ./emcv/cxcore/*.cpp)
EMCV_FILES+=$(wildcard ./emcv/cv/*.cpp)
EMCV_FILES+=$(wildcard ./emcv/cvaux/*.cpp)
EMCV_FILES+=$(wildcard ./emcv/highgui/*.cpp)

EMCV_FILES+=./libfacedetection_capi.cc
EMCV_FILES+=./libfacedetection/src/facedetectcnn.cpp
EMCV_FILES+=./libfacedetection/src/facedetectcnn-floatdata.cpp
EMCV_FILES+=./libfacedetection/src/facedetectcnn-int8data.cpp
EMCV_FILES+=./libfacedetection/src/facedetectcnn-model.cpp

EMCV_CFLAGS:=-I./emcv/cxcore
EMCV_CFLAGS+=-I./emcv/cv
EMCV_CFLAGS+=-I./emcv/cvaux
EMCV_CFLAGS+=-I./emcv/highgui

default:
	${DOCKER_RUN} emcc -s ALLOW_MEMORY_GROWTH=1 ${EMCV_CFLAGS} hello.cc ${EMCV_FILES} -o index.html -O1 --preload-file lena512color.bmp
	go run server.go

sh:
	${DOCKER_RUN} sh

clean:
