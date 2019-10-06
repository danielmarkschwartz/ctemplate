test: ctemplate
	./ctemplate tests/hello.mustache

ctemplate: *.c
	$(CC) -o ctemplate $(C_FLAGS) $(LD_FLAGS) *.c
