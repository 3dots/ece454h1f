/*
 * 
 * Original implementation:
 * |padding|Prolgue Header|Prologue Footer||Header|Payload|Footer||Header|Payload|Footer|...|Header|Payload|Footer||Epilogue Header|
 * 
 *
 * New implementation:
 * Occupied block:
 * |Header|Payload|Footer|
 * 
 * Free blocks:
 * |Header|Ptr to header of next free|Ptr to header of prev free|Payload|Footer|
 * 
 * Prologues and Epiloges kept, prologue is really head.
 * 
 * Changed CHUNKSIZE from weird bit shift obfuscation of 128, to 2Words for Head+Footer + 2Words for addresses + minimum payload
 * of 2words = 6words = 48
 * 
 * I am dissapoint, dear marker. For all the careful treatment of void pointers, where you cast them as (char *) before
 * adding byte counts; you disregard your own policy in the mm_init function, and treat the void * heap_listp as if it is a char *.
 * Thankfully, the compiler is smart and understood what you meant to do. But to be safe Ill change it to follow your overall policy.
 * 
 * 
 * 
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "Teamname",
    /* First member's full name */
    "Viktor Riabtsev",
    /* First member's email address */
    "viktor.riabtsev@mail.utoronto.ca",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/*************************************************************************
 * Basic Constants and Macros
 * You are not required to use these macros but may find them helpful.
*************************************************************************/
#define WSIZE       sizeof(void *)            /* word size (bytes) */
#define DSIZE       (2 * WSIZE)            /* doubleword size (bytes) */
#define CHUNKSIZE   (48)      /* initial heap size (bytes), FREAKING 128, COULDNT PUT IT SIMPLY? */

#define MAX(x,y) ((x) > (y)?(x) :(y))

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc) ((size) | (alloc))

/* Read and write a word at address p */
#define GET(p)          (*(uintptr_t *)(p))
#define PUT(p,val)      (*(uintptr_t *)(p) = (val))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p)     (GET(p) & ~(DSIZE - 1))
#define GET_ALLOC(p)    (GET(p) & 0x1)

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp)        ((char *)(bp) - 3*WSIZE) //links accounted
#define FTRP(bp)        ((char *)(bp) + GET_SIZE(HDRP(bp)) - 4*WSIZE)

/* Given block ptr bp, compute address of next and previous blocks DATA PLACES */
#define NEXT_BLKP(bp) ((char *)(bp) - 2*WSIZE)
#define PREV_BLKP(bp) ((char *)(bp) - 1*WSIZE)

/* Given block ptr bp, get the actual addresses of next and prev */
#define GET_NEXT_BLKP(bp) (GET(NEXT_BLKP(bp)))
#define GET_PREV_BLKP(bp) (GET(PREV_BLKP(bp)))

/* Given block ptr bp, get the footer and header of the physically left block*/
#define GET_PHYS_LEFT_AS_FREEP(bp) ( (char *)(bp) - GET_SIZE(HDRP(bp) - WSIZE) )
#define GET_PHYS_RIGHT_AS_FREEP(bp) ( FTRP(bp) + 4*WSIZE )

#define DEBUG_MY_MALLOC 0

#define BUCKET_LIST_COUNT 20
#define HEAD_ADDR(i) ( (char *)(heap_listp) + i * WSIZE ) // 0 ... BUCKET_LIST_COUNT - 1

void* heap_listp = NULL;

size_t maxBucketSize = 1;


