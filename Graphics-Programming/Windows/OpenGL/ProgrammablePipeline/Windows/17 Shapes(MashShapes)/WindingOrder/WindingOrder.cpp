#include <Windows.h>
#include <GL/glew.h>
#include <GL/GL.h>
#include <stdio.h> // FILE
#include "vmath.h"

#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "opengl32.lib")

#define WIN_WIDTH	800
#define WIN_HEIGHT	600
#define ERROR_SIZE 1024

using namespace vmath;

enum GLGetErrorType
{
	enGlGET_Shader,
	enGlGET_Program
};


enum
{
	AMC_ATTRIBUTE_POSITION = 0,
	AMC_ATTRIBUTE_COLOR,
	AMC_ATTRIBUTE_NORMAL,
	AMC_ATTRIBUTE_TEXCORD0,

};

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
GLboolean GetGlErrorStatus(GLGetErrorType, GLuint, char*);

HWND ghWnd = NULL;
HDC ghDC = NULL;
HGLRC ghGlRC = NULL;

bool gbIsActiveWindow = false;
bool gbIsFullScreen = false;

FILE *gpLogFile = NULL;

DWORD gdwStyle = 0;
WINDOWPLACEMENT gwpPrev = { sizeof(WINDOWPLACEMENT) };

GLenum gGlEnumResult;
GLuint gGlShaderProgramObject;
GLuint vao;
GLuint vboPosition;
GLuint vboColor;
GLuint mvpUniform;
mat4 perspectiveProjectionMatrix;

GLuint vboPointElement;
int giPointElements = 0;

GLuint vboLinesElement;
int giLinesElements = 0;

GLuint vboSquareElement;
int giSquareElements = 0;

GLuint vboDiagonalSquareElement;
int giDigonalSquareElements = 0;

GLuint vboFanElement;
int giFanElements = 0;

GLuint vaoClrSqr;
GLuint vboColorSquare;
GLuint vboColorSquareElement;
int giColorSquareElements = 0;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR szCmdLine, int iCmdShow)
{
	int initialize();
	void display();

	bool bIsDone = false;
	int iRet = 0;
	TCHAR szAppName[] = TEXT("Programmable pipeline first window");
	TCHAR szErrMsg[MAX_PATH];
	WNDCLASSEX wndclass;
	HWND hWnd;
	MSG msg;

	iRet = fopen_s(&gpLogFile, "Log.txt", "w");
	if (0 != iRet)
	{
		ZeroMemory(szErrMsg, MAX_PATH);
		//StringCchPrintfW(szErrMsg, MAX_PATH, TEXT("Log file creation failed.\nError code : %d"), GetLastError());
		wsprintf(szErrMsg, TEXT("Log file creation failed.\nError code : %d"), GetLastError());
		MessageBox(NULL, szErrMsg, TEXT("Error"), MB_OK | MB_ICONERROR);
		return 0;
	}

	fprintf_s(gpLogFile, "Log file created.\n");

	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hInstance = hInstance;
	wndclass.lpfnWndProc = WndProc;
	wndclass.lpszClassName = szAppName;
	wndclass.lpszMenuName = NULL;
	wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;

	if (0 == RegisterClassEx(&wndclass))
	{
		return 0;
	}

	hWnd = CreateWindowEx(WS_EX_APPWINDOW,
		szAppName,
		TEXT("Programmble first window"),
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
		100,
		100,
		WIN_WIDTH,
		WIN_HEIGHT,
		NULL,
		NULL,
		hInstance,
		0);


	ghWnd = hWnd;
	iRet = initialize();

	if (iRet == -1)
	{
		fprintf_s(gpLogFile, "ChoosePixelFormat() failed.\n");
		DestroyWindow(hWnd);
	}
	else if (iRet == -2)
	{
		fprintf_s(gpLogFile, "SetPixelFormat() failed.\n");
		DestroyWindow(hWnd);
	}
	else if (iRet == -3)
	{
		fprintf_s(gpLogFile, "wglCreateContext() failed.\n");
		DestroyWindow(hWnd);
	}
	else if (iRet == -4)
	{
		fprintf_s(gpLogFile, "wglMakeCurrent() failed.\n");
		DestroyWindow(hWnd);
	}
	else if (iRet == -5)
	{
		fprintf_s(gpLogFile, "Error in programmable pipeline related code.\n");
		DestroyWindow(hWnd);
	}
	else
	{
		fprintf_s(gpLogFile, "initialize() successful.\n");
	}

	ShowWindow(hWnd, iCmdShow);
	UpdateWindow(hWnd);
	SetFocus(hWnd);

	while (false == bIsDone)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				bIsDone = true;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);

		}
		else
		{
			if (gbIsActiveWindow == true)
			{
				display();
			}
			else
			{

			}
		}
	}
	return ((int)msg.wParam);
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	void resize(int, int);
	//void display();
	void ToggleFullScreen();
	void uninitialize();

	switch (iMsg)
	{
	case WM_SETFOCUS:
		gbIsActiveWindow = true;
		break;

	case WM_KILLFOCUS:
		gbIsActiveWindow = false;
		break;

	case WM_SIZE:
		resize(LOWORD(lParam), HIWORD(lParam));
		break;

	case WM_ERASEBKGND:
		return 0;

	case WM_PAINT:
		//display();
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
		}
		break;
	case WM_CHAR:
		switch (wParam)
		{
		case 'F':
		case 'f':
			ToggleFullScreen();
			break;
		}
		break;
	case WM_DESTROY:
		uninitialize();
		PostQuitMessage(0);
		break;
	}

	return DefWindowProc(hWnd, iMsg, wParam, lParam);
}

