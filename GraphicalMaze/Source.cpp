#include<Windows.h>
#include<stdio.h>
#include<gl/GL.h>
#include<gl/GLU.h>
#include <GL/glu.h>
#include<math.h>
#include"resource.h"
#include <string.h>

#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"glu32.lib")
#pragma comment(lib,"Winmm.lib")

#define WIN_WIDTH 800
#define WIN_HEIGHT 600

//callback
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

//global variables
HWND ghWnd = NULL;
bool isFullScreen = false;
DWORD dwStyle;
WINDOWPLACEMENT wpPrev = { sizeof(WINDOWPLACEMENT) };

FILE *gpFile = NULL;
HDC ghdc = NULL;
HGLRC ghrc = NULL;
bool gbActiveWindow = false;

bool bLight = false;

typedef struct Light {
	GLfloat Ambient[4];
	GLfloat Diffuse[4];
	GLfloat Specular[4];
	GLfloat Position[4];
	GLfloat angle;
}Light_T;

Light_T gLight[3];


EXTERN_C_START
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
	int reset;
}element;

typedef struct stack {
	element data;
	struct stack *next;
}stack_t;

stack_t *gStack = NULL;
stack_t *gPath = NULL;

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
	newElement->data.reset = aNewElement.reset;
	newElement->next = NULL;
	return newElement;
}

