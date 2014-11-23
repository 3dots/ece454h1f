/*****************************************************************************
 * life.c
 * The original sequential implementation resides here.
 * Do not modify this file, but you are encouraged to borrow from it
 ****************************************************************************/
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



