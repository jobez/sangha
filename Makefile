CFLAGS = -fPIC -O0 -g $$(pkg-config --libs --cflags glfw3 gstreamer-plugins-base-1.0 gstreamer-plugins-bad-1.0 gstreamer-gl-1.0 gstreamer-1.0 gl glew x11)
LDLIBS = -ldl

all: main libapp.so

main: main.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $< $(LDLIBS)

libapp.so: app.c
	$(CC) $(CFLAGS) -shared $(LDFLAGS) -o $@ $< $(LDLIBS)

test: main libapp.so
	./$<

clean:
	$(RM) main libapp.so
