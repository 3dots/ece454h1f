
CC = g++-4.7
CFLAGS =  -O3 -lpthread

all: randtrack 

randtrack: list.h hash.h defs.h randtrackThreads.cc
	$(CC) $(CFLAGS) randtrackThreads.cc -o $(prog_name)


clean:
	rm -f *.o randtrack randtrack_global_lock randtrack_tm randtrack_list_lock randtrack_element_lock randtrack_reduction
