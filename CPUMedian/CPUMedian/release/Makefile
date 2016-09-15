
CC=g++
#CFLAGS=-c -Wall -I
CFLAGS=-c -Wall 

OBJECTS = bmp.o ctmf.o CPUMedian.o

VPATH = ../

#this is how you define a pattern
%.o : %.cpp
	#$(CC) -c $< -o $@
	$(CC) -O3 -c $< -o $@

%.o : %.c
	#$(CC) -c $< -o $@
	$(CC) -O3 -c $< -o $@

all: CPUMedian

CPUMedian: $(OBJECTS)
	$(CC) -O3 $(OBJECTS) -o CPUMedian


#CPUMedian.o: 
CPUMedian.o: CPUMedian.cpp

bmp.o: bmp.cpp

ctmf.o: ctmf.c

clean:
	rm -rf *o cpumedian


