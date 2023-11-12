CC       = gcc
CPPFLAGS =
CFLAGS   = -Wall -Wextra -Wno-implicit-fallthrough -std=gnu17 -fPIC -O2

.PHONY: all clean

all: seq_example

seq.o: seq.c seq.h
memory_tests.o: memory_tests.c memory_tests.h


libseq.so: seq.o memory_tests.o
	gcc -shared -Wl,--wrap=malloc -Wl,--wrap=calloc -Wl,--wrap=realloc -Wl,\
	--wrap=reallocarray -Wl,--wrap=free -Wl,--wrap=strdup -Wl,\
	--wrap=strndup -o $@ $^

seq_example: seq_example.c libseq.so
	gcc -L. -g -o $@ $< -lseq

seq: seq.o
	gcc -o $@ $<

clean:
	rm -rf seq_example libseq seq *.a *.so *.o
