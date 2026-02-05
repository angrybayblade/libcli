lib=lib
cc=gcc
bin=bin
examples=examples

$(bin)/flags: $(lib)/cli.h $(lib)/cli.c $(examples)/flags.c
	$(cc) $(examples)/flags.c $(lib)/cli.c -o $(bin)/flags -DDEBUG
