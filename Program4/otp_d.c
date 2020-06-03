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

#define MAX_SIZE 100000

void error(const char *msg) { perror(msg); exit(1); } // Error function used for reporting issues


/* ********************************************************************************** 
 ** Description:
 ** Input(s): 	File descriptor number representing new client connection
 ** Output(s): 	 
 ** Returns: 
 ** *******************************************************************************/

void childCon(int newConnFD)
{
	// Child process sleeps for 2 seconds
	sleep(2);	

	// Read the client's message from the socket
	char buffer[MAX_SIZE];
	int charsRead;
	memset(buffer, '\0', MAX_SIZE);
	charsRead = recv(newConnFD, buffer, 1023, 0);
	if (charsRead < 0) error("ERROR reading from socket");

	// Extract mode, user and encrypted message sent by the client
	char mode[16];
	char user[32];
	char encryptedMsg[MAX_SIZE];
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
		// create a copy of the encrypted message with a newline char at the end, so the encrypted
		// text file ends with a newline character as per the specifications
		int msgLength = strlen(encryptedMsg);
		char* encryptedMsg1 = malloc(msgLength + 2);
		strcpy(encryptedMsg1, encryptedMsg);
		strcat(encryptedMsg1, "\n");

		//printf("encrypted msg received at server plus newline = %s", encryptedMsg1);
		//fflush(stdout);
		
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
	
		//printf("encrypted msg received at server: %s\n", encryptedMsg1);
		//fflush(stdout);

		// write the encrypted message to a file
		write(filedescriptor, encryptedMsg1, strlen(encryptedMsg1)*sizeof(char));		

		// print the path to the encrypted file	
		printf("%s\n", filepath);
		fflush(stdout);
			
		// reset the file pointer back to the beginning of the file
		lseek(filedescriptor, 0, SEEK_SET);

		// close the user directory
		closedir(userDir);
	}	
	// *******************************************************************************************
	// GET MODE
	else if (strcmp(mode, "get") == 0)
	{
	// retrieve the contents of the oldest file for the user and send them to otp
	// then delete the ciphertext file
	
		time_t oldestTime = time(0);				// get current time to initialize oldestTime variable
		
		DIR* userDir = opendir(user);				// open user's directory
		struct dirent* userFile;				// holds information about a file in user's directory
		struct stat fileStats;					// hold stats about userFile
		char targetDirPrefix[64] = "cipherText";		// prefix of each encrypted text file
		char* oldestFile = malloc(sizeof(char)*MAX_SIZE);		// holds the name of the oldest file
		memset(oldestFile, '\0', MAX_SIZE);
		char fullpath[MAX_SIZE];
		memset(fullpath, '\0', MAX_SIZE);

		// if a directory for that user does not exit
		if (userDir == NULL)
		{
			fprintf(stderr, "SERVER: User has no encrypted messages\n");
		}
		// if user's directory can be opened
		else
		{
			// check each file in the directory
			while((userFile = readdir(userDir)) != NULL)
			{
				// if the file name has the desired prefix
				if (strstr(userFile->d_name, targetDirPrefix) != NULL)
				{
					// store full path to file in user's directory
					sprintf(fullpath, "./%s/%s", user, userFile->d_name);

					// get stats about the file and store in fileStats struct
					stat(fullpath, &fileStats);
		
					// if the timestamp on this file is the oldest so far, save
					// its name and timestamp
					if ((int)fileStats.st_mtime < oldestTime)
					{
						oldestTime = fileStats.st_mtime;			
						strcpy(oldestFile, fullpath);
					}
				}
			}
	
			// close user directory
			closedir(userDir);
			
			char fileToClient[MAX_SIZE];
		
			memset(fileToClient, '\0', MAX_SIZE);

			// if user has no encrypted messages, send 'none' to the client who
			// will display an error message
			if (strlen(oldestFile) == 0)
			{
				sprintf(fileToClient, "%s", "none");
			}
			else	
			{
				printf("oldest file name %s\n", oldestFile);
				fflush(stdout);	

				// store the contents of the oldest file in a string and send back to client
				FILE *filePtr;		// file stream pointer for encrypted file

				// open the oldest file for reading
				filePtr = fopen(oldestFile, "r");
	
				// holds a line of the file as it is read in
				char line[MAX_SIZE];
				memset(line, '\0', MAX_SIZE);

				// if error opening the file
				if (filePtr == NULL)
				{
					fprintf(stderr, "SERVER: Error opening user file\n");
					exit(1);
				}	
				// read the file into its array until reach end of file
				while(fgets(line, sizeof(line), filePtr) != NULL)
				{
					sprintf(fileToClient, "%s%s", fileToClient, line);
				}
				// remove the trailing '\n' that fgets adds
				fileToClient[strcspn(fileToClient, "\n")] = '\0';
	
				// close the file pointer
				fclose(filePtr);
	
				// delete the encrypted file
				remove(oldestFile);
			}
			// get size of encrypted message
			int msgSize = strlen(fileToClient);	

			//printf("msg to send: %s\n", fileToClient);
			//fflush(stdout);
			//printf("msg to send: size %d\n", msgSize);
			//fflush(stdout);

			// send encrypted file contents (or the word 'none') client		
			charsRead = send(newConnFD, fileToClient, strlen(fileToClient), 0);
			//printf("sending encrypted message, sent %d\n", charsRead);
			//fflush(stdout);
			
			if (charsRead < 0) perror("ERROR writing to socket\n");					
		}
	}		

	// close existing socket which is connected to the client	
	close(newConnFD);
}


/* ********************************************************************************** 
 ** Description: Main function
 ** Input(s): 	
 ** Output(s): 	 
 ** Returns: 
 ** *******************************************************************************/

int main(int argc, char *argv[])
{
	srand(time(0));							// seed random number generator - used later when creating files
	//int noProcesses = 0;						// to keep track of the number of child processes (max 5)
	int listenSocketFD, establishedConnectionFD, portNumber, charsRead;
	socklen_t sizeOfClientInfo;
	char buffer[MAX_SIZE];
	struct sockaddr_in serverAddress, clientAddress;

	// Check usage and args
	if (argc < 2) { fprintf(stderr,"USAGE: %s port\n", argv[0]); exit(1); }

	// Set up the address struct for this process (the server)
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); 	// Clear out the address struct
	portNumber = atoi(argv[1]); 					// Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; 				// Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); 			// Store the port number
	serverAddress.sin_addr.s_addr = INADDR_ANY; 			// Any address is allowed for connection to this process

	// Create and set up the socket
	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); 
	if (listenSocketFD < 0) error("ERROR opening socket");

	// Enable the socket to begin listening and connect socket to port
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
		error("ERROR on binding");
	listen(listenSocketFD, 5); 					// Flip the socket on - it can now receive up to 5 connections
	sizeOfClientInfo = sizeof(clientAddress); 			// Get the size of the address for the client that will connect


	// Accept a connection, blocking if one is not available until one connects
	while(1)
	{	
		establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
		if (establishedConnectionFD < 0) error("ERROR on accept");

		// create a new child process
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

			// pass file descriptor of new connection as argument
			childCon(establishedConnectionFD);

			// exit once child process returns and re-enter outer infinite while loop
			exit(0);	
	
		}		
		// if spawnPid is not equal to 0	
		else
		{
			close(establishedConnectionFD);
		}	

	}

	close(listenSocketFD);
	return 0;
}
