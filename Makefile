CC=gcc
# CC=gcc -Wall

mysh: clean get_path.o which.o shell-with-builtin.o	dlobj
	$(CC) -g shell-with-builtin.c get_path.o which.o \
	dl_builtin.o \
	simple_ls.o \
	-I. -o mysh

shell-with-builtin.o: shell-with-builtin.c sh.h
	$(CC) -g -c shell-with-builtin.c

get_path.o: get_path.c get_path.h
	$(CC) -g -c get_path.c

which.o: which.c get_path.h
	$(CC) -g -c which.c

dlobj:
	$(CC) -g -c dl_builtin.c -I.
	$(CC) -g -c simple_ls.c -I.

clean:
	rm -rf shell-with-builtin.o get_path.o which.o mysh
