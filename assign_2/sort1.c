/* Assignment 2 - Question 1
 * File: sort1.c
 * Description: Sorts integer numbers given in an input file
 * Author: Asmit De 10/CSE/53
 * Date: 02-Feb-2013
 */

#include <stdio.h>
#include <stdlib.h>

// Function definition for QuickSort algorithm
void QuickSort(int *array, int leftIndex, int rightIndex)
{
	int i, j, pivot, temp;

	if(leftIndex >= rightIndex)
		return;

	i = leftIndex;
	j = rightIndex - 1;
	pivot = array[rightIndex];

	while(i <= j)
	{
		while(i <= j && pivot >= array[i])
		{
			i = i + 1;
		}

		while(j >= i && pivot <= array[j])
		{
			j = j - 1;
		}

		if(i < j)
		{
			temp = array[i];
			array[i] = array[j];
			array[j] = temp;
		}
	}

	temp = array[rightIndex];
	array[rightIndex] = array[i];
	array[i] = temp;

	QuickSort(array, leftIndex, i - 1);
	QuickSort(array, i + 1, rightIndex);
}

int main(int argc, char *argv[])
{
	FILE *inputFile;								// File pointer for input file
	int array[1000], index = 0, noOfElements;					// array - stores the numbers for sorting;
											// index - iterator variable;
											// noOfElements - stores the no. of elements to be sorted

	// Error handling for incorrect argument list
	if(argc != 2)
	{
		fprintf(stderr, "\nError: Incorrect number of arguments.\nPress <enter> to exit...");
		getchar();
		exit(1);
	}
	
	// Open input file for reading
	inputFile = fopen(argv[1], "r");
	
	// Error handling for file opening errors
	if(inputFile == NULL)
	{
		puts("\nError: Cannot open file.\nPress <enter> to exit...");
		getchar();
		exit(1);
	}

	// Read elements from input file and store in array
	while(fscanf(inputFile, "%d", &array[index++]) != EOF);
	noOfElements = index - 1;
	fclose(inputFile);

	// Sort the elements in array
	QuickSort(array, 0, noOfElements - 1);

	// Display the sorted array
	puts("Sorted List:");
	for(index = 0; index < noOfElements; index++)
	{
		printf("%d\n", array[index]);
	}
	
	puts("\nPress <enter> to exit...");
	getchar();

	return 0;
}
