# sdl_include="-I/usr/include"
# sdl_lib="-L/usr/lib"
cflags="-ggdb -Wall -Wextra -Wno-unknown-pragmas -O3 $@"
sdl="$sdl_include $sdl_lib -lSDL3"

set -xe
cc -o main $cflags main.c $sdl -lm
