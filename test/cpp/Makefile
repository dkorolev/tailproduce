# TODO(dkorolev): we should probably keep g++ as the default,
# although I seem to like clang++'s speed and error messages better.

#CPP=g++
CPP=clang++

CPPFLAGS=-std=c++11 -g -I ../../ -I .

# TODO(dkorolev): This required `-lsnappy -L/home/dima/github/dkorolev/TAILPRODUCE/tailproduce`
LDFLAGS=-pthread -lgflags -lglog  -lleveldb -lboost_system -lboost_filesystem

SRC=$(wildcard *.cc)
OBJ=$(SRC:%.cc=build/%.o)
GTEST_OBJ=/usr/src/gtest/libgtest.a
EXE=build/run_all_tests

.PHONY: all test clean

test: all
	rm -fr ../leveldbTest
	./${EXE} --gtest_death_test_style=threadsafe --question="six by nine" --answer=42
	rm -fr ../leveldbTest

all: build ${EXE}

build:
	mkdir -p build

build/%.o: %.cc
	${CPP} ${CPPFLAGS} -o $@ -c $<

${EXE}: ${OBJ}
	${CPP} ${CPPFLAGS} -o $@ ${OBJ} ${GTEST_OBJ} ../../lib/libtailproduce.a ${LDFLAGS} 

clean:
	rm -rf build ../leveldbTest
