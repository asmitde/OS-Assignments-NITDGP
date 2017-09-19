/* Assignment 2 - Question 1
 * File: xsort.c
 * Description: Implements a command that uses sort1 to sort integers from a given file (integers are separated by whitespace)
 * Author: Asmit De 10/CSE/53
 * Date: 02-Feb-2013
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	pid_t pid;										// Holds process id of child process
	char *cmdTerm[] = {"/usr/bin/xterm", "-e", "./sort1", argv[1], NULL};			// Command list for opening a new xterm window;
												//	sort1 - program to be executed in xterm

	// Error handling for incorrect argument list
	if(argc != 2)
	{
		fprintf(stderr, "\nError: Incorrect number of arguments.\nPress <enter> to exit...");
		getchar();
		exit(1);
	}

	// Fork a new process and open a new xterm window from the forked process
	pid = fork();
	if(pid == 0)
	{
		execv(cmdTerm[0], cmdTerm);		
	}
	// Error handling for fork errors
	else if(pid == -1)
	{
		fprintf(stderr, "\nError: Cannot create a new process.\nPress <enter> to exit...");
		getchar();
		exit(1);
	}

	return 0;
}
