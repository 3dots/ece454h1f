
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>


#include "defs.h"
#include "hash_reduction.h"

#define SAMPLES_TO_COLLECT   10000000
#define RAND_NUM_UPPER_BOUND   100000
#define NUM_SEED_STREAMS            4

/* 
 * ECE454 Students: 
 * Please fill in the following team struct 
 */
team_t team = {
    "Viktor",                  /* Team name */

    "Viktor Riabtsev",                    /* First member full name */
    "997544420",                 /* First member student number */
    "viktor.riabtsev@utoronto.ca",                 /* First member email address */

    "",                           /* Second member full name */
    "",                           /* Second member student number */
    ""                            /* Second member email address */
};

void process_stream(int i, void *p);
void * full(void *p);
void * half0(void *p);
void * half1(void *p);
void * quarter0(void *p);
void * quarter1(void *p);
void * quarter2(void *p);
void * quarter3(void *p);

unsigned num_threads;
unsigned samples_to_skip;

class sample;

class sample {
  unsigned my_key;
 public:
  sample *next;
  unsigned count;

  sample(unsigned the_key){my_key = the_key; count = 0;};
  unsigned key(){return my_key;}
  void print(FILE *f){printf("%d %d\n",my_key,count);}
};

// This instantiates an empty hash table
// it is a C++ template, which means we define the types for
// the element and key value here: element is "class sample" and
// key value is "unsigned".  
hash<sample,unsigned> h0;
hash<sample,unsigned> h1;
hash<sample,unsigned> h2;
hash<sample,unsigned> h3;

int  
main (int argc, char* argv[]){
  int i;

  // Print out team information
  printf( "Team Name: %s\n", team.team );
  printf( "\n" );
  printf( "Student 1 Name: %s\n", team.name1 );
  printf( "Student 1 Student Number: %s\n", team.number1 );
  printf( "Student 1 Email: %s\n", team.email1 );
  printf( "\n" );
  printf( "Student 2 Name: %s\n", team.name2 );
  printf( "Student 2 Student Number: %s\n", team.number2 );
  printf( "Student 2 Email: %s\n", team.email2 );
  printf( "\n" );

  // Parse program arguments
  if (argc != 3){
    printf("Usage: %s <num_threads> <samples_to_skip>\n", argv[0]);
    exit(1);  
  }
  sscanf(argv[1], " %d", &num_threads); // not used in this single-threaded version
  sscanf(argv[2], " %d", &samples_to_skip);



  // process streams starting with different initial numbers
  if(num_threads > 4 || num_threads <= 0 || num_threads == 3 || NUM_SEED_STREAMS != 4){
	  return -1;
  }

  // initialize a 16K-entry (2**14) hash of empty lists
  h0.setup(14);
  h1.setup(14);
  h2.setup(14);
  h3.setup(14);

  pthread_t threads[4];
  int err;

  if(num_threads == 1){
	  err = pthread_create(&threads[0], NULL, full, &h0);
	  if(err){
		  printf("Thread creation error: %d", err);
		  exit(EXIT_FAILURE);
	  }

	  pthread_join(threads[0], NULL);
  }
  else {
	  if(num_threads == 2){
		  err = pthread_create(&threads[0], NULL, half0, &h0);
		  if(err){
			  printf("Thread creation error: %d", err);
			  exit(EXIT_FAILURE);
		  }

		  err = pthread_create(&threads[1], NULL, half1, &h1);
		  if(err){
			  printf("Thread creation error: %d", err);
			  exit(EXIT_FAILURE);
		  }

		  pthread_join(threads[0], NULL);
		  pthread_join(threads[1], NULL);

		  h0.combine_with(&h1);
	  }
	  else{// 4


		  err = pthread_create(&threads[0], NULL, quarter0, &h0);
		  if(err){
			  printf("Thread creation error: %d", err);
			  exit(EXIT_FAILURE);
		  }

		  err = pthread_create(&threads[1], NULL, quarter1, &h1);
		  if(err){
			  printf("Thread creation error: %d", err);
			  exit(EXIT_FAILURE);
		  }

		  err = pthread_create(&threads[2], NULL, quarter2, &h2);
		  if(err){
			  printf("Thread creation error: %d", err);
			  exit(EXIT_FAILURE);
		  }

		  err = pthread_create(&threads[3], NULL, quarter3, &h3);
		  if(err){
			  printf("Thread creation error: %d", err);
			  exit(EXIT_FAILURE);
		  }

		  pthread_join(threads[0], NULL);
		  pthread_join(threads[1], NULL);
		  pthread_join(threads[2], NULL);
		  pthread_join(threads[3], NULL);

		  h0.combine_with(&h3);
		  h0.combine_with(&h2);
		  h0.combine_with(&h1);
	  }
  }



  h0.print();

  h0.cleanup();
  h1.cleanup();
  h2.cleanup();
  h3.cleanup();

  exit(EXIT_SUCCESS);
}

void process_stream(int i, void *p){
	int j,k;
	int rnum;
	unsigned key;
	sample *s;
	hash<sample,unsigned> *h = (hash<sample,unsigned> *) p;
	rnum = i;
	// collect a number of samples
	for (j=0; j<SAMPLES_TO_COLLECT; j++){

	  // skip a number of samples
	  for (k=0; k<samples_to_skip; k++){
	rnum = rand_r((unsigned int*)&rnum);
	  }

	  // force the sample to be within the range of 0..RAND_NUM_UPPER_BOUND-1
	  key = rnum % RAND_NUM_UPPER_BOUND;

	  // if this sample has not been counted before
	  if (!(s = h->lookup(key))){
	
	// insert a new element for it into the hash table
	s = new sample(key);
	h->insert(s);
	  }

	  // increment the count for the sample
	  s->count++;
	}
}

void * full(void *p){
	process_stream(0, p);
	process_stream(1, p);
	process_stream(2, p);
	process_stream(3, p);

	return p;
}

void * half0(void *p){
	process_stream(0, p);
	process_stream(1, p);
	//("%d\n", *((int *) p));
	return p;
}

void * half1(void *p){
	process_stream(2, p);
	process_stream(3, p);
	//printf("%d\n", *((int *) p));
	return p;
}

void * quarter0(void *p){
	process_stream(0, p);
	return p;
}

void * quarter1(void *p){
	process_stream(1, p);
	return p;
}

void * quarter2(void *p){
	process_stream(2, p);
	return p;
}

void * quarter3(void *p){
	process_stream(3, p);
	return p;
}


