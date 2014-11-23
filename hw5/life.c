/*****************************************************************************
 * life.c
 * Parallelized and optimized implementation of the game of life resides here
 ****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "life.h"
#include "util.h"




/*****************************************************************************
 * Helper function definitions
 ****************************************************************************/
void * game_of_life_thread_func(void *p){
	struct thread_data *input = (struct thread_data *) p;
	return (void *) sequential_game_of_life_parallel(input->outboard,
											input->inboard,
											input->nrows,
											input->ncols,
											input->gens_max,
											input->sector,
											input->status,
											input->mutex,
											input->cv);
}



/*****************************************************************************
 * Game of life implementation
 ****************************************************************************/
char*
game_of_life (char* outboard, 
	      char* inboard,
	      const int nrows,
	      const int ncols,
	      const int gens_max)
{

	int status = 0;
	pthread_mutex_t mutex;
	pthread_cond_t cv;

	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&cv, NULL);

	pthread_t threads[4];
	int err;

	struct thread_data ctx[4];
	int i;
	for(i = 0; i < 4; i++){
		ctx[i].outboard = outboard;
		ctx[i].inboard = inboard;
		ctx[i].nrows = nrows;
		ctx[i].ncols = ncols;
		ctx[i].gens_max = gens_max;
		ctx[i].status = &status;
		ctx[i].mutex = &mutex;
		ctx[i].cv = &cv;

		ctx[i].sector = i;

		err = pthread_create(&threads[i], NULL, game_of_life_thread_func, &ctx[i]);
		if(err){
			printf("Thread creation error: %d", err);
			pthread_mutex_destroy(&mutex);
			pthread_cond_destroy(&cv);
			exit(EXIT_FAILURE);
		}
	}

	void *t_status;

	pthread_join(threads[0], &t_status);
	pthread_join(threads[1], NULL);
	pthread_join(threads[2], NULL);
	pthread_join(threads[3], NULL);

	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&cv);



	return (char *) t_status;
}
