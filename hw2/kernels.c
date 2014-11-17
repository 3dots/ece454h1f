/********************************************************
 * Kernels to be optimized for the CS:APP Performance Lab
 ********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "defs.h"

/* 
 * ECE454 Students: 
 * Please fill in the following team struct 
 */
team_t team = {
    "teamName",              /* Team name */

    "Viktor Riabtsev",     /* First member full name */
    "viktor.riabtsev@mail.utoronto.ca",  /* First member email address */

    "",                   /* Second member full name (leave blank if none) */
    ""                    /* Second member email addr (leave blank if none) */
};

/***************
 * ROTATE KERNEL
 ***************/

/******************************************************
 * Your different versions of the rotate kernel go here
 ******************************************************/

/* 
 * naive_rotate - The naive baseline version of rotate 
 */
char naive_rotate_descr[] = "naive_rotate: Naive baseline implementation";
void naive_rotate(int dim, pixel *src, pixel *dst) 
{
    int i, j;

    for (i = 0; i < dim; i++)
	for (j = 0; j < dim; j++)
	    dst[RIDX(dim-1-j, i, dim)] = src[RIDX(i, j, dim)];
}

/*
 * ECE 454 Students: Write your rotate functions here:
 */ 

/* 
 * rotate - Your current working version of rotate
 * IMPORTANT: This is the version you will be graded on
 */
char rotate_descr[] = "rotate: Current working version";
void rotate(int dim, pixel *src, pixel *dst) 
{
	int i, j;
	int k, l;
	
	if(dim <= 512) 
	{
		//Pure 32x32 tiling
		
		for (j = 0; j < dim; j+=32)
		{
			for (i = 0; i < dim; i+=32)	
			{
				for(l = j; l < min(j + 32, dim) ; l++)			
				{
					for(k = i; k < min(i + 32, dim); k++)
					{
						dst[RIDX(dim-1-l, k, dim)] = src[RIDX(k, l, dim)];

					}
					
				
				}	
			}
				
		}
	}
	else
	{
		//256x16 tiling, with +=4 unrolling in the 256 dimension.
		int indexDst;
		int indexSrc;
		

		for (j = 0; j < dim; j+=256)
		{
			for (i = 0; i < dim; i+=16)	
			{
				for(l = j; l < min(j + 256, dim) ; l+=4)			
				{
					for(k = i; k < min(i + 16, dim); k++)
					{
						//dst[RIDX(dim-1-l, k, dim)] = src[RIDX(k, l, dim)];
						
						indexDst = RIDX(dim-1-l, k, dim);
						indexSrc = RIDX(k, l, dim);
													
						dst[indexDst] = src[indexSrc++];
						indexDst-=dim;
						dst[indexDst] = src[indexSrc++];
						indexDst-=dim;
						dst[indexDst] = src[indexSrc++];
						indexDst-=dim;
						dst[indexDst] = src[indexSrc];

					}
					
				
				}	
			}
				
		}
	}

}

//Self-Reminder RIDX(i,j,n) ((i)*(n)+(j))
 

//No noticeble improvement
char rotate_two_descr[] = "2. First real attempt. Manual LICM on source index. Assuming multiplication is same as addition.";
void attempt_two(int dim, pixel *src, pixel *dst) 
{
    int i, j;
    //int dstIndexTmp;
    int srcIndexTmp;

    for (i = 0; i < dim; i++)
    {
    	//Note, could keep adding dim to itself? Faster then multiplication. Will try seprately later.
    	srcIndexTmp = i * dim;
    	
    	for (j = 0; j < dim; j++)
		{
    		dst[RIDX(dim-1-j, i, dim)] = src[srcIndexTmp + j];
		}
    		    
    }
	
}

