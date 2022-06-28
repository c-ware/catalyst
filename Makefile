OBJS=./src/main.o ./src/cstring/cstring.o ./src/libmatch/read.o ./src/libmatch/cond.o ./src/libmatch/cursor.o ./src/libmatch/match.o ./src/libpath/libpath.o ./src/parsers/parsers.o 
TESTOBJS=./src/cstring/cstring.o ./src/libmatch/read.o ./src/libmatch/cond.o ./src/libmatch/cursor.o ./src/libmatch/match.o ./src/libpath/libpath.o ./src/parsers/parsers.o 
TESTS=
CC=cc
PREFIX=/usr/local

all: $(OBJS) $(TESTS) catalyst

clean:
	rm -rf $(OBJS)
	rm -rf $(TESTS)
	rm -rf vgcore.*
	rm -rf core*
	rm -rf catalyst

install:
	mkdir -p $(PREFIX)
	mkdir -p $(PREFIX)/bin
	install -m 755 catalyst $(PREFIX)/bin

uninstall:
	rm -f $(PREFIX)/bin/catalyst

./src/main.o: ./src/main.c ./src/catalyst.h
	$(CC) -c $(CFLAGS) ./src/main.c -o ./src/main.o $(LDFLAGS) $(LDLIBS)

./src/cstring/cstring.o: ./src/cstring/cstring.c ./src/cstring/cstring.h
	$(CC) -c $(CFLAGS) ./src/cstring/cstring.c -o ./src/cstring/cstring.o $(LDFLAGS) $(LDLIBS)

./src/libmatch/read.o: ./src/libmatch/read.c ./src/libmatch/libmatch.h
	$(CC) -c $(CFLAGS) ./src/libmatch/read.c -o ./src/libmatch/read.o $(LDFLAGS) $(LDLIBS)

./src/libmatch/cond.o: ./src/libmatch/cond.c ./src/libmatch/libmatch.h
	$(CC) -c $(CFLAGS) ./src/libmatch/cond.c -o ./src/libmatch/cond.o $(LDFLAGS) $(LDLIBS)

./src/libmatch/cursor.o: ./src/libmatch/cursor.c ./src/libmatch/libmatch.h
	$(CC) -c $(CFLAGS) ./src/libmatch/cursor.c -o ./src/libmatch/cursor.o $(LDFLAGS) $(LDLIBS)

./src/libmatch/match.o: ./src/libmatch/match.c ./src/libmatch/libmatch.h
	$(CC) -c $(CFLAGS) ./src/libmatch/match.c -o ./src/libmatch/match.o $(LDFLAGS) $(LDLIBS)

./src/libpath/libpath.o: ./src/libpath/libpath.c ./src/libpath/libpath.h ./src/libpath/lp_inter.h
	$(CC) -c $(CFLAGS) ./src/libpath/libpath.c -o ./src/libpath/libpath.o $(LDFLAGS) $(LDLIBS)

./src/parsers/parsers.o: ./src/parsers/parsers.c ./src/catalyst.h
	$(CC) -c $(CFLAGS) ./src/parsers/parsers.c -o ./src/parsers/parsers.o $(LDFLAGS) $(LDLIBS)

catalyst: $(OBJS)
	$(CC) $(OBJS) -o catalyst $(LDFLAGS) $(LDLIBS)
