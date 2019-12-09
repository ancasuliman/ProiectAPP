all: homework-serial homework-pthread homework-mpi homework-openmp

homework-serial: serial/homework.c serial/homework.h serial/main.c
	gcc -o homework-serial serial/homework.c serial/main.c -Wall -lm

homework-pthread: pthreads/homework.c pthreads/homework.h pthreads/main.c
	gcc -o homework-pthread pthreads/homework.c pthreads/main.c -Wall -lm -pthread

homework-mpi: mpi/homework.c
	mpicc -o homework-mpi mpi/homework.c -Wall -lm

homework-openmp: openmp/homework.c openmp/homework.h openmp/main.c
	gcc -o homework-openmp openmp/homework.c openmp/main.c -Wall -lm -fopenmp

.PHONY: clean
clean:
	rm -f homework-serial homework-pthread homework-mpi homework-openmp
	rm -rf out/*
