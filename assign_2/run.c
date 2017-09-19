/* Assignment 2 - Question 2
 * File: run.c
 * Description: Executes 'myshell' (shell)
 * Author: Asmit De 10/CSE/53
 * Date: 07-Feb-2013
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int main()
{
	pid_t pid;										// Stores process id of child process
	int exitStatus;										// Stores exit status of child process

	// Fork a new process and exec shell from the forked process
	pid = fork();
	if(pid == 0)
	{
		execlp("./shell", NULL);
		
		// Error handling for exec errors
		perror("Error: Command not found");
		printf("Press <enter> to exit...\n");
		getchar();
		exit(1);	
	}
	// Error handling for fork errors
	else if(pid == -1)
	{
		perror("\nError: Cannot create a new process");
		printf("\nPress <enter> to exit...");
		getchar();
		exit(1);
	}
	
	wait(&exitStatus);
	
	return 0;
}