int initialize()
{
	void resize(int, int);
	char szglErrorStr[ERROR_SIZE] = "";
	GLboolean bErrorStatus = GL_TRUE;
	GLuint gGlVertexShaderObject;
	GLuint gGlFragmentShaderObject;

	PIXELFORMATDESCRIPTOR pfd;
	int iPixelFormatIndex = 0;

	memset((void*)&pfd, NULL, sizeof(PIXELFORMATDESCRIPTOR));

	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cDepthBits = 32;
	pfd.cRedBits = 8;
	pfd.cGreenBits = 8;
	pfd.cBlueBits = 8;
	pfd.cBlueBits = 8;
	pfd.cAlphaBits = 8;

	ghDC = GetDC(ghWnd);

	iPixelFormatIndex = ChoosePixelFormat(ghDC, &pfd);
	if (iPixelFormatIndex == 0)
	{
		return -1;
	}

	if (SetPixelFormat(ghDC, iPixelFormatIndex, &pfd) == FALSE)
	{
		return -2;
	}

	ghGlRC = wglCreateContext(ghDC);
	if (NULL == ghGlRC)
	{
		return -3;
	}

	if (wglMakeCurrent(ghDC, ghGlRC) == FALSE)
	{
		return -4;
	}

	gGlEnumResult = glewInit();
	if (GLEW_OK != gGlEnumResult)
	{
		fprintf_s(gpLogFile, "glewInit() failed.\n");
		return -5;
	}

	// define vertex shader object
	gGlVertexShaderObject = glCreateShader(GL_VERTEX_SHADER);

	// write vertex shader code
	const GLchar *pszVertexShaderSourceCode =
		"#version 450 core" \
		"\n" \
		"in vec4 vPosition;" \
		"in vec4 vColor;" \
		"uniform mat4 u_mvp_matrix;" \
		"out vec4 out_Color;" \
		"void main(void)" \
		"{" \
		"gl_Position = u_mvp_matrix * vPosition;" \
		"out_Color = vColor;" \
		"}";

	// specify above shader source code to vertex shader object
	glShaderSource(gGlVertexShaderObject,  // shader object
		1, // number of strings
		(const GLchar**)&pszVertexShaderSourceCode, // array of strings
		NULL // array of strings length
	);

	// compile vertex shader
	glCompileShader(gGlVertexShaderObject);

	// Error code checking function by AG
	memset(szglErrorStr, 0, ERROR_SIZE);
	strcpy_s(szglErrorStr, ERROR_SIZE, "Error in vertex shader object code : ");
	bErrorStatus = GetGlErrorStatus(enGlGET_Shader, gGlVertexShaderObject, szglErrorStr);
	if (GL_FALSE == bErrorStatus)
	{
		fprintf_s(gpLogFile, "%s.\n", szglErrorStr);
		return -5;
	}

	// define fragment shader object
	gGlFragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);

	// write fragment shader code
	const GLchar *pszFragmentShaderSourceCode[] = {
		"#version 450 core" \
		"\n" \
		"in vec4 out_Color;" \
		"out vec4 fragColor;" \
		"void main(void)" \
		"{"	\
		"fragColor = out_Color;" \
		"}" };

	// specify above shader source code to vertex shader object
	glShaderSource(gGlFragmentShaderObject,  // shader object
		1, // number of strings
		(const GLchar**)&pszFragmentShaderSourceCode, // array of strings
		NULL // array of strings length
	);

	// compile vertex shader
	glCompileShader(gGlFragmentShaderObject);

	// Error code checking function by AG
	memset(szglErrorStr, 0, ERROR_SIZE);
	strcpy_s(szglErrorStr, ERROR_SIZE, "Error in fragment shader object code : ");
	bErrorStatus = GetGlErrorStatus(enGlGET_Shader, gGlFragmentShaderObject, szglErrorStr);
	if (GL_FALSE == bErrorStatus)
	{
		fprintf_s(gpLogFile, "%s.\n", szglErrorStr);
		return -5;
	}

	// create shader program object
	gGlShaderProgramObject = glCreateProgram();

	// attach vertex shader to shader program
	glAttachShader(gGlShaderProgramObject, gGlVertexShaderObject);

	// attach fragment shader to shader program
	glAttachShader(gGlShaderProgramObject, gGlFragmentShaderObject);

	// prelinking binding to vertex attributes
	glBindAttribLocation(gGlShaderProgramObject, AMC_ATTRIBUTE_POSITION, "vPosition");

	// link the shader program
	glLinkProgram(gGlShaderProgramObject);

	// Error code checking function by AG
	memset(szglErrorStr, 0, ERROR_SIZE);
	strcpy_s(szglErrorStr, ERROR_SIZE, "Error in program object : ");
	bErrorStatus = GetGlErrorStatus(enGlGET_Program, gGlShaderProgramObject, szglErrorStr);
	if (GL_FALSE == bErrorStatus)
	{
		fprintf(gpLogFile, "%s.\n", szglErrorStr);
		return -5;
	}

	// post linking retreving uniform location
	mvpUniform = glGetUniformLocation(gGlShaderProgramObject, "u_mvp_matrix");

	// glBegin() ... glEnd() // vertices, colors, texture co-ordinates, normals // make their array
	float vertices[] = {
		-1.0f, 1.0f, 0.0f,
		-0.5f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.5f, 1.0f, 0.0f,

		-1.0f, 0.5f, 0.0f,
		-0.5f, 0.5f, 0.0f,
		0.0f, 0.5f, 0.0f,
		0.5f, 0.5f, 0.0f,

		-1.0f, 0.0f, 0.0f,
		-0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		0.5f, 0.0f, 0.0f,

		-1.0f, -0.5f, 0.0f,
		-0.5f, -0.5f, 0.0f,
		0.0f, -0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
	};

	float fColorSquareVertices[] = {
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 1.0f,

		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 1.0f,

		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 1.0f,

		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 1.0f,

	};

	float colorVertices[48] = { 0.0f };
	for (int iItr = 0; iItr < 48; iItr++)
	{
		colorVertices[iItr] = 1.0f;
	}

	// create vao
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(1, &vboPosition);
	glBindBuffer(GL_ARRAY_BUFFER, vboPosition);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);

	// GL_POINTS indices
	int iPointIndices[16];
	giPointElements = 16;
	for (int iItr = 0; iItr < 16; iItr++)
	{
		iPointIndices[iItr] = iItr;
	}
	glGenBuffers(1, &vboPointElement);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboPointElement);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(iPointIndices), iPointIndices, GL_STATIC_DRAW);
	
	int iLineIndices[30];
	int iIndex = 0;
	iLineIndices[iIndex++] = 0;
	iLineIndices[iIndex++] = 12;
	iLineIndices[iIndex++] = 1;
	iLineIndices[iIndex++] = 13;
	iLineIndices[iIndex++] = 2;
	iLineIndices[iIndex++] = 14;

	iLineIndices[iIndex++] = 0;
	iLineIndices[iIndex++] = 3;
	iLineIndices[iIndex++] = 4;
	iLineIndices[iIndex++] = 7;
	iLineIndices[iIndex++] = 8;
	iLineIndices[iIndex++] = 11;

	iLineIndices[iIndex++] = 4;
	iLineIndices[iIndex++] = 1;
	iLineIndices[iIndex++] = 8;
	iLineIndices[iIndex++] = 2;
	iLineIndices[iIndex++] = 12;
	iLineIndices[iIndex++] = 3;
	iLineIndices[iIndex++] = 13;
	iLineIndices[iIndex++] = 7;
	iLineIndices[iIndex++] = 14;
	iLineIndices[iIndex++] = 11;

	giLinesElements = iIndex;
	glGenBuffers(1, &vboLinesElement);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboLinesElement);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(iLineIndices), iLineIndices, GL_STATIC_DRAW);

	int iSquareIndices[30];
	iIndex = 0;
	iSquareIndices[iIndex++] = 0;
	iSquareIndices[iIndex++] = 12;
	iSquareIndices[iIndex++] = 1;
	iSquareIndices[iIndex++] = 13;
	iSquareIndices[iIndex++] = 2;
	iSquareIndices[iIndex++] = 14;
	iSquareIndices[iIndex++] = 3;
	iSquareIndices[iIndex++] = 15;

	iSquareIndices[iIndex++] = 0;
	iSquareIndices[iIndex++] = 3;
	iSquareIndices[iIndex++] = 4;
	iSquareIndices[iIndex++] = 7;
	iSquareIndices[iIndex++] = 8;
	iSquareIndices[iIndex++] = 11;
	iSquareIndices[iIndex++] = 12;
	iSquareIndices[iIndex++] = 15;

	giSquareElements = iIndex;
	glGenBuffers(1, &vboSquareElement);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSquareElement);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(iSquareIndices), iSquareIndices, GL_STATIC_DRAW);


	int iDiagonalSquareIndices[30];
	iIndex = 0;
	iDiagonalSquareIndices[iIndex++] = 0;
	iDiagonalSquareIndices[iIndex++] = 12;
	iDiagonalSquareIndices[iIndex++] = 1;
	iDiagonalSquareIndices[iIndex++] = 13;
	iDiagonalSquareIndices[iIndex++] = 2;
	iDiagonalSquareIndices[iIndex++] = 14;
	iDiagonalSquareIndices[iIndex++] = 3;
	iDiagonalSquareIndices[iIndex++] = 15;

	iDiagonalSquareIndices[iIndex++] = 0;
	iDiagonalSquareIndices[iIndex++] = 3;
	iDiagonalSquareIndices[iIndex++] = 4;
	iDiagonalSquareIndices[iIndex++] = 7;
	iDiagonalSquareIndices[iIndex++] = 8;
	iDiagonalSquareIndices[iIndex++] = 11;
	iDiagonalSquareIndices[iIndex++] = 12;
	iDiagonalSquareIndices[iIndex++] = 15;

	iDiagonalSquareIndices[iIndex++] = 4;
	iDiagonalSquareIndices[iIndex++] = 1;
	iDiagonalSquareIndices[iIndex++] = 8;
	iDiagonalSquareIndices[iIndex++] = 2;
	iDiagonalSquareIndices[iIndex++] = 12;
	iDiagonalSquareIndices[iIndex++] = 3;
	iDiagonalSquareIndices[iIndex++] = 13;
	iDiagonalSquareIndices[iIndex++] = 7;
	iDiagonalSquareIndices[iIndex++] = 14;
	iDiagonalSquareIndices[iIndex++] = 11;

	giDigonalSquareElements = iIndex;
	glGenBuffers(1, &vboDiagonalSquareElement);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboDiagonalSquareElement);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(iDiagonalSquareIndices), iDiagonalSquareIndices, GL_STATIC_DRAW);


	int iFanIndices[30];
	iIndex = 0;
	iFanIndices[iIndex++] = 0;
	iFanIndices[iIndex++] = 12;
	iFanIndices[iIndex++] = 0;
	iFanIndices[iIndex++] = 13;
	iFanIndices[iIndex++] = 0;
	iFanIndices[iIndex++] = 14;
	iFanIndices[iIndex++] = 0;
	iFanIndices[iIndex++] = 15;
	iFanIndices[iIndex++] = 0;
	iFanIndices[iIndex++] = 11;
	iFanIndices[iIndex++] = 0;
	iFanIndices[iIndex++] = 7;
	iFanIndices[iIndex++] = 0;
	iFanIndices[iIndex++] = 3;
	iFanIndices[iIndex++] = 12;
	iFanIndices[iIndex++] = 15;
	iFanIndices[iIndex++] = 15;
	iFanIndices[iIndex++] = 3;

	giFanElements = iIndex;
	glGenBuffers(1, &vboFanElement);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboFanElement);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(iFanIndices), iFanIndices, GL_STATIC_DRAW);

	glGenBuffers(1, &vboColor);
	glBindBuffer(GL_ARRAY_BUFFER, vboColor);
	glBufferData(GL_ARRAY_BUFFER, sizeof(colorVertices), colorVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);

	glBindBuffer(GL_ARRAY_BUFFER, 0);


	glGenVertexArrays(1, &vaoClrSqr);
	glBindVertexArray(vaoClrSqr);
	glGenBuffers(1, &vboPosition);
	glBindBuffer(GL_ARRAY_BUFFER, vboPosition);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);

	int iColorSquareIndices[300];
	iIndex = 0;
	iColorSquareIndices[iIndex++] = 1;
	iColorSquareIndices[iIndex++] = 0;
	iColorSquareIndices[iIndex++] = 4;

	iColorSquareIndices[iIndex++] = 1;
	iColorSquareIndices[iIndex++] = 4;
	iColorSquareIndices[iIndex++] = 5;

	iColorSquareIndices[iIndex++] = 5;
	iColorSquareIndices[iIndex++] = 4;
	iColorSquareIndices[iIndex++] = 8;

	iColorSquareIndices[iIndex++] = 5;
	iColorSquareIndices[iIndex++] = 8;
	iColorSquareIndices[iIndex++] = 9;

	iColorSquareIndices[iIndex++] = 9;
	iColorSquareIndices[iIndex++] = 8;
	iColorSquareIndices[iIndex++] = 12;

	iColorSquareIndices[iIndex++] = 9;
	iColorSquareIndices[iIndex++] = 12;
	iColorSquareIndices[iIndex++] = 13;

	iColorSquareIndices[iIndex++] = 2;
	iColorSquareIndices[iIndex++] = 1;
	iColorSquareIndices[iIndex++] = 5;

	iColorSquareIndices[iIndex++] = 2;
	iColorSquareIndices[iIndex++] = 5;
	iColorSquareIndices[iIndex++] = 6;

	iColorSquareIndices[iIndex++] = 6;
	iColorSquareIndices[iIndex++] = 5;
	iColorSquareIndices[iIndex++] = 9;

	iColorSquareIndices[iIndex++] = 6;
	iColorSquareIndices[iIndex++] = 9;
	iColorSquareIndices[iIndex++] = 10;

	iColorSquareIndices[iIndex++] = 10;
	iColorSquareIndices[iIndex++] = 9;
	iColorSquareIndices[iIndex++] = 13;

	iColorSquareIndices[iIndex++] = 10;
	iColorSquareIndices[iIndex++] = 13;
	iColorSquareIndices[iIndex++] = 14;


	iColorSquareIndices[iIndex++] = 3;
	iColorSquareIndices[iIndex++] = 2;
	iColorSquareIndices[iIndex++] = 6;

	iColorSquareIndices[iIndex++] = 3;
	iColorSquareIndices[iIndex++] = 6;
	iColorSquareIndices[iIndex++] = 7;

	iColorSquareIndices[iIndex++] = 7;
	iColorSquareIndices[iIndex++] = 6;
	iColorSquareIndices[iIndex++] = 10;

	iColorSquareIndices[iIndex++] = 7;
	iColorSquareIndices[iIndex++] = 10;
	iColorSquareIndices[iIndex++] = 11;

	iColorSquareIndices[iIndex++] = 11;
	iColorSquareIndices[iIndex++] = 10;
	iColorSquareIndices[iIndex++] = 14;

	iColorSquareIndices[iIndex++] = 11;
	iColorSquareIndices[iIndex++] = 14;
	iColorSquareIndices[iIndex++] = 15;


	/*iColorSquareIndices[iIndex++] = 0;
	iColorSquareIndices[iIndex++] = 3;
	iColorSquareIndices[iIndex++] = 4;
	iColorSquareIndices[iIndex++] = 7;
	iColorSquareIndices[iIndex++] = 8;
	iColorSquareIndices[iIndex++] = 11;
	iColorSquareIndices[iIndex++] = 12;
	iColorSquareIndices[iIndex++] = 15;*/

	giColorSquareElements = iIndex;
	glGenBuffers(1, &vboColorSquareElement);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboColorSquareElement);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(iColorSquareIndices), iColorSquareIndices, GL_STATIC_DRAW);

	glGenBuffers(1, &vboColorSquare);
	glBindBuffer(GL_ARRAY_BUFFER, vboColorSquare);
	glBufferData(GL_ARRAY_BUFFER, sizeof(fColorSquareVertices), fColorSquareVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepth(1.0f);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	resize(WIN_WIDTH, WIN_HEIGHT);

	return 0;
}

