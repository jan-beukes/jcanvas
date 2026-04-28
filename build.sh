# sdl_include="-I/usr/include"
# sdl_lib="-L/usr/lib"
cflags="-ggdb -Wall -Wextra -O3 -fopenmp"
sdl="$sdl_include $sdl_lib -lSDL3"

set -xe
cc -o main $cflags main.c $sdl -lm
