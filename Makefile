.PHONY: test clean

test:
	(cd test/cpp && make test)

clean:
	(cd test/cpp && make clean)
