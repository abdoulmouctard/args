CC = gcc
EXEC = args-run
FILES = args.c args.h main.c
FLAGS = -std=c11

build: $(FILES)
	$(CC) -c args.c $(FLAGS)
	$(CC) -o $(EXEC) main.c args.o $(FLAGS)

clean:
	rm -vf *.o $(EXEC)

rebuild: clean build
