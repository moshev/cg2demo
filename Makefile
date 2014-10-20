CC=gcc
CXX=g++
CFLAGS=-std=c11 -Wall -Werror -march=native -Og -g
CXXFLAGS=-std=c++11 -Wall -Werror -Wno-comment -Wno-error=comment -march=native -O2 -g `pkg-config --cflags-only-I sdl2` -I/usr/include/malloc -DGL_GLEXT_FUNCTION_POINTERS=1

all: cg2demo

audio.cpp: audio.h

cg2demo.h: protodef.inc

cg2demo.cpp: cg2demo.h protodecl.inc protoget.inc audio.h

scene.cpp: scene.h scenedsl.h

shaders.cpp: shaders.h shaders.inc

text.cpp: text.h

cg2demo: cg2demo.o scene.o shaders.o text.o audio.o
# -lGL has to be at end or you start getting awful warnings!
#  basically if you put -lGL in front, the libGL symbols will
#  overshadow the ones in the .o files
	${CXX} -o cg2demo -lm `pkg-config --libs sdl2` -L/System/Library/Frameworks/OpenGL.framework/Libraries cg2demo.o scene.o shaders.o text.o audio.o -lGL

texttest: texttest.o
	${CC} ${LDFLAGS} -o texttest texttest.o

clean:
	rm -rf *.o cg2demo texttest

