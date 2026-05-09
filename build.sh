# sdl_include="-I/usr/include"
# sdl_lib="-L/usr/lib"
cflags="-ggdb -Wall -Wextra -O3 $@"
sdl="$sdl_include $sdl_lib -lSDL3"

if [ ! -f stb_image.o ]; then
    cc -o stb_image.o -c -x c -D STB_IMAGE_IMPLEMENTATION -O3 stb_image.h
fi

set -xe
cc -o main $cflags main.c stb_image.o $sdl -lm
