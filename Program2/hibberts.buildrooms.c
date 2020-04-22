
/* ********************************************************************************** 
 ** Program Name:	Program 2 - Adventure
 ** Author:			Susan Hibbert
 ** Date:			22nd April 2020  			      
 ** Description:
 ** *******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>

struct room
{
	char* name;	
	char* type;	// START_ROOM, END_ROOM or MID_ROOM
	int numOutboundConnections;	// to keep track of number of outbound connections to other rooms
	struct room* outBoundConnections[6];	//at least 3, and at most 6
};


/* ********************************************************************************** 
 ** Description: Returns whether all of the outbound connections have been set for 
				 every room i.e. whether each room has at least 3 outbound connections
 ** Input(s):	 Pointer to an array of room structs
 ** Output(s):
 ** Returns:	Returns an integer value - returns 1 (true) if all rooms have at least 
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
 ** Input(s):	 Pointer to an array of room structs
 ** Output(s):
 ** Returns:	A room struct at a random index in the array of room structs passed 
				as a parameter
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
 ** Output(s):
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
 ** Description: Returns whether a connection exists between two rooms
 ** Input(s):	 Pointers to two room structs
 ** Output(s):
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
				 Room on left is connected to the room on the right and not vice versa
 ** Input(s):	Two pointers to room structs
 ** Output(s):	Room on left is connected to room on right and added to its struct room
				array of outbound connections
 ** Returns:	No return value
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
 ** Input(s):	 Two pointers to room structs
 ** Output(s):
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
 ** Input(s): No input parameters
 ** Output(s):
 ** Returns: No return value
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
 ** Description: Main function
 ** Input(s):
 ** Output(s):
 ** Returns:
 ** *******************************************************************************/
int main (void)
{
	// seed the random number generator using the current time
	srand(time(0));

	// create an array of 7 room structs called gameRoom
	struct room* gameRoom[7];

	// allocate memory for each struct
	int i;
	for (i = 0; i < 7; i++)
	{
		gameRoom[i] = malloc(sizeof(struct room));
	}


	// create a 10 char arrays which are 9 characters each (8 characters plus 1 for null terminator)
	// to represent the room names to be randomly assigned to each of the rooms
	char roomName[10][9] = { "Library", "Arcade", "Cellar", "Kitchen", "Pool", "Hallway", "Bathroom", "Lab", "Bedroom", "Garage"};

	// create a 3 char arrays which are 11 characters each (10 characters plus 1 for null terminator)
	// to represent the room types to be randomly assigned to each of the rooms (only one room can be
	// assigned START_ROOM and END_ROOM)
	char roomType[3][11] = {"START_ROOM", "END_ROOM", "MID_ROOM"}; 
	
	
	// assign the room names at random to each of the rooms
	int num = 0;

	// int array to hold boolean values to determine whether a room name has been assigned or not,
	// initialized to 0 (i.e. false, not assigned)
	int boolArr[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

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

	// assign the room types at random to each of the rooms
	int num1 = 0;

	// booleans to keep track if the START_ROOM and END_ROOM have been assigned, initialized
	// to 0 (i.e. false, not assigned)
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
		// if MID_ROOM comes up, assign it to the room
		if (num1 == 2)
		{
			gameRoom[i]->type = roomType[2];
		}
	}

	// initialize the number of outbound connections for all the rooms to zero
	for (i = 0; i < 7; i++)
	{
		gameRoom[i]->numOutboundConnections = 0;
	}

	//create all connections in the graph
	while((isGraphFull(gameRoom)) == 0)
	{
		addRandomConnection(gameRoom);
	}

	int con = 0;
	int j;
	struct room* temp;
	char* tempName;
	for (i = 0; i < 7; i++)
	{
		printf("ROOM NAME: %s\n", gameRoom[i]->name);
		con = gameRoom[i]->numOutboundConnections;
		for (j = 0; j < con; j++)
		{
			temp = gameRoom[i]->outBoundConnections[j];
			tempName = temp->name;
			printf("CONNECTION %d : %s\n", (j + 1), tempName);
		}
		printf("ROOM TYPE: %s\n", gameRoom[i]->type);
	}


	// create directory and append the name with the process ID
	int pid = getpid();
	printf("%d/n", pid);
	
 	char dirName[30];
	char* prefix = "hibberts.rooms.";
	sprintf(dirName,"%s" "%d", prefix, pid);


	// create a directory with standard permissions
	mkdir(dirName,0755);

	// open the directory
	opendir(dirName);
/*
	char *filePath[7];

	// generate 7 different room files within the directory, one gameRoom struct per file
	for (i = 0; i < 7; i++)
	{
		strcpy(filePath[i], "dirName/room" + i);	
	}
*/
	/*char file1[] = "hibberts.rooms.$$/room1";	
	char file2[] = "hibberts.rooms.$$/room2";	
	char file3[] = "hibberts.rooms.$$/room3";	
	char file4[] = "hibberts.rooms.$$/room4";	
	char file5[] = "hibberts.rooms.$$/room5";	
	char file6[] = "hibberts.rooms.$$/room6";	
	char file7[] = "hibberts.rooms.$$/room7";*/	

	// create 7 file descriptors and open the files for reading and writing, creating them if they don't
	// exist, and if they do exist write over their contents
	
/*	int file_descriptor[7];
	
	for (i = 0; i < 7; i++)
	{
		file_descriptor[i] = open(filePath[i], O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	}	
*/
	/*int file_descriptor1 = open(file1, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	int file_descriptor2 = open(file2, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	int file_descriptor3 = open(file3, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	int file_descriptor4 = open(file4, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	int file_descriptor5 = open(file5, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	int file_descriptor6 = open(file6, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	int file_descriptor7 = open(file7, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);*/
	
	// write the second gameRoom structs to each of the 7 files

	// close the directory
	//closedir("hibberts.rooms.$$");
	
	// free memory for each gameRoom struct
	for (i = 0; i < 7; i++)
	{
		free(gameRoom[i]);
	}

	return 0;
}
