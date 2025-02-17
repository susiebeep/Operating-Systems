
/* ********************************************************************************** 
 ** Program Name:	Program 2 - Adventure (hibberts.buildrooms.c)
 ** Author:		Susan Hibbert
 ** Date:		24th April 2020  			      
 ** Description:	This program creates a series of files that holds descriptions
			of the in-game rooms and how the rooms are connected
 ** *******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>

//global declaration of room struct
struct room
{
	char* name;	
	char* type;	// START_ROOM, END_ROOM or MID_ROOM
	int numOutboundConnections;	// to keep track of number of outbound connections to other rooms
	struct room* outBoundConnections[6];	//array to store room's outbound connections (at least 3, and at most 6)
};


/* ********************************************************************************** 
 ** Description: Initializes and allocated dynamic memory for an array of room struct
		 pointers
 ** Input(s):	 Pointer to a room struct array
 ** Output(s):   No output
 ** Returns:	 No return
 ** *******************************************************************************/
void init(struct room** roomArr)
{
	// allocate dynamic memory for each room struct in array
	int i;
	for (i = 0; i < 7; i++)
	{
		roomArr[i] = malloc(sizeof(struct room));
	}
}


/* ********************************************************************************** 
 ** Description: Returns whether all of the outbound connections have been set for 
		 every room i.e. whether each room has at least 3 outbound connections
 ** Input(s):	 Pointer to a room struct pointer/array
 ** Output(s):   No output
 ** Returns:	 Returns an integer value - returns 1 (true) if all rooms have at least 
		 3 outbound connections, otherwise returns 0 (false)
 ** *******************************************************************************/
int isGraphFull(struct room** roomArr)
{
	// returns true (1) if all rooms have 3-6 outbound connections, false (0) otherwise
	int i;
	for (i = 0; i < 7; i++)
	{
		// if any room has less than 3 connections, return false
		if ((roomArr[i]->numOutboundConnections) < 3)
		return 0;
	}
	//if reached here, all rooms have at least 3 connections
	return 1;
}


/* ********************************************************************************** 
 ** Description: Returns a random room struct only - does not validate if connection
		 can be added
 ** Input(s):	 Pointer to a room struct pointer/array
 ** Output(s):	 No output
 ** Returns:	 Returns a room struct at a random index in the array of room structs
		 passed  as a parameter
 ** *******************************************************************************/
struct room* getRandomRoom(struct room** roomArr)
{
	// generate a random index number between 0 and 6
	int roomNum = (rand() % 7);

	// return the room at the randomly generated index number
	return roomArr[roomNum];
}


/* ********************************************************************************** 
 ** Description: Returns whether a connection can be added to a room (i.e. if it has
		 less than 6 connections to other rooms)
 ** Input(s): 	 Pointer to a room struct
 ** Output(s):	 No output
 ** Returns: 	 Returns an integer value - returns 1 (true) if room has less than 6
		 connections, else returns 0 (false)
 ** *******************************************************************************/
int canAddConnection(struct room* x)
{
	if (x->numOutboundConnections < 6)
		return 1;
	else
		return 0;
}


/* ********************************************************************************** 
 ** Description: Returns whether a connection exists between two rooms x and y
 ** Input(s):	 Pointers to two room structs
 ** Output(s):	 No output
 ** Returns:	 Returns an integer value - returns 1 (true) if a connection exists 
		 between the rooms, else returns 0 (false)
 ** *******************************************************************************/
int connectionExists(struct room* x, struct room* y)
{
	// retrieve x's outbound connections and number of outbound connections
	int numConnections = x->numOutboundConnections;
	struct room* tempConnection;
	char *tempName;

	int i;
	for (i = 0; i < numConnections; i++)
	{
		tempConnection = x->outBoundConnections[i];
		tempName = tempConnection->name;
		
		//if one of x's outbound connections matches y, the two rooms are connected
		if (strcmp(tempName, y->name) == 0)
			return 1;
	}
	// if reach here, y is not one of x's outbound connections
	return 0;
}


/* ********************************************************************************** 
 ** Description: Connects two rooms together - does not check if connection is valid.
		 Room x is connected to room y (but not vice versa)
 ** Input(s):	 Pointers to two room structs
 ** Output(s):	 Room y is added to room x's member variable array of outbound 
		 connections and one is added to the number of x's outbound connections
 ** Returns:	 No return value
 ** *******************************************************************************/
void connectRoom(struct room* x, struct room* y)
{
	// check for number of current connections
	int indexToAdd = x->numOutboundConnections;

	// add y to x's outbound connections
	x->outBoundConnections[indexToAdd] = y;

	// increase numOutbound connections
	(x->numOutboundConnections)++;
}


