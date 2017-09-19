/* Assignment 5 - Question 3
 * File: prodcons.c
 * Description: Implementation of m-producer, n-consumer problem using semaphores
 * Author: Asmit De 10/CSE/53
 * Date: 30-Mar-2013
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
 
#define P(s) semop(s, &pop, 1)						// Simulates the wait() operation
#define V(s) semop(s, &vop, 1)						// Simulates the signal() operation
#define SIZE 20								// Size of the buffer
  
// Queue structure and its functions
// Begin
	typedef struct Queue
	{
		int _queue[SIZE];
		int front, rear;
	}Queue;

	void InitQueue(Queue *q)
	{
		q->front = -1;
		q->rear = -1;
	}

	int IsFull(Queue *q)
	{
		if((q->rear == SIZE - 1 && q->front == -1) || (q->front == q->rear && q->front != -1))
			return 1;
		else
			return 0;
	}

	int IsEmpty(Queue *q)
	{
		if(q->front == q->rear && q->front == -1)
			return 1;
		else
			return 0;
	}

	int Enqueue(Queue *q, int data)
	{
		if(IsFull(q))
		{
			printf("\nError: Queue is full...");
			return 1;
		}

		q->rear = (q->rear + 1) % SIZE;
		q->_queue[q->rear] = data;
		return 0;
	}

	int Dequeue(Queue *q, int *data)
	{
		if(IsEmpty(q))
		{
			printf("\nError: Queue is empty...");
			return 1;
		}

		q->front = (q->front + 1) % SIZE;
		*data = q->_queue[q->front];
		return 0;
	}
// End

// Signal handler for SIGUSR1
void catchSIGUSR1()
{
	// Terminate the consumer
	exit(0);
}

int main()
{
	pid_t	*pidP,							// Pointer to process id of producers
		*pidC;							// Pointer to process id of consumers
		
	int	m,							// Stores the number of producers
		n,							// Stores the number of consumers
		*sum,							// Pointer variable to shared memory segment containing sum
		shmidSum,						// Id of the shared memory segment containing sum
		shmidBuffer,						// Id of the shared memory segment containing buffer
		semidCS,						// Id of the semaphore for critical section
		semidBuffFull,						// Id of the semaphore for buffer full condition
		semidBuffEmpty,						// Id of the semaphore for buffer empty condition
		i;							// Iterator for loops
		
	Queue	*buffer;						// Pointer variable to shared memory segment containing buffer
		
	struct sembuf	pop,						// Structure for P() operation
			vop;						// Structure for V() operation
			
	// Obtain a shared memory segment and attach it to sum
	shmidSum = shmget(IPC_PRIVATE, sizeof(int), 0777 | IPC_CREAT);
	sum = (int*)shmat(shmidSum, 0, 0);
	
	// Initialize the shared sum to 0
	*sum = 0;
	
	// Obtain a shared memory segment and attach it to buffer
	shmidBuffer = shmget(IPC_PRIVATE, sizeof(Queue), 0777 | IPC_CREAT);
	buffer = (Queue*)shmat(shmidBuffer, 0, 0);
	
	// Initialize the shared buffer
	InitQueue(buffer);
	
	// Create a semaphore for critical section and set the value
	semidCS = semget(IPC_PRIVATE, 1, 0777 | IPC_CREAT);
	semctl(semidCS, 0, SETVAL, 1);
	
	// Create a semaphore for buffer empty condition and set the value
	semidBuffEmpty = semget(IPC_PRIVATE, 1, 0777 | IPC_CREAT);
	semctl(semidBuffEmpty, 0, SETVAL, 0);
	
	// Create a semaphore for buffer full condition and set the value
	semidBuffFull = semget(IPC_PRIVATE, 1, 0777 | IPC_CREAT);
	semctl(semidBuffFull, 0, SETVAL, SIZE);
	
	// Initialize the pop and vop structure fields
	pop.sem_num = vop.sem_num = 0;
	pop.sem_flg = vop.sem_flg = 0;
	pop.sem_op = -1;
	vop.sem_op = 1;
	
	// Get the number of producers and consumers and create their pid arrays
	printf("Enter the number of producers and consumers: ");
	scanf("%d %d", &m, &n);
	pidP = (pid_t*)calloc(m, sizeof(pid_t));
	pidC = (pid_t*)calloc(n, sizeof(pid_t));

	// Function definition for producers
	void Producer()
	{
		int j;
		for(j = 1; j <= 50; j++)
		{
			// Check and lock semaphore for buffer full condition
			P(semidBuffFull);
			
			// Check and lock semaphore for critical section
			P(semidCS);
			
			// Critical section
			// Begin
			Enqueue(buffer, j);
			//printf("\nProducer produces %d", j);
			// End
			
			// Release semaphore for critical section
			V(semidCS);
			
			// Release semaphore for buffer empty condition
			V(semidBuffEmpty);
		}
	}
	
	// Function definition for consumers
	void Consumer()
	{
		// Initialize handler for SIGALRM
		signal(SIGUSR1, catchSIGUSR1);
		
		int data, j;
		for(;;)
		{
			// If all data has been consumed after all producers
			// have finished producing, but parent is still creating
			// consumers, terminate the consumer
			if(*sum == m*1275)
				exit(0);
				
			// Check and lock semaphore for buffer empty condition
			P(semidBuffEmpty);
			
			// Check and lock semaphore for critical section
			P(semidCS);
						
			// Critical section
			// Begin
			Dequeue(buffer, &data);
			*sum = *sum + data;
			
			// Check if all data has been consumed after all producers have finished producing
			if(*sum == m*1275)
			{
				// Send signal to all consumers
				for(j = 0; j < n; j++)
					kill(pidC[j], SIGUSR1);
			}
			// End
			
			// Release semaphore for critical section
			V(semidCS);
			
			// Release semaphore for buffer full condition
			V(semidBuffFull);
		}
	}
	
	// Set to ignore SIGUSR1 if received
	signal(SIGUSR1, SIG_IGN);
	
	// Create m producers
	for(i = 0; i < m; i++)
	{
		pidP[i] = fork();
		if(pidP[i] == 0)
		{
			Producer();
			exit(0);
		}
		else if(pidP[i] == -1)
		{
			// Error handling for fork errors
			perror("Error");
		}
	}
	
	// Create n consumers
	for(i = 0; i < n; i++)
	{
		pidC[i] = fork();
		if(pidC[i] == 0)
		{
			Consumer();
		}
		else if(pidC[i] == -1)
		{
			// Error handling for fork errors
			perror("Error");
		}
	}
	
	// Parent process waits for all producers and consumers to exit
	for(i = 0; i < m; i++)
		waitpid(pidP[i]);

	for(i = 0; i < n; i++)
		waitpid(pidC[i]);
		
	// Print the final shared sum
	printf("Sum = %d\n", *sum);
	
	// Delete all semaphores
	semctl(semidCS, 0, IPC_RMID, 0);
	semctl(semidBuffFull, 0, IPC_RMID, 0);
	semctl(semidBuffEmpty, 0, IPC_RMID, 0);
	
	// Detach sum and buffer from their shared memory segments and delete the segments
	shmdt(sum);
	shmctl(shmidSum, IPC_RMID, 0);
	shmdt(buffer);
	shmctl(shmidBuffer, IPC_RMID, 0);
	
	return 0;
}
