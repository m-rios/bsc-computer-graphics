CC	= g++
# Linux
CFLAGS = -I. -I/usr/X11R6/include -I/sw/include -c `sdl-config --cflags` -DLINUX  -Wall -O3 -fopenmp 
LDFLAGS = -L/usr/X11R6/lib -L/sw/lib -lm -lz `sdl-config --libs` -O3 -fopenmp

#MacOS
#CFLAGS = -I. -I/usr/X11R6/include -I/sw/include -c -DDARWIN
#LDFLAGS = -L/usr/X11R6/lib -L/sw/lib -lGL -lGLU -lglut -lm -framework Cocoa -framework OpenGL -bind_at_load -lpng -lSDL_image

OBJS = main.o vector.o camera.o raytracer.o light.o material.o object.o transform.o sphere.o plane.o noise.o referenced.o csg.o

all: main

docs:
	@echo "Note that you need the doxygen and graphiz packages to re-generate the documentation"
	doxygen Doxyfile
clean: 
	rm main *.o *~

main: ${OBJS}
	${CC} ${OBJS} -o main ${LDFLAGS}

%.o: %.cc
	${CC} -c $< -o $@ ${CFLAGS}
