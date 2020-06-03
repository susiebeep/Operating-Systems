/* ********************************************************************************** 
 ** Program Name:   Program 4 - Dead Drop (otp.c)
 ** Author:         Susan Hibbert
 ** Date:           3rd June 2020  			      
 ** Description:    This program acts as the client. It connects to otp_d and asks it
		    to store or retrieve messages for it. It has two modes, post and
		    get (details described below)
 ** *******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#define MAX_SIZE 100000

void error(const char *msg) { perror(msg); exit(0); } // Error function used for reporting issues

/* ********************************************************************************** 
 ** Description: Encrypt function. Encrypts plaintext file using a key
 ** Input(s): 	 3 string inputs - String representing name of plaintext file you wish
		 to encrypt; string representing name of key file containing the key
		 you wish to use to encrypt the text; string to hold the final
		 encrypted message
 ** Output(s): 	 Displays error message if there is an issue opening either file.
		 Displays error message if the key file is shorter than plaintext
		 file, or if either file contains any bad characters. Program will
		 subsequently terminate and set the exit value to 1
 ** Returns: 	 Returns 0 if encryption was successful, otherwise returns 1 if an
		 error occurred
 ** *******************************************************************************/
int encrypt(char* file, char* key, char* cipherMsg)
{
	// file stream pointers for plaintext file and key file
	FILE* filePtr;
	FILE* keyPtr;

	// char arrays to hold the plaintext file and key contents
	char fileArr[MAX_SIZE];
	char keyArr[MAX_SIZE];

	memset(fileArr, '\0', MAX_SIZE);
	memset(keyArr, '\0', MAX_SIZE);

	
	// holds a line of the file as it is read in
	char line[MAX_SIZE];

	memset(line, '\0', MAX_SIZE);

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
		sprintf(fileArr, "%s%s", fileArr, line);
	}
	fileArr[strcspn(fileArr, "\n")] = '\0'; // Remove the trailing \n that fgets adds	

	while(fgets(line, sizeof(line), keyPtr) != NULL)
	{
		sprintf(keyArr,"%s%s", keyArr, line);
	}
	keyArr[strcspn(keyArr, "\n")] = '\0'; // Remove the trailing \n that fgets adds

	// close the file pointers
	fclose(filePtr);
	fclose(keyPtr);
	
	// get the size of the two files
	int keyLength = strlen(keyArr);
	int fileLength = strlen(fileArr);

	// check the length of the key is long enough for the plaintext file - if the key is too
	// short return to main function where program will terminate
	if (keyLength < fileLength)
	{
		fprintf(stderr, "CLIENT: Key file is shorter than plaintext file!\n");
		return 1;
	}

	// create two integer arrays to hold the alphanumeric values of the letters in the key and
	// plaintext files
	int keyMap[fileLength];
	int fileMap[fileLength];
	int i;
	
	// first, check for any bad characters in the key or plaintext files. If any are 
	// found return to main function where program will terminate
	for (i = 0; i < fileLength; i++)
	{
		// if ASCII value is larger than value for 'Z'
		if (keyArr[i] > 90 || fileArr[i] > 90)
		{
			fprintf(stderr, "CLIENT: Bad character in file!\n");
			return 1;
		}
		// if ASCII value is smaller than value for 'A' and not a space character
		else if (keyArr[i] != 32 && keyArr[i] < 65)
		{
			fprintf(stderr, "CLIENT: Bad character in file!\n");
			return 1;
		}	
		else if (fileArr[i] != 32 && fileArr[i] < 65)
		{
			fprintf(stderr, "CLIENT: Bad character in file!\n");
			return 1;
		}
		else
		{
			continue;
		}
	}

	// if make it here - no bad characters were found in the key or plaintext files
	for (i = 0; i < fileLength; i++)
	{	
		// if the character is a space, directly assign it the value 26 for the purpose
		// of the encryption process
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
	int sumMap[fileLength];
	int temp = 0;

	for (i = 0; i < fileLength; i++)
	{
		temp = keyMap[i] + fileMap[i];

		// if the number is larger than 27 then the remainder, after subtracting 27, is taken
		if (temp > 27)
		{
			temp -= 27;	
		}
		sumMap[i] = temp % 27;
	}
	
	// convert into the final encrypted message
	for (i = 0; i < fileLength; i++)
	{
		cipherMsg[i] = sumMap[i] + 65;
		
		// if the char should be a represented as a space char (which was assigned the value of 26 -
		// 26 + 65 = 91)
		if (cipherMsg[i] == 91)
		{
			cipherMsg[i] = 32;
		}
	}
	// add a null terminator to the end
	cipherMsg[fileLength] = '\0';

	return 0;
}


