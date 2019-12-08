#include "homework.h"
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

extern int num_threads;
extern int resize_factor;
int main(int argc, char * argv[]) {
	//agrv[1] input
	//argv[2] output
	//argv[3] resize_factor
	//argv[4] num_threads
	if(argc < 5) {
		printf("Incorrect number of arguments\n");
		exit(-1);
	}

	resize_factor = atoi(argv[3]);
	num_threads = atoi(argv[4]);

	image input;
	image output;

	readInput(argv[1], &input);
	resize(&input, &output);
	writeData(argv[2], &output);
	
	return 0;
}
