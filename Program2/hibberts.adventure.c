
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
	struct room* outBoundConnections[6];
};

/* ********************************************************************************** 
 ** Description: Initializes the room structs, which represent the game rooms
 ** Input(s): 	 Pointer to a room struct array/pointer 	
 ** Output(s): 	
 ** Returns: 	 
 ** *******************************************************************************/
void init(struct room** roomArr)
{	
	// dynamically allocate memory for each gameRoom struct and its name and type
	// member variables
	int i;
	for (i = 0; i < 7; i++)
	{
		roomArr[i] = malloc(sizeof(struct room));
		roomArr[i]->name = malloc(32*sizeof(char));
		roomArr[i]->type = malloc(32*sizeof(char));
	}
	

	// set all 6 pointers to a room's outbound connections to null	
	int j;
	for (i = 0; i < 7; i++)
	{
		for (j = 0; j < 6; j ++)
		{
			roomArr[i]->outBoundConnections[j] = NULL;
		}
	}
}

/* ********************************************************************************** 
 ** Description: 
 ** Input(s): 	 No input
 ** Output(s): 	 The player's current location and a list of possible connections that
		 can be followed
 ** Returns: 	 
 ** *******************************************************************************/
void interface()
{
	
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
 ** Description: Main function
 ** Input(s): 	 No input
 ** Output(s):
 ** Returns:
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
	
	// free any dynamically allocated memory	
	for (i = 0; i < 7; i++)
	{
		free(gameRoom[i]);
		free(filePath[i]);
	}
	
	free(shineyDir);	
	return 0;
}
