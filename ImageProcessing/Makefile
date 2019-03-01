
all: test

test: gfximage_test
	./gfximage_test

gfximage_test: gfxcolor.hh gfxfilter.hh gfximage.hh gfxmath.hh gfxppm.hh gfximage_test.cc
	g++ -std=c++11 gfximage_test.cc -o gfximage_test

clean:
	rm -f gfximage_test
