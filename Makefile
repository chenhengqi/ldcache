build:
	gcc -Wall -o gcc.out ldcache.c main.c
	clang -Wall -o clang.out ldcache.c main.c

test:
	./gcc.out ./testdata/ld.so.cache.old
	./clang.out ./testdata/ld.so.cache.old
	./gcc.out ./testdata/ld.so.cache
	./clang.out ./testdata/ld.so.cache

clean:
	rm -f *.out
