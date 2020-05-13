/* ********************************************************************************** 
 ** Program Name:   Program 3 - smallsh (main_smallsh.c)
 ** Author:         Susan Hibbert
 ** Date:           7th May 2020  			      
 ** Description:    Main function file for the smallsh program
 ** *******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

/* ********************************************************************************** 
 ** Description: Signal catcher for SIGINT (CTRL-C). When user enters CTRL-C, the
 		 foreground command is terminated (if one is running)
 ** Input(s): 	 Integer value representing a signal number
 ** Output(s): 	 
 ** Returns: 	 No return value
 ** *******************************************************************************/
void catchSIGINT(int sigNo)
{
	char* msg = "Caught SIGINT, terminating foreground command\n";
	write(STDOUT_FILENO, msg, 48);
	exit(0);
}


/* ********************************************************************************** 
 ** Description: Signal catcher for SIGTSTP (CTRL-Z)
 ** Input(s): 	 Integer value representing a signal number
 ** Output(s): 	 
 ** Returns: 	 No return value
 ** *******************************************************************************/
void catchSIGTSTP(int sigNo)
{
	char* msg = "Caught SIGTSTP, entering foreground only mode\n";
	write(STDOUT_FILENO, msg, 47);
	exit(0);
}

/* ********************************************************************************** 
 ** Description: Main function
 ** Input(s): 	 
 ** Output(s): 	 
 ** Returns: 	 
 ** *******************************************************************************/