void resize(int iWidth, int iHeight)
{
	glViewport(0, 0, (GLsizei)iWidth, (GLsizei)iHeight);

	perspectiveProjectionMatrix = perspective(45.0f,
		(GLfloat)iWidth / (GLfloat)iHeight,
		0.1f,
		100.0f);
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(gGlShaderProgramObject);

	// declaration of metrices
	mat4 modelViewMatrix;
	mat4 translationMatrix;
	mat4 rotationMatrix;
	mat4 scaleMatrix;
	mat4 modelViewProjectionMatrix;

	// initializing above metrices to identity
	modelViewMatrix = mat4::identity();
	modelViewProjectionMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	rotationMatrix = mat4::identity();
	scaleMatrix = mat4::identity();

	// do neccessry transformation, rotation, scaling
	translationMatrix = translate(-3.0f, 1.0f, -7.0f);

	// do necessary matrix multiplication
	modelViewProjectionMatrix = perspectiveProjectionMatrix * ((translationMatrix * scaleMatrix)*rotationMatrix); //modelViewMatrix = (translationMatrix * scaleMatrix) * rotationMatrix;

	// send necessary metrices to shader in respective uniforms
	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);

	// bind with vao // this will avoid repetdly binding with vbo
	glBindVertexArray(vao);

	// similary bind with texture if any
	glPointSize(2.5f);
	// draw the necessary scene 
	//glDrawArrays(GL_POINTS, 0, 16);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboPointElement);
	glDrawElements(GL_POINTS, giPointElements, GL_UNSIGNED_INT, 0/*  */);

	// unbind vao
	glBindVertexArray(0);

	// initializing above metrices to identity
	modelViewMatrix = mat4::identity();
	modelViewProjectionMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	rotationMatrix = mat4::identity();
	scaleMatrix = mat4::identity();

	// do neccessry transformation, rotation, scaling
	translationMatrix = translate(0.0f, 1.0f, -7.0f);

	// do necessary matrix multiplication
	modelViewProjectionMatrix = perspectiveProjectionMatrix * ((translationMatrix * scaleMatrix)*rotationMatrix); //modelViewMatrix = (translationMatrix * scaleMatrix) * rotationMatrix;

	// send necessary metrices to shader in respective uniforms
	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);

	// bind with vao // this will avoid repetdly binding with vbo
	glBindVertexArray(vao);

	// similary bind with texture if any

	// draw the necessary scene 
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboLinesElement);
	glDrawElements(GL_LINES, giLinesElements, GL_UNSIGNED_INT, 0/*  */);

	// unbind vao
	glBindVertexArray(0);

	// initializing above metrices to identity
	modelViewMatrix = mat4::identity();
	modelViewProjectionMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	rotationMatrix = mat4::identity();
	scaleMatrix = mat4::identity();

	// do neccessry transformation, rotation, scaling
	translationMatrix = translate(2.5f, 1.0f, -7.0f);

	// do necessary matrix multiplication
	modelViewProjectionMatrix = perspectiveProjectionMatrix * ((translationMatrix * scaleMatrix)*rotationMatrix); //modelViewMatrix = (translationMatrix * scaleMatrix) * rotationMatrix;

	// send necessary metrices to shader in respective uniforms
	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);

	// bind with vao // this will avoid repetdly binding with vbo
	glBindVertexArray(vao);

	// similary bind with texture if any

	// draw the necessary scene 
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSquareElement);
	glDrawElements(GL_LINES, giSquareElements, GL_UNSIGNED_INT, 0/*  */);

	// unbind vao
	glBindVertexArray(0);

	// initializing above metrices to identity
	modelViewMatrix = mat4::identity();
	modelViewProjectionMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	rotationMatrix = mat4::identity();
	scaleMatrix = mat4::identity();

	// do neccessry transformation, rotation, scaling
	translationMatrix = translate(-3.0f, -1.0f, -7.0f);

	// do necessary matrix multiplication
	modelViewProjectionMatrix = perspectiveProjectionMatrix * ((translationMatrix * scaleMatrix)*rotationMatrix); //modelViewMatrix = (translationMatrix * scaleMatrix) * rotationMatrix;

	// send necessary metrices to shader in respective uniforms
	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);

	// bind with vao // this will avoid repetdly binding with vbo
	glBindVertexArray(vao);

	// similary bind with texture if any

	// draw the necessary scene 
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboDiagonalSquareElement);
	glDrawElements(GL_LINES, giDigonalSquareElements, GL_UNSIGNED_INT, 0/*  */);

	// unbind vao
	glBindVertexArray(0);

	// initializing above metrices to identity
	modelViewMatrix = mat4::identity();
	modelViewProjectionMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	rotationMatrix = mat4::identity();
	scaleMatrix = mat4::identity();

	// do neccessry transformation, rotation, scaling
	translationMatrix = translate(0.0f, -1.0f, -7.0f);

	// do necessary matrix multiplication
	modelViewProjectionMatrix = perspectiveProjectionMatrix * ((translationMatrix * scaleMatrix)*rotationMatrix); //modelViewMatrix = (translationMatrix * scaleMatrix) * rotationMatrix;

	// send necessary metrices to shader in respective uniforms
	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);

	// bind with vao // this will avoid repetdly binding with vbo
	glBindVertexArray(vao);

	// similary bind with texture if any

	// draw the necessary scene 
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboFanElement);
	glDrawElements(GL_LINES, giFanElements, GL_UNSIGNED_INT, 0/*  */);

	// unbind vao
	glBindVertexArray(0);

	// initializing above metrices to identity
	modelViewMatrix = mat4::identity();
	modelViewProjectionMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	rotationMatrix = mat4::identity();
	scaleMatrix = mat4::identity();

	// do neccessry transformation, rotation, scaling
	translationMatrix = translate(2.5f, -1.0f, -7.0f);

	// do necessary matrix multiplication
	modelViewProjectionMatrix = perspectiveProjectionMatrix * ((translationMatrix * scaleMatrix)*rotationMatrix); //modelViewMatrix = (translationMatrix * scaleMatrix) * rotationMatrix;

	// send necessary metrices to shader in respective uniforms
	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);

	//glPolygonMode(GL_POLYGON_OFFSET_FILL, GL_FILL);
	// bind with vao // this will avoid repetdly binding with vbo
	glBindVertexArray(vaoClrSqr);

	// similary bind with texture if any

	// draw the necessary scene 
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboColorSquareElement);
	//glDrawElements(GL_TRIANGLES, giColorSquareElements, GL_UNSIGNED_INT, 0/*  */);
	glDrawElements(GL_LINES, giColorSquareElements, GL_UNSIGNED_INT, 0/*  */);

	// unbind vao
	glBindVertexArray(0);

	// un-use the program
	glUseProgram(0);

	SwapBuffers(ghDC);
}