/**********************************************************
 * mm_init
 * Initialize the heap, including "allocation" of the
 * prologue and epilogue
 **********************************************************/
 int mm_init(void)
 {
   if ((heap_listp = mem_sbrk((4 + 2*BUCKET_LIST_COUNT)*WSIZE)) == (void *)-1)
         return -1;
     PUT((char *)(heap_listp), 0);                         		// alignment padding
     
     int i;
     for(i=1; i <= BUCKET_LIST_COUNT; i++){
    	 PUT((char *)(heap_listp) + i * WSIZE, NULL); //next
    	 PUT((char *)(heap_listp) + (i+1) * WSIZE, NULL); //prev Actual head points after
     }
     PUT((char *)(heap_listp) + (2 + 2*BUCKET_LIST_COUNT - 1) * WSIZE, PACK(2*DSIZE, 1));   // prologue footer
     PUT((char *)(heap_listp) + (3 + 2*BUCKET_LIST_COUNT - 1) * WSIZE, PACK(2*DSIZE, 1));   // prologue footer
     PUT(((char *)(heap_listp) + (4 + 2*BUCKET_LIST_COUNT - 1) * WSIZE), PACK(0, 1));   		// epilogue header
     heap_listp = (char *)(heap_listp) + 3*WSIZE;
     
     if(DEBUG_MY_MALLOC) {
     printf("\nmm_init ptr to first byte of alloc memory. %d\n", mem_heap_lo());
     printf("\nmm_init ptr to last byte of alloc memory. %d\n", mem_heap_hi());
     printf("\nmm_init mem_heap size:. %d\n", mem_heapsize());
     printf("\nmm_init success.\n");}
     
     for(i=BUCKET_LIST_COUNT-1; i>0; i--)
    	 maxBucketSize *=2;
     maxBucketSize = 32 + 16*maxBucketSize;
     
     return 0;
 }

/**********************************************************
 * coalesce
 * Covers the 4 cases discussed in the text:
 * - both neighbours are allocated
 * - the next block is available for coalescing
 * - the previous block is available for coalescing
 * - both neighbours are available for coalescing
 * 
 * This function coalesces, then also picks what size list 
 * to put into.
 **********************************************************/
