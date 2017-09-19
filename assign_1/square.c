/* Assignment 1 - Question 1
 * File: square.c
 * Description: Calculates square of the given input
 * Author: Asmit De 10/CSE/53
 * Date: 03-Feb-2013
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char *argv[], char *envp[])
{
	int operand, result;							// operand - last argument on which operation is to be performed;
										// result - result of operation on operand

	// Error handling for incorrect argument list
	if(argc == 1)
	{
		fprintf(stderr, "\nError: Incorrect number of arguments.\nPress <enter> to exit...");
		getchar();
		exit(1);
	}

	// Convert operand from string to integer and perform operation
	sscanf(argv[argc - 1], "%d", &operand);
	result = operand * operand;

	printf("Process id of square: %d\n", getpid());

	// Check if argument list contains more programs and execute the next in sequence
	if(argc > 2)
	{
		// Update argument list for the next program in sequence
		sprintf(argv[argc - 1], "%d", result);
		argv[argc] = NULL;
		
		// exec the next program in sequence
		execve(argv[1], &argv[1], envp);
		
		// Error handling for exec errors
		fprintf(stderr, "\nError: Cannot execute %s.\nPress <enter> to exit...", argv[1]);
		getchar();
		exit(1);
	}

	// Display result for the last program executed
	printf("\nResult = %d\n", result);

	return 0;
}
