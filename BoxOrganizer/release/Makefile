
CC=g++
#CFLAGS=-c -Wall -I
CFLAGS=-I -c -Wall 

OBJECTS = BoxesCollection.o BoxOrganizer.o

VPATH = ../BoxOrganizer/

#this is how you define a pattern
%.o : %.cpp
	$(CC) -c $< -o $@

%.o : %.c
	$(CC) -c $< -o $@

all: BoxOrganizer

BoxOrganizer: $(OBJECTS) libBoxLib.a
	$(CC) -o BoxOrganizer $(OBJECTS) -L. -lBoxLib 
	#ar rcs libBoxLib.a $(OBJECTS)

BoxesCollection.o: BoxesCollection.cpp

BoxOrganizer.o: BoxOrganizer.cpp 

clean:
	rm -rf *o BoxMerger

