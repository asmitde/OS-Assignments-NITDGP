/* Assignment 3 - Question 1
 * File: search.c
 * Description: Searches an array of integers for a given integer
 * Author: Asmit De 10/CSE/53
 * Date: 09-Feb-2013
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#define MAX_SIZE 100							// Maximum size of array
#define STREAM_SIZE 3							// Size of stream buffer

int main(void)
{
	pid_t	child1,							// Stores the process id of 1st child
		child2;							// Stores the process id of 2nd child
		
	int 	array[MAX_SIZE],					// Stores the integers
		noOfElements,						// Stores the total number of elements entered
		key,							// Stores the element to be searched
		arrayIndex,						// Iterator for the array
		pipeFD[2];						// Pipe for communication between parent and child
		
	char	stream[STREAM_SIZE];					// Stream buffer for communication through pipe
	
	// Enter the elements and the search key
	printf("Enter the total number of elements: ");
	scanf("%d", &noOfElements);
	
	for(arrayIndex = 0; arrayIndex < noOfElements; arrayIndex++)
	{
		printf("Enter element #%d: ", arrayIndex);
		scanf("%d", &array[arrayIndex]);
	}
	
	printf("Enter the key to be searched: ");
	scanf("%d", &key);
	
	// Create pipe for communication
	if(pipe(pipeFD) == -1)
	{
		// Error handling for pipe errors
		perror("Error");
		printf("Press <enter> to exit...");
		getchar();
		exit(1);
	}
	
	// Create the first child
	child1 = fork();
	if(child1 == -1)
	{
		// Error handling for fork errors
		perror("Error");
		printf("Press <enter> to exit...");
		getchar();
		exit(1);
	}
	else if(child1 == 0)
	{
		// Child 1 searches 1st half of the array
	
		// Close read end of pipe
		close(pipeFD[0]);
		
		// Iterate over the first half of the array
		for(arrayIndex = 0; arrayIndex < noOfElements / 2; arrayIndex++)
		{
			// Check for a match
			if(array[arrayIndex] == key)
			{
				// Break if key found
				break;
			}
		}
		
		// Set arrayIndex to -1 if key not found
		if(arrayIndex == noOfElements / 2)
			arrayIndex = -1;
			
		// Write array index to stream
		sprintf(stream, "%d", arrayIndex);
		write(pipeFD[1], stream, STREAM_SIZE);
		close(pipeFD[1]);
		
		exit(0);
	}
	
	// Create the second child
	child2 = fork();
	if(child2 == -1)
	{
		// Error handling for fork errors
		perror("Error");
		printf("Press <enter> to exit...");
		getchar();
		exit(1);
	}
	else if(child2 == 0)
	{
		// Child 2 searches 2nd half of the array
	
		// Close read end of pipe
		close(pipeFD[0]);
		
		// Iterate over the second half of the array
		for(arrayIndex = noOfElements / 2; arrayIndex < noOfElements; arrayIndex++)
		{			
			// Check for a match
			if(array[arrayIndex] == key)
			{
				// Break if key found
				break;
			}
		}
		
		// Set arrayIndex to -1 if key not found
		if(arrayIndex == noOfElements)
			arrayIndex = -1;
			
		// Write array index to stream
		sprintf(stream, "%d", arrayIndex);
		write(pipeFD[1], stream, STREAM_SIZE);
		close(pipeFD[1]);
		
		exit(0);
	}
	
	// Read pipe for each child and display search result accordingly
	close(pipeFD[1]);
	read(pipeFD[0], stream, STREAM_SIZE);
	if(!strcmp(stream, "-1"))
	{
		read(pipeFD[0], stream, STREAM_SIZE);
		if(!strcmp(stream, "-1"))
		{
			close(pipeFD[0]);
			printf("No match found.\nPress <enter> to exit...\n");
			getchar();
			return 0;
		}
	}
	close(pipeFD[0]);
	printf("Match found at index #%s.\nPress <enter> to exit...\n", stream);
	getchar();
	
	return 0;
}
