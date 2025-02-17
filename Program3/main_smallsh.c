/* ********************************************************************************** 
 ** Program Name:   Program 3 - smallsh (main_smallsh.c)
 ** Author:         Susan Hibbert
 ** Date:           20th May 2020  			      
 ** Description:    Program file for the smallsh program
 ** *******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

// global bool variable to check if SIGTSTP signal has been received and if the shell is
// only running foreground process
// 0 = Shell operating normally - background and foreground processes can be run
// 1 = SIGTSTP signal has been received - in foreground-only mode
int foregroundMode = 0;
	
pid_t backgroundPids[128];	// global variable which stores the pids of background processes

/* ********************************************************************************** 
 ** Description: Signal catcher for SIGTSTP (CTRL-Z) -  when the program receives a
		 SIGTSTP signal the parent process is directed to this signal handler
		 which puts the shell into foreground-process-only mode (child
		 processes are not affected). When a second SIGTSTP signal is received,
		 the shell will return to normal operation and processes can be run in
		 the background again
 ** Input(s): 	 Integer value representing a signal number
 ** Output(s): 	 Message is displayed on screen indicating whether program is entering
		 foreground-only mode or whether it is exiting foreground-only mode
 ** Returns: 	 No return value
 ** *******************************************************************************/
void catchSIGTSTP(int sigNo)
{
	// if receiving first SIGTSTP signal, put shell in foreground-only mode
	if (foregroundMode == 0)
	{
		char *msg = "\nEntering foreground-only mode (& is now ignored)\n:";
		write(STDOUT_FILENO, msg, 51);
		fflush(stdout);		// flush output buffers after printing
		foregroundMode = 1;
	}
	// if previously received one SIGTSTP signal and currently operating in foreground
	// only mode, switch back to normal shell operation
	else if (foregroundMode == 1)
	{	
		char* msg = "\nExiting foreground-only mode\n:";
		write(STDOUT_FILENO, msg, 32);
		fflush(stdout);		// flush output buffers after printing
		foregroundMode = 0;
	}
}


/* ********************************************************************************** 
 ** Description: Signal catcher for SIGINT (CTRL-C) - when a SIGINT signal is received
		 by the program the parent process and all background processes will be
		 directed to this signal handler which simply returns, as the parent
		 process and background processes should not be terminated by this
		 signa;
 ** Input(s): 	 Integer value representing a signal number
 ** Output(s): 	 No output 
 ** Returns: 	 No return value
 ** *******************************************************************************/
void catchSIGINT(int sigNo)
{
	// the parent shell process and any background processes will be directed here
	// when ctrl-c pressed is. Neither will be terminated by the signal and the
	// program will just return
	return;
}

/* ********************************************************************************** 
 ** Description: Main function - where the shell is implemented. User is prompted for
		 command line arguments and the shell will run these commands. This 
		 includes built-in commands (exit, cd, status) and non-built-in 
		 commands. The shell also allows for the redirection of standard input
		 and output and supports both foreground and background processes. It
		 can also support comments, which are lines beginning with the #
		 character
 ** Input(s): 	 No input
 ** Output(s): 	 User is prompted to enter command line arguments, the output of which
		 are displayed on the screen. Messages are displayed when background
		 processes are completed. Error messages are displayed on screen should
		 any errors arise during the operation of the shell (invalid command,
		 invalid file name etc).
		 If the shell receives a SIGTSTP or SIGINT signal, respective messages
		 appear on the screen informing the user of any changes to the shell's
		 operation.
 ** Returns: 	 Returns an exit status of 0
 ** *******************************************************************************/
