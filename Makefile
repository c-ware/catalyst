OBJS=./src/main.o ./src/cstring/cstring.o ./src/libpath/libpath.o 
TESTOBJS=./src/cstring/cstring.o ./src/libpath/libpath.o 
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

./src/libpath/libpath.o: ./src/libpath/libpath.c ./src/libpath/libpath.h ./src/libpath/lp_inter.h
	$(CC) -c $(CFLAGS) ./src/libpath/libpath.c -o ./src/libpath/libpath.o $(LDFLAGS) $(LDLIBS)

catalyst: $(OBJS)
	$(CC) $(OBJS) -o catalyst $(LDFLAGS) $(LDLIBS)