void Push(stack **stack, element aNewElement)
{
	stack_t *newNode = CreateElement(aNewElement);
	stack_t *trav = NULL;
	if (!*stack)
	{
		//empty stack
		*stack = newNode;
	}
	else
	{
		trav = *stack;
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
	element data = { -1,-1,-1 ,0 };
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

#define EXIT_ROW 24
#define EXIT_COL 24

#define MAZE_ROW 25
#define MAZE_COL 25

int mark[MAZE_ROW][MAZE_COL];
int maze[MAZE_ROW][MAZE_COL];

void initMazeMark(const char *filename)
{
	memset(&mark, 0, sizeof(mark));
	memset(&maze, 0, sizeof(maze));
	FILE *fPtr;
	int numberOfElements = MAZE_COL * MAZE_ROW;
	if (fopen_s(&fPtr, filename, "r") != 0) {
		MessageBox(NULL, TEXT("Unable to Read Sample File"), TEXT("FILE IO ERROR"), MB_OK);
	}
	for (int i = 0; i < MAZE_ROW; i++)
	{
		for (int j = 0; j < MAZE_COL; j++)
		{
			fscanf_s(fPtr, "%d ", &maze[i][j]);
		}
	}
	fclose(fPtr);
}

void findTrack()
{
	int row, col, nextRow, nextCol, dir, found = FALSE;
	element temp;
	memset(&temp, -1, sizeof(element));
	mark[0][0] = 1;
	memset(&temp, 0, sizeof(element));
	Push(&gStack, temp);
	while (!found)
	{
		temp = Pop();
		row = temp.row;
		col = temp.col;
		dir = temp.dir;
		temp.reset = 1;
		//printf("row: %d col: %d dir:%d \n", row, col, dir);
		Push(&gPath, temp);
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
				Push(&gPath, Element{ nextRow,nextCol,dir,0 });
				//	printf("row: %d col: %d dir:%d \n", nextRow, nextCol, dir);
				mark[nextRow][nextCol] = 1;
				temp.row = row;
				temp.col = col;
				temp.dir = ++dir;
				Push(&gStack, temp);
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
	Push(&gStack, temp);

	temp.row = EXIT_ROW;
	temp.col = EXIT_COL;
	temp.dir = dir;

	Push(&gStack, temp);

	if (found)
	{
		stack_t *trav = gStack;
		//printf("\n the Path is : \n");
		//printf(" row col \n");
		while (trav)
		{
			//printf("%2d%5d\n", trav->data.row, trav->data.col);
			trav = trav->next;
		}
	}
	else
	{
		//printf("\n No Path found\n");
	}

}

EXTERN_C_END


#define CHECK_IMAGE_WIDTH (8*MAZE_ROW)
#define CHECK_IMAGE_HEIGHT (8*MAZE_COL)
stack_t *trav = NULL;

GLbyte checkImage[CHECK_IMAGE_WIDTH][CHECK_IMAGE_HEIGHT][4];

GLuint textureImage;

GLubyte space[] =
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

GLubyte letters[][13] = {
{0x00, 0x00, 0xc3, 0xc3, 0xc3, 0xc3, 0xff, 0xc3, 0xc3, 0xc3, 0x66, 0x3c, 0x18},
{0x00, 0x00, 0xfe, 0xc7, 0xc3, 0xc3, 0xc7, 0xfe, 0xc7, 0xc3, 0xc3, 0xc7, 0xfe},
{0x00, 0x00, 0x7e, 0xe7, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xe7, 0x7e},
{0x00, 0x00, 0xfc, 0xce, 0xc7, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc7, 0xce, 0xfc},
{0x00, 0x00, 0xff, 0xc0, 0xc0, 0xc0, 0xc0, 0xfc, 0xc0, 0xc0, 0xc0, 0xc0, 0xff},
{0x00, 0x00, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xfc, 0xc0, 0xc0, 0xc0, 0xff},
{0x00, 0x00, 0x7e, 0xe7, 0xc3, 0xc3, 0xcf, 0xc0, 0xc0, 0xc0, 0xc0, 0xe7, 0x7e},
{0x00, 0x00, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xff, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3},
{0x00, 0x00, 0x7e, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x7e},
{0x00, 0x00, 0x7c, 0xee, 0xc6, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06},
{0x00, 0x00, 0xc3, 0xc6, 0xcc, 0xd8, 0xf0, 0xe0, 0xf0, 0xd8, 0xcc, 0xc6, 0xc3},
{0x00, 0x00, 0xff, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0},
{0x00, 0x00, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xdb, 0xff, 0xff, 0xe7, 0xc3},
{0x00, 0x00, 0xc7, 0xc7, 0xcf, 0xcf, 0xdf, 0xdb, 0xfb, 0xf3, 0xf3, 0xe3, 0xe3},
{0x00, 0x00, 0x7e, 0xe7, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xe7, 0x7e},
{0x00, 0x00, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xfe, 0xc7, 0xc3, 0xc3, 0xc7, 0xfe},
{0x00, 0x00, 0x3f, 0x6e, 0xdf, 0xdb, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0x66, 0x3c},
{0x00, 0x00, 0xc3, 0xc6, 0xcc, 0xd8, 0xf0, 0xfe, 0xc7, 0xc3, 0xc3, 0xc7, 0xfe},
{0x00, 0x00, 0x7e, 0xe7, 0x03, 0x03, 0x07, 0x7e, 0xe0, 0xc0, 0xc0, 0xe7, 0x7e},
{0x00, 0x00, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0xff},
{0x00, 0x00, 0x7e, 0xe7, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3},
{0x00, 0x00, 0x18, 0x3c, 0x3c, 0x66, 0x66, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3},
{0x00, 0x00, 0xc3, 0xe7, 0xff, 0xff, 0xdb, 0xdb, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3},
{0x00, 0x00, 0xc3, 0x66, 0x66, 0x3c, 0x3c, 0x18, 0x3c, 0x3c, 0x66, 0x66, 0xc3},
{0x00, 0x00, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3c, 0x3c, 0x66, 0x66, 0xc3},
{0x00, 0x00, 0xff, 0xc0, 0xc0, 0x60, 0x30, 0x7e, 0x0c, 0x06, 0x03, 0x03, 0xff}
};

GLuint fontOffset;

void makeRasterFont(void)
{
	GLuint i, j;
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	fontOffset = glGenLists(128);
	for (i = 0, j = 'A'; i < 26; i++, j++) {
		glNewList(fontOffset + j, GL_COMPILE);
		glBitmap(8, 10, 0.0, 2.0, 10.0, 0.0, letters[i]);
		glEndList();
	}
	glNewList(fontOffset + ' ', GL_COMPILE);
	glBitmap(8, 10, 0.0, 2.0, 10.0, 0.0, space);
	glEndList();
}


void printString(const char *s)
{
	glPushAttrib(GL_LIST_BIT);
	glListBase(fontOffset);
	glCallLists((GLsizei)strlen(s), GL_UNSIGNED_BYTE, (GLubyte *)s);
	glPopAttrib();
}

static unsigned int count = 1;

void update(void)
{
	void makeCheckImage(void);

	gLight[0].angle += 0.05f;
	if (gLight[0].angle >= 360)
		gLight[0].angle = 0.0f;

	gLight[1].angle += 0.05f;
	if (gLight[1].angle >= 360)
		gLight[1].angle = 0.0f;

	gLight[2].angle += 0.05f;
	if (gLight[2].angle >= 360)
		gLight[2].angle = 0.0f;

	//stack_t *trav = NULL;
	if (trav == NULL) {
		if (count == 0)
		{
			PlaySound(NULL, 0, 0);
			//play sync cheering music
			PlaySound(MAKEINTRESOURCE(ID_CHEER_FILE), NULL, SND_SYNC | SND_NODEFAULT | SND_RESOURCE);
			// again play ID_ 100
			PlaySound(MAKEINTRESOURCE(ID_RES_FILE), NULL, SND_ASYNC | SND_NODEFAULT | SND_RESOURCE);

			// turn on Lighting
			//bLight = true;
			//glEnable(GL_LIGHTING);
		}
		trav = gPath;
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, checkImage);
		makeCheckImage();
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, CHECK_IMAGE_HEIGHT, CHECK_IMAGE_WIDTH, GL_RGBA, GL_UNSIGNED_BYTE, checkImage);
	}
	else {
		count = count + 1;
		if (count > 100)
		{
			trav = trav->next;
			count = 0;
		}
		else
		{
			//return;
		}
	}

	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, checkImage);
	if (trav != NULL)
	{
		//for each of trav->data.row, trav->data.col make all pixels green
		for (int i = trav->data.row * 8; i < (trav->data.row + 1) * 8; i++)
		{
			for (int j = trav->data.col * 8; j < (trav->data.col + 1) * 8; j++)
			{
				if (trav->data.reset) {
					checkImage[CHECK_IMAGE_WIDTH - i][j][0] = (GLbyte)255;
					checkImage[CHECK_IMAGE_WIDTH - i][j][1] = (GLbyte)128;
					checkImage[CHECK_IMAGE_WIDTH - i][j][2] = (GLbyte)0;
					checkImage[CHECK_IMAGE_WIDTH - i][j][3] = (GLbyte)255;
				}
				else
				{
					checkImage[CHECK_IMAGE_WIDTH - i][j][0] = (GLbyte)0;
					checkImage[CHECK_IMAGE_WIDTH - i][j][1] = (GLbyte)255;
					checkImage[CHECK_IMAGE_WIDTH - i][j][2] = (GLbyte)0;
					checkImage[CHECK_IMAGE_WIDTH - i][j][3] = (GLbyte)255;
				}
			}
		}
	}
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, CHECK_IMAGE_HEIGHT, CHECK_IMAGE_WIDTH, GL_RGBA, GL_UNSIGNED_BYTE, checkImage);

}

