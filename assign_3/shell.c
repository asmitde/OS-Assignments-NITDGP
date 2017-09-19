/* Assignment 3 - Question 2
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
#include <sys/sendfile.h>
#include <fcntl.h>

#define BUFFER_SIZE 1024							// Buffer size for command and pwd
#define MAX_ARGUMENTS 64							// Maximum number of arguments for external commands
#define TIME_SIZE 32								// String length of time
#define MAX_PIPES 5								// Maximum number of piped commands

int main(void)
{
	char	command[BUFFER_SIZE],						// Stores input command
		pwd[BUFFER_SIZE],						// Stores working directory
		*argToken,							// Stores currently processed command argument
		*argList[MAX_ARGUMENTS],					// Stores argument list for external commands
		time[TIME_SIZE],						// Stores timestamp of files in string format
		*filename1, *filename2;						// Stores filename of files used in  copying, redirection, etc.
		
	int 	argIndex,							// Iterator for argList
		exitStatus,							// Stores exit status of forked external command
		src,								// Stores file descriptor of source file
		dest,								// Stores file descriptor of destination file
		pipeIndex,							// Iterator for pipeFD
		pipeFD[MAX_PIPES][2],						// Pipe for piped commands
		ifd,								// Saves standard input file descriptor
		hasInputRedirection,						// Flag for checking input redirection
		hasOutputRedirection,						// Flag for checking output redirection
		hasInputPipe,							// Flag for checking input pipe
		hasOutputPipe;							// Flag for checking output pipe
					
	off_t	offset;								// Stores the file offset while copying
	
	struct dirent	*dptr;							// Iterator pointer for directory entries
	struct stat	fileInfo;						// Stores file statistics
	struct passwd	*user;							// Stores file owner information
	struct group	*grp;							// Stores file group information
	struct tm	*timestamp;						// Stores file timestamp
	
	DIR	*dirp;								// Stores pointer to directory structure
	pid_t	pid;								// Stores process id of forked external command
	
	system("clear");
	while(1)
	{
		// Display prompt and accept input command
		printf("myshell> ");
		fflush(stdin);
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
		else if(!strcmp(argToken, "cp"))				// Command: cp <source filepath/name> <destination filepath/name>
		{								// Function: copy file from source to destination
			// Open source file for reading
			argToken = strtok(NULL, " ");
			if(argToken == NULL)
			{
				// Error handling for incorrect number of arguments
				fprintf(stderr, "Error: source file not given.\n");
				continue;
			}
			filename1 = strrchr(argToken, '/');
			if(filename1 == NULL)
			{
				// Source filepath is not given; set filename to source filename
				filename1 = argToken;
			}
			else
			{
				// Source filepath is given; set filename to point to
				// character after the last occurrence of '/'
				filename1 = filename1 + 1;
			}
			src = open(argToken, O_RDONLY);
			if(src == -1)
			{
				// Error handling for source file opening errors
				perror("Error: source file");
				continue;
			}
			fstat(src, &fileInfo);
			
			// Open destination file for writing
			argToken = strtok(NULL, " ");
			if(argToken == NULL)
			{
				// Error handling for incorrect number of arguments
				fprintf(stderr, "Error: destination file not given.\n");
				close(src);
				continue;
			}
			if(argToken[strlen(argToken) - 1] == '/')
			{
				// Concatenate the filename of source file to destination path
				// if destination filename is not given
				strcat(argToken, filename1);
			}
			dest = open(argToken, O_WRONLY | O_CREAT, fileInfo.st_mode);
			if(dest == -1)
			{
				// Error handling for destination file opening errors
				perror("Error: destination file");
				close(src);
				continue;
			}
			
			// Copy file contents from source to destination
			offset = 0;
			sendfile(dest, src, &offset, fileInfo.st_size);
			
			// Close files
			close(dest);
			close(src);
		}
		else if(!strcmp(argToken, "exit"))				// Command: exit
		{								// Function: Exit the shell
			break;
		}
		else								// Function: Execute other commands
		{
			hasInputPipe = 1;
			pipeIndex = 0;
			
			// Save the standard input file descriptor
			if((ifd = dup(0)) == -1)
			{
				// Error handling for file descriptor duplication errors
				perror("Error");
				close(dest);
				exit(1);
			}
			
			while(pipeIndex < MAX_PIPES && hasInputPipe == 1)
			{	
				// Create null terminated argument list
				argIndex = 0;
				
				if(pipeIndex != 0)
				{
					close(pipeFD[pipeIndex - 1][1]);
					if(dup2(pipeFD[pipeIndex - 1][0], STDIN_FILENO) == -1)
					{
						// Error handling for file descriptor duplication errors
						perror("Error");
						close(dest);
						exit(1);
					}
				}
				
				// Initialize redirection and pipe flags
				hasOutputRedirection = 0;
				hasInputRedirection = 0;
				hasOutputPipe = 0;
				hasInputPipe = 0;
			
				while(argToken != NULL)
				{
					if(!strcmp(argToken, "|"))			// Pipe: <command1> [arg1] ... | <command2> [arg1] ... | ...
					{
						hasOutputPipe = 1;
						pipe(pipeFD[pipeIndex]);
						
						argToken = strtok(NULL, " ");
						hasInputPipe = 1;
						break;
					}
					else if(!strcmp(argToken, "<"))			// Input Redirection: <command> < <intput filename>
					{
						hasInputRedirection = 1;
						filename1 = strtok(NULL, " ");
					
						argToken = strtok(NULL, " ");
						if(argToken != NULL && !strcmp(argToken, ">"))	// Both Input & Output Redirection: 
						{					// 	<command> < <input filename> > <output filename>					
							hasOutputRedirection = 1;	// NOTE: This will only work when input redirection is given before output redirection
							filename2 = strtok(NULL, " ");
						}
					
						break;
					}
					else if(!strcmp(argToken, ">"))			// Output Redirection: <command> > <output filename>
					{
						hasOutputRedirection = 1;
						filename2 = strtok(NULL, " ");
						break;
					}
				
					argList[argIndex++] = argToken;
					argToken = strtok(NULL, " ");
				}
				argList[argIndex] = NULL;
			
				// Create a child process to exec the input command
				pid = fork();
				if(pid == 0)
				{
					if(hasOutputPipe == 1)
					{
						close(pipeFD[pipeIndex][0]);
						if(dup2(pipeFD[pipeIndex][1], STDOUT_FILENO) == -1)
						{
							// Error handling for file descriptor duplication errors
							perror("Error");
							close(dest);
							exit(1);
						}
					}
				
					if(hasInputRedirection == 1)
					{
						src = open(filename1, O_RDONLY);
						if(src == -1)
						{
							// Error handling for destination file opening errors
							perror("Error: destination file");
							exit(1);
						}

						if(dup2(src, STDIN_FILENO) == -1)
						{
							// Error handling for file descriptor duplication errors
							perror("Error");
							close(src);
							exit(1);
						}
					}
				
					if(hasOutputRedirection == 1)
					{
						dest = open(filename2, O_WRONLY | O_CREAT, 0644);
						if(dest == -1)
						{
							// Error handling for destination file opening errors
							perror("Error: destination file");
							exit(1);
						}

						if(dup2(dest, STDOUT_FILENO) == -1)
						{
							// Error handling for file descriptor duplication errors
							perror("Error");
							close(dest);
							exit(1);
						}
					}
				
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
				
				// Increment pipe index
				pipeIndex++;
			}
			
			if((pipeIndex - 1) != 0)
			{				
				// Reset the standard input
				if(dup2(ifd, 0) == -1)
				{
					// Error handling for file descriptor duplication errors
					perror("Error");
					close(dest);
					exit(1);
				}
			}
		}
	}
	
	return 0;
}