//No change. I conclude that -O2 already does this. If so, i shall try to attack the other index instead.
//I tried another thing here... srcIndexTmp actually sort of steps through all... so no need to seperate addidtion, it should all
//be pure increments
char rotate_three_descr[] = "3. Manual LICM on source index. Using addition instead of multiplication.";
void attempt_three(int dim, pixel *src, pixel *dst) 
{
	
	int i, j;
	//int dstIndexTmp;
	int srcIndexTmp = 0;

	for (i = 0; i < dim; i++)
	{
		//Note, could keep adding dim to itself? Faster then multiplication. Will try seprately later.
			
		for (j = 0; j < dim; j++)
		{
			dst[RIDX(dim-1-j, i, dim)] = src[srcIndexTmp++];
		}//at exit we result in srcIndexTmp+=dim which is what we need anyway
			
	}
	
    
	
}

//No effect..... I conclude that the compiler does LICM just fine on its own. Lets try to unroll this loop next
char rotate_four_descr[] = "4. Manual LICM on dst index.";
void attempt_four(int dim, pixel *src, pixel *dst) 
{
    
	int i, j;
	int dstIndexTmp = 0;
	int defaultDstIndexPart = (dim-1)*dim;
	//int srcIndexTmp = 0;

	for (i = 0; i < dim; i++)
	{
		
		dstIndexTmp = i + defaultDstIndexPart;
		
		for (j = 0; j < dim; j++)
		{
			
			dst[dstIndexTmp] = src[RIDX(i, j, dim)];
			
			dstIndexTmp -= dim;
		}
			
	}
}

//WOW 1.5 mean improvement. Halleua! This was for unrolling external loop.

char rotate_five_descr[] = "5. Trying to unroll";
void attempt_five(int dim, pixel *src, pixel *dst) 
{
	int i, j;
	int indexDst;
	int indexSrc;

	for (i = 0; i < dim - 1; i+=2)
	{
		for (j = 0; j < dim; j++)
		{
			indexDst = RIDX(dim-1-j, i, dim);
			indexSrc = RIDX(i, j, dim);
			dst[indexDst] = src[indexSrc];
			dst[indexDst+1] = src[indexSrc + dim];
			
		}
	}
	
		
	
}

//Unrolling both doesn't do anything.
//Will try to unroll inner only: No improvement at all.. hmm
//Unrolling outerloop +=4. YEY 1.9 mean improvement
char rotate_six_descr[] = "5. Unrolling outer loop at +4";
void attempt_six(int dim, pixel *src, pixel *dst) 
{


	int i, j;
	int indexDst;
	int indexSrc;

	for (i = 0; i < dim - 3; i+=4)
	{
		for (j = 0; j < dim; j++)
		{
			indexDst = RIDX(dim-1-j, i, dim);
			indexSrc = RIDX(i, j, dim);
			dst[indexDst++] = src[indexSrc];
			indexSrc+=dim;
			dst[indexDst++] = src[indexSrc];
			indexSrc+=dim;
			dst[indexDst++] = src[indexSrc];
			indexSrc+=dim;
			dst[indexDst] = src[indexSrc];
		}
	}
	
		
	
}

//Unrolling at +=8. 2.5 Mean increase. Well.. I like this pattern.
char rotate_seven_descr[] = "Unrolling outer loop at +8";
void attempt_seven(int dim, pixel *src, pixel *dst) 
{


	int i, j;
	int indexDst;
	int indexSrc;

	for (i = 0; i < dim - 7; i+=8)
	{
		for (j = 0; j < dim; j++)
		{
			indexDst = RIDX(dim-1-j, i, dim);
			indexSrc = RIDX(i, j, dim);
			dst[indexDst++] = src[indexSrc];
			indexSrc+=dim;
			dst[indexDst++] = src[indexSrc];
			indexSrc+=dim;
			dst[indexDst++] = src[indexSrc];
			indexSrc+=dim;
			dst[indexDst++] = src[indexSrc];
			indexSrc+=dim;
			
			dst[indexDst++] = src[indexSrc];
			indexSrc+=dim;
			dst[indexDst++] = src[indexSrc];
			indexSrc+=dim;
			dst[indexDst++] = src[indexSrc];
			indexSrc+=dim;
			dst[indexDst] = src[indexSrc];
		}
	}
	
		
	
}

