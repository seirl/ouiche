.PHONY: all clean

all: build/Makefile
	$(MAKE) -s -C build

build/Makefile: CMakeLists.txt
	mkdir -p build
	cd build && cmake ..

clean:
	rm -f TextMiningApp TextMiningCompiler
	rm -rf build
