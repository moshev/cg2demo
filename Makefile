CC=gcc
CXX=g++
CFLAGS=-std=c11 -Wall -Werror -march=native -Og -g
CXXFLAGS=-std=c++11 -Wall -Werror -Wno-comment -Wno-error=comment -march=native -O2 -g `pkg-config --cflags-only-I sdl2`

cg2demo: cg2demo.o scene.o shaders.o
# -lGL has to be at end or you start getting awful warnings!
#  basically if you put -lGL in front, the libGL symbols will
#  overshadow the ones in the .o files
	${CXX} -o cg2demo -lm `pkg-config --libs sdl2` cg2demo.o scene.o shaders.o -lGL

texttest: texttest.o
	${CC} ${LDFLAGS} -o texttest texttest.o

clean:
	rm -rf *.o cg2demo texttest