// 2.7 mean increase. Well we're slowing down. Also our input is multiple of 32. So lets try the 32 unroll.
char rotate_eight_descr[] = "Unrolling outer loop at +16";
void attempt_eight(int dim, pixel *src, pixel *dst) 
{
	int i, j;
	int indexDst;
	int indexSrc;
	
	

	for (i = 0; i < dim; i+=16)
	{
		for (j = 0; j < dim; j++)
		{
			indexDst = RIDX(dim-1-j, i, dim);
			indexSrc = RIDX(i, j, dim);
			dst[indexDst++] = src[indexSrc];
			indexSrc+=dim;
			dst[indexDst++] = src[indexSrc];
			indexSrc+=dim;
			dst[indexDst++] = src[indexSrc];
			indexSrc+=dim;
			dst[indexDst++] = src[indexSrc];
			indexSrc+=dim;
			
			dst[indexDst++] = src[indexSrc];
			indexSrc+=dim;
			dst[indexDst++] = src[indexSrc];
			indexSrc+=dim;
			dst[indexDst++] = src[indexSrc];
			indexSrc+=dim;
			dst[indexDst++] = src[indexSrc];
			indexSrc+=dim;
			
			dst[indexDst++] = src[indexSrc];
			indexSrc+=dim;
			dst[indexDst++] = src[indexSrc];
			indexSrc+=dim;
			dst[indexDst++] = src[indexSrc];
			indexSrc+=dim;
			dst[indexDst++] = src[indexSrc];
			indexSrc+=dim;
			
			dst[indexDst++] = src[indexSrc];
			indexSrc+=dim;
			dst[indexDst++] = src[indexSrc];
			indexSrc+=dim;
			dst[indexDst++] = src[indexSrc];
			indexSrc+=dim;
			dst[indexDst] = src[indexSrc];
		}
	}			
}

// Back down to 2.5 mean increase. This would probably speedup more for higher numbers perhaps? Either way, this is a step backwards
char rotate_nine_descr[] = "Unrolling outer loop at +32";
void attempt_nine(int dim, pixel *src, pixel *dst) 
{
	int i, j;
	int indexDst;
	int indexSrc;

	for (i = 0; i < dim - 31; i+=32)
	{
		for (j = 0; j < dim; j++)
		{
			indexDst = RIDX(dim-1-j, i, dim);
			indexSrc = RIDX(i, j, dim);
			dst[indexDst++] = src[indexSrc];
			indexSrc+=dim;
			dst[indexDst++] = src[indexSrc];
			indexSrc+=dim;
			dst[indexDst++] = src[indexSrc];
			indexSrc+=dim;
			dst[indexDst++] = src[indexSrc];
			indexSrc+=dim;
			
			dst[indexDst++] = src[indexSrc];
			indexSrc+=dim;
			dst[indexDst++] = src[indexSrc];
			indexSrc+=dim;
			dst[indexDst++] = src[indexSrc];
			indexSrc+=dim;
			dst[indexDst++] = src[indexSrc];
			indexSrc+=dim;
			
			dst[indexDst++] = src[indexSrc];
			indexSrc+=dim;
			dst[indexDst++] = src[indexSrc];
			indexSrc+=dim;
			dst[indexDst++] = src[indexSrc];
			indexSrc+=dim;
			dst[indexDst++] = src[indexSrc];
			indexSrc+=dim;
			
			dst[indexDst++] = src[indexSrc];
			indexSrc+=dim;
			dst[indexDst++] = src[indexSrc];
			indexSrc+=dim;
			dst[indexDst++] = src[indexSrc];
			indexSrc+=dim;
			dst[indexDst++] = src[indexSrc];
			indexSrc+=dim; //16 statements
			
			dst[indexDst++] = src[indexSrc];
			indexSrc+=dim;
			dst[indexDst++] = src[indexSrc];
			indexSrc+=dim;
			dst[indexDst++] = src[indexSrc];
			indexSrc+=dim;
			dst[indexDst++] = src[indexSrc];
			indexSrc+=dim;
			
			dst[indexDst++] = src[indexSrc];
			indexSrc+=dim;
			dst[indexDst++] = src[indexSrc];
			indexSrc+=dim;
			dst[indexDst++] = src[indexSrc];
			indexSrc+=dim;
			dst[indexDst++] = src[indexSrc];
			indexSrc+=dim;
			
			dst[indexDst++] = src[indexSrc];
			indexSrc+=dim;
			dst[indexDst++] = src[indexSrc];
			indexSrc+=dim;
			dst[indexDst++] = src[indexSrc];
			indexSrc+=dim;
			dst[indexDst++] = src[indexSrc];
			indexSrc+=dim;
			
			dst[indexDst++] = src[indexSrc];
			indexSrc+=dim;
			dst[indexDst++] = src[indexSrc];
			indexSrc+=dim;
			dst[indexDst++] = src[indexSrc];
			indexSrc+=dim;
			dst[indexDst] = src[indexSrc];
		}
	}			
}

