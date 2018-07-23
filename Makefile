CFLAGS = -fpic -O0 $$(pkg-config --libs --cflags aubio fftw3f glfw3 gstreamer-plugins-base-1.0 gstreamer-plugins-bad-1.0 gstreamer-gl-1.0 gstreamer-app-1.0 gstreamer-1.0 gl glew x11 libpng) $$(llvm-config --ldflags --libs all)
LDLIBS = -lOSCFaust -lfaust -ljack

all: main libvengine.so libaengine.so

main: main.c
	$(CXX) $(CFLAGS) $(LDFLAGS) -o $@ $< $(LDLIBS)

libvengine.so: libvengine.cc avcapture.cc
	$(CXX) $(CFLAGS) -shared $(LDFLAGS) -o $@ $^ $(LDLIBS)

libaengine.so: libaengine.cc SanghaFaust.cc SanghaAudio.cpp
	$(CXX) $(CFLAGS) -shared $(LDFLAGS) -o $@ $^ $(LDLIBS)

test: main libvengine.so libaengine.so
	optirun ./$<

clean:
	$(RM) main *.so
