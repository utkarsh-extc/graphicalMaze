#include<stdio.h>
#include<stdlib.h>
#include<string.h>

/*
stack using linked list
maze path finder
grid using array


moves	Dir			vert	  horiz
N		6			-1			0
//NE	7			-1			1
E		0			0			1
//SE	1			1			1
S		2			1			0
//SW	3			1			-1
W		4			0			-1
//NW	5			-1			-1

*/

#define FALSE 0
#define TRUE 1

typedef struct {
	int vert;
	int horitz;
}offset;

offset move[8] = { {0,1},
// {1,1},
{1,0},
// {1,-1},
{0,-1},
// {-1,-1},
{-1,0},
// {-1,1},
};

typedef struct Element {
	int row;
	int col;
	int dir;
}element;

typedef struct stack {
	element data;
	struct stack *next;
}stack_t;

stack_t *gStack = NULL;

stack_t* CreateElement(element aNewElement)
{
	stack_t *newElement = (stack_t *)malloc(sizeof(stack_t));
	if (!newElement) {
		printf("\n Unable to allocate memory \n");
	}
	memset(newElement, 0, sizeof(stack_t));
	newElement->data.row = aNewElement.row;
	newElement->data.col = aNewElement.col;
	newElement->data.dir = aNewElement.dir;
	newElement->next = NULL;
	return newElement;
}

void Push(element aNewElement)
{
	stack_t *newNode = CreateElement(aNewElement);
	stack_t *trav = NULL;
	if (!gStack)
	{
		//empty stack
		gStack = newNode;
	}
	else
	{
		trav = gStack;
		while (trav->next)
		{
			trav = trav->next;
		}
		trav->next = newNode;
	}
}

element Pop(void)
{
	stack_t *trav, *prev = NULL;
	element data = { -1,-1,-1 };
	if (!gStack->next)
	{
		//only one element
		data = gStack->data;
		gStack = NULL;
		return(data);
	}
	else
	{
		trav = gStack;
		while (trav->next)
		{
			prev = trav;
			trav = trav->next;
		}
		data = trav->data;
		free(trav);
		prev->next = NULL;
	}
	return(data);
}

#define EXIT_ROW 6
#define EXIT_COL 16

#define MAZE_ROW 7
#define MAZE_COL 17

int mark[MAZE_ROW][MAZE_COL];
int maze[MAZE_ROW][MAZE_COL] = {
	{0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0},
	{1,0,1,1,0,0,0,0,0,0,1,1,1,1,1,1,0},
	{1,0,0,0,1,0,1,1,0,1,1,1,1,1,0,1,0},
	{1,1,1,0,1,1,1,1,0,0,0,0,0,0,0,1,0},
	{1,0,0,0,0,1,0,0,0,1,1,1,0,1,0,1,1},
	{1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
	{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
};

int main()
{
	int row, col, nextRow, nextCol, dir, found = FALSE;
	element temp;
	memset(&temp, -1, sizeof(element));
	mark[0][0] = 1;
	memset(&temp, 0, sizeof(element));
	Push(temp);
	while (!found)
	{
		temp = Pop();
		row = temp.row;
		col = temp.col;
		dir = temp.dir;
		printf("row: %d col: %d dir:%d \n", row, col, dir);
		while (dir < (sizeof(move) / sizeof(move[0])) && !found)
		{
			//move in direction dir
			nextRow = row + move[dir].vert;
			nextCol = col + move[dir].horitz;
			if (nextRow == EXIT_ROW && nextCol == EXIT_COL)
			{
				mark[nextRow][nextCol] = 1;
				found = TRUE;
			}
			else if (((0 <= nextCol && nextCol < MAZE_COL) && (0 <= nextRow && nextRow < MAZE_ROW)) && !maze[nextRow][nextCol] && !mark[nextRow][nextCol])
			{
				printf("row: %d col: %d dir:%d \n", nextRow, nextCol, dir);
				mark[nextRow][nextCol] = 1;
				temp.row = row;
				temp.col = col;
				temp.dir = ++dir;
				Push(temp);
				row = nextRow;
				col = nextCol;
				dir = 0;
			}
			else
			{
				++dir;
			}
		}
	}

	temp.row = row;
	temp.col = col;
	temp.dir = dir;
	Push(temp);

	temp.row = EXIT_ROW;
	temp.col = EXIT_COL;
	temp.dir = dir;

	Push(temp);

	if (found)
	{
		stack_t *trav = gStack;
		printf("\n the Path is : \n");
		printf(" row col \n");
		while (trav)
		{
			printf("%2d%5d\n", trav->data.row, trav->data.col);
			trav = trav->next;
		}
	}
	else
	{
		printf("\n No Path found\n");
	}

	return(0);
}