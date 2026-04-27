WAYLAND_XML = $(wildcard wayland/*.xml)
WAYLAND_HEADERS = $(WAYLAND_XML:.xml=.h)
WAYLAND_SOURCES = $(WAYLAND_XML:.xml=.c)

all: $(WAYLAND_HEADERS) $(WAYLAND_SOURCES)

$(WAYLAND_HEADERS): wayland/%.h: wayland/%.xml
	wayland-scanner client-header $< $@

$(WAYLAND_SOURCES): wayland/%.c: wayland/%.xml
	wayland-scanner public-code $< $@

sources:
	@echo $(WAYLAND_SOURCES)

clean:
	rm $(WAYLAND_SOURCES) $(WAYLAND_HEADERS)
