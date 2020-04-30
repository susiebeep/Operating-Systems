
/* ********************************************************************************** 
 ** Program Name:   Program 2 - Adventure (hibberts.adventure.c)
 ** Author:         Susan Hibbert
 ** Date:           27th April 2020  			      
 ** Description:    This program provides an interface for playing the game using the
		    most recently generated room files. 
		    
		    The player will begin in the starting room and will navigate
		    through each room's outbound connections until they reach the end
		    room. When they reach the end room they automatically win the game
		    and the path that they took through the rooms will be displayed.
		    
		    During the game, the player can also enter a command that returns
		    the current time. 
 ** *******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>


// global struct room declaration
struct room
{
	char* name;
	char* type;
	int numOutboundConnections; 
	struct room* outBoundConnections[6];
};

/* ********************************************************************************** 
 ** Description: Initializes the room structs, which represent the game rooms, and
		 their member variables
 ** Input(s): 	 Pointer to a room struct array
 ** Output(s): 	 No output
 ** Returns: 	 No return value
 ** *******************************************************************************/
void init(struct room** roomArr)
{	
	// dynamically allocate memory for each gameRoom struct and its member
	// variables name and type
	int i;
	for (i = 0; i < 7; i++)
	{
		roomArr[i] = malloc(sizeof(struct room));
		roomArr[i]->name = malloc(32*sizeof(char));
		roomArr[i]->type = malloc(32*sizeof(char));
	}
	

	// set the number of each room's outbound connections to 0 and set all 6
	// pointers to each room's outbound connections to null
	int j;
	for (i = 0; i < 7; i++)
	{
		for (j = 0; j < 6; j ++)
		{
			roomArr[i]->numOutboundConnections = 0; 
			roomArr[i]->outBoundConnections[j] = NULL;
		}
	}
}


/* ********************************************************************************** 
 ** Description: Displays the player's current room location in the game along with
		 the current room's connecting rooms
 ** Input(s): 	 Pointer to a room struct
 ** Output(s): 	 The player's current location and a list of possible room connections
		 is displayed on the screen
 ** Returns: 	 No return value
 ** *******************************************************************************/
void printLocation(struct room* roomPtr)
{
	printf("CURRENT LOCATION: %s\n", roomPtr->name);	
	printf("POSSIBLE CONNECTIONS: ");
	int i;
	
	for (i = 0; i < roomPtr->numOutboundConnections; i++)
	{
		// if printing the last room connection, add a full stop and start a new line
		if (i == (roomPtr->numOutboundConnections - 1))	
		{
			printf("%s.", roomPtr->outBoundConnections[i]->name);
			printf("\n");
		}
		else
		{
			printf("%s, ", roomPtr->outBoundConnections[i]->name);
		}
	} 
}


/* ********************************************************************************** 
 ** Description: Displays the player's current room location and list of possible room
		 connections (by calling printLocation) and gets the user's choice of
		 connecting room. Displays error message if user does not enter one of 
		 the connecting room names from the list
 ** Input(s): 	 Pointer to a room struct
 ** Output(s): 	 Prompts user for their choice of room name. Displays error message and
		 re-prompts user if they do not enter a valid connecting room name
 ** Returns: 	 Returns an integer which represents a valid room index of the current
		 room's outbound connection array, where the player wants to move to
 ** *******************************************************************************/
int getMove(struct room* roomPtr)
{
	char* nameEntered = NULL; 	// points to a buffer allocated by getline() to hold input string
	int numCharsEntered = 0;	// how many chars were entered by user
	size_t bufferSize = 0;		// holds how large the allocated buffer is

	int i;
	int strLength;

	// get user's input for their choice of room name
	printLocation(roomPtr);	
	printf("WHERE TO? >");	
	numCharsEntered = getline(&nameEntered, &bufferSize, stdin);
	printf("\n");

	for (i = 0; i < roomPtr->numOutboundConnections; i++)
	{
		strLength = strlen(roomPtr->outBoundConnections[i]->name);

		// if the name the user entered matches the name of one of the connecting rooms,
		// return the index of that room
		//
		// N.B. when the user enters a room name then presses the enter key, a newline
		// character is added to the nameEntered variable, so we need to ignore this
		// in our strncmp function (otherwise it will fail)-  we compare the two room
		// names up to the size of the connecting room's name
		if (strncmp(roomPtr->outBoundConnections[i]->name, nameEntered, strLength) == 0)
		{
			return i;
		}	
	} 

	// if make it here, user did not enter a valid room name - inform them that they
	// did not enter a valid name and keep prompting the user until they enter a
	// valid connecting room name	
	printf("HUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n");
	printf("\n");
	getMove(roomPtr);
}


/* ********************************************************************************** 
 ** Description: Simulates the player's movement in the game, from one room into a
		 connecting room. Updates player's current location and adds one to
		 the number of steps they have taken, and also adds the name of the
		 previous room to an array storing the path the user is taking
 ** Input(s): 	 Integer value (room index to move to), integer value (number of steps 
		 currently taken), pointer to a pointer to a room struct (i.e. the 
		 address of a room struct pointer), pointer to a string array
		 (stores current path taken by the player)
 ** Output(s): 	 No output
 ** Returns: 	 No return value
 ** *******************************************************************************/