void *coalesce(void *bp, int addToFreeLists) 
{
	
	/*printf("BP: %d\n", bp); 
	printf("Header: %d\n", HDRP(bp)); 
	printf("Footer: %d\n", FTRP(bp)); 
	printf("Size: %d\n", GET_SIZE(HDRP(bp)) );
	printf("prev_alloc: %d\n", GET_ALLOC(HDRP(bp) - WSIZE) );
	printf("next_alloc: %d\n", GET_ALLOC(FTRP(bp) + WSIZE) ); */
    size_t prev_alloc = GET_ALLOC(HDRP(bp) - WSIZE); 	//accessing footer of physcailly left location.
    //printf("()");
    size_t next_alloc = GET_ALLOC(FTRP(bp) + WSIZE);	//accessing header of physcailly right location.
    //printf("()");
    size_t size = GET_SIZE(HDRP(bp));
    //printf("coalesceStart %d-%d.\n", prev_alloc, next_alloc);;
    //mm_printMem();

    if (prev_alloc && next_alloc) {       /* Case 1 */
    	
    	if(addToFreeLists){
			int index = head_index(size);
			PUT(NEXT_BLKP(bp), GET_NEXT_BLKP(HEAD_ADDR(index)));	// Next element is the start of the linked list. (Or NULL if it was empy)
			PUT(PREV_BLKP(bp), HEAD_ADDR(index));					// Alwayes placing it at the front
			
			if(GET_NEXT_BLKP(HEAD_ADDR(index)) != NULL){
				PUT(PREV_BLKP(GET_NEXT_BLKP(HEAD_ADDR(index))), bp);
			}
			
			PUT(NEXT_BLKP(HEAD_ADDR(index)), bp);					//Make the Head point to this freed block
			
			
    	}
    	if(DEBUG_MY_MALLOC) {printf("coalesce. bpIn = %d\n", bp);}
    	return bp;
    }

    else if (prev_alloc && !next_alloc) { /* Case 2 */
        size += GET_SIZE(FTRP(bp) + WSIZE);
        void *rafp = GET_PHYS_RIGHT_AS_FREEP(bp); //right as free 
        
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0)); 
        
        // For the right block.
        /* The previous free block's pointer to the next is the next of the current one. */
        PUT(NEXT_BLKP(GET_PREV_BLKP(rafp)), GET_NEXT_BLKP(rafp));
        /* The next free block's pointer to the previous is the previous of the current one.*/
        if(GET_NEXT_BLKP(rafp) != NULL){
        	PUT(PREV_BLKP(GET_NEXT_BLKP(rafp)), GET_PREV_BLKP(rafp));
        }
        
        if(addToFreeLists){
			/* The free block on the physical right has been removed from the doubly linked list.
			 * i.e. made to be skipped over. */
			int index = head_index(size);
			PUT(NEXT_BLKP(bp), GET_NEXT_BLKP(HEAD_ADDR(index)));	// Next element is the start of the linked list. (Or NULL if it was empy)
			PUT(PREV_BLKP(bp), HEAD_ADDR(index));					// Alwayes placing it at the front
			
			if(GET_NEXT_BLKP(HEAD_ADDR(index)) != NULL){
				PUT(PREV_BLKP(GET_NEXT_BLKP(HEAD_ADDR(index))), bp);
			}
			
			PUT(NEXT_BLKP(HEAD_ADDR(index)), bp);					//Make the Head point to this freed block
        }
        if(DEBUG_MY_MALLOC) {printf("coalesce. bpIn = %d bpRight = %d -> bp = %d\n", bp, rafp, bp);}
        return (bp);
    }

    else if (!prev_alloc && next_alloc) { /* Case 3 */
    	
    	
    	
        size += GET_SIZE(HDRP(bp) - WSIZE);
        
        //printf("()");
        void *lafp = GET_PHYS_LEFT_AS_FREEP(bp); //left as free 
        
        //printf("()");
        PUT(FTRP(bp), PACK(size, 0));
        
        PUT(HDRP(lafp), PACK(size, 0));
        
        //printf("lafp=%d prevOfLafp=%d", lafp, GET_PREV_BLKP(lafp), GET_NEXT_BLKP(lafp));
        
        
        //printf("()");
        // For the left block.
		/* The previous free block's pointer to the next is the next of the current one. */
		PUT(NEXT_BLKP(GET_PREV_BLKP(lafp)), GET_NEXT_BLKP(lafp));
		

		/* The next free block's pointer to the previous is the previous of the current one.*/
		if(GET_NEXT_BLKP(lafp) != NULL){
			PUT(PREV_BLKP(GET_NEXT_BLKP(lafp)), GET_PREV_BLKP(lafp));
		}
		/* The free block on the physical left has been removed from the doubly linked list.
		 * i.e. made to be skipped over. */
		
		if(addToFreeLists){
		int index = head_index(size);
			PUT(NEXT_BLKP(lafp), GET_NEXT_BLKP(HEAD_ADDR(index)));	// Next element is the start of the linked list. (Or NULL if it was empy)
			PUT(PREV_BLKP(lafp), HEAD_ADDR(index));					// Alwayes placing it at the front
			
			if(GET_NEXT_BLKP(HEAD_ADDR(index)) != NULL){
				PUT(PREV_BLKP(GET_NEXT_BLKP(HEAD_ADDR(index))), lafp);
			}
			
			PUT(NEXT_BLKP(HEAD_ADDR(index)), lafp);					//Make the Head point to this freed block
		}
           
		if(DEBUG_MY_MALLOC) {printf("coalesce. bpLeft = %d bpIn = %d\n  -> bp = %d\n", lafp, bp, lafp);}
        return (lafp);
    }

    else {            /* Case 4 */
    	
    	//printf("()WHAT");
        size += GET_SIZE(HDRP(bp) - WSIZE)  +
        		GET_SIZE(FTRP(bp) + WSIZE);
        
        void *rafp = GET_PHYS_RIGHT_AS_FREEP(bp); //right as free 
        void *lafp = GET_PHYS_LEFT_AS_FREEP(bp); //left as free
        
        PUT(HDRP(lafp), PACK(size,0));              
        PUT(FTRP(rafp), PACK(size,0));
        
        // For the left block.
		/* The previous free block's pointer to the next is the next of the current one. */
		PUT(NEXT_BLKP(GET_PREV_BLKP(lafp)), GET_NEXT_BLKP(lafp));
		/* The next free block's pointer to the previous is the previous of the current one.*/
		if(GET_NEXT_BLKP(lafp) != NULL){
			PUT(PREV_BLKP(GET_NEXT_BLKP(lafp)), GET_PREV_BLKP(lafp));
		}
		
		/* The free block on the physical left has been removed from the doubly linked list.
		 * i.e. made to be skipped over. */
	
		
		// For the right block.
		/* The previous free block's pointer to the next is the next of the current one. */
		PUT(NEXT_BLKP(GET_PREV_BLKP(rafp)), GET_NEXT_BLKP(rafp));
		/* The next free block's pointer to the previous is the previous of the current one.*/
		if(GET_NEXT_BLKP(rafp) != NULL){
			PUT(PREV_BLKP(GET_NEXT_BLKP(rafp)), GET_PREV_BLKP(rafp));
		}
		
		/* The free block on the physical right has been removed from the doubly linked list.
		 * i.e. made to be skipped over. */  
		
		if(addToFreeLists){
			int index = head_index(size);
			PUT(NEXT_BLKP(lafp), GET_NEXT_BLKP(HEAD_ADDR(index)));	// Next element is the start of the linked list. (Or NULL if it was empy)
			PUT(PREV_BLKP(lafp), HEAD_ADDR(index));					// Alwayes placing it at the front
			
			if(GET_NEXT_BLKP(HEAD_ADDR(index)) != NULL){
				PUT(PREV_BLKP(GET_NEXT_BLKP(HEAD_ADDR(index))), lafp);
			}
			
			PUT(NEXT_BLKP(HEAD_ADDR(index)), lafp);					//Make the Head point to this freed block
		}
		if(DEBUG_MY_MALLOC) {printf("coalesce. bpLeft = %d bpIn = %d bpRight = %d -> bp = %d\n", lafp, bp, rafp, lafp);}
        return (lafp);
    }
}

