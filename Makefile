lib=lib
cc=gcc
bin=bin
test=test

$(bin)/flags: $(lib)/cli.h $(lib)/cli.c $(test)/flags.c
	$(cc) $(test)/flags.c $(lib)/cli.c -o $(bin)/flags
