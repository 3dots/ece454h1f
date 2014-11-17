#include <stdio.h>
#include <string.h>



int main(int argc, char *argv[])
{ //–2147483648 to 2147483647   

// if pos, then len max is 10
// if neg, max is also 10 (neg sign not part of string)
	char test[30] = "4294967294";
	int a = atoi(test);

	char b[11];
	sprintf(b, "%d", a);
		

	int c = strcmp(test, b);
	printf("\n%d\n", c);
	/*
	double result1 = 0, result2 = 0, intermediate1, intermediate2;
	int i;
	
	for(i = 0 ; i < 100 ; i++)
	{
		start_comp_counter();
		intermediate1 = get_comp_counter();
		result1+=intermediate1;
		
		start_comp_counter();
		printf("Blank counter = %f\n", intermediate1);
		intermediate2 = get_comp_counter();
		result2+=intermediate2;
		printf("PrintfClockCount = %f\n", intermediate2);
	}
	
	printf("Blank counter average = %f\n", result1/100.0);
	printf("PrintfClockCount average= %f\n", result2/100.0);
	*/
	/*
	printf("\nsame stuff except its all compensatin.\n");
	start_comp_counter();
	result1 = get_comp_counter();
	
	start_comp_counter();
	printf("Blank counter = %f\n", result1);
	result2 = get_comp_counter();
	printf("PrintfClockCount = %f\n", result2);
	*/
	
	
	
	return 0;
}