/* ********************************************************************************** 
 ** Description: Decrypt function. Decrypts an encrypted message into plaintext
 ** Input(s): 	 2 string inputs - string representing the encrypted message you wish
		 to decrypt; string representing the name of the key file which
		 contains the key you wish to use to decrypt the message
 ** Output(s): 	 Displays error message if unable to open the key file, or if the key
		 file is shorter than plaintext file. Program will subsequently
		 terminate and set the exit value to 1
 ** Returns:	 Returns 0 if decryption was successful, otherwise returns 1 if an
		 error occurred 
 ** *******************************************************************************/
int decrypt(char* encryptedTxt, char* key1)
{	
	// file stream pointer for key file
	FILE* keyPtr;

	// char arrays to hold contents of key file and a line of the file as it
	// is read in
	char keyArr[MAX_SIZE];
	char line[MAX_SIZE];	

	memset(keyArr, '\0', MAX_SIZE);
	memset(line, '\0', MAX_SIZE);

	// open key file
	keyPtr = fopen(key1, "r");

	// if there is an error opening the key file
	if (keyPtr == NULL)
	{
		fprintf(stderr, "CLIENT: Error opening key file\n");	
		exit(1);
	}

	// read contents of key file into char array
	while(fgets(line, sizeof(line), keyPtr) != NULL)
	{
		sprintf(keyArr, "%s%s", keyArr, line);
	}
	keyArr[strcspn(keyArr, "\n")] = '\0'; // Remove the trailing \n that fgets adds
	
	// close file pointer
	fclose(keyPtr);

	// get size of encrypted text and key
	int msgLength = strlen(encryptedTxt);
	int keyLength = strlen(keyArr);

	// check the length of the key is long enough for the encrypted message, if the key is too
	// short return to main function where an error message will be displayed and the program
	// will terminate
	if (keyLength < msgLength)
	{
		return 1;
	}
	// create integer arrays to hold alphanumeric values of letters in the encrypted text and key
	int msgMap[msgLength];
	int keyMap[msgLength];
	int i;

	for (i = 0; i < msgLength; i++)
	{
		// if the character is a space, directly assign it the value 26 for the purpose of
		// the decryption process
		if (encryptedTxt[i] == 32 || keyArr[i] == 32)
		{
			if (encryptedTxt[i] == 32 && keyArr[i] == 32)
			{
				keyMap[i] = 26;
				msgMap[i] = 26;
			}
			else if (keyArr[i] == 32)
			{
				keyMap[i] = 26;
				msgMap[i] = encryptedTxt[i] - 65;
			}
			else
			{
				msgMap[i] = 26;
				keyMap[i] = keyArr[i] - 65;
			}
		}
		else
		{
			// subtract 65 from each char's decimal ASCII value to get its
			// alphanumeric value (A = 0; B = 1 etc) and store in integer array
			msgMap[i] = encryptedTxt[i] - 65;
			keyMap[i] = keyArr[i] - 65;
		}
	}


	// subtract each corresponding number in the key from the encrypted text and perform
	// modulus 27 on the resultant value
	int minusMap[msgLength];
	int temp = 0;

	for (i = 0; i < msgLength; i++)
	{
		temp = msgMap[i] - keyMap[i];
		
		// if the subtraction results in a negative number, add 27 to make the number 0 or higher
		if (temp < 0)
		{
			temp += 27;
		} 
		minusMap[i] = temp % 27;
	}	

	
	// convert into final decrypted message
	char finalMsg[MAX_SIZE];

	for (i = 0; i < msgLength; i++)
	{
		finalMsg[i] = minusMap[i] + 65;
		
		// if the char should be a space character (which was assigned the value of 26 -
		// 26 + 65 = 91)
		if (finalMsg[i] == 91)
		{
			finalMsg[i] = 32;
		}
	}
	// add a newline and null terminator to the end
	finalMsg[msgLength] = '\n';
	finalMsg[msgLength + 1] = '\0';

	// print decrypted message to stdout
	printf("%s", finalMsg);
	fflush(stdout);

	return 0;
}
/* ********************************************************************************** 
 ** Description: Main function, where otp sets up a connection and connects to otp_d
		 in post or get mode
 ** Input(s): 	 Command line arguments indicating mode, user, key, plaintext file name
		 (if applicable) and connecting port
 ** Output(s): 	 Displays an error message if it cannot connect to otp_d server
 ** Return: 	 Returns 0 upon successfully running and terminating, otherwise returns
		 1 if an error occurred
 ** *******************************************************************************/