//WinMain
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
	//functions 
	int initialize(void);
	void display(void);

	//variable declarations
	HWND hWnd;
	WNDCLASSEX wndClass;
	MSG msg;
	TCHAR szAppName[] = TEXT("MyApp");

	int iRet = 0;
	bool bDone = false;

	//code
	//create log file
	if (fopen_s(&gpFile, "Log.txt", "w") != 0)
	{
		MessageBox(NULL, TEXT("Log File can not be created"), TEXT("FILE IO ERROR"), MB_OK);
		exit(0);
	}
	else
	{
		fprintf(gpFile, "Successfully created Log File\n");
	}
	//initialize WNDCLASSEX
	wndClass.cbClsExtra = 0;
	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.cbWndExtra = 0;
	wndClass.hbrBackground = (HBRUSH)GetStockObject(GRAY_BRUSH);
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	wndClass.hInstance = hInstance;
	wndClass.lpfnWndProc = WndProc;
	wndClass.lpszClassName = szAppName;
	wndClass.lpszMenuName = NULL;
	wndClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;//do not discard my handle to device context

	//register class
	RegisterClassEx(&wndClass);

	hWnd = CreateWindowEx(WS_EX_APPWINDOW,
		szAppName,
		TEXT("MAZE - Utkarsh"),
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
		(GetSystemMetrics(SM_CXFULLSCREEN) - WIN_WIDTH) / 2,
		(GetSystemMetrics(SM_CYFULLSCREEN) - WIN_HEIGHT) / 2,
		WIN_WIDTH,
		WIN_HEIGHT,
		NULL,
		NULL,
		hInstance,
		NULL);
	ghWnd = hWnd;
	iRet = initialize();
	//check for iRet value
	if (iRet == -1)
	{
		fprintf(gpFile, "ChoosePixelFormat() failed\n");
		DestroyWindow(hWnd);
	}
	else if (iRet == -2)
	{
		fprintf(gpFile, "SetPixelFormat() failed\n");
		DestroyWindow(hWnd);
	}
	else if (iRet == -3)
	{
		fprintf(gpFile, "wglCreateContext() failed\n");
		DestroyWindow(hWnd);
	}
	else if (iRet == -4)
	{
		fprintf(gpFile, "wglMakeCurrent() failed\n");
		DestroyWindow(hWnd);
	}
	else
	{
		fprintf(gpFile, "Initialize is successfully done\n");
	}

	ShowWindow(hWnd, iCmdShow);
	SetForegroundWindow(hWnd);
	SetFocus(hWnd);

	//game loop
	while (bDone == false)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				bDone = true;
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			if (gbActiveWindow == true)
			{
				//Here call update
				update();
			}
			//Here call display() play game
			display();
		}
	}

	return((int)msg.wParam);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	//function declarations
	void toggleFullScreen(void);

	void resize(int, int);
	void uninitialize(void);

	//code
	switch (iMsg)
	{
	case WM_ERASEBKGND:
		//dont go to DefWindowProc
		return(0);
		break;
	case WM_SETFOCUS:
		gbActiveWindow = true;
		break;
	case WM_KILLFOCUS:
		gbActiveWindow = false;
		break;
	case WM_SIZE:
		resize(LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_ESCAPE:
			DestroyWindow(hWnd);
			break;
		case 0x46:
			toggleFullScreen();
			break;
		}
		break;
	case WM_DESTROY:
		uninitialize();
		PostQuitMessage(0);
		break;
	}
	return(DefWindowProc(hWnd, iMsg, wParam, lParam));
}

