
CC = g++-4.7
CFLAGS =  -O3 

all: randtrack 

randtrack: list.h hash.h defs.h randtrack.cc
	$(CC) $(CFLAGS) randtrack.cc -o randtrack

randtrackThreads: list.h hash.h defs.h randtrackThreads.cc
	$(CC) $(CFLAGS) randtrackThreads.cc -o randtrack -lpthread
	
randtrack_global_lock: list.h hash.h defs.h randtrack_global_lock.cc
	$(CC) $(CFLAGS) randtrack_global_lock.cc -o randtrack_global_lock -lpthread
	
randtrack_tm: list.h hash.h defs.h randtrack_tm.cc
	$(CC) $(CFLAGS) randtrack_tm.cc -o randtrack_tm -fgnu-tm -lpthread
	
randtrack_list_lock: list.h hash_list_lock.h defs.h randtrack_list_lock.cc
	$(CC) $(CFLAGS) randtrack_list_lock.cc -o randtrack_list_lock -lpthread

clean:
	rm -f *.o randtrack randtrack_global_lock randtrack_tm randtrack_list_lock randtrack_element_lock randtrack_reduction
