CC	= g++
# Linux
GLMPATH = ../glm-0.9.2.7
CFLAGS = -I. -I/usr/X11R6/include -I/sw/include -c `sdl-config --cflags` -DLINUX -I${GLMPATH}
LDFLAGS = -L/usr/X11R6/lib -L/sw/lib -lGL -lGLU -lm -lpng -lz `sdl-config --libs` -lSDL_image -lGLEW

OBJS = main.o glUtil.o

all: main

clean: 
	rm main *.o *~ 

main: ${OBJS}
	${CC} ${OBJS} -o main ${LDFLAGS}

%.o: %.cc
	${CC} -c $< -o $@ ${CFLAGS}


