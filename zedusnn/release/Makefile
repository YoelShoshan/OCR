
CC=g++

INCLUDE_DIRS = /usr/include/eigen3

#CFLAGS=-c -Wall -I
#CFLAGS= -v -I -c -Wall
CFLAGS= -O3 -v -shared -fPIC -I $(INCLUDE_DIRS) -I ../SFMT -c
#CFLAGS= -v -fpic -I $(INCLUDE_DIRS) -c

OBJECTS = SFMT.o zedusOS.o bmp.o NeuralNetwork.o NN_Executer.o Trainer_BackProp.o Training_Set.o NeuralNetworkUpscaling.o 

VPATH = ../:../SFMT

#this is how you define a pattern
%.o : %.cpp
	$(CC) $(CFLAGS) $< -o $@

%.o : %.c
	$(CC) $(CFLAGS) $< -o $@

all: libzedusnn.so

libzedusnn.so: $(OBJECTS)
#	ar rcs zedusnn.a $(OBJECTS)
	$(CC) -O3 -shared -Wall $(OBJECTS) -o libzedusnn.so

SFMT.o: SFMT.c

zedusOS.o: zedusOS.cpp

bmp.o: bmp.cpp 

NeuralNetwork.o: NeuralNetwork.cpp 

NN_Executer.o: NN_Executer.cpp

Trainer.o: Trainer_BackProp.cpp

NeuralNetworkUpscaling.o : NeuralNetworkUpscaling.cpp

clean:
	rm -rf *o zedusnn.a