/**********************************************************
 * extend_heap
 * Extend the heap by "words" words, maintaining alignment
 * requirements of course. Free the former epilogue block
 * and reallocate its new header
 **********************************************************/
void *extend_heap(size_t words)
{
    char *bp;
    size_t size;
    //printf("%d\n", sizeof(void *));
    //printf("extend_heap: words=%d\n", words);

    /* Allocate an even number of words to maintain alignments */
    size = (words % 2) ? (words+1) * WSIZE : words * WSIZE;
    if ( (bp = mem_sbrk(size)) == (void *)-1 ){
    	mm_printMem();
    	return NULL;
    }
        
    
    //printf("ptr to last byte of alloc memory. %d\n", mem_heap_hi());
    //printf("extend_heap: size=%d\n", size);
    //printf("extend_heap: bp=%d bpRounded=%d\n", bp, 16*((long)bp/16));
    //printf("extend_heap: heap_listp=%d heap_listpRounded=%d\n", heap_listp, 16*((long)heap_listp/16));
    
    /* This makes bp point to the actual payload.
     * This is so we can keep using the provided, albeit modfied, macros.
     */
    bp = (char *)(bp) + 2*WSIZE; 
    

    /* Initialize free block header/footer and the epilogue header */
    PUT(HDRP(bp), PACK(size, 0));             		// kill previous epilogue/free block header
    PUT(FTRP(bp), PACK(size, 0));              		// free block footer
    
    // Select which bucket list:
    int index = head_index(size);
    //printf("%d\n", index);
    PUT(NEXT_BLKP(bp), GET_NEXT_BLKP(HEAD_ADDR(index)));	// Next element is the start of the linked list. (Or NULL if it was empy)
    //printf("hmm\n");
    PUT(PREV_BLKP(bp), HEAD_ADDR(index));					// Alwayes placing it at the front
    //printf("hmm\n");
    
    //printf("hmm\n");
    
    if(GET_NEXT_BLKP(HEAD_ADDR(index)) != NULL){
    	PUT(PREV_BLKP(GET_NEXT_BLKP(HEAD_ADDR(index))), bp);	//Make the next after the head point backward to the new block.
    }
    //printf("Header address, Footer Address %d %d\n", HDRP(bp), FTRP(bp));
    //printf("hmm\n");
    
    PUT(NEXT_BLKP(HEAD_ADDR(index)), bp);					//Make the Head point to this newly allocated block
    //printf("%d\n", GET_SIZE(HDRP(bp)));
    
    
    //printf("extend_heap: epilogue address=%d endofMem_sbrk(last word) = %d\n", FTRP(bp) + WSIZE, (char *)(bp) - 2*WSIZE + size - WSIZE);
    PUT(FTRP(bp) + WSIZE, PACK(0, 1));     		// new epilogue header

    /* DO NOT COALESCE if the previous block was free. There is no point. */
    if(DEBUG_MY_MALLOC) {printf("extend_heap. bp = %d\n", bp);}
   
    return (bp);
}