int main(int argc, char *argv[])
{
	int socketFD, portNumber, charsWritten, charsRead;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	char buffer[MAX_SIZE];
  
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
	// In post mode, otp will encrypt a plaintext file using a key. It will then send the respective user
	// name and the encrypted message to otp_d, where otp_d stores it in the user's directory
	if (strcmp(mode, "post") == 0)
	{
		// Check usage and args
		if (argc < 6) { fprintf(stderr,"Not enough arguments for POST mode\n"); exit(0); }

		// Set up the server address struct
		memset((char*)&serverAddress, '\0', sizeof(serverAddress)); 	// Clear out the address struct
		portNumber = atoi(argv[5]); 					// Get the port number, convert to an integer from a string
		serverAddress.sin_family = AF_INET; 				// Create a network-capable socket
		serverAddress.sin_port = htons(portNumber); 			// Store the port number
		serverHostInfo = gethostbyname("localhost");			// Convert the machine name into a special form of address
		if (serverHostInfo == NULL) 
		{
			fprintf(stderr, "CLIENT: ERROR, no such host\n"); 
			exit(2);
		} 
		
		// Copy in the address
		memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length);

		// get name of file in the current directory which contains the plaintext you want to encrypt
		strcpy(fileName,argv[3]);

		// get the name of the file in the current directory holding the key
		strcpy(keyFile, argv[4]);

		// call encryption function to generate encrypted message
		char encryptMsg[MAX_SIZE];
		memset(encryptMsg, '\0', MAX_SIZE);
		int encryptSuccess = encrypt(fileName, keyFile, encryptMsg);

		// if there was an error with the encryption (key file is too short, bad characters), 
		// terminate and set the exit value to 1		
		if (encryptSuccess == 1)
		{
			exit(1);
		}
		
		else
		{
			// Create and set up the socket
			socketFD = socket(AF_INET, SOCK_STREAM, 0);
			if (socketFD < 0) 
			{
				fprintf(stderr, "CLIENT: ERROR opening socket\n");
				exit(2);
			}

			// Connect socket to address in order to connect to server
			if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0)
			{
				// if unable to connect to otp_d server, report error to stderr with attempted
				// port and set exit value to 2
				fprintf(stderr, "CLIENT: ERROR connecting on port %d\n", portNumber);
				exit(2);
			}
	
			// concatenate user name, mode and encrypted msg into one string, delimited with '-'
			// character, prior to sending
			char msgToServer[MAX_SIZE];
			memset(msgToServer, '\0', MAX_SIZE);	
			strcat(user, "-");
			strcat(msgToServer, user);
			strcat(mode, "-");
			strcat(msgToServer, mode);
			strcat(msgToServer, encryptMsg);

			// Send user name, mode and encrypted message to server
			memset(buffer, '\0', sizeof(buffer)); 				// Clear out the buffer array
			snprintf(buffer, (sizeof(buffer) - 1), msgToServer); 		// Store message in buffer, trunc to buffer - 1 chars, leaving \0

			charsWritten = send(socketFD, buffer, strlen(buffer), 0); 	// Write to the server
			if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
			if (charsWritten < strlen(buffer)) printf("CLIENT: WARNING: Not all data written to socket!\n");
		}
	}

	// **************************************************************************************************	
	// GET MODE	
	// In get mode, otp will send a request for an encrypted message for a user, which it receives from
	// otp_d (if user has a message stored). It then uses a key to decrypt the message and print the
	// decrypted message to stdout
	else if (strcmp(mode, "get") == 0)
	{	
		// Check usage and args
		if (argc < 5) { fprintf(stderr,"Not enough arguments for GET mode\n"); exit(0); }
			
		// Set up the server address struct
		memset((char*)&serverAddress, '\0', sizeof(serverAddress)); 	// Clear out the address struct
		portNumber = atoi(argv[4]); 					// Get the port number, convert to an integer from a string
		serverAddress.sin_family = AF_INET; 				// Create a network-capable socket
		serverAddress.sin_port = htons(portNumber); 			// Store the port number
		serverHostInfo = gethostbyname("localhost"); 			// Convert the machine name into a special form of address
		if (serverHostInfo == NULL) 
		{ 
			fprintf(stderr, "CLIENT: ERROR, no such host\n"); 
			exit(2); 
		}
		
		// Copy in the address
		memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length);
		
		// get the name of the key file in the current directory holding the key
		strcpy(keyFile, argv[3]);

		// Create and set up the socket
		socketFD = socket(AF_INET, SOCK_STREAM, 0); 
		if (socketFD < 0)
		{
			fprintf(stderr, "CLIENT: ERROR opening socket\n");
			exit(2);
		}
	
		// Connect socket to address in order to connect to server
		if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) 
		{
			// if unable to connect to otp_d server, report error to stderr with attempted port
			// and set exit value to 2
			fprintf(stderr, "CLIENT: ERROR connecting on port %d\n", portNumber);
			exit(2);
		}
		
		// concatenate user name and mode into one string, delimited by '-' character, before sending to
		// server
		char msgToServer[MAX_SIZE];
		memset(msgToServer, '\0', MAX_SIZE);	
		strcat(user, "-");
		strcat(msgToServer, user);
		strcat(msgToServer, mode);

		// Send user name and mode to server
		charsWritten = send(socketFD, msgToServer, strlen(msgToServer), 0);
		if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
		if (charsWritten < strlen(msgToServer)) printf("CLIENT: WARNING: Not all data written to socket!\n");


		// Get return message from server which sends the oldest file for this user which will be
		// decrypted by the client using the key and print the decrypted message to stdout	
		memset(buffer, '\0', sizeof(buffer)); 				// Clear out the buffer again for reuse
		charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0);	 // Read data from the socket, leaving \0 at end
		if (charsRead < 0) error("CLIENT: ERROR reading from socket");

		// if received 'none' from server, there were no encrypted messages for the specified user so
		// display error message
		if (strcmp(buffer, "none") == 0)
		{
			fprintf(stderr, "CLIENT: User has no encrypted messages!\n");
		}
		else
		{
			// pass encrypted text received from server to decrypt function
			int decryptSuccess = decrypt(buffer, keyFile);
	
			// if there was an error with the decryption, terminate and set the exit value to 1		
			if (decryptSuccess == 1)
			{
				fprintf(stderr, "CLIENT: Encrypted text and key are different lengths\n");
				exit(1);
			}
		}
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
