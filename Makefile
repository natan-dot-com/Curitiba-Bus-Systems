CC=gcc
BINARY=./binary
SOURCE=./src/*.c
MAIN=./main.c
INCLUDE=include
VALGRIND=valgrind --leak-check=yes --show-leak-kinds=all

# Finds current's makefile directory path
MAKEFILE_PATH=$(abspath $(lastword $(MAKEFILE_LIST)))
# Creates zip file name based on current project folder 
ZIP_NAME=$(notdir $(patsubst %/,%,$(dir $(MAKEFILE_PATH)))).zip
COMPILE_CONFIG=compile_commands.json

all:
	$(CC) $(SOURCE) $(MAIN) -o $(BINARY) -I$(INCLUDE)

run:
	$(BINARY)

valgrind:
	$(CC) -g $(SOURCE) $(MAIN) -I$(INCLUDE)
	$(VALGRIND) ./a.out

gdb:
	$(CC) -g $(SOURCE) $(MAIN) -I$(INCLUDE)
	gdb a.out

clear:
	rm -f $(BINARY) $(ZIP_NAME) ./a.out

zip:
	mv $(COMPILE_CONFIG) .$(COMPILE_CONFIG)
	zip -vr $(ZIP_NAME) ./*
	mv .$(COMPILE_CONFIG) $(COMPILE_CONFIG)