void uninitialize()
{
	void ToggleFullScreen();

	if (gbIsFullScreen == true)
	{
		ToggleFullScreen();
	}

	if (vboPosition)
	{
		glDeleteBuffers(1, &vboPosition);
	}

	if (vboColor)
	{
		glDeleteBuffers(1, &vboColor);
	}

	if (vao)
	{
		glDeleteVertexArrays(1, &vao);
	}

	GLsizei shaderCount = 0;
	GLsizei shaderNumber = 0;

	if (gGlShaderProgramObject)
	{
		glUseProgram(gGlShaderProgramObject);

		// ask program how many shader attached to it
		glGetProgramiv(gGlShaderProgramObject, GL_ATTACHED_SHADERS, &shaderCount);
		if (shaderCount > 0)
		{
			GLuint *pShaders = (GLuint*)malloc(sizeof(GLuint) * shaderCount);
			if (pShaders)
			{
				glGetAttachedShaders(gGlShaderProgramObject, shaderCount, &shaderCount, pShaders);
				for (shaderNumber = 0; shaderNumber < shaderCount; shaderNumber++)
				{
					glDetachShader(gGlShaderProgramObject, pShaders[shaderNumber]);

					// delete shader
					glDeleteShader(pShaders[shaderNumber]);
					pShaders[shaderNumber] = 0;
				}
				free(pShaders);
			}
		}
		glDeleteProgram(gGlShaderProgramObject);
		gGlShaderProgramObject = 0;
		glUseProgram(0);
	}



	/*if (0 != gGlShaderProgramObject)
	{
		glUseProgram(gGlShaderProgramObject);

		if (0 != gGlVertexShaderObject)
		{
			glDetachShader(gGlShaderProgramObject, gGlVertexShaderObject);
			glDeleteShader(gGlVertexShaderObject);
			gGlVertexShaderObject = 0;
		}

		if (0 != gGlFragmentShaderObject)
		{
			glDetachShader(gGlShaderProgramObject, gGlFragmentShaderObject);
			glDeleteShader(gGlFragmentShaderObject);
			gGlFragmentShaderObject = 0;
		}

		glDeleteProgram(gGlShaderProgramObject);
		gGlShaderProgramObject = 0;
	}
	glUseProgram(0);
	*/
	if (wglGetCurrentContext() == ghGlRC)
	{
		wglMakeCurrent(NULL, NULL);
		if (ghGlRC)
		{
			wglDeleteContext(ghGlRC);
			ghGlRC = NULL;
		}
	}

	if (ghDC)
	{
		ReleaseDC(ghWnd, ghDC);
		ghDC = NULL;
	}

	if (gpLogFile)
	{
		fprintf_s(gpLogFile, "Log file is closed");
		fclose(gpLogFile);
	}
}

