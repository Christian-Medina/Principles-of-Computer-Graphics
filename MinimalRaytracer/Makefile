
CPP = g++ -std=c++11 -g -O -frounding-math -fsignaling-nans

all: raytrace_demo test

test: gfxraytrace_test
	./gfxraytrace_test

gfxraytrace_test: headers gfxraytrace_test.cc
	${CPP} gfxraytrace_test.cc -o gfxraytrace_test

raytrace_demo: headers raytrace_demo.cc
	${CPP} raytrace_demo.cc -o raytrace_demo

headers: gfxcolor.hh gfximage.hh gfxmath.hh gfxppm.hh gfxraytrace.hh

clean:
	rm -f raytrace_demo
