cflags="-ggdb -Wall -Wextra"
if [[ -n $WAYlAND ]]; then
    # on wayland we need to use wayland-scanner to generate the protocol sources and headers
    # from the xml files in ./wayland and then link with them
    sources=$(make -s -f wayland.mk sources)
    make -s -f wayland.mk

    libs=" $sources -lwayland-cursor -lwayland-client -lxkbcommon -lm"
    cflags+=" -DRGFW_WAYLAND -I./wayland"
else
    libs="-lX11 -lXrandr -lm"
fi

cc -o main $cflags main.c $libs
