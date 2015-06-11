.PHONY: all clean test

MK=$(MAKE) -s -C build

all: build
	$(MK)

check: all build
	$(MK) check

tools: all build
	$(MK) tools

clean:
	rm -f TextMiningApp TextMiningCompiler
	rm -rf build

build: CMakeLists.txt
	mkdir -p build
	cd build && cmake ..
