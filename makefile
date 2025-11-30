CC=gcc
CFLAGS=-I.
DEPS = gilyshell.h
OBJ =	Builtins.o	launch.o	parser.o	introduction.o	main.o

%.o: %.c $(DEPS)
		$(CC) -c -o $@ $< $(CFLAGS)

gilyshell: $(OBJ)
		gcc -o $@ $^ $(CFLAGS)