CC=gcc
CFLAGS=-Iinclude -Wall
DEPS = include/shell.h include/Builtins.h include/execute.h include/introduction.h include/parser.h include/utils.h
VPATH = src

OBJ = utils.o Builtins.o execute.o parser.o introduction.o main.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

gilyshell: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -f *.o gilyshell