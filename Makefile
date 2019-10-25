test: ctemplate
	./ctemplate tests/hello.mustache > tests/out.c
	$(CC) -o tests/out $(CFLAGS) $(LDFLAGS) -lsqlite3 tests/out.c
	./tests/out tests/test.db --table 'users' --where 'id=2'

ctemplate: *.c
	$(CC) -o ctemplate $(CFLAGS) $(LDFLAGS) *.c

clean:
	rm ctemplate tests/out*
