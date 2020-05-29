/* ********************************************************************************** 
 ** Program Name:   Program 4 - Dead Drop (otp.c)
 ** Author:         Susan Hibbert
 ** Date:           26th May 2020  			      
 ** Description:    CLIENT 
 ** *******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

void error(const char *msg) { perror(msg); exit(0); } // Error function used for reporting issues

/* ********************************************************************************** 
 ** Description: Encrypt function
 ** Input(s): 	
 ** Output(s): 	 
 ** Returns: 
 ** *******************************************************************************/
void encrypt(char* file, char* key, char* cipherMsg)
{
	// file stream pointer for plaintext file and key file
	FILE* filePtr;
	FILE* keyPtr;

	// char arrays to hold the file and key contents
	char fileArr[256];
	char keyArr[256];
	
	// hold a line of the file as it is read in
	char line[256];

	// open the two files
	filePtr = fopen(file, "r");
	keyPtr = fopen(key, "r");

	// if there is an error opening either file
	if (filePtr == NULL || keyPtr == NULL)
	{
		fprintf(stderr, "CLIENT: Error opening file\n");	
		exit(1);
	}
	
	// read each file into its respective array until reach the end of the file
	while(fgets(line, sizeof(line), filePtr) != NULL)
	{
		sprintf(fileArr, line);
	}
	fileArr[strcspn(fileArr, "\n")] = '\0'; // Remove the trailing \n that fgets adds

	while(fgets(line, sizeof(line), keyPtr) != NULL)
	{
		sprintf(keyArr, line);
	}
	keyArr[strcspn(keyArr, "\n")] = '\0'; // Remove the trailing \n that fgets adds

	// NEED TO REMOVE \N FROM BOTH FILES

	// close the file pointers
	fclose(filePtr);
	fclose(keyPtr);
	
	// get the size of the two files
	int keyLength = strlen(keyArr);
	int fileLength = strlen(fileArr);
	
	// strip off the newline chars at the end of both files
	keyArr[keyLength - 1] = '\0';	
	fileArr[fileLength - 1] = '\0';	

	// subtract one from the length of both files after removing the newline char
	keyLength--;
	fileLength--;
	

	// check the length of the key is long enough for the plaintext file
	if (fileLength != keyLength)
	{
		fprintf(stderr, "CLIENT: Plaintext and key files are different lengths\n");
		exit(1);
	}

	// create two integer arrays to hold the alphanumeric values of the letters in the key and
	// plaintext files
	int keyMap[keyLength];
	int fileMap[keyLength];
	int i;
	
	for (i = 0; i < keyLength; i++)
	{
		// if the character is a space, directly assign it the value 26
		if (keyArr[i] == 32 || fileArr[i] == 32)
		{
		
			if (keyArr[i] == 32 && fileArr[i] == 32)
			{
				keyMap[i] = 26;
				fileMap[i] = 26;
			}
			else if (keyArr[i] == 32)
			{
				keyMap[i] = 26;
				fileMap[i] = fileArr[i] - 65;
			}
			else
			{
				fileMap[i] = 26;
				keyMap[i] = keyArr[i] - 65;
			}
		}
		else
		{
			// subtract 65 from the char's ASCII decimal value to get its alphanumerical value (A = 0; B = 1 etc)
			// and store in its respective array
			keyMap[i] = keyArr[i] - 65;
			fileMap[i] = fileArr[i] - 65;
		}
	
	}	

	// sum together each corresponding number in the two arrays then perform a modulus 27 on
	// their result sum
	int sumMap[keyLength];
	int temp = 0;

	for (i = 0; i < keyLength; i++)
	{
		temp = keyMap[i] + fileMap[i];
		sumMap[i] = temp % 27;
	}
	
	// convert into the final encrypted message

	for (i = 0; i < keyLength; i++)
	{
		cipherMsg[i] = sumMap[i] + 65;
		
		// if the char should be a represented as a space char (which was assigned the value of 26
		// (26 + 65 = 91)
		if (cipherMsg[i] == 91)
		{
			cipherMsg[i] = 32;
		}
		printf("%c", cipherMsg[i]);
	}
	// add a null terminator to the end
	cipherMsg[keyLength] = '\0';
	//cipherMsg[keyLength + 1] = '\0';

}


/* ********************************************************************************** 
 ** Description: Decrypt function
 ** Input(s): 	
 ** Output(s): 	 
 ** Returns: 
 ** *******************************************************************************/



/* ********************************************************************************** 
 ** Description: Main function
 ** Input(s): 	
 ** Output(s): 	 
 ** Returns: 
 ** *******************************************************************************/