void makeMove(int index, int* steps, struct room** roomPtr, char** nameArr)
{
	// store path taken by user (do not store start room name) - store name of
	// current room the user is in in nameArr
	if (strcmp((*roomPtr)->type, "START_ROOM") != 0)
	{
		nameArr[*steps] = (*roomPtr)->name;
	}

	//point to the connecting room, chosen by the user
	*roomPtr = (*roomPtr)->outBoundConnections[index];

	// increase steps taken
	*steps = *steps + 1;		
}


/* ********************************************************************************** 
 ** Description: This is called when the player has reached the end room and completed
		 the game
 ** Input(s): 	 Integer value (the number of steps taken by the user) and a pointer
	      	 to a string array (names of the rooms the player visited in 
		 the game)	
 ** Output(s): 	 Congratulatory message is displayed on the screen, followed by the
		 number of steps taken by the player and the route they took through
		 the game
 ** Returns:	 No return value
 ** *******************************************************************************/
void gameOver(int* steps, char** nameArr)
{
	printf("YOU HAVE FOUND THE END ROOM! CONGRATULATIONS!\n");
	
	// print number of steps taken
	printf("YOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:", *steps);
	
	// display path taken by user
	int i;
	for (i = 0; i < *steps; i++)
	{
		printf("%s\n", nameArr[i]);
	}	
}


/* ********************************************************************************** 
 ** Description: Returns the name of the most recently generated rooms directory for
		 use in the game
 ** Input(s): 	 No input
 ** Output(s): 	 No output
 ** Returns: 	 Returns a char pointer, representing the name of the most recently
		 generated rooms subdirectory
 ** *******************************************************************************/
char* recentDir()
{

	// resource used: 2.4 Manipulating Directories, from Block 2
	int newestDirTime = -1; 			// modified timestamp of most recent subdirectory
	char targetDirPrefix[64] = "hibberts.rooms."; 	// prefix of the rooms directory
	char *newestDirName;	 			// stores the name of the most recent rooms directory
	newestDirName = malloc(sizeof(char)*1024);
	memset(newestDirName, '\0', sizeof(newestDirName));

	DIR* dirToCheck;		// holds starting directory
	struct dirent *fileInDir;	// holds the current subdirectory of the start directory
	struct stat dirAttributes;	// hold information about subdirectory
	
	dirToCheck = opendir(".");	// open current directory

	if (dirToCheck > 0)		// if the current directory can be opened
	{
		while ((fileInDir = readdir(dirToCheck)) != NULL)			// check each entry in the directory
		{
			if (strstr(fileInDir->d_name, targetDirPrefix) != NULL) 	// if entry has desired prefix
			{
				stat(fileInDir->d_name, &dirAttributes);		// get attributes of entry
			
				if ((int)dirAttributes.st_mtime > newestDirTime) 	// if this time is bigger/more recent
				{
					newestDirTime = (int)dirAttributes.st_mtime;
					memset(newestDirName, '\0', sizeof(newestDirName));
					strcpy(newestDirName, fileInDir->d_name);
				}
			}
		}
	}
	
	closedir(dirToCheck);		// close the directory we opened
	return newestDirName;

}
/* ********************************************************************************** 
 ** Description: Main function - reads the data from the room files in the most
		 recently created directory into room structs for use in the game.
		 Displays the interface for the game.
 ** Input(s): 	 No input
 ** Output(s):	 Displays game stats to the user (location, possible room connections). 
		 Displays prompts to the user for their room choices as they play the
		 game. Displays final message to the user when they complete the game.
 ** Returns:	 Sets the exit status code to 0
 ** *******************************************************************************/