/**********************************************************
 * find_fit
 * Traverse the heap searching for a block to fit asize
 * Return NULL if no free blocks can handle that size
 * Assumed that asize is aligned
 **********************************************************/
void * find_fit(size_t asize)
{
    void *bp;
    /* The pointers point to actual blocks like before, except we 
     * check for next to equal NULL
     */
    int index;
    for(index = head_index(asize); index < BUCKET_LIST_COUNT ; index++){//Up to last or not?
        for (bp = GET_NEXT_BLKP(HEAD_ADDR(index)); bp != NULL; bp = GET_NEXT_BLKP(bp))
        {
        	if(DEBUG_MY_MALLOC) {
        	//printf("fit:%d\n", bp);
        	if(GET_ALLOC(HDRP(bp)) == 1){
        		printf("%d\n", bp);
        		mm_printMem();
        		printf("%d\n", bp);
        	}}
            if ( asize <= GET_SIZE(HDRP(bp)) ) //No need to check that not allocated REALLLY?
            {
            	if(DEBUG_MY_MALLOC) {printf("find_fit success. bp = %d (bp - heap_listp) = %d\n", bp, bp - heap_listp);}
            	/*if(GET_ALLOC(HDRP(bp))){
            		mm_printMem();
            		printf("%d\n", bp);
            	}*/
                
            	//If you went into higher indicies, you want to split the block into pieces
            	if(GET_SIZE(HDRP(bp)) - asize >= 48){
            		 
            		
            		//Remove from current list            		
					/* The previous free block's pointer to the next is the next of the current one. */
					PUT(NEXT_BLKP(GET_PREV_BLKP(bp)), GET_NEXT_BLKP(bp));
					
					/* The next free block's pointer to the previous is the previous of the current one.*/
					if(GET_NEXT_BLKP(bp) != NULL){ //Checking if deleting right most list element
						PUT(PREV_BLKP(GET_NEXT_BLKP(bp)), GET_PREV_BLKP(bp));
					}
					
					//Can split
					void *bp_right = (char *)(bp) + asize;
					size_t rsize = GET_SIZE(HDRP(bp)) - asize;
					
					//PUT(HDRP(bp_right), PACK(rsize, 0));             		
					//PUT(FTRP(bp_right), PACK(rsize, 0));
            		
            		PUT(HDRP(bp), PACK(asize, 0));             		
            		PUT(FTRP(bp), PACK(asize, 0));
            		
            		 // Select which bucket list:
					index = head_index(asize);
					//printf("indexOfNew = %d\n", index);
					
					PUT(NEXT_BLKP(bp), GET_NEXT_BLKP(HEAD_ADDR(index))); // Next element is the start of the linked list. (Or NULL if it was empy)					
					PUT(PREV_BLKP(bp), HEAD_ADDR(index));		// Alwayes placing it at the front
									
					if(GET_NEXT_BLKP(HEAD_ADDR(index)) != NULL){
						PUT(PREV_BLKP(GET_NEXT_BLKP(HEAD_ADDR(index))), bp);	//Make the next after the head point backward to the new block.
					}

					PUT(NEXT_BLKP(HEAD_ADDR(index)), bp);					//Make the Head point to this newly allocated block
					
					//printf("maxbucketsiz =%d\n", maxBucketSize);
					//printf("rsize =%d\n", rsize);
					//printf("rsize - maxBucketSize = %d\n", rsize - maxBucketSize - 48);

					
					// Trickle down the bigass chunk.
					//At this point, our pristine desired chunk has been split off
					//What we want now, is to make sure that the remaining chunk is less than then the maximum bucket size
					//if not, we need to keep splitting off highest size chunks until nothing is left as 
					while((long)(rsize - maxBucketSize - 48)  >= 0){
						void *bp_left = bp_right;
						bp_right = (char *)(bp_right) + maxBucketSize;
						size_t lsize = maxBucketSize;
						
						rsize = rsize - maxBucketSize;
								
						PUT(HDRP(bp_left), PACK(lsize, 0));             		
						PUT(FTRP(bp_left), PACK(lsize, 0));
						
						index = head_index(lsize);
						//printf("indexOfLeftOvers = %d\n", index);    BUCKET_LIST_COUNT
						
						PUT(NEXT_BLKP(bp_left), GET_NEXT_BLKP(HEAD_ADDR(index))); // Next element is the start of the linked list. (Or NULL if it was empy)					
						PUT(PREV_BLKP(bp_left), HEAD_ADDR(index));		// Alwayes placing it at the front
										
						if(GET_NEXT_BLKP(HEAD_ADDR(index)) != NULL){
							PUT(PREV_BLKP(GET_NEXT_BLKP(HEAD_ADDR(index))), bp_left);	//Make the next after the head point backward to the new block.
						}

						PUT(NEXT_BLKP(HEAD_ADDR(index)), bp_left);					//Make the Head point to this newly allocated block
					
						
					}
					
					
					PUT(HDRP(bp_right), PACK(rsize, 0));             		
					PUT(FTRP(bp_right), PACK(rsize, 0));
					
					index = head_index(rsize);
					//printf("indexOfLeftOvers = %d\n", index);    BUCKET_LIST_COUNT
					
					PUT(NEXT_BLKP(bp_right), GET_NEXT_BLKP(HEAD_ADDR(index))); // Next element is the start of the linked list. (Or NULL if it was empy)					
					PUT(PREV_BLKP(bp_right), HEAD_ADDR(index));		// Alwayes placing it at the front
									
					if(GET_NEXT_BLKP(HEAD_ADDR(index)) != NULL){
						PUT(PREV_BLKP(GET_NEXT_BLKP(HEAD_ADDR(index))), bp_right);	//Make the next after the head point backward to the new block.
					}

					PUT(NEXT_BLKP(HEAD_ADDR(index)), bp_right);					//Make the Head point to this newly allocated block

					
					return bp;
            		
            		
            	}		
            	else{
            		return bp;
            	}

            	
            }
        }
    }
    

        if(DEBUG_MY_MALLOC) {printf("find_fit fail.\n");}
    return NULL;
}

