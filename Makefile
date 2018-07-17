CFLAGS = -fpic -O0 $$(pkg-config --libs --cflags glfw3 gstreamer-plugins-base-1.0 gstreamer-plugins-bad-1.0 gstreamer-gl-1.0 gstreamer-app-1.0 gstreamer-1.0 gl glew x11 libpng)
LDLIBS =

all: main librenderer.so

main: main.c
	$(CXX) $(CFLAGS) $(LDFLAGS) -o $@ $< $(LDLIBS)

librenderer.so: librenderer.cc record.cc
	$(CXX) $(CFLAGS) -shared $(LDFLAGS) -o $@ $^ $(LDLIBS)

test: main librenderer.so
	optirun ./$<

clean:
	$(RM) main librenderer.so
