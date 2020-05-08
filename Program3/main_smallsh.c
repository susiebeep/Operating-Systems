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
int main(void)
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


	// sleep, but wake up to signals
	//while(1)
	//{
	//	pause();
	//}

	//	GET COMMAND FROM PROMPT

	char* lineEntered = NULL; 	// points to a buffer allocated by getline() to hold input string
	size_t bufferSize = 0;		// holds how large the allocated buffer is 
	int numCharsEntered = 0;	// hold the number of chars entered by user
	int currChar = 0;		// tracks where we are when we print out every character

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

			// if user entered a comment (line beginning with #), ignore and reprompt user for another command
			else if (lineEntered[0] == '#')
			{
				continue;
			}

			// otherwise exit the inner while loop as have valid prompt
			else
			{
				break;
			}
		}

		// break brings you to the outer while loop

		if (strcmp(lineEntered, "status\n") == 0)
		{
			// prints out the exit status or the terminating signal of the last foreground process
			system("exitStatus=$(echo $?);echo exit value $exitStatus"); 
		}

		// if cd command entered without arguments
		else if (strcmp(lineEntered, "cd\n") == 0)
		{
			// change to home directory
			system("cd ~");
		}

		else if (strcmp(lineEntered, "exit\n") == 0)
		{
			// kill all processes in the same process group
			execlp("kill", "kill", "-9", "0", ">", "/dev/null", NULL);
			//return 0;
		}

		// all other commands
		else
		{
			system(lineEntered);
		}
	}	

}
