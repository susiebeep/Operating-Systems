/* ********************************************************************************** 
 ** Program Name:   Program 4 - Dead Drop (keygen.c)
 ** Author:         Susan Hibbert
 ** Date:           3rd June 2020  			      
 ** Description:    This program creates a key file of specified length, passed in
		    on the command line. The key is the random sequence of characters
		    that will be used to convert Plaintext to Ciphertext and back
		    again. The key is never re-used, otherwise the encryption is in
		    danger of being compromised.
	
		    The characters in the file generated will be any of the 27 
		    permitted characters (26 capital letters and space character) 
 ** *******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>


int main(int argc, char* argv[])
{
	// seed the random number generator with the current time
	srand(time(0));
	
	// convert the length of the key specified by the user on the command line from a
	// character to an integer
	int keyLength = atoi(argv[1]);

	// if there is an error output a message to stderr
	if (keyLength == 0)
	{
		perror("Error converting key length to integer\n");
	}

	int i;

	// holds the random ascii value generated
	int keyChar = 0;

	// ascii value for 'Z' set as upper limit for rand() function
	int upper = 90;	

	// although the ascii value for 'A' is 65, the lower limit is set to 64 - if the number
	// 64 is generated this will be interpreted as the space character for the purpose of
	// this function (its real ascii value is 32)
	int lower = 64;		

	// allocate memory to hold two strings - one to hold the final key string and one to
	// hold intermediate versions of the key string
	char* oldKey = malloc(128*sizeof(char));
	char* newKey = malloc(128*sizeof(char));

	for (i = 0; i < keyLength; i++)
	{
		// generate a random number in the range of the 26 capital letters (65 to 90)
		// or space (64 for these purposes)
		keyChar = (rand()%(upper - lower + 1)) + lower;
	
		// if the space character is generated (64 for these purposes), change the ascii
		// value to 32 (actual ascii value for space)
		if (keyChar == 64)
		{
			keyChar = 32;
		}

		// copy the random ascii value into the intermediate key string as its respective
		// character
		sprintf(oldKey,"%c", keyChar);
	
		//add each character to the final key string
		strcat(newKey, oldKey);
	}
		
	
	// add a newline character to the end of the final key string
	strcat(newKey, "\n");
	
	// output final key string to stdout
	printf("%s", newKey);

	return 0;
}
