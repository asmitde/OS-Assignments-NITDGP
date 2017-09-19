/* Assignment 2 - Question 2
 * File: shell.c
 * Description: Implements a shell as a command interpreter
 * Author: Asmit De 10/CSE/53
 * Date: 04-Feb-2013
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

#define BUFFER_SIZE 1024							// Buffer size for command and pwd
#define MAX_ARGUMENTS 64							// Maximum number of arguments for external commands
#define TIME_SIZE 32								// String length of time

int main(void)
{
	char	command[BUFFER_SIZE],						// Stores input command
		pwd[BUFFER_SIZE],						// Stores working directory
		*argToken,							// Stores currently processed command argument
		*argList[MAX_ARGUMENTS],					// Stores argument list for external commands
		time[TIME_SIZE];						// Stores timestamp of files in string format
		
	int 	argIndex,							// Iterator for argList
		exitStatus;							// Stores exit status of forked external command
		
	struct	dirent *dptr;							// Iterator pointer for directory entries
	struct	stat fileInfo;							// Stores file statistics
	struct	passwd *user;							// Stores file owner information
	struct	group *grp;							// Stores file group information
	struct	tm *timestamp;							// Stores file timestamp
	DIR	*dirp;								// Stores pointer to directory structure
	pid_t	pid;								// Stores process id of forked external command
	
	system("clear");
	while(1)
	{
		// Display prompt and accept input command
		printf("myshell> ");
		gets(command);
		if(!strcmp(command, "")) continue;
		
		// Split the input command into argument tokens.
		// argToken - Contains the current token processed
		argToken = strtok(command, " ");
		
		// Implementation of some built-in functions
		if(!strcmp(argToken, "cd"))					// Command: cd <directory path/name>
		{								// Function: Change the current working directory
			argToken = strtok(NULL, " ");
			
			// Change working directory
			if(chdir(argToken) == -1)
			{
				// Error handling for chdir errors
				perror("Error");
			}
		}
		else if(!strcmp(argToken, "pwd"))				// Command: pwd
		{								// Function: Display the current working directory
			getcwd(pwd, BUFFER_SIZE - 1);
			printf("%s\n", pwd);
		}
		else if(!strcmp(argToken, "mkdir"))				// Command: mkdir <directory path/name> [<directory path/name>]...
		{								// Function: Create new directories	
			argToken = strtok(NULL, " ");
			
			// Iterate over all arguments
			do
			{
				// Create directory and set permissions
				if(mkdir(argToken, 0755) == -1)
				{
					// Error handling for mkdir errors
					perror("Error");
				}
				
				argToken = strtok(NULL, " ");
			}while(argToken != NULL);
		}
		else if(!strcmp(argToken, "rmdir"))				// Command: rmdir <directory path/name> [<directory path/name>]...
		{								// Function: Delete directories if empty
			argToken = strtok(NULL, " ");
			
			// Iterate over all arguments
			do
			{
				// Delete directory if empty
				if(rmdir(argToken) == -1)
				{
					// Error handling for rmdir errors
					perror("Error");
				}
				
				argToken = strtok(NULL, " ");
			}while(argToken != NULL);
		}
		else if(!strcmp(argToken, "ls"))				// Command: ls [-l] [<directory path/name>]
		{								// Function: List directory structure
			argToken = strtok(NULL, " ");
			
			// Default format
			if(argToken == NULL || strcmp(argToken, "-l"))
			{
				if(argToken == NULL)
				{
					// No <directory path/name> given, get current working directory
					getcwd(pwd, BUFFER_SIZE - 1);
				}
				else
				{
					// Use <directory path/name> given
					strcpy(pwd, argToken);
				}
				
				// Open directory for reading
				dirp = opendir(pwd);
				
				// Error handling for opendir errors
				if(dirp == NULL)
				{
					perror("Error");
					continue;
				}
				
				// Iterate over all entries in directory structure
				while(dptr = readdir(dirp))
				{
					// Skip hidden files
					if(dptr->d_name[0] == '.') continue;
				
					//  Display file names	
					printf("%s\t", dptr->d_name);
					
				}
				printf("\n");
				closedir(dirp);
			}
			// Long List format
			else
			{
				argToken = strtok(NULL, " ");
				
				if(argToken == NULL)
				{
					// No <directory path/name> given, get current working directory
					getcwd(pwd, BUFFER_SIZE - 1);
				}
				else
				{
					// Use <directory path/name> given
					strcpy(pwd, argToken);
				}
				
				// Open directory for reading
				dirp = opendir(pwd);
				
				// Error handling for opendir errors
				if(dirp == NULL)
				{
					perror("Error");
					continue;
				}
				
				// Iterate over all entries in directory structure
				while(dptr = readdir(dirp))
				{
					// Skip hidden files
					if(dptr->d_name[0] == '.') continue;
					
					// Get file information
					stat(dptr->d_name, &fileInfo);
					
					// Check and display file permissions
					printf((S_ISDIR(fileInfo.st_mode)) ? "d" : "-");
					printf((fileInfo.st_mode & S_IRUSR) ? "r" : "-");
					printf((fileInfo.st_mode & S_IWUSR) ? "w" : "-");
					printf((fileInfo.st_mode & S_IXUSR) ? "x" : "-");
					printf((fileInfo.st_mode & S_IRGRP) ? "r" : "-");
					printf((fileInfo.st_mode & S_IWGRP) ? "w" : "-");
					printf((fileInfo.st_mode & S_IXGRP) ? "x" : "-");
					printf((fileInfo.st_mode & S_IROTH) ? "r" : "-");
					printf((fileInfo.st_mode & S_IWOTH) ? "w" : "-");
					printf((fileInfo.st_mode & S_IXOTH) ? "x" : "-");
					
					// Get owner and group of file
					user = getpwuid(fileInfo.st_uid);
					grp = getgrgid(fileInfo.st_gid);
					
					// Disply number of links, owner, group and size of file
					printf(". %d %s\t%s\t%d\t", fileInfo.st_nlink, user->pw_name, grp->gr_name, fileInfo.st_size);
					
					// Get and display timestamp
					timestamp = localtime(&fileInfo.st_mtime);
					strftime(time, TIME_SIZE - 1, "%a %b %d %H:%M", timestamp);
					printf("%s", time);
					
					// Display filename
					printf(" %s\n", dptr->d_name);	
				}
				closedir(dirp);
			}
		}
		else if(!strcmp(argToken, "exit"))				// Command: exit
		{								// Function: Exit the shell
			break;
		}
		else								// Function: Execute other commands
		{
			// Create null terminated argument list
			argIndex = 0;
			while(argToken != NULL)
			{
				argList[argIndex++] = argToken;
				argToken = strtok(NULL, " ");
			}
			argList[argIndex] = NULL;
			
			// Create a child process to exec the input command
			pid = fork();
			if(pid == 0)
			{
				// exec input command with arguments
				execvp(argList[0], argList);
				
				// Error handling for incorrect commands
				perror("Error: command not found");
				exit(1);
			}
			else if(pid == -1)
			{
				// Error handling for fork errors
				perror("Error");
			}
			
			// Parent process waits for child to finish execution
			wait(&exitStatus);
		}
	}
	
	return 0;
}
