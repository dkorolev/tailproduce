.PHONY: test clean

test: cereal
	(cd test/cpp && make test)

clean:
	rm -rf cereal/ ; (cd test/cpp && make clean)

# From git clone git@github.com:USCiLab/cereal.git
cereal:
	tar xjf cereal.tar.bz2