/**********************************************************
 * place
 * Mark the block as allocated
 * Had to change, in order to remove from doubly linked list
 **********************************************************/
void place(void* bp, size_t asize)
{
	
	
	//printf("BP: %d", bp);
	
	//mm_printMem();
	/* Get the current block size */
	size_t bsize = GET_SIZE(HDRP(bp));
	
	PUT(HDRP(bp), PACK(bsize, 1));
	
	PUT(FTRP(bp), PACK(bsize, 1));
	
	//mm_printMem();
	// Removing from linked list
	/* The previous free block's pointer to the next is the next of the current one. */
	PUT(NEXT_BLKP(GET_PREV_BLKP(bp)), GET_NEXT_BLKP(bp));
	
	/* The next free block's pointer to the previous is the previous of the current one.*/
	if(GET_NEXT_BLKP(bp) != NULL){ //Checking if deleting right most list element
		PUT(PREV_BLKP(GET_NEXT_BLKP(bp)), GET_PREV_BLKP(bp));
	}
	/* The free block on the physical right has been removed from the doubly linked list.
	 * i.e. made to be skipped over. */
	
	/*if(bp - heap_listp == 854656){
		mm_printMem();
	}*/
	
	if(DEBUG_MY_MALLOC) {printf("place. bp = %d (bp - heap_listp) = %d\n", bp, bp - heap_listp);}
}

/**********************************************************
 * mm_free
 * Free the block and coalesce with neighbouring blocks
 **********************************************************/
void mm_free(void *bp)
{
    if(bp == NULL){
      return;
    }
    
    bp = (char *) bp + 2*WSIZE; //Making sure, the relational macros work
    
    size_t size = GET_SIZE(HDRP(bp));
    PUT(HDRP(bp), PACK(size,0));
    PUT(FTRP(bp), PACK(size,0));
    
    
    //Coalesce takes care of linked list addition
    

    if(DEBUG_MY_MALLOC) {printf("free. bp = %d (bp - heap_listp) = %d\n", bp, bp - heap_listp);}
    coalesce(bp, 1);
}


/**********************************************************
 * mm_malloc
 * Allocate a block of size bytes.
 * The type of search is determined by find_fit
 * The decision of splitting the block, or not is determined
 *   in place(..)
 * If no block satisfies the request, the heap is extended
 **********************************************************/