void ToggleFullScreen()
{
	MONITORINFO mi;

	if (gbIsFullScreen == false)
	{
		gdwStyle = GetWindowLong(ghWnd, GWL_STYLE);

		if (gdwStyle & WS_OVERLAPPEDWINDOW)
		{
			mi.cbSize = sizeof(MONITORINFO);

			if (
				GetWindowPlacement(ghWnd, &gwpPrev) &&
				GetMonitorInfo(MonitorFromWindow(ghWnd, MONITORINFOF_PRIMARY), &mi)
				)
			{
				SetWindowLong(ghWnd, GWL_STYLE, gdwStyle & ~WS_OVERLAPPEDWINDOW);
				SetWindowPos(ghWnd,
					HWND_TOP,
					mi.rcMonitor.left,
					mi.rcMonitor.top,
					mi.rcMonitor.right - mi.rcMonitor.left,
					mi.rcMonitor.bottom - mi.rcMonitor.top,
					SWP_NOZORDER | SWP_FRAMECHANGED);

				ShowCursor(FALSE);
				gbIsFullScreen = true;
			}
		}
	}
	else
	{
		SetWindowLong(ghWnd, GWL_STYLE, gdwStyle | WS_OVERLAPPEDWINDOW);

		SetWindowPlacement(ghWnd, &gwpPrev);

		SetWindowPos(ghWnd,
			HWND_TOP,
			0,
			0,
			0,
			0,
			SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER);
		ShowCursor(TRUE);
		gbIsFullScreen = false;
	}
}

