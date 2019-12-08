all: homework-serial homework-pthread

homework-serial: serial/homework.c serial/homework.h serial/main.c
	gcc -o homework-serial serial/homework.c serial/main.c -Wall -lm

homework-pthread: pthreads/homework.c pthreads/homework.h pthreads/main.c
	gcc -o homework-pthread pthreads/homework.c pthreads/main.c -Wall -lm -pthread

.PHONY: clean
clean:
	rm -f homework-serial homework-pthread
	rm -rf out/*