int main(int argc, char* argv[])
{
	// SET UP SIGNAL HANDLERS 

	// initialize the sigaction structs
	struct sigaction SIGINT_action = {0};
	struct sigaction SIGTSTP_action = {0};

	SIGINT_action.sa_handler = catchSIGINT;		// point sa_handler function pointer to catchSIGINT function
	sigfillset(&SIGINT_action.sa_mask);		// delay all signals while this mask in place
	SIGINT_action.sa_flags = 0;			// do not set any flags


	SIGTSTP_action.sa_handler = catchSIGTSTP;	// point sa_handler function pointer to catchSIGSTP function
	sigfillset(&SIGTSTP_action.sa_mask);		// delay all signals while this mask in place
	SIGTSTP_action.sa_flags = 0;			// do not set any flags

	sigaction(SIGINT, &SIGINT_action, NULL);	// register signal handler for SIGNINT
	sigaction(SIGTSTP, &SIGTSTP_action, NULL);	// register signal handler for SIGSTP


	// GET COMMAND FROM PROMPT

	char* lineEntered = NULL; 	// points to a buffer allocated by getline() to hold input string
	size_t bufferSize = 0;		// holds how large the allocated buffer is 
	int numCharsEntered = 0;	// hold the number of chars entered by user
	int currChar = 0;		// tracks where we are when we print out every character

	pid_t backgroundPids[128];	// stores the pids of background processes
	int backgroundNum = 0;		// tracks number of background processes	
	int childExitMethod = -5;	// stores the result of how a child process exited

	while(1)
	{
		while(1)
		{
			printf(":");		//colon symbol used as a prompt for each command line

			// get user input
			numCharsEntered = getline(&lineEntered, &bufferSize, stdin);

			// if user did not enter any commands, reprompt user for another command
			if (lineEntered[0] == '\n')
			{
				continue;
			}

			// if user entered a command greater than 2048 characters, reprompt user for another command
			else if (numCharsEntered >= 2048)
			{
				printf("Command exceeds 2048 characters");
				fflush(stdout);		// flush output buffers after printing
				continue;
			}

			// if user entered more than 512 arguments, reprompt user
			else if (argc >= 512)
			{
				printf("Maximum number of arguments 512");
				fflush(stdout);		// flush output buffers after printing
				continue;
			}
			// if user entered a comment (line beginning with #), ignore and reprompt user for another command
			else if (lineEntered[0] == '#')
			{
				continue;
			}

			// otherwise exit the inner while loop as user entered valid prompt
			else
			{
				break;
			}
		}

		// break brings you to the outer while loop

		if (strcmp(lineEntered, "status\n") == 0)
		{
			// prints out the exit status or the terminating signal of the last foreground process
			if (WIFEXITED(childExitMethod) != 0)
			{
				printf("exit value %d\n", WEXITSTATUS(childExitMethod));		
			}
			else if (WIFSIGNALED(childExitMethod) != 0)
			{	
				printf("terminated by signal %d\n", WTERMSIG(childExitMethod));
			}	
			else
			{
				printf("exit status 0");
			}
		}

		// if cd command entered
		else if (strstr(lineEntered, "cd") != NULL)
		{
			// change to directory specified in HOME environment if cd entered with no arguments
			if (strcmp(lineEntered, "cd\n") == 0)
			{
				chdir(getenv("HOME"));
			}
			// change to the directory specified in lineEntered string
			else
			{
				// search through lineEntered for the specified directory
				char* newPath = strtok(lineEntered, " ");
				while (newPath != NULL)
				{
					// when you find the name of the directory in the lineEntered string
					// exit the loop
					if (strcmp(newPath, "cd") != 0)
					{
						break;
					}
					else
					{
						newPath = strtok(NULL, " ");
					}
				}
				
				// remove newline character from newPath before changing directory
				newPath[strlen(newPath) -1] = '\0';
				chdir(newPath);
			}
		}

		else if (strcmp(lineEntered, "exit\n") == 0)
		{
			// kill all processes with SIGKILL command (-9) in the same process group (pid == 0)
			execlp("kill", "kill", "-9", "0", ">", "/dev/null", NULL);
		}

		// non-built in commands
		else
		{
			// extract each argument entered by splitting up lineEntered, using a space
			// as a delimiter
			char* argPtr = strtok(lineEntered, " ");
			char* args[] = {""};
			int i = 0;
			int numArgs = 0;	// keep track of the number of arguments
			while (argPtr != NULL)
			{
				args[i] = argPtr;
				i++;
				numArgs++;
				argPtr = strtok(NULL, " ");
			}

			//when a non-built in command is received, the parent forks() off a child	
			pid_t spawnPid = -5;
			
			spawnPid = fork();
			// if an error occured in forking process
			if (spawnPid == -1)
			{
				perror("Unable to spawn child process\n");
				exit(1);
			}
			// child process created successfully
			else if (spawnPid == 0)
			{
				// IN CHILD PROCESS
				// child process carries out input/output redirection with dup2()
			
				// if user only entered one argument
				if (numArgs == 1)
				{
					// remove newline character from argument before calling execlp	
					args[numArgs - 1][strlen(args[numArgs - 1]) - 1] = '\0';
					execlp(args[0], args[0], NULL);
				
					// if non-built in command does not exist, display error message
					// and exit
					perror("Execlp did not work\n");
					exit(1);
					break;
					//set value retrieved by built-in status command to 1	
				}
				// if user entered more than 1 argument
				else
				{
					//remove newline character from last argument
					args[numArgs - 1][strlen(args[numArgs - 1]) - 1] = '\0';		
	
					// add NULL to end of arguments array prior to passing to execvp function
					args[numArgs] = NULL;
						
					// check if child process is to be run in the background
					if (strchr(args[numArgs - 1], '&') != NULL)
					{
						//pass in the arguments except the ampersand
						execlp(args[0], args[0], args[1], args[3]);
					}			
					else
					{
						execvp(args[0], args);
					}
					// if non-built in command does not exist, display error message
					// and exit
					perror("Execvp did not work\n");
					exit(1);
					break;	
					//set value retrieved by built-in status command to 1	
				}
				
			}			
			// IN PARENT PROCESS
					
			// check if child process is to be run in the background
			if (strchr(args[numArgs - 1], '&') != NULL)
			{
				// print out process id of background process when it begins
				printf("background pid is %d\n", spawnPid);

				// add this pid to backgroundPid array
				backgroundPids[backgroundNum] = spawnPid;
				// add one to the number of background processes
				backgroundNum++;	
			}
			else
			{
				// if child process is to be run in the foreground, block the parent until
				// the child process with the specified PID terminates
				waitpid(spawnPid, &childExitMethod, 0);
			}

			//check if any background processes have finished before prompting for new command
			int done = 0;
			int backgroundStatus = 0;
			for (i = 0; i < backgroundNum; i++)
			{
				done  = waitpid(backgroundPids[i], &childExitMethod, WNOHANG);
			
				// if a background process has finished
				if (done != 0)
				{
					// print out process ID and exit status of the terminated background process - if no exit status display its terminating signal
					backgroundStatus = WEXITSTATUS(&childExitMethod);
					printf("background pid %d is done: exit status %d\n", backgroundPids[i], backgroundStatus);
					//remove from backgroundPid array and subtract one from number of background processes
				}
			}
		}
	}	
	return 0;
}