void *mm_malloc(size_t size)
{
    size_t asize; /* adjusted block size */
    size_t extendsize; /* amount to extend heap if no fit */
    char * bp;

    /* Ignore spurious requests */
    if (size == 0)
        return NULL;
    
    //printf("size = %d", size);
    
    /* Adjust block size to include overhead and alignment reqs. */
    if (size <= DSIZE)
        asize = 2 * DSIZE;
    else
        asize = DSIZE * ((size + (DSIZE) + (DSIZE-1))/ DSIZE);
    
    
    /* Search the free list for a fit */
    if ((bp = find_fit(asize)) != NULL) {
        place(bp, asize);
        
        bp = (char *) bp - 2*WSIZE;
        if(DEBUG_MY_MALLOC) {printf("malloc. bp = %d\n", bp);}
        return bp;
    }

    /* No fit found. Get more memory and place the block */
    extendsize = MAX(asize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize/WSIZE)) == NULL)
        return NULL;
    place(bp, asize);
    
    bp = (char *) bp - 2*WSIZE;
    if(DEBUG_MY_MALLOC) {printf("malloc. bp = %d\n", bp);}
    return bp;

}

/**********************************************************
 * mm_realloc
 * Implemented simply in terms of mm_malloc and mm_free
 *********************************************************/
void *mm_realloc(void *ptr, size_t size)
{
	/* If size == 0 then this is just free, and we return NULL. */
	    if(size == 0){
	      mm_free(ptr);
	      return NULL;
	    }
	    /* If oldptr is NULL, then this is just malloc. */
	    if (ptr == NULL)
	      return (mm_malloc(size));

	    void *oldptr = ptr;
	    void *newptr;
	    size_t copySize;

	    newptr = mm_malloc(size);
	    if (newptr == NULL)
	      return NULL;

	    /* Copy the old data. */
	    copySize = GET_SIZE(HDRP((char *)(oldptr) + 2*WSIZE));
	    if (size < copySize)
	      copySize = size;
	    memcpy(newptr, oldptr, copySize);
	    mm_free(oldptr);
	    return newptr;
    

    

}

/**********************************************************
 * mm_check
 * Check the consistency of the memory heap
 * Return nonzero if the heap is consistant.
 *********************************************************/
int mm_check(void){
  return 1;
}

/**********************************************************
 * mm_printMem
 * Print the state of the memory so far.
 *********************************************************/

void mm_printMem(void){
	void *bpHeader = (char *)(heap_listp) + (4 + 2*BUCKET_LIST_COUNT - 1) * WSIZE - 3*WSIZE; //First header
	void *bpFooter = NULL;
	printf("\n");
	//Physically sequental
	for(bpHeader; GET_SIZE(bpHeader) > 0; bpHeader = (char *)(bpHeader) + GET_SIZE(bpHeader)){
		printf("bp\tHdr_Alloc\tHdr_Size\tNext\tPrev\tFtr_Alloc\tFtr_Size\n");
		if(GET_ALLOC(bpHeader)== 0){
			printf("%d\t", bpHeader + 3*WSIZE);
		}
		else{
			printf("%d\t", bpHeader + WSIZE);
		}
		printf("%d\t%d\t",GET_ALLOC(bpHeader), GET_SIZE(bpHeader));
		
		if(GET_ALLOC(bpHeader)== 0){
			printf("%d\t%d\t", GET_NEXT_BLKP(bpHeader+3*WSIZE), GET_PREV_BLKP(bpHeader+3*WSIZE));
		}
		else{
			printf("N/A\tN/A\t");
		}
		
		bpFooter = (char *)(bpHeader) + GET_SIZE(bpHeader) - WSIZE;
		printf("%d\t%d\n", GET_ALLOC(bpFooter), GET_SIZE(bpFooter));
	}
}

/**********************************************************
 * head_index
 * The index of the list to put this memory chunk into
 **********************************************************/
int head_index(size_t size){
	int index = 0;
	for(size = (size - 32)/32; size>0 && index < BUCKET_LIST_COUNT; size/=2){
		//printf("%d\n", size);
		index++;
	}
	return index;
}
