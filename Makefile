.PHONY: test lib clean

lib:
	make -f Makefile.tailproduce

clean:
	rm -rf cereal/ ; (make -f Makefile.tailproduce clean; cd test/cpp && make clean)

test:
	(make -f Makefile.tailproduce; cd test/cpp && make test)

# From git clone git@github.com:USCiLab/cereal.git
cereal:
	tar xjf cereal.tar.bz2
