CC = gcc
CFLAGS = -ggdb -Wall -Wextra -O3
LIBS = -lSDL3 -lm

SDL_INCLUDE ?=
SDL_LIB ?=

demo: demo.c jcanvas.o stb_image.o
	cc -o demo $(CFLAGS) $^ $(SDL_INCLUDE) $(SDL_LIB) $(LIBS)

jcanvas.o: jcanvas.h
	cc -o jcanvas.o -c $(CFLAGS) -x c jcanvas.h

stb_image.o: stb_image.h
	cc -o stb_image.o -c -x c -D STB_IMAGE_IMPLEMENTATION -O3 stb_image.h
