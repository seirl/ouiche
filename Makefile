.PHONY: all clean test

MK=$(MAKE) -s -C build
ARCHIVE_NAME=pietri_a-lecubi_a-textmining

all: build
	$(MK)

check: all build
	$(MK) check

tools: all build
	$(MK) tools

export:
	git archive HEAD --prefix=$(ARCHIVE_NAME)/ \
		| bzip2 > $(ARCHIVE_NAME).tar.bz2

clean:
	rm -f TextMiningApp TextMiningCompiler
	rm -rf build

build: CMakeLists.txt
	mkdir -p build
	cd build && cmake ..
