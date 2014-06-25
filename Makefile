.PHONY: test lib clean

lib:
	make -f Makefile.tailproduce

test:
	(make -f Makefile.tailproduce; cd test/cpp && make test)

clean:
	(make -f Makefile.tailproduce clean; cd test/cpp && make clean)
