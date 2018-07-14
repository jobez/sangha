all: libapp.so main

CXX = g++
LDLIBS = -ldl
CFLAGS =-fPIC -O0 -g

%.h: %.cc.lisp
	cm-cxx -E "(require :cm-ifs)" -E '(setf cm-ifs:*gen-interface* t)' $< -o $@

%.cc: %.cc.lisp
	cm-cxx -E "(require :cm-ifs)" -E '(setf cm-ifs:*gen-interface* nil)'  $< -o $@

libapp.so: api.h app.cc
	$(CXX) $^ -fpermissive $(CFLAGS) -shared $(LDFLAGS) -o $@ $< $(LDLIBS)

main: main.cc
	$(CXX) $(CFLAGS) $(LDFLAGS) -o $@ $< $(LDLIBS)

test: main libapp.so
	./$<

clean:
	rm -f *.cc *.h *.so
