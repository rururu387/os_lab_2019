CC=gcc
CFLAGS=-I/home/lavrentiy/Документы/МИЭТ/ОСиС/os_lab_2019/lab4/src/ -I/home/lavrentiy/Документы/МИЭТ/ОСиС/os_lab_2019/lab3/src/ -std=c99 -w
VPATH=/home/lavrentiy/Документы/МИЭТ/ОСиС/os_lab_2019/lab3/src/ /home/lavrentiy/Документы/МИЭТ/ОСиС/os_lab_2019/lab4/src/
LAB3PATH=/home/lavrentiy/Документы/МИЭТ/ОСиС/os_lab_2019/lab3/src/

all : utils.o find_min_max.o sum.o parallel_sum.o parallel_min_max.o parallel_sum parallel_min_max process_memory clean

parallel_min_max.o : parallel_min_max.c
	$(CC) -o $@ -c $(LAB3PATH)parallel_min_max.c $(CFLAGS)

parallel_min_max : utils.o find_min_max.o
	$(CC) -o $@ utils.o find_min_max.o parallel_min_max.o $(CFLAGS)

utils.o : utils.h utils.c
	$(CC) -o $@ -c $(LAB3PATH)utils.c $(CFLAGS)

find_min_max.o : utils.h find_min_max.h utils.c find_min_max.c
	$(CC) -o $@ -c $(LAB3PATH)find_min_max.c $(CFLAGS)

parallel_sum.o : parallel_sum.c sum.c sum.h
	$(CC) -o $@ -c parallel_sum.c $(CFLAGS)

sum.o : sum.c sum.h
	$(CC) -o $@ -c sum.c $(CFLAGS)

parallel_sum : parallel_sum.o utils.o find_min_max.o
	$(CC) -o $@ parallel_sum.c find_min_max.o utils.o sum.o $(CFLAGS) -lpthread

process_memory : process_memory.c
	$(CC) -o $@ process_memory.c -std=c99 -w

clean :
	rm utils.o find_min_max.o parallel_min_max.o parallel_sum.o sum.o

cleanAll :
	rm utils.o find_min_max.o parallel_min_max.o parallel_sum.o sum.o parallel_min_max parallel_sum process_memory text.txt