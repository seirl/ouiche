CC=g++
CFLAGS=-Wall -Wextra -std=c++14 -O3 -DNDEBUG
#CFLAGS+=-fprofile-arcs -ftest-coverage

all: TextMiningCompiler TextMiningApp

TextMiningCompiler: src/compiler.cc
	$(CC) $(CFLAGS) $^ -o $@

test/%: test/%.cc
	$(CC) $(CFLAGS) -Isrc $^ -o $@

TextMiningApp: src/app.cc
	$(CC) $(CFLAGS) $^ -o $@


clean:
	rm -f TextMiningCompiler TextMiningApp test/print test/deserialize-print \
		test/serialize test/deserialize
