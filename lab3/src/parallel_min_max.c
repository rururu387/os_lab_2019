#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <getopt.h>

#include "find_min_max.h"
#include "utils.h"

char* itoa(int val, int base){
	
	static char buf[32] = {0};
	
	int i = 30;
	
	for(; val && i ; --i, val /= base)
	
		buf[i] = "0123456789abcdef"[val % base];
	
	return &buf[i+1];
	
}

int main(int argc, char **argv) {
	int seed = -1;
	int array_size = -1;
	int pnum = -1;
	bool with_files = false;

	while (true) {
		int current_optind = optind ? optind : 1;

		static struct option options[] = {{"seed", required_argument, 0, 0},
										{"array_size", required_argument, 0, 0},
										{"pnum", required_argument, 0, 0},
										{"by_files", no_argument, 0, 'f'},
										{0, 0, 0, 0}};

		int option_index = 0;
		int c = getopt_long(argc, argv, "f", options, &option_index);

		if (c == -1) break;

		switch (c) {
			case 0:
				switch (option_index) {
					case 0:
						seed = atoi(optarg);
						if (seed < 0)
						{
								printf("Seed must be a positive number or 0. Now seed is %d", seed);
								return -1;
						}
						break;
					case 1:
						array_size = atoi(optarg);
						if (array_size < 0)
						{
								printf("Array_size must be a positive number. Now array_size is %d", array_size);
								return -1;
						}
					break;
					case 2:
						pnum = atoi(optarg);
						if (pnum < 1)
						{
								printf("Pnum must be a positive number. Now pnum is %d", pnum);
								return -1;
						}
						if (pnum > array_size)
						{
								printf("Pnum must be less than array size. %d > %d (pnum > array_size)", pnum, array_size);
								return -1;
						}
						break;
					case 3:
						with_files = true;
						break;

					defalut:
						printf("Index %d is out of options\n", option_index);
				}
				break;
			case 'f':
				with_files = true;
				break;

			case '?':
				break;

			default:
				printf("getopt returned character code 0%o?\n", c);
		}
	}

	if (optind < argc) {
		printf("Has at least one no option argument\n");
		return 1;
	}

	if (seed == -1 || array_size == -1 || pnum == -1) {
		printf("Usage: %s --seed \"num\" --array_size \"num\" --pnum \"num\" \n",
					 argv[0]);
		return 1;
	}

	int *array = (int*)malloc(sizeof(int) * array_size);
	GenerateArray(array, array_size, seed);
	int active_child_processes = 0;

	struct timeval start_time;
	gettimeofday(&start_time, NULL);
	int len = array_size / (pnum - 1);

	int* pipeArray = NULL;
	FILE* fl = NULL;
	if (with_files)
	{
		fl = fopen("test.txt", "w+");
		if (!fl)
		{
			printf("Failed to open file");
			return 1;
		}
	}
	else
	{
		pipeArray = (int*)malloc(sizeof(int) * pnum * 2);
		for (int i = 0; i < pnum; i++)
		{
			if (pipe(pipeArray + 2 * i) == -1)
			{
				printf("Pipe failed");
				return 1;
			}
		}
	}
	for (int i = 0; i < pnum; i++) {
		pid_t child_pid = fork();
		if (child_pid >= 0) {
			// successful fork
			active_child_processes += 1;
			if (child_pid == 0) {
				struct MinMax min_maxtmp;
				if (i == pnum - 1)
				{
					min_maxtmp = GetMinMax(array, (active_child_processes - 1) * len, array_size);
				}
				else
				{
					min_maxtmp = GetMinMax(array, (active_child_processes - 1) * len, active_child_processes * len);
				}
				// child process

				// parallel somehow
				int errFlag = -1;
				if (with_files) {
					errFlag = fwrite(min_maxtmp.min, sizeof(int), 1, fl);
					errFlag += fwrite(min_maxtmp.max, sizeof(int), 1, fl);
					if(errFlag < 2)
					{
						printf("Could not write to file/pipe(2)");
					}
					// use files here
				} else {
					errFlag = write(pipeArray[i * 2 + 1], min_maxtmp.min, sizeof(int));
					errFlag = write(pipeArray[i * 2 + 1], min_maxtmp.max, sizeof(int));
					close(pipeArray[i * 2 + 1]);
					// use pipe here
				}
				if(errFlag == -1)
				{
					printf("Could not write to file/pipe");
				}
				return 0;
			}

		} else {
			printf("Fork failed!\n");
			return 1;
		}
	}

	while (active_child_processes > 0) {
		wait(NULL);

		active_child_processes -= 1;
	}

	struct MinMax min_max;
	min_max.min = INT_MAX;
	min_max.max = INT_MIN;

	if (with_files)
	{
		close(fl);
		fl = fopen("test.txt", "r+");
		if (!fl)
		{
			printf("Failed to open file to read");
			return 1;
		}
	}

	for (int i = 0; i < pnum; i++) {
		char string[100] = {0};
		int min = INT_MAX;
		int max = INT_MIN;
		int errFlag = -1;
		if (with_files) {
			// read from files
			errFlag = fread(&min, sizeof(int), 1, fl);
			errFlag += fread(&max, sizeof(int), 1, fl);
			if(!feof(fl))
			{
				if (errFlag < 2)
				{
					printf("Could not read from file");
					return 1;
				}
			}
		} else {
			// read from pipes
			errFlag = read(pipeArray[2 * i], &min, sizeof(int));
			errFlag = read(pipeArray[2 * i], &max, sizeof(int));
			close(pipeArray[2 * i]);
		}
		if(errFlag == -1)
		{
			printf("Read from pipe error");
			return 1;
		}
		if (min < min_max.min) min_max.min = min;
		if (max > min_max.max) min_max.max = max;
	}

	if (with_files)
		close(fl);

	struct timeval finish_time;
	gettimeofday(&finish_time, NULL);

	double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
	elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

	if(pipeArray != NULL)
	{
		free(pipeArray);
	}
	free(array);

	printf("Min: %d\n", min_max.min);
	printf("Max: %d\n", min_max.max);
	printf("Elapsed time: %fms\n", elapsed_time);
	fflush(NULL);
	return 0;
}