int main(void)
{
	// declare an array of 7 room structs called gameRoom which will be populated
	// with the data in the 7 room files
	struct room* gameRoom[7];

	// intialize the gameRoom structs
	init(gameRoom);
	
	// find the most recently generated rooms directory and open it
	char* shineyDir = recentDir();		

	DIR* recentDir;
	recentDir = opendir(shineyDir);

	char *filePath[7];	
	int i;
	// store the file path and name for each of the room files
	for (i = 0; i < 7; i++)
	{
		filePath[i] = malloc(128*sizeof(char));
		sprintf(filePath[i],"%s/room%d", shineyDir, i);
	}
	
	
	//RETRIEVE EACH ROOM'S NAME AND TYPE FROM THE FILES
	FILE* file_descriptor[7];
	char roomName[32];		// holds the name of the room extracted from the file
	char roomType[32];		// holds the type of the room extracted from the file
	
	char line[32];			// holds a line of the file as it is read in
	char foo[32];			// to hold unwanted string from file
	char bar[32];			// to hold unwanted string from file	
	int baz[8];			// to hold unwanted integer from file

	int startRoomIndex = 0;		// saves the index of the START_ROOM in the gameRoom array

	for (i = 0; i < 7; i++)
	{
		// open a file stream to read each room file
		file_descriptor[i] = fopen(filePath[i], "r");

	
		// display error message if unable to open file stream and exit program
		if (file_descriptor[i] == NULL)
		{
			printf("Could not open file: room%d", i);
			exit(1);
		}
		

		// read file contents until reached the end of the file	(i.e. when feof returns 0)
		while (fgets(line, sizeof(line), file_descriptor[i]) != NULL)
		{
			//printf(line);
	
			// if reading in the line specifying the room's name	
			if (strstr(line, "ROOM NAME: ") != NULL)
			{	
				sscanf(line, "%s %s %s",  foo, bar, roomName);
				strcpy(gameRoom[i]->name, roomName);
			}
		

			// if reading in the line specifying the room's type	
			if (strstr(line, "ROOM TYPE: ") != NULL)
			{	
				sscanf(line, "%s %s %s",  foo, bar, roomType);
				strcpy(gameRoom[i]->type, roomType);
				
				// save the array index of the gameRoom that is the START_ROOM	
				if (strcmp(roomType, "START_ROOM") == 0)
				{
					startRoomIndex = i;
				}
			}
	
		}

		//close the file stream
		fclose(file_descriptor[i]);	
	}


	// SET UP THE CONNECTIONS BETWEEN EACH ROOM
	char roomConnection[32];	// holds the name of a room's connection extracted from the file	
	int conIndex = 0;		// holds the current array index of the roomConnection array
	int j;

	for (i = 0; i < 7; i++)
	{
		// open a file stream to read each room file
		file_descriptor[i] = fopen(filePath[i], "r");

	
		// display error message if unable to open file stream and exit program
		if (file_descriptor[i] == NULL)
		{
			printf("Could not open file: room%d", i);
			exit(1);
		}
		

		// read file contents until reached the end of the file	(i.e. when feof returns 0)
		while (fgets(line, sizeof(line), file_descriptor[i]) != NULL)
		{
			j = 0;

			// find the lines in the file pertaining to the room's outbound connections
			if (strstr(line, "CONNECTION ") != NULL)
			{	
				sscanf(line, "%s %d %s %s",  foo, baz, bar, roomConnection);
			
				// search through all the gameRoom structs for the room with that name
				while(strcmp(gameRoom[j]->name, roomConnection) != 0)
				{
					j++;
				}
				//when find the gameRoom struct, add it to that gameRoom's connections
				gameRoom[i]->outBoundConnections[conIndex] = gameRoom[j];

				conIndex++;
			}	
		}

		//close the file stream
		fclose(file_descriptor[i]);
		
		// store the number of outbound connections for the room
		gameRoom[i]-> numOutboundConnections = conIndex;		

		// reset the index of the room connection array
		conIndex = 0;
	}


	// print out the structs
	for (i = 0; i < 7; i++)
	{
		printf("Room name: %s\n", gameRoom[i]->name);
		printf("Room type: %s\n", gameRoom[i]->type);
		for (j = 0; j < 6; j++)
		{
			if (gameRoom[i]->outBoundConnections[j] != NULL)
			{
				printf("connection %d: %s\n", j, gameRoom[i]->outBoundConnections[j]->name);
			}
		}
	}


	// create struct room pointer to keep track of the player's location and direct it to start in the
	// start room
	struct room* gamePtr;
	gamePtr = gameRoom[startRoomIndex];

	// START THE GAME:
	// initialize the number of steps taken to 0
	int stepsTaken = 0;
	
	// create a array of strings to hold the names of the rooms visited by the user and dynamically
	// allocate memory to each string
	char** routeArr = malloc(1024*sizeof(char*));
	for (i = 0; i < 1024; i++)
	{
		routeArr[i] = malloc(sizeof(char));
	}
	
	// display the player's starting location and ask the player what room they want to move to
	int validRoomIndex = 0;
	validRoomIndex = getMove(gamePtr);
	
	//pass the index to the makeMove function and the address of the gamePtr to move the player into this room
	makeMove(validRoomIndex, &stepsTaken, &gamePtr, routeArr);
	
	
	// while user has not visited the end room, continue to ask them for their next move and display
	// their current location and a list of possible room connections they can move to
	while (strcmp(gamePtr->type, "END_ROOM") != 0)
	{
		validRoomIndex = getMove(gamePtr);
		makeMove(validRoomIndex, &stepsTaken, &gamePtr, routeArr);
	}
	
	// when the user has reached the end room, the game displays a congratulatory message, along
	// with the path the user took to get there and the number of steps it took them
	gameOver(&stepsTaken, routeArr);	

	//append the end room name to the list of rooms visited	
	printf("%s\n", gamePtr->name);


	// free any dynamically allocated memory	
	for (i = 0; i < 7; i++)
	{
		free(gameRoom[i]);
		free(filePath[i]);
	}
	
	free(shineyDir);	

	// set exit status code to 0
	exit(0);
}