void toggleFullScreen(void)
{
	//variable 
	MONITORINFO mi;

	//code
	if (isFullScreen == false)
	{
		dwStyle = GetWindowLong(ghWnd, GWL_STYLE);
		if (dwStyle & WS_OVERLAPPEDWINDOW)
		{
			mi = { sizeof(MONITORINFO) };
			if (GetWindowPlacement(ghWnd, &wpPrev) &&
				GetMonitorInfo(MonitorFromWindow(ghWnd, MONITORINFOF_PRIMARY), &mi))
			{
				SetWindowLong(ghWnd, GWL_STYLE, dwStyle &(~WS_OVERLAPPEDWINDOW));
				SetWindowPos(ghWnd,
					HWND_TOP,
					mi.rcMonitor.left,
					mi.rcMonitor.top,
					mi.rcMonitor.right - mi.rcMonitor.left,
					mi.rcMonitor.bottom - mi.rcMonitor.top,
					SWP_NOZORDER | SWP_FRAMECHANGED);
			}
		}
		ShowCursor(FALSE);
		isFullScreen = true;
	}
	else
	{
		SetWindowLong(ghWnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(ghWnd, &wpPrev);
		SetWindowPos(ghWnd,
			HWND_TOP,
			0,
			0,
			0,
			0,
			SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOSIZE);
		ShowCursor(TRUE);
		isFullScreen = false;
	}
}

void initLights(void) {
	gLight[0] = {
		{ 0.0f,0.0f,0.0f,1.0f },
		{ 1.0f,0.0f,0.0f,1.0f },
		{ 1.0f,0.0f,0.0f,1.0f },
		{ 0.0f,0.0f,0.0f,1.0f },
		0.0f,
	};

	gLight[1] = {
		{ 0.0f,0.0f,0.0f,1.0f },
		{ 0.0f,1.0f,0.0f,1.0f },
		{ 0.0f,1.0f,0.0f,1.0f },
		{ 0.0f,0.0f,0.0f,1.0f },
		0.0f,
	};

	gLight[2] = {
		{ 0.0f,0.0f,0.0f,1.0f },
		{ 0.0f,0.0f,1.0f,1.0f },
		{ 0.0f,0.0f,1.0f,1.0f },
		{ 0.0f,0.0f,0.0f,1.0f },
		0.0f,
	};

}


int initialize(void)
{
	//function
	void resize(int, int);
	void LoadTexture(void);
	void findTrack(void);
	void initLights(void);

	//varible
	PIXELFORMATDESCRIPTOR pfd;
	int iPixelFormatDescriptor;

	//code
	//init PixelFormatDescriptor
	ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1; // this value is 1 as openGL support is frezed at 1.5 by windowsOS
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cRedBits = 8;
	pfd.cGreenBits = 8;
	pfd.cBlueBits = 8;
	pfd.cAlphaBits = 8;
	pfd.cDepthBits = 32;
	//get dc
	ghdc = GetDC(ghWnd);
	//find
	iPixelFormatDescriptor = ChoosePixelFormat(ghdc, &pfd);
	if (iPixelFormatDescriptor == 0)
	{
		return(-1);
	}
	if (SetPixelFormat(ghdc, iPixelFormatDescriptor, &pfd) == FALSE)
	{
		return(-2);
	}
	ghrc = wglCreateContext(ghdc);
	if (ghrc == NULL)
	{
		return(-3);
	}
	if (wglMakeCurrent(ghdc, ghrc) == FALSE)
	{
		return(-4);
	}
	glShadeModel(GL_SMOOTH);
	//clear screen by openGL color (openGL is state machine)
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepth(1.0f);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	//init lights
	initLights();

	//config light
	glLightfv(GL_LIGHT0, GL_AMBIENT, gLight[0].Ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, gLight[0].Diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, gLight[0].Specular);
	//glLightfv(GL_LIGHT0, GL_POSITION, LightPositionZero);
	glEnable(GL_LIGHT0);

	glLightfv(GL_LIGHT1, GL_AMBIENT, gLight[1].Ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, gLight[1].Diffuse);
	glLightfv(GL_LIGHT1, GL_SPECULAR, gLight[1].Specular);
	//glLightfv(GL_LIGHT1, GL_POSITION, LightPositionOne);
	glEnable(GL_LIGHT1);

	glLightfv(GL_LIGHT2, GL_AMBIENT, gLight[2].Ambient);
	glLightfv(GL_LIGHT2, GL_DIFFUSE, gLight[2].Diffuse);
	glLightfv(GL_LIGHT2, GL_SPECULAR, gLight[2].Specular);
	//glLightfv(GL_LIGHT2, GL_POSITION, LightPositionTwo);
	glEnable(GL_LIGHT2);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	initMazeMark("sampleMaze.txt");

	glEnable(GL_TEXTURE_2D);
	LoadTexture();

	makeRasterFont();

	findTrack();
	count = 1;
	PlaySound(MAKEINTRESOURCE(ID_RES_FILE), NULL, SND_ASYNC | SND_NODEFAULT | SND_RESOURCE);

	//WarmUp call to resize(int,int)
	resize(WIN_WIDTH, WIN_HEIGHT);
	return(0);
}

int gWidth, gHeight;

void resize(int width, int height)
{
	gWidth = width;
	gHeight = height;
	if (height == 0)
	{
		height = 1;
	}
	glViewport(0, 0, (GLsizei)width, (GLsizei)height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0f, GLfloat(width) / GLfloat(height), 1.0f, 30.0f);
}

void display(void)
{
	//code
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glPushMatrix();

	gluLookAt(0.0f, 0.0f, 3.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

	//Light0
	glPushMatrix();
	//pitching
	glRotatef(gLight[0].angle, 1.0, 0.0, 0.0);
	//only y and z changes
	gLight[0].Position[1] = gLight[0].angle;
	glLightfv(GL_LIGHT0, GL_POSITION, gLight[0].Position);
	glPopMatrix();

	//Light1
	glPushMatrix();
	//spinning
	glRotatef(gLight[1].angle, 0.0, 1.0, 0.0);
	//while spinning only x and z changes
	gLight[1].Position[2] = gLight[1].angle;
	glLightfv(GL_LIGHT1, GL_POSITION, gLight[1].Position);
	glPopMatrix();

	//Light2
	glPushMatrix();
	//rotate
	glRotatef(gLight[2].angle, 0.0, 0.0, 1.0);
	//while spinning only x and y changes
	gLight[2].Position[0] = gLight[2].angle;
	glLightfv(GL_LIGHT2, GL_POSITION, gLight[2].Position);
	glPopMatrix();
	glBindTexture(GL_TEXTURE_2D,0);


	glRasterPos2i(1100, 10);
	printString("THE QUICK BROWN FOX JUMPS");
	glRasterPos2i(1200, 40);
	printString("OVER A LAZY DOG");

	glTranslatef(0.0f, 0.0f, -2.2f);

	glBindTexture(GL_TEXTURE_2D, textureImage);
	glBegin(GL_QUADS);

	glTexCoord2f(0.0f, 0.01f);
	glVertex3f(-1.5f, -1.0f, 0.0f);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(-1.5f, 1.0f, 0.0f);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(0.5f, 1.0f, 0.0f);
	glTexCoord2f(1.0f, 0.01f);
	glVertex3f(0.5f, -1.0f, 0.0f);

	glEnd();

	SwapBuffers(ghdc);
}

void LoadTexture(void)
{
	void makeCheckImage(void);

	makeCheckImage();

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &textureImage);
	glBindTexture(GL_TEXTURE_2D, textureImage);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, CHECK_IMAGE_HEIGHT, CHECK_IMAGE_WIDTH, 0, GL_RGBA, GL_UNSIGNED_BYTE, checkImage);

	// how pixel color will be modulated
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_INCR);
}

