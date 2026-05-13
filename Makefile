CC = gcc
CFLAGS = -ggdb -Wall -Wextra -O3
LIBS = -lSDL3 -lm

SDL_INCLUDE ?=
SDL_LIB ?=

main: main.c stb_image.o
	cc -o main $(CFLAGS) $^ $(SDL_INCLUDE) $(SDL_LIB) $(LIBS)

stb_image.o: stb_image.h
	cc -o stb_image.o -c -x c -D STB_IMAGE_IMPLEMENTATION -O3 stb_image.h
