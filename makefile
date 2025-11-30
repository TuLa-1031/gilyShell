CC=gcc
CFLAGS=-I.
DEPS = shell.h
OBJ =	Builtins.o	launch.o	parser.o	introduction.o	main.o

%.o: %.c $(DEPS)
		$(CC) -c -o $@ $< $(CFLAGS)

shell: $(OBJ)
		gcc -o $@ $^ $(CFLAGS)