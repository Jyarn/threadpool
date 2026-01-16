make build CFLAGS=-ggdb EXP_INC=/dev/null INC_DIR=/dev/null &&
    gcc -ggdb -Wall -Wpedantic -Wextra test.c -Llib -lthreadpool -o obj/test

if [ $? -eq 0 ]; then
    echo "+===========================+"
    ./obj/test
else
    echo "Build Failed :("
fi
