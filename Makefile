.PHONY: all clean test

MK=$(MAKE) -s -C build

all: build
	$(MK)

test: build
	$(MK) test

clean:
	rm -f TextMiningApp TextMiningCompiler
	rm -rf build

build: CMakeLists.txt
	mkdir -p build
	cd build && cmake ..
