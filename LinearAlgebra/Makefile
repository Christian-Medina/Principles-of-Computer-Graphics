
all: test

test: gfxmath_test
	./gfxmath_test

gfxmath_test: gfxmath.hh gfxmath_test.cc
	g++ -std=c++11 gfxmath_test.cc -o gfxmath_test

clean:
	rm -f gfxmath_test
