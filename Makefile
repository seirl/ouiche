.PHONY: all clean

all:
	mkdir -p build
	cd build && cmake ..
	$(MAKE) -s -C build

clean:
	rm -f TextMiningApp TextMiningCompiler
	rm -rf build
