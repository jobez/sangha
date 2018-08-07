CFLAGS =-std=c++17  -fpic -O0 $$(pkg-config --libs --cflags fftw3 glfw3 glm gstreamer-plugins-base-1.0 gstreamer-plugins-bad-1.0 gstreamer-gl-1.0 gstreamer-app-1.0 gstreamer-1.0 gl glew x11 libpng) $$(llvm-config --ldflags --libs all)
LDLIBS = -lOSCFaust -lfaust -ljack

all: main libvengine.so libaengine.so

main: main.c
	$(CXX) $(CFLAGS) $(LDFLAGS) -o $@ $< $(LDLIBS)

libvengine.so: mouse_events.cc libvengine.cc avcapture.cc utils.cc node.cpp camera.cpp utils_viewer.cpp ext/imgui/imgui.cpp ext/imgui/imgui_demo.cpp ext/imgui/imgui_draw.cpp ext/imgui/imgui_impl_glfw.cpp ext/imgui/imgui_impl_opengl3.cpp
	$(CXX) $(CFLAGS) -shared $(LDFLAGS) -o $@ $^ $(LDLIBS)

libaengine.so: libaengine.cc SanghaFaust.cc SanghaAudio.cpp utils.cc
	$(CXX) $(CFLAGS) -shared $(LDFLAGS) -o $@ $^ $(LDLIBS)

test: main libvengine.so libaengine.so
	optirun gdb ./$<

run: main libvengine.so libaengine.so
	optirun ./$<

clean:
	$(RM) main *.so
