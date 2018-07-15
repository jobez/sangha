CFLAGS = -fPIC -O0 -g $$(pkg-config --libs --cflags glfw3 gstreamer-plugins-base-1.0 gstreamer-plugins-bad-1.0 gstreamer-gl-1.0 gstreamer-1.0 gl glew x11)
LDLIBS = -ldl

all: main librenderer.so

main: main.c
	$(CXX) $(CFLAGS) $(LDFLAGS) -o $@ $< $(LDLIBS)

librenderer.so: librenderer.c
	$(CXX) $(CFLAGS) -shared $(LDFLAGS) -o $@ $< $(LDLIBS)

test: main librenderer.so
	optirun ./$<

clean:
	$(RM) main librenderer.so
