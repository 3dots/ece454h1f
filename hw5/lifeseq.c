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

//#define MEMORY_BLOCKING_J
//#define MEMORY_BLOCKING_I
#define J_BLOCK_SIZE 4
//#define I_BLOCK_SIZE 32


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

/*This macro was used heavily in the final versions of the code, where
 * the overlap regions were first dealt with separately, before invoking the main loop.
 * Later versions use the next macro, given how in this version, we are doing unnecessary memory
 * access.
 */
#define COUNT_AND_BOARD(__inboard, __outboard, __neighbor_count, __i, __j, __inorth, __isouth, __jwest, __jeast) 	__neighbor_count = \
		BOARD (__inboard, __inorth, __jwest) + 	\
		BOARD (__inboard, __inorth, __j) +		\
		BOARD (__inboard, __inorth, __jeast) +	\
		BOARD (__inboard, __i, __jwest) +			\
		BOARD (__inboard, __i, __jeast) +			\
		BOARD (__inboard, __isouth, __jwest) +	\
		BOARD (__inboard, __isouth, __j) +		\
		BOARD (__inboard, __isouth, __jeast);		\
											\
		BOARD(__outboard, __i, __j) = alivep (__neighbor_count, BOARD (__inboard, __i, __j))

/*
 * This is actually the code that was "called" in the semi-final version's main loop.
 * It had the benefit of no if/else statements.
 */
#define COUNT_AND_BOARD_IJ(__inboard, __outboard, __neighbor_count, __i, __j) COUNT_AND_BOARD(__inboard, __outboard, __neighbor_count, __i, __j, __i-1, __i+1, __j-1, __j+1)

/*
 * After I realized that we are doing far too many memory accesses then necessary, and
 * we actually only really need a running sum, I implemented such a version.
 * I noticed a small improvement when it was unrolled once, so this macro was made for
 * the overlapping loops over i.
 */
#define I_CODE_WITH_J(__j, __jminus, __jplus) do { \
				mem_access[0] = 0;	\
			\
				mem_access[1] = BOARD (inboard, row_start-1, __jminus) +	\
								BOARD (inboard, row_start-1, __j) +	\
								BOARD (inboard, row_start-1, __jplus);	\
								\
								\
				mem_access[2] = BOARD (inboard, row_start, __jminus) +	\
								BOARD (inboard, row_start, __j) +	\
								BOARD (inboard, row_start, __jplus);	\
								\
				cent = 0;	\
				\
				neighbor_count = mem_access[1] + mem_access[2];	\
				\
				for(i = row_start; i < row_end - 1; i+=2)	\
				{	\
					neighbor_count += cent;	\
					\
					cent = BOARD (inboard, i, __j);	\
					\
					neighbor_count = neighbor_count - mem_access[0] - cent;	\
					\
					mem_access[0] = mem_access[1];	\
					\
					mem_access[1] = mem_access[2];	\
					\
					mem_access[2] = BOARD (inboard, i+1, __jminus) +	\
									BOARD (inboard, i+1, __j) +		\
									BOARD (inboard, i+1, __jplus);	\
									\
					neighbor_count += mem_access[2];	\
					\
					BOARD(outboard, i, __j) = alivep (neighbor_count, cent);	\
					\
					\
					neighbor_count += cent;	\
					\
					cent = BOARD (inboard, i+1, __j);	\
					\
					neighbor_count = neighbor_count - mem_access[0] - cent;	\
					\
					mem_access[0] = mem_access[1];	\
					\
					mem_access[1] = mem_access[2];	\
					\
					mem_access[2] = BOARD (inboard, i+2, __jminus) +	\
									BOARD (inboard, i+2, __j) +	\
									BOARD (inboard, i+2, __jplus);	\
									\
					neighbor_count += mem_access[2];	\
					\
					BOARD(outboard, i+1, __j) = alivep (neighbor_count, cent);	\
					\
					\
					\
				}	\
				\
				neighbor_count += cent;	\
				\
				cent = BOARD (inboard, i, __j);	\
				\
				neighbor_count = neighbor_count - mem_access[0] - cent;	\
				\
				mem_access[0] = mem_access[1];	\
				\
				mem_access[1] = mem_access[2];	\
				\
				mem_access[2] = BOARD (inboard, i+1, __jminus) +	\
								BOARD (inboard, i+1, __j) +	\
								BOARD (inboard, i+1, __jplus);	\
								\
				neighbor_count += mem_access[2];	\
				\
				BOARD(outboard, i, __j) = alivep (neighbor_count, cent);	\
				\
			} while(0)