int main(int argc, char* argv[])
{
	// SET UP SIGNAL HANDLERS FOR PARENT PROCESS 

	// initialize the sigaction structs
	struct sigaction SIGINT_action = {0};
	struct sigaction SIGTSTP_action = {0};

	SIGINT_action.sa_handler = catchSIGINT;		// parent process will ignore the SIGINT signal
	sigfillset(&SIGINT_action.sa_mask);		// delay all signals while this mask in place
	SIGINT_action.sa_flags = SA_RESTART;		// SA_RESTART flag restarts system calls automatically


	SIGTSTP_action.sa_handler = catchSIGTSTP;	// point sa_handler function pointer to catchSIGSTP function
	sigfillset(&SIGTSTP_action.sa_mask);		// delay all signals while this mask in place
	SIGTSTP_action.sa_flags = SA_RESTART;		// SA_RESTART flag restarts system calls automatically

	sigaction(SIGINT, &SIGINT_action, NULL);	// register signal handler for SIGNINT
	sigaction(SIGTSTP, &SIGTSTP_action, NULL);	// register signal handler for SIGTSTP


	// GET COMMAND LINE ARGUMENTS FROM PROMPT

	char* lineEntered = NULL; 	// points to a buffer allocated by getline() to hold input string
	size_t bufferSize = 0;		// holds how large the allocated buffer is 
	int numCharsEntered = 0;	// hold the number of chars entered by user

	int backgroundNum = 0;		// tracks number of background processes	
	int runBackground = 0;		// bool indicatting whether process should be run in background (0 = no, 1 = yes)
	int childExitMethod = -5;	// stores the result of how a child process exited

	while(1)
	{
		while(1)
		{
			//colon symbol used as a prompt for each command line
			printf(":");		

			// get user input
			numCharsEntered = getline(&lineEntered, &bufferSize, stdin);

			// INPUT VALIDATION

			// if user did not enter any commands or entered a comment (line beginning with #), 
			// ignore and reprompt user for another command, after checking if any background
			// processes have finished
			if (lineEntered[0] == '\n' || lineEntered[0] == '#')
			{
				//check if any background processes have finished before prompting for new command
				int i;
				int done = 0;
				int backgroundStatus = 0;
				for (i = 0; i < backgroundNum; i++)
				{
					done  = waitpid(backgroundPids[i], &childExitMethod, WNOHANG);
			
					// if a background process has finished
					if (done != 0)
					{
						// prints out the exit status
						if (WIFEXITED(childExitMethod) != 0)
						{
							printf("background pid %d is done: exit value %d\n", backgroundPids[i], WEXITSTATUS(childExitMethod));		
							fflush(stdout);		// flush output buffers after printing
						}
						// print out the terminating signal
						else if (WIFSIGNALED(childExitMethod) != 0)
						{	
							printf("background pid %d is done: terminated by signal %d\n", backgroundPids[i], WTERMSIG(childExitMethod));
							fflush(stdout);		// flush output buffers after printing
						}	
					
						//remove pid of background process from backgroundPid array and subtract one from number of background processes
						backgroundPids[i] = 0;
						backgroundNum--;
					}
				}
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
			// otherwise exit the inner while loop as user entered valid prompt
			else
			{
				break;
			}
		}

		// break brings you to the outer while loop - valid input entered
		
		// BUILT-IN COMMANDS
		// STATUS
		if (strstr(lineEntered, "status") != NULL)
		{
			// remove & from lineEntered so this command cannot be run in the background
			if (strstr(lineEntered, "&") != NULL)
			{
				char* remove = strstr(lineEntered, "&");
				strcpy(remove, "\0");	
			} 
			// print out the exit status or the terminating signal of the last foreground process
			if (WIFEXITED(childExitMethod) != 0)
			{
				printf("exit value %d\n", WEXITSTATUS(childExitMethod));		
				fflush(stdout);		// flush output buffers after printing
			}
			else if (WIFSIGNALED(childExitMethod) != 0)
			{	
				printf("terminated by signal %d\n", WTERMSIG(childExitMethod));
				fflush(stdout);		// flush output buffers after printing
			}	
			// if command is run before any foreground command is run then return exit status of 0
			else
			{
				printf("exit status 0");
				fflush(stdout);		// flush output buffers after printing
			}
		}

		// CD
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
				char* args[] = {""};
				int i = 0;
				long int mypid = getpid();		// get process ID to replace any instances of $$ in command
				char* pid = malloc(sizeof(long int));		
				sprintf(pid, "%ld",(long)getpid());
				strcat(pid, "\n");			// new line char will be removed later, added for continuity
				while (newPath != NULL)
				{
					// replace any instances of $$ at the end of the path with process ID of shell
					if (strstr(newPath, "$$\n") != NULL)
					{
						args[i] = malloc(128*sizeof(char));
						newPath[(strlen(newPath)) - 3] = '\0';
						strcpy(args[i], newPath);
						strcat(args[i], pid);
						break;
					}
					// when you find the name of the directory in the lineEntered string exit the loop
					else if (strcmp(newPath, "cd") != 0)
					{
						args[i] = newPath;
						break;
					}
					else
					{
						newPath = strtok(NULL, " ");
						i++;
					}
				}
				
				// remove newline character from newPath before changing directory
				args[i][strlen(args[i]) - 1] = '\0';
				chdir(args[i]);
			}
		}

		// EXIT
		else if (strcmp(lineEntered, "exit\n") == 0)
		{	
			// kill all processes with SIGKILL command (-9) in the same process group (pid == 0)
			execlp("kill", "kill", "-9", "0","&>", "/dev/null",  NULL);
		}

		// NON BUILT-IN COMMANDS
		else
		{
			// extract each argument entered by user by splitting up lineEntered, using a space
			// as a delimiter
			char* argPtr = strtok(lineEntered, " ");
			char* args[] = {""};
			int i = 0;
			int numArgs = 0;			// keep track of the number of arguments
			char* pid = malloc(sizeof(long int));		
			sprintf(pid, "%ld",(long)getpid());	// get process ID to replace any instances of $$ in command
			int inputRedir = 0;			// bool to check if there is to be input redirection (0 = no, 1 = yes)
			int outputRedir = 0;			// bool to check if there is to be output redirection (0 = no, 1 = yes)
			int outputIdx = 0;			// index of output redirection operator
			int inputIdx = 0;			// index of input redirection operator

			while (argPtr != NULL)
			{
				// replace any instances of $$ in a command with process ID of shell
				if (strstr(argPtr, "$$") != NULL)
				{
					// if $$ is its own argument at the end of the line
					if (strcmp(argPtr, "$$\n") == 0)
					{
						strcat(pid, "\n");
						args[i] = pid;
					}
					// if $$ is at the end of an argument
					else
					{
						args[i] = malloc(128*sizeof(char));
						argPtr[(strlen(argPtr)) - 3] = '\0';
						strcpy(args[i], argPtr);
						strcat(pid, "\n");
						strcat(args[i], pid);
					}
				}
				// check if input redirection operator entered <
				else if (strchr(argPtr, '<') != NULL)
				{
					// switch bool value to true
					inputRedir = 1;
					
					// save index of <
					inputIdx = i;
				} 
				// check if output redirection operator entered >
				else if (strchr(argPtr, '>') != NULL)
				{
					// switch bool value to true
					outputRedir = 1;
					
					// save index of >
					outputIdx = i;
				}
				else
				{
					args[i] = argPtr;
				}
				i++;
				numArgs++;
				argPtr = strtok(NULL, " ");
			}
			
			// check if process is to be run in the background
			if (strcmp(args[numArgs - 1], "&\n") == 0)
			{
				// switch bool to true i.e. run in background
				runBackground = 1;
			}

			//when a non-built in command is received, the parent forks() off a child	
			pid_t spawnPid = -5;
			
			spawnPid = fork();
			// if an error occured in forking process
			if (spawnPid == -1)
			{
				perror("Unable to spawn child process\n");
				fflush(stderr);		// flush output buffers after printing
				exit(1);
			}
			// child process created successfully
			else if (spawnPid == 0)
			{
				// IN CHILD PROCESS
		
				// SET UP SIGNAL HANDLERS FOR CHILD PROCESS

				// initialize the sigaction structs
				struct sigaction SIGINT_child_action = {0};
				struct sigaction SIGTSTP_child_action = {0};

				// set child processes to react to the default behaviour of the SIGINT signal (i.e. terminate)	
				SIGINT_child_action.sa_handler = SIG_DFL;		
				sigfillset(&SIGINT_child_action.sa_mask);		// delay all signals while this mask in place
				SIGINT_action.sa_flags = 0;			


				// set child processes to ignore if the SIGTSTP signal is received
				SIGTSTP_child_action.sa_handler = SIG_IGN;	
				sigfillset(&SIGTSTP_child_action.sa_mask);		// delay all signals while this mask in place
				SIGTSTP_child_action.sa_flags = 0;

				sigaction(SIGINT, &SIGINT_child_action, NULL);		// register signal handler for SIGNINT
				sigaction(SIGTSTP, &SIGTSTP_child_action, NULL);	// register signal handler for SIGTSTP

				// if user only entered one argument
				if (numArgs == 1)
				{
					// remove newline character from argument before calling execlp	
					args[numArgs - 1][strlen(args[numArgs - 1]) - 1] = '\0';
					execlp(args[0], args[0], NULL);
				
					// if non-built in command does not exist, display error message
					// and exit
					printf("%s: no such file or directory\n", args[0]);
					fflush(stdout);		// flush output buffers after printing
					//set exit status to 1	
					exit(1);
					break;
				}
				// if user entered more than 1 argument
				else
				{
					// check if user entered any redirection symbols
					if (inputRedir == 1 || outputRedir == 1)
					{	
						// if redirecting both stdout and stdin
						 if (inputRedir == 1 && outputRedir == 1)
						{
							// remove newline character from last argument
							args[numArgs - 1][strlen(args[numArgs - 1]) - 1] = '\0';

							// open the output file for writing only, create it if it doesn't exist and
							// write over the contents if it does exist
							int output = open(args[outputIdx + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
						
							// open the input file for reading only
							int input = open(args[inputIdx + 1], O_RDONLY);
							
							// if either file cannot be opened, display error message, exit status will be set to 1
							// by default							
							if (input == -1 || output == -1)
							{
								perror("Could not open input or output file\n");														
								fflush(stderr);		// flush output buffers after printing
							}

							// redirect stdout to output to file specified and redirect stdin to read from file specified
							int redirect1 = dup2(output, 1);
							int redirect0 = dup2(input, 0);

							if (redirect0 == -1 || redirect1 == -1)							
							{
								perror("Error with input/output redirection\n");
								fflush(stderr);		// flush output buffers after printing
								exit(1);
							}
							
							//copy all the arguments except for the redirection symbols and input and output file names
							int i;
							char* validargs[8];
							int newIdx = 0;
							
							// find the index of where to copy to - copy up to the index of the input or output redirection
							// operator, whichever appears first
							if (inputIdx < outputIdx)
							{	
								newIdx = inputIdx;
							}			
							else
							{
								newIdx = outputIdx;			
							}

							for (i = 0; i < newIdx; i++)
							{
								validargs[i] = malloc(sizeof(char));
								strcpy(validargs[i], args[i]);
							}
							// add NULL to end of arguments array prior to passing to execvp function
							validargs[newIdx] = NULL;

							//pass in the arguments except the redirection symbol and file names
							execvp(validargs[0], validargs);
							printf("%s: no such file or directory\n", validargs[0]);
							fflush(stdout);		// flush output buffers after printing
							//set exit status to 1	
							exit(1);
							break;
						}
						// if redirecting stdout
						else if (outputRedir == 1)
						{
							// remove newline character from last argument
							args[numArgs - 1][strlen(args[numArgs - 1]) - 1] = '\0';

							// open the output file for writing only, create it if it doesn't exist and
							// write over the contents if it does exist
							int output = open(args[outputIdx + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
						
							// if file cannot be opened, display error message, exit status will be set to 1
							// by default
							if (output == -1)
							{
								printf("Could not open %s for output\n", args[outputIdx + 1]);														
								fflush(stdout);		// flush output buffers after printing
							}

							// redirect stdout to file specified
							int redirect1 = dup2(output, 1);
							if (redirect1 == -1)							
							{
								perror("Error with output redirection\n");
								fflush(stderr);		// flush output buffers after printing
								exit(1);
							}

							//copy all the arguments except for the redirection symbol and output file name
							int i;
							char* validargs[8];
								
							for (i = 0; i < outputIdx; i++)
							{
								validargs[i] = malloc(sizeof(char));
								strcpy(validargs[i], args[i]);
							}
							// add NULL to end of arguments array prior to passing to execvp function
							validargs[outputIdx] = NULL;

							//pass in the arguments except the redirection symbol and output file name
							execvp(validargs[0], validargs);
							perror("Cannot understand command!\n");
							fflush(stderr);		// flush output buffers after printing
							//set exit status to 1	
							exit(1);
							break;
						
						}
						// if redirecting stdin
						else if (inputRedir == 1)
						{
							// remove newline character from last argument
							args[numArgs - 1][strlen(args[numArgs - 1]) - 1] = '\0';

							// open the input file for reading only
							int input = open(args[inputIdx + 1], O_RDONLY);
							
							//ensure the file pointer is at the start of the file
							lseek(input, 0, SEEK_SET);

							// if file cannot be opened, display error message, exit status will be set to 1
							// by default							
							if (input == -1)
							{
								printf("Could not open %s for input\n", args[inputIdx + 1]);														
								fflush(stdout);		// flush output buffers after printing
							}
	
							// redirect stdin to read from file specified
							int redirect0 = dup2(input, 0);		
							if (redirect0 == -1)							
							{
								perror("Error with input redirection\n");
								fflush(stderr);		// flush output buffers after printing
								exit(1);
							}
							//copy all the arguments except for the redirection symbol and input file name
							int i;
							char* validargs[8];
								
							for (i = 0; i < inputIdx; i++)
							{
								validargs[i] = malloc(sizeof(char));
								strcpy(validargs[i], args[i]);
							}
							// add NULL to end of arguments array prior to passing to execvp function
							validargs[inputIdx] = NULL;

							//pass in the arguments except the redirection symbol and input file name
							execvp(validargs[0], validargs);
							perror("Cannot understand command!\n");
							//set exit status to 1	
							exit(1);
							break;
						}
					}
					
					// user did not enter any redirection symbols				
					else
					{
						//remove newline character from last argument
						args[numArgs - 1][strlen(args[numArgs - 1]) - 1] = '\0';		
	
						// add NULL to end of arguments array prior to passing to execvp function
						args[numArgs] = NULL;
						
						// check if child process is to be run in the background
						if (runBackground == 1)
						{	
							// SET UP SIGNAL HANDLERS FOR BACKGROUND PROCESS
							// tell the background process to ignore the SIGINT signal
							struct sigaction SIGINT_background = {0};

							SIGINT_background.sa_handler = SIG_IGN;		
							sigfillset(&SIGINT_background.sa_mask);		// delay all signals while this mask in place
							SIGINT_background.sa_flags = 0;			
							sigaction(SIGINT, &SIGINT_background, NULL);

							int redirect;
							int target = open("/dev/null", O_WRONLY);

							// if no output redirection given for a background command,
							// redirect their output to /dev/null
							if (outputRedir == 0)
							{
								redirect = dup2(target, 1);
								if (redirect == -1)
								{
									perror("Error with output redirection\n");
									fflush(stderr);		// flush output buffers after printing
									exit(1);
								}
							}
												
							// if no input redirection given for a background command,
							// redirect their input to /dev/null
							if (inputRedir == 0)
							{
								redirect = dup2(target, 0);
								if (redirect == -1)
								{
									perror("Error with input redirection\n");
									fflush(stderr);		// flush output buffers after printing
									exit(1);
								}
							}					
		
							//pass in the arguments except the ampersand
							execlp(args[0], args[0], args[1], args[3]);
						}			
						else
						{
							execvp(args[0], args);
						}
						// if non-built in command does not exist, display error message
						// and exit
						perror("Cannot understand command!\n");
						fflush(stderr);		// flush output buffers after printing
						//set exit status to 1
						exit(1);
						break;
					}	
				}
				
			}			
			// IN PARENT PROCESS
				
			// check if child process is to be run in the background
			if (runBackground == 1)
			{

				// if no SIGTSTP signal has been received, put process in background
				if (foregroundMode == 0)
				{
					// print out process id of background process when it begins
					printf("background pid is %d\n", spawnPid);
					fflush(stdout);		// flush output buffers after printing

					// add this pid to backgroundPid array
					backgroundPids[backgroundNum] = spawnPid;
					// add one to the number of background processes
					backgroundNum++;	
				}
				// if SIGTSTP signal has been received, in foreground-only mode
				else
				{
					// run child process in the foreground, and block the parent until
					// the child process terminates
					waitpid(spawnPid, &childExitMethod, 0);
					
					// if child process was terminated by a signal, the parent prints out the number
					// of the signal that killed the foreground child process
	
					if (WIFSIGNALED(childExitMethod) != 0)
					{	
						printf("terminated by signal %d\n", WTERMSIG(childExitMethod));
						fflush(stdout);		// flush output buffers after printing
					}	
				}
					
			}
			else
			{
				// if child process is to be run in the foreground, block the parent until
				// the child process with the specified PID terminates
				waitpid(spawnPid, &childExitMethod, 0);
				
				// if child process was terminated by a signal, the parent prints out the number
				// of the signal that killed the foreground child process

				if (WIFSIGNALED(childExitMethod) != 0)
				{	
					printf("terminated by signal %d\n", WTERMSIG(childExitMethod));
					fflush(stdout);		// flush output buffers after printing
				}	

			}
			
			
			// switch bool back to false
			runBackground = 0;
			
			//check if any background processes have finished before prompting for new command
			int done = 0;
			int backgroundStatus = 0;
			for (i = 0; i < backgroundNum; i++)
			{
				done  = waitpid(backgroundPids[i], &childExitMethod, WNOHANG);
			
				// if a background process has finished
				if (done != 0)
				{
					// print out the exit status
					if (WIFEXITED(childExitMethod) != 0)
					{
						printf("background pid %d is done: exit value %d\n", backgroundPids[i], WEXITSTATUS(childExitMethod));		
						fflush(stdout);		// flush output buffers after printing
					}
					// print out the terminating signal
					else if (WIFSIGNALED(childExitMethod) != 0)
					{	
						printf("background pid %d is done: terminated by signal %d\n", backgroundPids[i], WTERMSIG(childExitMethod));
						fflush(stdout);		// flush output buffers after printing
					}	
				
					//remove from backgroundPid array and subtract one from number of background processes
					backgroundPids[i] = 0;
					backgroundNum--;
				}
			}
		}
		fflush(stdout); 	// flush stdout between commands
	}	
	return 0;
}	
