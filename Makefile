LEVELDB_WITH_VERSION=leveldb-1.15.0

.PHONY: test deps lib clean cleanall love indent check

lib: deps
	make -f Makefile.tailproduce

check: deps
	make -f Makefile.tailproduce check

deps: leveldb/libleveldb.a cereal

clean:
	(make -f Makefile.tailproduce clean; cd test/cpp && make clean)

cleanall: clean
	rm -rf leveldb ${LEVELDB_WITH_VERSION} cereal

test: lib
	(cd test/cpp && make test)

test_coverage: lib
	(cd test/cpp && make coverage)

indent:
	find src/ test/ -regextype posix-egrep -regex ".*\.(cc|h)" | xargs clang-format-3.5 -i

# Keep dependencies static.

# From http://code.google.com/p/leveldb/downloads/list
leveldb/libleveldb.a: ${LEVELDB_WITH_VERSION}.tar.gz
	tar xzf ${LEVELDB_WITH_VERSION}.tar.gz
	ln -sf ${LEVELDB_WITH_VERSION} leveldb
	(cd leveldb ; CXX=clang++ make)

# From git clone git@github.com:USCiLab/cereal.git
cereal:
	tar xjf cereal.tar.bz2
