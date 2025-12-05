CC=gcc
CFLAGS=-I.
DEPS = gilyshell.h
OBJ =	utils.o	Builtins.o	execute.o	parser.o	introduction.o	main.o

%.o: %.c $(DEPS)
		$(CC) -c -o $@ $< $(CFLAGS)

gilyshell: $(OBJ)
		gcc -o $@ $^ $(CFLAGS)