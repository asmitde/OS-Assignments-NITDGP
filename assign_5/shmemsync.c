/* Assignment 5 - Question 2
 * File: shmemsync.c
 * Description: Implementation of shared memory with synchronization using semaphores
 * Author: Asmit De 10/CSE/53
 * Date: 29-Mar-2013
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
 
#define P(s) semop(s, &pop, 1)						// Simulates the wait() operation
#define V(s) semop(s, &vop, 1)						// Simulates the signal() operation
 
int main()
{
	pid_t	pidA,							// Stores process id of child process A
		pidB;							// Stores process id of child process B
		
	int	*x,							// Pointer variable to shared memory segment
		temp,							// Intermediate variable for incrementation/decrementation
		shmid,							// Id of the shared memory segment
		semid,							// Id of the semaphore
		i;							// Iterator for loops
		
	struct sembuf	pop,						// Structure for P() operation
			vop;						// Structure for V() operation
			
	// Obtain a shared memory segment and attach it to x
	shmid = shmget(IPC_PRIVATE, sizeof(int), 0777 | IPC_CREAT);
	x = (int*)shmat(shmid, 0, 0);
	
	// Initialize the shared value to 0
	*x = 0;
	
	// Create a semaphore for synchronization and set the value
	semid = semget(IPC_PRIVATE, 1, 0777 | IPC_CREAT);
	semctl(semid, 0, SETVAL, 1);
	
	// Initialize the pop and vop structure fields
	pop.sem_num = vop.sem_num = 0;
	pop.sem_flg = vop.sem_flg = 0;
	pop.sem_op = -1;
	vop.sem_op = 1;
	
	// Fork the child process A
	pidA = fork();
	if(pidA == 0)
	{
		// Increment by 1 the shared value 10 times
		for(i = 1; i <= 10; i++)
		{	
			// Check and lock semaphore
			P(semid);
			
			// Critical section - start
			temp = *x;					// Incrementation is broken down into three steps
			sleep(0.1);					// and sleep() call is placed in between to simulate
			temp = temp + 1;				// race condition with child process B. However, this
			*x = temp;					// race condition is prevented by the semaphore.
			// Critical section - end
			
			// Release the semaphore
			V(semid);
		}
		
		exit(0);
	}
	
	// Fork the child process B
	pidB = fork();
	if(pidB == 0)
	{
		// Decrement by 1 the shared value 10 times
		for(i = 1; i <= 10; i++)
		{							
			// Check and lock semaphore
			P(semid);
			
			// Critical section - start
			temp = *x;					// Decrementation is broken down into three steps
			temp = temp - 1;				// and sleep() call is placed in between to simulate
			sleep(0.1);					// race condition with child process A. However, this
			*x = temp;					// race condition is prevented by the semaphore.
			// Critical section - end
			
			// Release the semaphore
			V(semid);
		}
		
		exit(0);
	}
	
	// Parent process waits for both child A and child B to exit
	// and prints the final shared value
	waitpid(pidA);
	waitpid(pidB);
	printf("x = %d", *x);
	
	// Delete the semaphore
	semctl(semid, 0, IPC_RMID, 0);
	
	// Detach x from the shared memory segment and delete the segment
	shmdt(x);
	shmctl(shmid, IPC_RMID, 0);
	
	return 0;
}
