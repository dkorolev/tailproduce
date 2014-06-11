CPP=g++
CPPFLAGS=-DTAILPRODUCE_TEST

SRC=$(wildcard *.cc)
EXE=$(SRC:%.cc=build/%)

.PHONY: all test clean

test: all
	for i in ${EXE} ; do ./$$i ; done

all: build ${EXE} 

build:
	mkdir -p build

build/%: build/%.o
	${CPP} ${CPPFLAGS} -o $@ $< /usr/src/gtest/libgtest.a /usr/src/gtest/libgtest_main.a -lpthread

build/%.o: %.cc
	${CPP} ${CPPFLAGS} -o $@ -c $<

clean:
	rm -rf build