/*
 * A similiar code for looping solely over j, thing is, this slows us down, so it wasnt used.
 */
#define J_CODE_WITH_I(__i, __iminus, __iplus) do { \
				mem_access[0] = 0;	\
			\
				mem_access[1] = BOARD (inboard, __iminus, col_start-1) +	\
								BOARD (inboard, __i, col_start-1) +	\
								BOARD (inboard, __iplus, col_start-1);	\
								\
								\
				mem_access[2] = BOARD (inboard, __iminus, col_start) +	\
								BOARD (inboard, __i, col_start) +	\
								BOARD (inboard, __iplus, col_start);	\
								\
				cent = 0;	\
				\
				neighbor_count = mem_access[1] + mem_access[2];	\
				\
				for(j = col_start; j < col_end; j++)	\
				{	\
					neighbor_count += cent;	\
					\
					cent = BOARD (inboard, __i, j);	\
					\
					neighbor_count = neighbor_count - mem_access[0] - cent;	\
					\
					mem_access[0] = mem_access[1];	\
					\
					mem_access[1] = mem_access[2];	\
					\
					mem_access[2] = BOARD (inboard, __iminus, j+1) +	\
									BOARD (inboard, __i, j+1) +		\
									BOARD (inboard, __iplus, j+1);	\
									\
					neighbor_count += mem_access[2];	\
					\
					BOARD(outboard, __i, j) = alivep (neighbor_count, cent);	\
				}	\
				\
			} while(0)

/*
 * This is used in the original code version. It changed the modulus function call
 * To if/else statements. Which were later removed in other versions
 */
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



