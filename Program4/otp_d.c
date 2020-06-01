/* ********************************************************************************** 
 ** Program Name:   Program 4 - Dead Drop (otp_d.c)
 ** Author:         Susan Hibbert
 ** Date:           26th May 2020  			      
 ** Description:    SERVER
 ** *******************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include <dirent.h>
#include <fcntl.h>


void error(const char *msg) { perror(msg); exit(1); } // Error function used for reporting issues

int main(int argc, char *argv[])
{
	srand(time(0));			// seed random number generator - used later when creating files
	//int noProcesses = 0;		// to keep track of the number of child processes (max 5)
	int listenSocketFD, establishedConnectionFD, portNumber, charsRead;
	socklen_t sizeOfClientInfo;
	char buffer[256];
	struct sockaddr_in serverAddress, clientAddress;

	if (argc < 2) { fprintf(stderr,"USAGE: %s port\n", argv[0]); exit(1); } // Check usage & args

	// Set up the address struct for this process (the server)
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[1]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process

	// Set up the socket
	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (listenSocketFD < 0) error("ERROR opening socket");

	// Enable the socket to begin listening
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to port
		error("ERROR on binding");
	listen(listenSocketFD, 5); // Flip the socket on - it can now receive up to 5 connections

	// Accept a connection, blocking if one is not available until one connects
	while(1)
	{
		sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect
		establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
		if (establishedConnectionFD < 0) error("ERROR on accept");

		// create a child process
		pid_t spawnPid = -5;
		spawnPid = fork();
	
		// if an error occurred when forking a child
		if (spawnPid == -1)
		{
			perror("Error spawning child process\n");
		}		
		else if (spawnPid == 0)
		{
			// Child process closes the listening socket
			close(listenSocketFD); 

			// Child process sleeps for 2 seconds
			sleep(2);

			// Read the client's message from the socket
			memset(buffer, '\0', 256);
			charsRead = recv(establishedConnectionFD, buffer, 255, 0);
			if (charsRead < 0) error("ERROR reading from socket");

			// Extract mode, user and encrypted message sent by the client
			char mode[16];
			char user[32];
			char encryptedMsg[1024];
			memset(encryptedMsg, '\0', sizeof(encryptedMsg));
			char* token;

			token = strtok(buffer, "-");
			int field = 0;

			while (token != NULL)
			{
				if (field == 0)
				{
					strcpy(user, token);
				}
				else if (field == 1)
				{
					strcpy(mode, token);
				}
				else if (field == 2)
				{
					strcpy(encryptedMsg, token);
				}
				field++;
				token = strtok(NULL, "-");		
			}
			// *******************************************************************************************
			// POST MODE
			if (strcmp(mode, "post") == 0)
			{
				// add a newline char to end of encrypted message, so the encrypted
				// text file ends with a newline character as per the specifications
				int msgLength = strlen(encryptedMsg);
				encryptedMsg[msgLength] = '\n';
				
				// create and open a directory for the user
				mkdir(user, 0755);
				DIR* userDir = opendir(user);
		
				// file name to store encrypted message
				char cipherText[64];
				char* fileName = "cipherText";

				// generate a random number between 0 and 100 and append to the end of the file name
				int randNo = rand() % 100;
				sprintf(cipherText, "%s%d", fileName, randNo);

				// create a filepath variable and store the path to the user's directory
				char* filepath = malloc(128*sizeof(char));
				sprintf(filepath, "./%s/%s", user, cipherText);
				int filedescriptor = open(filepath, O_RDWR | O_CREAT | O_TRUNC, 0600);
			
				// write the encrypted message to a file
				write(filedescriptor, encryptedMsg, strlen(encryptedMsg)*sizeof(char));		
	
				// print the path to the encrypted file	
				printf("%s\n", filepath);
				fflush(stdout);
					
				// reset the file pointer back to the beginning of the file
				lseek(filedescriptor, 0, SEEK_SET);

				// close the user directory
				closedir(userDir);

				// exit from the child process
				exit(0);
			}	
	
			// *******************************************************************************************
			// GET MODE
			else if (strcmp(mode, "get") == 0)
			{
			// retrieve the contents of the oldest file for the user and send them to otp
			// then delete the ciphertext file
				exit(0);
			}
		}	
			
		// if spawnPid is not equal to 0	
		else
		{
			close(establishedConnectionFD);
		}	

	}
	
	return 0;
}