/* ********************************************************************************** 
 ** Description: Returns whether two rooms are the same room
 ** Input(s):	 Pointers to two room structs
 ** Output(s):	 No output
 ** Returns:	 Returns an integer value - returns 1 (true) if the two rooms are the
		 same room, else returns 0 (false)
 ** *******************************************************************************/
int isSameRoom(struct room* x, struct room* y)
{
	// returns true if x and y are the same room i.e. they have the same room name
	if (strcmp(x->name, y->name) == 0)
		return 1;
	else
		return 0;
}


/* ********************************************************************************** 
 ** Description: Adds a random, valid outbound connection from one room to another
 ** Input(s): 	 Pointer to a room struct pointer/array
 ** Output(s): 	 Adds room struct B to room struct A's member variable array of 
		 outbound connections, and vice versa (both chosen at random from the
		 room struct array passed as a parameter)
 ** Returns: 	 No return value
 ** *******************************************************************************/
void addRandomConnection(struct room** roomArr)
{
	struct room* A;
	struct room* B;
	
	// find a random room A that can have an outbound connection (< 6 connections)
	while (1)
	{
		A = getRandomRoom(roomArr);
		if (canAddConnection(A) == 1)
			break;
	}

	// find a random room B that can have an outbound connection ( < 6 connections), that
	// is a different room to A, and a connection doesn't already exist between them
	do
	{
		B = getRandomRoom(roomArr);
	}
	
	while (canAddConnection(B) == 0 || isSameRoom(A, B) == 1 || connectionExists(A, B) == 1);

	// connect room A and B, and room B and A
	connectRoom(A, B);
	connectRoom(B, A);
}



/* ********************************************************************************** 
 ** Description: Main function - creates a series of files that hold descriptions of
		 the in-game rooms and how the rooms are connected.
 ** Input(s):	 No input
 ** Output(s): 	 Local directory containing 7 room files, one room per file
 ** Returns:  	 Returns an integer - returns 0 if no errors, otherwise returns 1 (if
	      	 error occurred when trying to open a file for writing to)
 ** *******************************************************************************/
