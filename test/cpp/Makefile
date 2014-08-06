PWD=$(shell pwd)

CPP=clang++
CPP_FOR_COVERAGE=g++

CPPFLAGS=-std=c++11 -g -I ../../ -I ../../leveldb/include/
CPPFLAGS_WITH_COVERAGE=${CPPFLAGS} -fprofile-arcs -ftest-coverage
LDFLAGS=-pthread -lgflags -lglog -lboost_system ../../leveldb/libleveldb.a -lsnappy # -lboost_filesystem

SRC=$(wildcard *.cc)
OBJ=$(SRC:%.cc=build/%.o)
OBJ_WITH_COVERAGE=$(SRC:%.cc=build/coverage/test/cpp/%.o)
GTEST_OBJ=/usr/src/gtest/libgtest.a
EXE=build/run_all_tests
EXE_WITH_COVERAGE=build/run_all_tests_with_coverage

TEST_EXE_FLAGS=--gtest_death_test_style=threadsafe --question="six by nine" --answer=42
# Uncomment the below line and replace the test name to debug a specific test.
# TEST_EXE_FLAGS+= --gtest_filter=StreamManagerTest/1.UserFriendlySyntaxCompiles --logtostderr -v 3

.PHONY: all test clean

test: all
	rm -fr ../leveldbTest
	./${EXE} ${TEST_EXE_FLAGS}
	rm -fr ../leveldbTest

coverage: ${EXE_WITH_COVERAGE}
	rm -fr ../leveldbTest
	./${EXE_WITH_COVERAGE} ${TEST_EXE_FLAGS}
	rm -fr ../leveldbTest
	( \
		cd build/coverage/test/cpp/ ; \
		geninfo . --output-file coverage.info ; \
		for i in *.cc ; do gcov -l $$i ; done > /dev/null ; \
		geninfo . --output-file coverage.info ;\
		genhtml coverage.info --output-directory ../../../coverage_report | grep -A 2 "^Overall" \
	)

all: build ${EXE}

build:
	mkdir -p build
	mkdir -p build/coverage/test/cpp build/coverage/
	ln -sf ${PWD}/../../src ${PWD}/build/coverage/
	for i in *.cc ; do ln -sf ${PWD}/$$i ${PWD}/build/coverage/test/cpp/$$i ; done

build/%.o: %.cc
	${CPP} ${CPPFLAGS} -o $@ -c $<

build/coverage/test/cpp/%.o: %.cc
	${CPP_FOR_COVERAGE} ${CPPFLAGS_WITH_COVERAGE} -o $@ -c $<

${EXE}: build ${OBJ}
	${CPP} ${CPPFLAGS} -o $@ ${OBJ} ${GTEST_OBJ} ../../lib/libtailproduce.a ${LDFLAGS} 

# TODO(dkorolev): Is it worth it to cover libtailproduce.a with tests,
# or does header coverage provide us with enough data?
${EXE_WITH_COVERAGE}: build ${OBJ_WITH_COVERAGE}
	${CPP_FOR_COVERAGE} ${CPPFLAGS_WITH_COVERAGE} -o $@ ${OBJ_WITH_COVERAGE} ${GTEST_OBJ} ../../lib/libtailproduce.a ${LDFLAGS} 

clean:
	rm -rf build ../testdata*
