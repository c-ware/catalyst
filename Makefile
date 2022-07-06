OBJS=src/main.o src/cstring/cstring.o src/libc99/stdlib.o src/libc99/stdio.o src/libmatch/read.o src/libmatch/cond.o src/libmatch/cursor.o src/libmatch/match.o src/libpath/libpath.o src/common/common.o src/jobs/jobs.o src/libproc/libproc.o src/libproc/sleep.o src/testing/testing.o src/parsers/parsers.o src/parsers/values.o 
TESTOBJS=src/cstring/cstring.o src/libc99/stdlib.o src/libc99/stdio.o src/libmatch/read.o src/libmatch/cond.o src/libmatch/cursor.o src/libmatch/match.o src/libpath/libpath.o src/common/common.o src/jobs/jobs.o src/libproc/libproc.o src/libproc/sleep.o src/testing/testing.o src/parsers/parsers.o src/parsers/values.o 
TESTS=tests/test_a tests/test_b 
CC=cc
PREFIX=/usr/local
LDFLAGS=
LDLIBS=-lm
CFLAGS=

all: $(OBJS) $(TESTS) catalyst

clean:
	rm -rf $(OBJS)
	rm -rf $(TESTS)
	rm -rf vgcore.*
	rm -rf core*
	rm -rf catalyst

install:
	mkdir -p $(PREFIX)
	install -m 755 catalyst $(PREFIX)/bin

uninstall:
	rm -f $(PREFIX)/bin/catalyst

tests/test_a: tests/test_a.c tests/common.h $(TESTOBJS)
	$(CC) tests/test_a.c -o tests/test_a $(TESTOBJS) $(CFLAGS) $(LDFLAGS) $(LDLIBS)

tests/test_b: tests/test_b.c tests/common.h $(TESTOBJS)
	$(CC) tests/test_b.c -o tests/test_b $(TESTOBJS) $(CFLAGS) $(LDFLAGS) $(LDLIBS)

src/main.o: src/main.c src/catalyst.h src/jobs/jobs.h src/common/common.h src/parsers/parsers.h
	$(CC) -c $(CFLAGS) src/main.c -o src/main.o $(LDFLAGS) $(LDLIBS)

src/cstring/cstring.o: src/cstring/cstring.c src/cstring/cstring.h
	$(CC) -c $(CFLAGS) src/cstring/cstring.c -o src/cstring/cstring.o $(LDFLAGS) $(LDLIBS)

src/libc99/stdlib.o: src/libc99/stdlib.c src/libc99/libc99.h
	$(CC) -c $(CFLAGS) src/libc99/stdlib.c -o src/libc99/stdlib.o $(LDFLAGS) $(LDLIBS)

src/libc99/stdio.o: src/libc99/stdio.c src/libc99/libc99.h
	$(CC) -c $(CFLAGS) src/libc99/stdio.c -o src/libc99/stdio.o $(LDFLAGS) $(LDLIBS)

src/libmatch/read.o: src/libmatch/read.c src/libmatch/libmatch.h
	$(CC) -c $(CFLAGS) src/libmatch/read.c -o src/libmatch/read.o $(LDFLAGS) $(LDLIBS)

src/libmatch/cond.o: src/libmatch/cond.c src/libmatch/libmatch.h
	$(CC) -c $(CFLAGS) src/libmatch/cond.c -o src/libmatch/cond.o $(LDFLAGS) $(LDLIBS)

src/libmatch/cursor.o: src/libmatch/cursor.c src/libmatch/libmatch.h
	$(CC) -c $(CFLAGS) src/libmatch/cursor.c -o src/libmatch/cursor.o $(LDFLAGS) $(LDLIBS)

src/libmatch/match.o: src/libmatch/match.c src/libmatch/libmatch.h
	$(CC) -c $(CFLAGS) src/libmatch/match.c -o src/libmatch/match.o $(LDFLAGS) $(LDLIBS)

src/libpath/libpath.o: src/libpath/libpath.c src/libpath/libpath.h src/libpath/lp_inter.h
	$(CC) -c $(CFLAGS) src/libpath/libpath.c -o src/libpath/libpath.o $(LDFLAGS) $(LDLIBS)

src/common/common.o: src/common/common.c src/common/common.h src/catalyst.h src/parsers/parsers.h
	$(CC) -c $(CFLAGS) src/common/common.c -o src/common/common.o $(LDFLAGS) $(LDLIBS)

src/jobs/jobs.o: src/jobs/jobs.c src/jobs/jobs.h src/catalyst.h src/common/common.h src/parsers/parsers.h src/testing/testing.h
	$(CC) -c $(CFLAGS) src/jobs/jobs.c -o src/jobs/jobs.o $(LDFLAGS) $(LDLIBS)

src/libproc/libproc.o: src/libproc/libproc.c src/libproc/libproc.h
	$(CC) -c $(CFLAGS) src/libproc/libproc.c -o src/libproc/libproc.o $(LDFLAGS) $(LDLIBS)

src/libproc/sleep.o: src/libproc/sleep.c src/libproc/libproc.h
	$(CC) -c $(CFLAGS) src/libproc/sleep.c -o src/libproc/sleep.o $(LDFLAGS) $(LDLIBS)

src/testing/testing.o: src/testing/testing.c src/testing/testing.h src/catalyst.h src/jobs/jobs.h src/parsers/parsers.h
	$(CC) -c $(CFLAGS) src/testing/testing.c -o src/testing/testing.o $(LDFLAGS) $(LDLIBS)

src/parsers/parsers.o: src/parsers/parsers.c src/catalyst.h src/parsers/parsers.h
	$(CC) -c $(CFLAGS) src/parsers/parsers.c -o src/parsers/parsers.o $(LDFLAGS) $(LDLIBS)

src/parsers/values.o: src/parsers/values.c src/catalyst.h src/parsers/parsers.h
	$(CC) -c $(CFLAGS) src/parsers/values.c -o src/parsers/values.o $(LDFLAGS) $(LDLIBS)

catalyst: $(OBJS)
	$(CC) $(OBJS) -o catalyst $(LDFLAGS) $(LDLIBS)