//We have achieved 2.7 median increase, but inorder to achieve the mythical 3.0, we must try to maybe optimize the inner loop somehow
//not just unroll the outer one?
//Seprating steps? The higher numbers speed up as hell!!! But lower ones slow down. 0.9 median. WAIT!!! I CAN DO IF STATEMENT!!
//
char rotate_ten_descr[] = "Separate Steps. No unrolling.";
void attempt_ten(int dim, pixel *src, pixel *dst)
{
	int i, j;
	
	
	//Can we seprate the steps?
	pixel *inter = (pixel *) malloc(dim*dim*sizeof(pixel));
	
	//Transpose
	for (i = 0; i < dim ; i++)
	{
		for (j = 0; j < dim; j++)
		{		
			inter[RIDX(i, j, dim)] = src[RIDX(j, i, dim)];
		}
	}
	
	//Flip rows
    for (i = 0; i < dim ; i++)
    {
    	
    	for (j = 0; j < dim ; j++)
		{
		
			dst[RIDX(i, j, dim)] = inter[RIDX(dim-1-i, j, dim)];				
		
		}
    }
	
    free(inter);
	    
}

char rotate_eleven_descr[] = "Separate Steps. Lets unroll first inner loop at +=8";
void attempt_eleven(int dim, pixel *src, pixel *dst)
{
	int i, j;
	
	int indexDst;
	int indexSrc;
	
	//Can we seprate the steps?
	pixel *inter = (pixel *) malloc(dim*dim*sizeof(pixel));
	
	//Transpose
	for (i = 0; i < dim - 7 ; i+=8)
	{
		for (j = 0; j < dim; j++)
		{
			indexDst = RIDX(j, i, dim);
			indexSrc = RIDX(i, j, dim);
			
			inter[indexDst++] = src[indexSrc];
			indexSrc+= dim;
			inter[indexDst++] = src[indexSrc];
			indexSrc+= dim;
			inter[indexDst++] = src[indexSrc];
			indexSrc+= dim;
			inter[indexDst++] = src[indexSrc];
			indexSrc+= dim;
			
			inter[indexDst++] = src[indexSrc];
			indexSrc+= dim;
			inter[indexDst++] = src[indexSrc];
			indexSrc+= dim;
			inter[indexDst++] = src[indexSrc];
			indexSrc+= dim;
			inter[indexDst] = src[indexSrc];
	

		}
	}
	
	//Flip rows
    for (i = 0; i < dim; i++)
    {
    	
    	for (j = 0; j < dim ; j++)
		{    		
    		dst[RIDX(i, j, dim)] = inter[RIDX(dim-1-i, j, dim)];

		}
    }
	
    free(inter);
	    
}