void makeCheckImage(void)
{
	int i, j, c, x, y;
	for (i = 0; i < CHECK_IMAGE_WIDTH; i++)
	{
		for (j = 0; j < CHECK_IMAGE_HEIGHT; j++)
		{
			//c = (((i & 0x08) == 0) ^ ((j & 0x08) == 0)) * 255;
			x = abs(int(floor(i / 0x08)));
			y = abs(int(floor(j / 0x08)));
			c = maze[x][y] * 255;
			checkImage[CHECK_IMAGE_WIDTH - i][j][0] = (GLbyte)c;
			checkImage[CHECK_IMAGE_WIDTH - i][j][1] = (GLbyte)c;
			checkImage[CHECK_IMAGE_WIDTH - i][j][2] = (GLbyte)c;
			checkImage[CHECK_IMAGE_WIDTH - i][j][3] = (GLbyte)255;
		}
	}
}

void uninitialize(void)
{
	//check for fullscreen if it is then restore normal size and proceed for uninit
	if (isFullScreen == true)
	{
		SetWindowLong(ghWnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(ghWnd, &wpPrev);
		SetWindowPos(ghWnd,
			HWND_TOP,
			0,
			0,
			0,
			0,
			SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOSIZE);
		ShowCursor(TRUE);
	}
	//break the current context
	if (wglGetCurrentContext() == ghrc)
	{
		wglMakeCurrent(NULL, NULL);
	}
	if (ghrc)
	{
		wglDeleteContext(ghrc);
		ghrc = NULL;
	}
	if (ghdc)
	{
		ReleaseDC(ghWnd, ghdc);
		ghdc = NULL;
	}
	if (gpFile)
	{
		fprintf(gpFile, "Log file Closed Successfully\n");
		fclose(gpFile);
		gpFile = NULL;
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	glDeleteTextures(1, &textureImage);
}
