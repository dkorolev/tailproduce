CPP=g++
CPPFLAGS=-std=c++11 -DTAILPRODUCE_TEST

SRC=$(wildcard *.cc)
OBJ=$(SRC:%.cc=build/%.o)
EXE=build/run_all_tests

.PHONY: all test clean

test: all
	./${EXE}

all: build ${EXE}

build:
	mkdir -p build

build/%.o: %.cc
	${CPP} ${CPPFLAGS} -o $@ -c $<

${EXE}: ${OBJ}
	${CPP} ${CPPFLAGS} -o $@ ${OBJ} /usr/src/gtest/libgtest.a /usr/src/gtest/libgtest_main.a -lpthread

clean:
	rm -rf build