char rotate_twelve_descr[] = "Separate Steps. Lets unroll first inner loop at +=16";
void attempt_twelve(int dim, pixel *src, pixel *dst)
{
	int i, j;
	
	int indexDst;
	int indexSrc;
	
	//Can we seprate the steps?
	pixel *inter = (pixel *) malloc(dim*dim*sizeof(pixel));
	
	//Transpose
	for (i = 0; i < dim - 15 ; i+=16)
	{
		for (j = 0; j < dim; j++)
		{
			indexDst = RIDX(j, i, dim);
			indexSrc = RIDX(i, j, dim);
			
			inter[indexDst++] = src[indexSrc];
			indexSrc+= dim;
			inter[indexDst++] = src[indexSrc];
			indexSrc+= dim;
			inter[indexDst++] = src[indexSrc];
			indexSrc+= dim;
			inter[indexDst++] = src[indexSrc];
			indexSrc+= dim;
			
			inter[indexDst++] = src[indexSrc];
			indexSrc+= dim;
			inter[indexDst++] = src[indexSrc];
			indexSrc+= dim;
			inter[indexDst++] = src[indexSrc];
			indexSrc+= dim;
			inter[indexDst++] = src[indexSrc];
			indexSrc+= dim;
			
			inter[indexDst++] = src[indexSrc];
			indexSrc+= dim;
			inter[indexDst++] = src[indexSrc];
			indexSrc+= dim;
			inter[indexDst++] = src[indexSrc];
			indexSrc+= dim;
			inter[indexDst++] = src[indexSrc];
			indexSrc+= dim;
			
			inter[indexDst++] = src[indexSrc];
			indexSrc+= dim;
			inter[indexDst++] = src[indexSrc];
			indexSrc+= dim;
			inter[indexDst++] = src[indexSrc];
			indexSrc+= dim;
			inter[indexDst] = src[indexSrc];
	
		}
	}
	
	//Flip rows
    for (i = 0; i < dim; i++)
    {
    	
    	for (j = 0; j < dim ; j++)
		{    		
    		dst[RIDX(i, j, dim)] = inter[RIDX(dim-1-i, j, dim)];

		}
    }
	
    free(inter);
	    
}


int min(int x, int y){
	if(x > y){
		return y;
	}
	else
		return x;
}

char rotate_thirteen_descr[] = "32x32 Tiles";
void attempt_thirteen(int dim, pixel *src, pixel *dst) 
{
	int i, j;
	
	int k, l;
	
	
	for (j = 0; j < dim; j+=32)
	{
		for (i = 0; i < dim; i+=32)	
		{
			for(l = j; l < min(j + 32, dim) ; l++)			
			{
				for(k = i; k < min(i + 32, dim); k++)
				{
					dst[RIDX(dim-1-l, k, dim)] = src[RIDX(k, l, dim)];

				}
				
			
			}	
		}
			
	}
		
		    			
}

char rotate_fourteen_descr[] = "16x32 Tiles";
void attempt_fourteen(int dim, pixel *src, pixel *dst) 
{
	int i, j;
	
	int k, l;
	
	
	for (j = 0; j < dim; j+=16)
	{
		for (i = 0; i < dim; i+=32)	
		{
			for(l = j; l < min(j + 16, dim) ; l++)			
			{
				for(k = i; k < min(i + 32, dim); k++)
				{
					dst[RIDX(dim-1-l, k, dim)] = src[RIDX(k, l, dim)];

				}
				
			
			}	
		}
			
	}
		
		    			
}

char rotate_fithteen_descr[] = "Tiled, +4, 256*16tiles";
void attempt_fithteen(int dim, pixel *src, pixel *dst) 
{
	int i, j;
	
	int k, l;
	
	int indexDst;
	int indexSrc;
	

	for (j = 0; j < dim; j+=256)
	{
		for (i = 0; i < dim; i+=16)	
		{
			for(l = j; l < min(j + 256, dim) ; l+=4)			
			{
				for(k = i; k < min(i + 16, dim); k++)
				{
					//dst[RIDX(dim-1-l, k, dim)] = src[RIDX(k, l, dim)];
					
					indexDst = RIDX(dim-1-l, k, dim);
					indexSrc = RIDX(k, l, dim);
												
					dst[indexDst] = src[indexSrc++];
					indexDst-=dim;
					dst[indexDst] = src[indexSrc++];
					indexDst-=dim;
					dst[indexDst] = src[indexSrc++];
					indexDst-=dim;
					dst[indexDst] = src[indexSrc];

				}
				
			
			}	
		}
			
	}		
		    			
}


