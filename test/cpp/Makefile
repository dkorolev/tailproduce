CPP=g++

# -fprofile-arcs -ftest-coverage are for gcov
# Extra requirements:
# * need to run the binary from the same directory as the source files
# * need to have header files available under the same relative paths
# * need to run gcov -l for each .cc file
# * install lcov 1.11
# * geninfo . --output-file coverage.info
# * genhtml coverage.info --output-directory out

CPPFLAGS=-std=c++11 -g -I ../../ -I ../../leveldb/include/ -fprofile-arcs -ftest-coverage

LDFLAGS=-pthread -lgflags -lglog -lboost_system -lboost_filesystem ../../leveldb/libleveldb.a -lsnappy

SRC=$(wildcard *.cc)
OBJ=$(SRC:%.cc=build/%.o)
GTEST_OBJ=/usr/src/gtest/libgtest.a
EXE=build/run_all_tests

TEST_EXE_FLAGS=--gtest_death_test_style=threadsafe --question="six by nine" --answer=42

# Uncomment the below line and replace the test name to debug a specific test.
# TEST_EXE_FLAGS+= --gtest_filter=StreamManagerTest/1.UserFriendlySyntaxCompiles --logtostderr -v 3

.PHONY: all test clean

test: all
	rm -fr ../leveldbTest
	./${EXE} ${TEST_EXE_FLAGS}
	rm -fr ../leveldbTest

all: build ${EXE}

build:
	mkdir -p build
	for i in *.cc ; do ln -sf $$PWD/$i $$PWD/build/$i ; done  # For gcov.

build/%.o: %.cc
	${CPP} ${CPPFLAGS} -o $@ -c $<

${EXE}: ${OBJ}
	${CPP} ${CPPFLAGS} -o $@ ${OBJ} ${GTEST_OBJ} ../../lib/libtailproduce.a ${LDFLAGS} 

clean:
	rm -rf build ../leveldbTest