GLboolean GetGlErrorStatus(GLGetErrorType enErrorType, GLuint uiErrorObject, char* pszErrorStrPrefix)
{
	GLint iErrorStatus = 0;
	GLint iInfoLogLength = 0;
	GLchar *szInfoLog = NULL;

	if (enGlGET_Shader == enErrorType)
	{
		glGetShaderiv(uiErrorObject, GL_COMPILE_STATUS, &iErrorStatus);
	}
	else if (enGlGET_Program == enErrorType)
	{
		glGetProgramiv(uiErrorObject, GL_LINK_STATUS, &iErrorStatus);
	}
	else
	{
		strcat_s(pszErrorStrPrefix, ERROR_SIZE, "Error enum not found\n");
		return GL_FALSE;
	}

	if (GL_FALSE == iErrorStatus)
	{
		if (enGlGET_Shader == enErrorType)
		{
			glGetShaderiv(uiErrorObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		}
		else if (enGlGET_Program == enErrorType)
		{
			glGetProgramiv(uiErrorObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		}

		if (0 < iInfoLogLength)
		{
			szInfoLog = (char*)malloc(iInfoLogLength);
			if (NULL != szInfoLog)
			{
				GLsizei written = 0;
				if (enGlGET_Shader == enErrorType)
				{
					glGetShaderInfoLog(uiErrorObject, iInfoLogLength, &written, szInfoLog);
				}
				else if (enGlGET_Program == enErrorType)
				{
					glGetProgramInfoLog(uiErrorObject, iInfoLogLength, &written, szInfoLog);
				}
				strcat_s(pszErrorStrPrefix, ERROR_SIZE, szInfoLog);
			}
			else
			{
				strcat_s(pszErrorStrPrefix, ERROR_SIZE, "malloc() failed\n");
			}
			return GL_FALSE;
		}
		else
		{
			return GL_TRUE;
		}
	}
	else
	{
		return GL_TRUE;
	}
}
