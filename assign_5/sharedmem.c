/* Assignment 5 - Question 1
 * File: sharedmem.c
 * Description: Implementation of shared memory without synchronization
 * Author: Asmit De 10/CSE/53
 * Date: 28-Mar-2013
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
 
int main()
{
	pid_t	pidA,							// Stores process id of child process A
		pidB;							// Stores process id of child process B
		
	int	*x,							// Pointer variable to shared memory segment
		temp,							// Intermediate variable for incrementation/decrementation
		shmid,							// Id of the shared memory segment
		i;							// Iterator for loops
		
	
	// Obtain a shared memory segment and attach it to x
	shmid = shmget(IPC_PRIVATE, sizeof(int), 0777 | IPC_CREAT);
	x = (int*)shmat(shmid, 0, 0);
	
	// Initialize the shared value to 0
	*x = 0;
	
	// Fork the child process A
	pidA = fork();
	if(pidA == 0)
	{
		// Increment by 1 the shared value 10 times
		for(i = 1; i <= 10; i++)
		{							// Incrementation is broken down into three steps
			temp = *x;					// and sleep() call is places in between to simulate
			sleep(0.1);					// race condition with child process B.
			temp = temp + 1;
			*x = temp;
		}
		
		exit(0);
	}
	
	// Fork the child process B
	pidB = fork();
	if(pidB == 0)
	{
		// Decrement by 1 the shared value 10 times
		for(i = 1; i <= 10; i++)
		{							// Decrementation is broken down into three steps
			temp = *x;					// and sleep() call is places in between to simulate
			temp = temp - 1;				// race condition with child process A.
			sleep(0.1);
			*x = temp;
		}
		
		exit(0);
	}
	
	// Parent process waits for both child A and child B to exit
	// and prints the final shared value
	waitpid(pidA);
	waitpid(pidB);
	printf("x = %d", *x);
	
	// Detach x from the shared memory segment and delete the segment
	shmdt(x);
	shmctl(shmid, IPC_RMID, 0);
	
	return 0;
}