char rotate_sixteen_descr[] = "Tiled, +4, 256*16tiles MANUAL";
void attempt_sixteen(int dim, pixel *src, pixel *dst) 
{
	int i, j;
	
	int k, l;
	
	int indexDst;
	int indexSrc;
	

	for (j = 0; j < dim; j+=256)
	{
		for (i = 0; i < dim; i+=16)	
		{
			for(l = j; l < min(j + 256, dim) ; l+=4)			
			{
				for(k = i; k < min(i + 16, dim); k++)
				{
					//dst[RIDX(dim-1-l, k, dim)] = src[RIDX(k, l, dim)];
					
					indexDst = RIDX(dim-1-l, k, dim);
					indexSrc = RIDX(k, l, dim);												
					
					dst[indexDst] = src[indexSrc];

					dst[indexDst - dim] = src[indexSrc + 1];

					dst[indexDst - 2*dim] = src[indexSrc + 2];

					dst[indexDst - 3*dim] = src[indexSrc + 3];

				}
				
			
			}	
		}
			
	}		
		    			
}

char rotate_seventeen_descr[] = "Tiled, +8, 512*16tiles";
void attempt_seventeen(int dim, pixel *src, pixel *dst) 
{
	int i, j;
	
	int k, l;
	
	int indexDst;
	int indexSrc;
	

	for (j = 0; j < dim; j+=512)
	{
		for (i = 0; i < dim; i+=16)	
		{
			for(l = j; l < min(j + 512, dim) ; l+=8)			
			{
				for(k = i; k < min(i + 16, dim); k++)
				{
					//dst[RIDX(dim-1-l, k, dim)] = src[RIDX(k, l, dim)];
					
					indexDst = RIDX(dim-1-l, k, dim);
					indexSrc = RIDX(k, l, dim);
					
					dst[indexDst] = src[indexSrc++];
					indexDst-=dim;
					dst[indexDst] = src[indexSrc++];
					indexDst-=dim;
					dst[indexDst] = src[indexSrc++];
					indexDst-=dim;
					dst[indexDst] = src[indexSrc++];
					indexDst-=dim;
					
					dst[indexDst] = src[indexSrc++];
					indexDst-=dim;
					dst[indexDst] = src[indexSrc++];
					indexDst-=dim;
					dst[indexDst] = src[indexSrc++];
					indexDst-=dim;
					dst[indexDst] = src[indexSrc];

				}
				
			
			}	
		}
			
	}		
		    			
}

/*********************************************************************
 * register_rotate_functions - Register all of your different versions
 *     of the rotate kernel with the driver by calling the
 *     add_rotate_function() for each test function. When you run the
 *     driver program, it will test and report the performance of each
 *     registered test function.  
 *********************************************************************/

void register_rotate_functions() 
{
    add_rotate_function(&naive_rotate, naive_rotate_descr);   
    add_rotate_function(&rotate, rotate_descr);   

    //add_rotate_function(&attempt_two, rotate_two_descr);   
    //add_rotate_function(&attempt_three, rotate_three_descr);   
    //add_rotate_function(&attempt_four, rotate_four_descr);   
    //add_rotate_function(&attempt_five, rotate_five_descr);   
    //add_rotate_function(&attempt_six, rotate_six_descr);   
    //add_rotate_function(&attempt_seven, rotate_seven_descr);   
    //add_rotate_function(&attempt_eight, rotate_eight_descr);   
    //add_rotate_function(&attempt_nine, rotate_nine_descr);   
    //add_rotate_function(&attempt_ten, rotate_ten_descr);   
    //add_rotate_function(&attempt_eleven, rotate_eleven_descr);
    //add_rotate_function(&attempt_twelve, rotate_twelve_descr);
    
    add_rotate_function(&attempt_thirteen, rotate_thirteen_descr);
    add_rotate_function(&attempt_fourteen, rotate_fourteen_descr);
    add_rotate_function(&attempt_fithteen, rotate_fithteen_descr);
    add_rotate_function(&attempt_sixteen, rotate_sixteen_descr);
    add_rotate_function(&attempt_seventeen, rotate_seventeen_descr);

    /* ... Register additional rotate functions here */
}