int main(int argc, char *argv[])
{
	int socketFD, portNumber, charsWritten, charsRead;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	char buffer[1024];

    
	// allocate memory to hold name of user and store name of user
	char* user = malloc(32*sizeof(char));
	user = argv[2];
	
	// allocate memory to hold mode type (post or get) and store type of mode
	char* mode = malloc(8*sizeof(char));
	mode = argv[1];

	// allocate memory to hold name of plaintext file name and key file name
	char* fileName = malloc(32*sizeof(char));
	char* keyFile = malloc(32*sizeof(char));


	// *************************************************************************************************	
	// POST MODE
	if (strcmp(mode, "post") == 0)
	{
		if (argc < 6) { fprintf(stderr,"Not enough arguments for POST mode\n"); exit(0); } // Check usage & args

		// Set up the server address struct
		memset((char*)&serverAddress, '\0', sizeof(serverAddress)); 	// Clear out the address struct
		portNumber = atoi(argv[5]); 					// Get the port number, convert to an integer from a string
		serverAddress.sin_family = AF_INET; 				// Create a network-capable socket
		serverAddress.sin_port = htons(portNumber); 			// Store the port number
		serverHostInfo = gethostbyname("localhost");			// Convert the machine name into a special form of address
		if (serverHostInfo == NULL) 
		{
			fprintf(stderr, "CLIENT: ERROR, no such host\n"); 
			exit(0);
		}
		memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address

		// get name of file in the current directory which contains the plaintext you want to encrypt
		fileName = argv[3];

		// get the name of the file in the current directory holding the key
		keyFile = argv[4];

		// call encryption function to generate encrypted message
		char encryptMsg[256];
		encrypt(fileName, keyFile, encryptMsg);


		// Set up the socket
		socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
		if (socketFD < 0) 
		{
			fprintf(stderr, "CLIENT: ERROR opening socket\n");
			exit(0);
		}
	

		// Connect to server
		if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to address
		{
			// if unable to connect to otp_d server, report error to stderr with attempted port and set exit value to 2
			fprintf(stderr, "CLIENT: ERROR connecting on port %d\n", portNumber);
			exit(2);
		}
	
		// concatenate user name, mode and encrypted msg into one string, delimited with '-' character, prior to sending
		char msgToServer[1024];
		memset(msgToServer, '\0', sizeof(msgToServer));	
		strcat(user, "-");
		strcat(msgToServer, user);
		strcat(mode, "-");
		strcat(msgToServer, mode);
		strcat(msgToServer, encryptMsg);
		printf("\nSending to server %s\n", msgToServer);

		// Send user name, mode and encrypted message to server
		printf("CLIENT: Sending POST mode info to server:\n");
		memset(buffer, '\0', sizeof(buffer)); 				// Clear out the buffer array
		snprintf(buffer, (sizeof(buffer) - 1), msgToServer); 		// Store message in buffer, trunc to buffer - 1 chars, leaving \0

		// Send message to server
		charsWritten = send(socketFD, buffer, strlen(buffer), 0); 	// Write to the server
		if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
		if (charsWritten < strlen(buffer)) printf("CLIENT: WARNING: Not all data written to socket!\n");

		// Get return message from server
		memset(buffer, '\0', sizeof(buffer)); 				// Clear out the buffer again for reuse
		charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0);	// Read data from the socket, leaving \0 at end
		if (charsRead < 0) error("CLIENT: ERROR reading from socket");
		printf("CLIENT: I received this from the server: \"%s\"\n", buffer);
	
	}

	// **************************************************************************************************	
	// GET MODE	
	else if (strcmp(mode, "get") == 0)
	{
		// get mode
		
		if (argc < 5) { fprintf(stderr,"Not enough arguments for GET mode\n"); exit(0); } // Check usage & args
		
		// Set up the server address struct
		memset((char*)&serverAddress, '\0', sizeof(serverAddress)); 	// Clear out the address struct
		portNumber = atoi(argv[4]); 					// Get the port number, convert to an integer from a string
		serverAddress.sin_family = AF_INET; 				// Create a network-capable socket
		serverAddress.sin_port = htons(portNumber); 			// Store the port number
		serverHostInfo = gethostbyname("localhost"); 			// Convert the machine name into a special form of address
		if (serverHostInfo == NULL) 
		{ 
			fprintf(stderr, "CLIENT: ERROR, no such host\n"); 
			exit(0); 
		}
		memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address
		
		// get the name of the file in the current directory holding the key
		keyFile = argv[3];

		// send a request for a message for user
		// call decrypt to decrypt the message and print the decrypted message to stdout
		// if user does not have a message, display an error message to stderr
		
		// Set up the socket
		socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
		if (socketFD < 0)
		{
			fprintf(stderr, "CLIENT: ERROR opening socket\n");
			exit(0);
		}
	
		// Connect to server
		if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to address
		{
			// if unable to connect to otp_d server, report error to stderr with attempted port and set exit value to 2
			fprintf(stderr, "CLIENT: ERROR connecting on port %d\n", portNumber);
			exit(2);
		}


		// Get input message from user
		printf("CLIENT: Enter text to send to the server, and then hit enter: ");
		memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer array
		fgets(buffer, sizeof(buffer) - 1, stdin); // Get input from the user, trunc to buffer - 1 chars, leaving \0
		buffer[strcspn(buffer, "\n")] = '\0'; // Remove the trailing \n that fgets adds

		// Send message to server
		charsWritten = send(socketFD, buffer, strlen(buffer), 0); // Write to the server
		if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
		if (charsWritten < strlen(buffer)) printf("CLIENT: WARNING: Not all data written to socket!\n");

		// Get return message from server
		memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
		charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0); // Read data from the socket, leaving \0 at end
		if (charsRead < 0) error("CLIENT: ERROR reading from socket");
		printf("CLIENT: I received this from the server: \"%s\"\n", buffer);	
	}

	// **************************************************************************************************
	else
	{
		perror("Enter 'get' or 'post' as the mode\n");
	}


	// **************************************************************************************************

	close(socketFD); // Close the socket
	return 0;
}
