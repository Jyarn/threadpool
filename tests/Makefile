.PHONY: run clean
.SILENT:

run: test
	echo
	./test
	echo

test: *.c lib/libthreadpool.a
	gcc -Wall -Wextra -Wpedantic -ggdb test.c -o test -I .. -L ./lib -lthreadpool

lib/libthreadpool.a: ../*.c ../*.h
	make -C .. CFLAGS=-ggdb PREFIX=`pwd` EXP_INC="" INC_DIR=/dev/null

clean:
	rm -rf lib obj test