int main (void)
{
	// seed the random number generator using the current time
	srand(time(0));

	// create an array of 7 room structs called gameRoom and initialize each one
	struct room* gameRoom[7];
	init(gameRoom);
		

	// ROOM NAMES: create 10 char arrays which are 9 characters each (8 characters plus 1 for null terminator)
	// to represent the room names to be randomly assigned to each of the rooms
	char roomName[10][9] = { "Library", "Arcade", "Cellar", "Kitchen", "Pool", "Hallway", "Bathroom", "Lab", "Bedroom", "Garage"};

	// ROOM TYPES: create 3 char arrays which are 11 characters each (10 characters plus 1 for null terminator)
	// to represent the room types to be randomly assigned to each of the rooms (only one room can be
	// assigned START_ROOM and END_ROOM)
	char roomType[3][11] = {"START_ROOM", "END_ROOM", "MID_ROOM"}; 
	
	// ROOM NAME ASSIGNMENT: assign the room names at random to each of the rooms
	int num = 0;

	// int array to hold boolean values to determine whether a room name has been assigned or not,
	// initialized to 0 (i.e. false, not assigned)
	int boolArr[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	int i;

	for (i = 0; i < 7; i++)
	{
		//generate a random number between 0 and 9
		// 0 = Library
		// 1 = Arcade and so on
		num = (rand() % 10);

		// find a room name that is still available in the roomName array
		while (boolArr[num] == 1)
		{
			if (num == 9)
				num = 0;

			num++;
		}
		
		//set the room's name to the unassigned room name from the roomName array
		gameRoom[i]->name = roomName[num];

		//change the bool to 1 to indicate the room name has been taken
		boolArr[num] = 1;
	}

	// ROOM TYPE ASSIGNMENT: assign the room types at random to each of the rooms
	int num1 = 0;

	// boolean variables to keep track whether the START_ROOM and END_ROOM have been assigned, 
	// initialized to 0 (i.e. false, not assigned)
	int start = 0;
	int end = 0;

	for (i = 0; i < 7; i++)
	{	
		//generate a random number between 0 and 2:
		// 0 = START_ROOM
		// 1 = END_ROOM
		// 2 = MID_ROOM
		num1 = (rand()% 3 + 0);

		// if START_ROOM comes up
		if (num1 == 0)
		{
			// if START_ROOM has not yet been assigned, assign it to the room
			if (start == 0)
			{
				gameRoom[i]->type = roomType[0];
				start = 1;
			}
			else
				// if START_ROOM has been assigned, check if END_ROOM has been assigned,
				// if it hasn't been assigned yet assign it to the room
				if (end == 0)
				{
					gameRoom[i]->type = roomType[1];
					end = 1;
				}
				// otherwise, if a START_ROOM and END_ROOM have both been assigned, 
				// assign it to be a MID_ROOM
				else
				{
					gameRoom[i]->type = roomType[2];
				}
				
		}
		// if END_ROOM comes up
		if (num1 == 1)
		{ 
			// if END_ROOM has not yet been assigned
			if (end == 0)
			{
				gameRoom[i]->type = roomType[1];
				end = 1;
			}
			else
				// if END_ROOM has been assigned, check if START_ROOM has been assigned,
				// if it hasn't been assigned yet assign it to the room
				if (start == 0)
				{
					gameRoom[i]->type = roomType[0];
					start = 1;
				}
				// otherwise, if a END_ROOM and START_ROOM have both been assigned, 
				// assign it to be a MID_ROOM
				else
				{
					gameRoom[i]->type = roomType[2];
				}
		}
		// if MID_ROOM comes up, assign that type to the room
		if (num1 == 2)
		{
			gameRoom[i]->type = roomType[2];
		}
	}

	// OUTBOUND CONNECTIONS:
	// initialize the number of outbound connections for all the rooms to zero
	for (i = 0; i < 7; i++)
	{
		gameRoom[i]->numOutboundConnections = 0;
	}

	// create all connections in the graph at random
	while((isGraphFull(gameRoom)) == 0)
	{
		addRandomConnection(gameRoom);
	}

	// CREATE ROOM FILES:
	// create directory to hold the room files and append its name with the process ID
	int pid = getpid();
	
 	char dirName[30];
	char* prefix = "hibberts.rooms.";
	sprintf(dirName,"%s" "%d", prefix, pid);


	// create a directory with standard permissions
	mkdir(dirName,0755);

	// open the directory
	DIR* dirToCheck; //holds the directory we're starting in
	dirToCheck = opendir(dirName);

	char *filePath[7];

	// allocate dynamic memory for each filePath variable and assign each a different room file name
	// within the above directory
	for (i = 0; i < 7; i++)
	{
		filePath[i] = malloc(128*sizeof(char));
		sprintf(filePath[i], "./%s/room%d", dirName, i);
	}


	// declare 7 file descriptors and temporary variables to be used when writing the 
	// gameRoom structs to the files
	
	int file_descriptor[7];
	int con = 0;		//track the number of outbound connections for each room
	int j;
	struct room* temp; 	//pointer to a room's outbound connection
	char* wrRoomName;	//used to write room's name to file
	char* wrTempName1;	//pointer to a name of room's outbound connection	
	char* wrTempName2;	//used to write connecting room names to file
	char* wrTypeName1;	//pointer to a type of room's outbound connection
	char* wrTypeName2;	//used to write room's type to file

	// allocate dynamic memory to 3 of the temporary variables (as they are not pointing to
	// any existing memory)
	wrRoomName = malloc(1024*sizeof(char));
	wrTempName2 = malloc(1024*sizeof(char));
	wrTypeName2 = malloc(1024*sizeof(char));

	// WRITE EACH ROOM TO A FILE
	for (i = 0; i < 7; i++)
	{
		// open each file for reading and writing, creating them if they don't exist,
		// and overwriting them if they do exist
		file_descriptor[i] = open(filePath[i], O_RDWR | O_CREAT | O_TRUNC, 0600);
	
		// display error and exit program if unable to open file and exit
		if (file_descriptor[i] == -1)
		{	
			printf("Could not open file: room%d", i);
			exit(1);
		}
	
		// write the name of the room to the file		
		sprintf(wrRoomName, "ROOM NAME: %s\n", gameRoom[i]->name);
		write(file_descriptor[i], wrRoomName, strlen(wrRoomName)*sizeof(char));
		con = gameRoom[i]->numOutboundConnections;

		// write each room's outbound connections to the file
		for (j = 0; j < con; j++)
		{
			temp = gameRoom[i]->outBoundConnections[j];
			wrTempName1 = temp->name;
			sprintf(wrTempName2, "CONNECTION %d : %s\n", (j + 1), wrTempName1);
			write(file_descriptor[i], wrTempName2, strlen(wrTempName2)*sizeof(char));
		}
		
		// write the room's type to the file
		wrTypeName1 = gameRoom[i]->type;
		sprintf(wrTypeName2, "ROOM TYPE: %s\n", wrTypeName1);
		write(file_descriptor[i], wrTypeName2, strlen(wrTypeName2)*sizeof(char));
	
		// reset the number of outbound connections to 0 before moving onto next room
		con = 0;
		
		// reset the file pointer to the beginning of the file
		lseek(file_descriptor[i], 0, SEEK_SET);		
	}	

	
	// close the directory
	closedir(dirToCheck);
	
	
	// free dynamic memory
	for (i = 0; i < 7; i++)
	{
		free(gameRoom[i]);
		free(filePath[i]);
	}

	free(wrRoomName);
	free(wrTempName2);
	free(wrTypeName2);	

	return 0;
}
