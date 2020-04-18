# Makefile for image retrieval program.

FLAGS= -Wall -std=gnu99 -lm

all : one_process image_retrieval

one_process : one_process.o worker.o
	gcc -o $@ one_process.o worker.o ${FLAGS}

image_retrieval : image_retrieval.o worker.o
	gcc -o $@ image_retrieval.o worker.o ${FLAGS}

# Separately compile each C file
%.o : %.c
	gcc -c $< ${FLAGS}

one_process.o : worker.h
image_retrieval.o : worker.h
worker.o : worker.h

clean :
	-rm *.o one_process
	-rm *.o image_retrieval