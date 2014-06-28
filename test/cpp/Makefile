CPP=g++
CPPFLAGS=-std=c++11 -I ../../
LDFLAGS=-pthread -lgflags -lglog -static

SRC=$(wildcard *.cc)
OBJ=$(SRC:%.cc=build/%.o)
GTEST_OBJ=/usr/src/gtest/libgtest.a
EXE=build/run_all_tests

.PHONY: all test clean

test: all
	./${EXE} --gtest_death_test_style=threadsafe --question="six by nine" --answer=42

all: build ${EXE}

build:
	mkdir -p build

build/%.o: %.cc
	${CPP} ${CPPFLAGS} -o $@ -c $<

${EXE}: ${OBJ}
	${CPP} ${CPPFLAGS} -o $@ ${OBJ} ${GTEST_OBJ} ${LDFLAGS}

clean:
	rm -rf build
