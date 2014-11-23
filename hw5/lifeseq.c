/*****************************************************************************
 * life.c
 * The original sequential implementation resides here.
 * Do not modify this file, but you are encouraged to borrow from it
 ****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "life.h"
#include "util.h"

/**
 * Swapping the two boards only involves swapping pointers, not
 * copying values.
 */
#define SWAP_BOARDS( b1, b2 )  do { \
  char* temp = b1; \
  b1 = b2; \
  b2 = temp; \
} while(0)

#define BOARD( __board, __i, __j )  (__board[(__i) + LDA*(__j)])


static inline void update(int i, int j, char* outboard,
        char* inboard,
        const int nrows,
        const int ncols){
	const int LDA = nrows;

	int inorth;
	int isouth;
	int jwest;
	int jeast;

	if(i == 0)
		inorth = nrows - 1;
	else
		inorth = i-1;

	if(i == nrows - 1)
		isouth = 0;
	else
		isouth = i+1;

	if(j == 0)
		jwest = ncols - 1;
	else
		jwest = j-1;

	if(j == ncols - 1)
		jeast = 0;
	else
		jeast = j+1;


	const char neighbor_count =
		BOARD (inboard, inorth, jwest) +
		BOARD (inboard, inorth, j) +
		BOARD (inboard, inorth, jeast) +
		BOARD (inboard, i, jwest) +
		BOARD (inboard, i, jeast) +
		BOARD (inboard, isouth, jwest) +
		BOARD (inboard, isouth, j) +
		BOARD (inboard, isouth, jeast);

	BOARD(outboard, i, j) = alivep (neighbor_count, BOARD (inboard, i, j));
}


    char*
sequential_game_of_life_parallel (char* outboard,
        char* inboard,
        const int nrows,
        const int ncols,
        const int gens_max,

        const int sector,
        int *status,
        pthread_mutex_t *mutex,
        pthread_cond_t *cv)
{
    /* HINT: in the parallel decomposition, LDA may not be equal to
       nrows! */

    int curgen, i, j;
    int row_start, col_start;
    int row_end, col_end;

    row_start = 0;
    row_end = nrows;
    /*
    //Splitting what quadrant we work on.
    if(sector == 0 || sector == 2){
    	row_start = 0;
    	row_end = nrows/2;
    }

    else{
    	row_start = nrows/2;
    	row_end = nrows;
    }*/

    if(sector == 0 || sector == 1){
    	col_start = 0;
    	col_end = ncols/2;
    }

    else{
    	col_start = ncols/2;
    	col_end = ncols;
    }



    for (curgen = 0; curgen < gens_max; curgen++)
    {

        /* HINT: you'll be parallelizing these loop(s) by doing a
           geometric decomposition of the output */
    	for (j = col_start; j < col_end; j++)
        {
            for (i = row_start; i < row_end; i++)
            {
            	update(i, j, outboard, inboard, nrows, ncols);

            }
        }
        //SWAP_BOARDS( outboard, inboard );
        //I don't like that weird do while wrapper.
        char *temp = outboard;
        outboard = inboard;
        inboard = temp;

        pthread_mutex_lock(mutex);
        *status = *status | (1 << sector);
        if(*status == 0b0101){
        	*status = 0;
        	pthread_cond_broadcast(cv);
        }
        else{
        	pthread_cond_wait(cv, mutex);
        }

        //Everyone finished working on their sector, can start the next sector
        pthread_mutex_unlock(mutex);

    }



    /*
     * We return the output board, so that we know which one contains
     * the final result (because we've been swapping boards around).
     * Just be careful when you free() the two boards, so that you don't
     * free the same one twice!!!
     */
    return inboard;
}

char*
sequential_game_of_life (char* outboard,
		char* inboard,
		const int nrows,
		const int ncols,
		const int gens_max)
{
	/* HINT: in the parallel decomposition, LDA may not be equal to
	   nrows! */

	int curgen, i, j;

	for (curgen = 0; curgen < gens_max; curgen++)
	{
		/* HINT: you'll be parallelizing these loop(s) by doing a
		   geometric decomposition of the output */
		for (j = 0; j < ncols; j++)
		{
			for (i = 0; i < nrows; i++)
			{
				update(i, j, outboard, inboard, nrows, ncols);

			}
		}
		//SWAP_BOARDS( outboard, inboard );
		//I don't like that weird do while wrapper.
		char *temp = outboard;
		outboard = inboard;
		inboard = temp;

	}
	/*
	 * We return the output board, so that we know which one contains
	 * the final result (because we've been swapping boards around).
	 * Just be careful when you free() the two boards, so that you don't
	 * free the same one twice!!!
	 */
	return inboard;
}