int min(int x, int y){
	if(x > y){
		return y;
	}
	else
		return x;
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

    //Splitting what quadrant we work on.
    if(sector == 0 || sector == 2){
    	row_start = 1;
    	row_end = nrows/2;
    }

    else{
    	row_start = nrows/2;
    	row_end = nrows - 1;
    }

    if(sector == 0 || sector == 1){
    	col_start = 1;
    	col_end = ncols/2;
    }

    else{
    	col_start = ncols/2;
    	col_end = ncols - 1;
    }

    const int LDA = nrows;
    char mem_access[3];
    char cent;

    for (curgen = 0; curgen < gens_max; curgen++)
    {
		char neighbor_count;

		//The overlapping sections
		if (sector == 0){
			//j == 0
			//i == 0
			COUNT_AND_BOARD(inboard, outboard, neighbor_count, 0, 0, nrows - 1, 1, ncols - 1, 1);

			//j == 0
			//i == 1 -> i == nrows/2 - 1

            I_CODE_WITH_J(0, ncols - 1, 1);

            //j == 1 -> j == ncols/2 - 1
            //i == 0
            //J_CODE_WITH_I(0, nrows - 1, 1);
            for (j = col_start; j < col_end; j++)
            {
            	COUNT_AND_BOARD(inboard, outboard, neighbor_count, 0, j, nrows - 1, 1, j - 1, j + 1);
            }
		}
		else if(sector == 1){
			//j == 0
			//i == nrows - 1
			COUNT_AND_BOARD(inboard, outboard, neighbor_count, nrows - 1, 0, nrows - 2, 0, ncols - 1, 1);

			//j == 0
			//i == nrows/2 -> i == nrows - 2
			I_CODE_WITH_J(0, ncols - 1, 1);


            //j == 1 -> j == ncols/2 - 1
            //i == nrows - 1
			//J_CODE_WITH_I(nrows - 1, nrows - 2, 0);
            for (j = col_start; j < col_end; j++)
            {
            	COUNT_AND_BOARD(inboard, outboard, neighbor_count, nrows - 1, j, nrows - 2, 0, j - 1, j + 1);
            }
		}
		else if(sector == 2){
			//j == ncols - 1
			//i == 0
			COUNT_AND_BOARD(inboard, outboard, neighbor_count, 0, ncols - 1, nrows - 1, 1, ncols - 2, 0);

			//j == ncols - 1
			//i == 1 -> i == nrows/2 - 1
			I_CODE_WITH_J(ncols - 1, ncols - 2, 0);


            //j == ncols/2 -> j == ncols - 2
            //i == 0
			//J_CODE_WITH_I(0, nrows - 1, 1);
            for (j = col_start; j < col_end; j++)
            {
            	COUNT_AND_BOARD(inboard, outboard, neighbor_count, 0, j, nrows - 1, 1, j - 1, j + 1);
            }
		}
		else{
			//j == ncols - 1
			//i == nrows - 1
			COUNT_AND_BOARD(inboard, outboard, neighbor_count, nrows - 1, ncols - 1, nrows - 2, 0, ncols - 2, 0);

			//j == ncols - 1
			//i == nrows/2 -> i == nrows - 2
			I_CODE_WITH_J(ncols - 1, ncols - 2, 0);

            //j == ncols/2 -> j == ncols - 2
            //i == nrows - 1
			//J_CODE_WITH_I(nrows - 1, nrows - 2, 0);
            for (j = col_start; j < col_end; j++)
            {
            	COUNT_AND_BOARD(inboard, outboard, neighbor_count, nrows - 1, j, nrows - 2, 0, j - 1, j + 1);
            }
		}

		//Main code part, no if/else branching
		//Unroleld once in the i dimension, as well as a block on j of size 4.

		int jj;
    	for (jj = col_start; jj < col_end; jj+= J_BLOCK_SIZE)
        {
			for (j = jj; j < min(jj + J_BLOCK_SIZE, col_end); j++)
			{

				//Initializing sum
				mem_access[0] = 0;

				mem_access[1] = BOARD (inboard, row_start-1, j-1) +
								BOARD (inboard, row_start-1, j) +
								BOARD (inboard, row_start-1, j+1);


				mem_access[2] = BOARD (inboard, row_start, j-1) +
								BOARD (inboard, row_start, j) +
								BOARD (inboard, row_start, j+1);

				cent = 0;

				neighbor_count = mem_access[1] + mem_access[2];

				for(i = row_start; i < row_end - 1; i+=2)
				{	//1
					neighbor_count += cent;

					cent = BOARD (inboard, i, j);

					neighbor_count = neighbor_count - mem_access[0] - cent;

					mem_access[0] = mem_access[1];

					mem_access[1] = mem_access[2];

					mem_access[2] = BOARD (inboard, i+1, j-1) +
									BOARD (inboard, i+1, j) +
									BOARD (inboard, i+1, j+1);

					neighbor_count += mem_access[2];

					BOARD(outboard, i, j) = alivep (neighbor_count, cent);

					//2
					neighbor_count += cent;

					cent = BOARD (inboard, i+1, j);

					neighbor_count = neighbor_count - mem_access[0] - cent;

					mem_access[0] = mem_access[1];

					mem_access[1] = mem_access[2];

					mem_access[2] = BOARD (inboard, i+2, j-1) +
									BOARD (inboard, i+2, j) +
									BOARD (inboard, i+2, j+1);

					neighbor_count += mem_access[2];

					BOARD(outboard, i+1, j) = alivep (neighbor_count, cent);

					//COUNT_AND_BOARD_IJ(inboard, outboard, neighbor_count, i, j);

				}

				neighbor_count += cent;

				cent = BOARD (inboard, i, j);

				neighbor_count = neighbor_count - mem_access[0] - cent;

				mem_access[0] = mem_access[1];

				mem_access[1] = mem_access[2];

				mem_access[2] = BOARD (inboard, i+1, j-1) +
								BOARD (inboard, i+1, j) +
								BOARD (inboard, i+1, j+1);

				neighbor_count += mem_access[2];

				BOARD(outboard, i, j) = alivep (neighbor_count, cent);

			}
        }



        //SWAP_BOARDS( outboard, inboard );
        //I don't like that weird do while wrapper.
        char *temp = outboard;
        outboard = inboard;
        inboard = temp;

        pthread_mutex_lock(mutex);
        *status = *status | (1 << sector);
        if(*status == 0b1111){
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



