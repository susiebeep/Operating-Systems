
/* ********************************************************************************** 
 ** Program Name: Program 2 - Adventure
 ** Author:  Susan Hibbert
 ** Date: 21st April 2020  			      
 ** Description:
 ** *******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

struct room
{
	char* name;	
	char* type;	// START_ROOM, END_ROOM or MID_ROOM
	int numOutboundConnections;	// to keep track of number of outbound connections to other rooms
	struct room* outBoundConnections[6];	//at least 3, and at most 6
};


/* ********************************************************************************** 
 ** Description:
 ** Input(s):
 ** Output(s):
 ** Returns:
 ** *******************************************************************************/
/* int isGraphFull()
{
	// returns 0 if false (not full) and 1 if true (full)
	// i.e. returns true if all rooms have 3-6 outbound connections, false otherwise
}

*/
/* ********************************************************************************** 
 ** Description: 
 ** Input(s):
 ** Output(s):
 ** Returns:
 ** *******************************************************************************/
/*struct room getRandomRoom()
{
	// returns a random struct room, does not validate if connection can be added
}

*/
/* ********************************************************************************** 
 ** Description: Returns whether a connection can be added to a room (i.e. if it has
		 less than 6 connections to other rooms)
 ** Input(s): 	 Pointer to a room struct
 ** Output(s):
 ** Returns: 	 Returns 1 (true) if room has less than 6 connections, else returns
	    	 0 (false)
 ** *******************************************************************************/
int canAddConnection(struct room* x)
{
	if (x->numOutboundConnections < 6)
		return 1;
	else
		return 0;
}


/* ********************************************************************************** 
 ** Description:
 ** Input(s):
 ** Output(s):
 ** Returns:
 ** *******************************************************************************/
/*int connectionExists(struct room* x, struct room* y)
{
	// returns true if a connection exists between room x and y
}

*/
/* ********************************************************************************** 
 ** Description:
 ** Input(s):
 ** Output(s):
 ** Returns:
 ** *******************************************************************************/
/*void connectRoom(struct room* x, struct room* y)
{
	//connects room x and y together - does not check if connection is valid	
}

*/
/* ********************************************************************************** 
 ** Description:
 ** Input(s):
 ** Output(s):
 ** Returns:
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
 ** Input(s):
 ** Output(s):
 ** Returns:
 ** *******************************************************************************/
/*void addRandomConnection()
{
	struct room A;
	struct room B;
	
	while()
	{
		A = getRandomRoom();
		if (canAddConnection(A) == 1)
			break;
	}
	do
	{
		B = getRandomRoom();
	}
	while(canAddConnection(B) == 0 || isSameRoom(A,B) == 1 || connectionExists(A, B) == 1)
	{
		connectRoom(A, B);
		connectRoom(B, A);
	}
}

*/
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
	struct room gameRoom[7];	

	// create a 10 char arrays which are 9 characters each (8 characters plus 1 for null terminator)
	// to represent the room names to be randomly assigned to each of the rooms
	char roomName[10][9] = { "Library", "Arcade", "Cellar", "Kitchen", "Pool", "Hallway", "Bathroom", "Lab", "Bedroom", "Garage"};

	// create a 3 char arrays which are 11 characters each (10 characters plus 1 for null terminator)
	// to represent the room types to be randomly assigned to each of the rooms (only one room can be
	// assigned START_ROOM and END_ROOM)
	char roomType[3][11] = {"START_ROOM", "END_ROOM", "MID_ROOM"}; 
	
	
	// assign the room names at random to each of the rooms
	int i;
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
		gameRoom[i].name = roomName[num];

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
				gameRoom[i].type = roomType[0];
				start = 1;
			}
			else
				// if START_ROOM has been assigned, check if END_ROOM has been assigned,
				// if it hasn't been assigned yet assign it to the room
				if (end == 0)
				{
					gameRoom[i].type = roomType[1];
					end = 1;
				}
				// otherwise, if a START_ROOM and END_ROOM have both been assigned, 
				// assign it to be a MID_ROOM
				else
				{
					gameRoom[i].type = roomType[2];
				}
				
		}
		// if END_ROOM comes up
		if (num1 == 1)
		{ 
			// if END_ROOM has not yet been assigned
			if (end == 0)
			{
				gameRoom[i].type = roomType[1];
				end = 1;
			}
			else
				// if END_ROOM has been assigned, check if START_ROOM has been assigned,
				// if it hasn't been assigned yet assign it to the room
				if (start == 0)
				{
					gameRoom[i].type = roomType[0];
					start = 1;
				}
				// otherwise, if a END_ROOM and START_ROOM have both been assigned, 
				// assign it to be a MID_ROOM
				else
				{
					gameRoom[i].type = roomType[2];
				}
		}
		// if MID_ROOM comes up, assign it to the room
		if (num1 == 2)
		{
			gameRoom[i].type = roomType[2];
		}
	}

	/*for (i = 0; i < 7; i++)
	{
		printf("room %d type: %s\n", i, gameRoom[i].type);
		printf("room %d name: %s\n", i, gameRoom[i].name);
	}*/

	//create all connections in the graph
	//while(isGraphFull() == 0)
	//{
	//	addRandomConnection();
	//}

	// create a directory with standard permissions
	//mkdir("hibberts.rooms.$$",0755);

	// open the directory
	//opendir("hibberts.rooms.$$");

	// generate 7 different room files within the directory, one gameRoom struct per file


	return 0;
}
