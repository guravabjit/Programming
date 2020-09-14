#include <Windows.h>
#include <GL/glew.h>
#include <GL/GL.h>
#include <stdio.h> // FILE
#include <time.h>
#include "vmath.h"
#include "DynamicIndia.h"

#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "Winmm.lib")

#define WIN_WIDTH	800
#define WIN_HEIGHT	600
#define ERROR_SIZE 1024
#define NUMBER_OF_VERTICES 5000 * 2
#define PI 3.14f
#define CHAR_HEIGHT 1.5f
#define CHAR_WIDTH 0.05f
#define CHAR_SIZE 0.1f

using namespace vmath;


GLfloat fTransIX = 0.0f;
GLfloat fTransIY = 0.0f;
GLfloat fTransNY = 0.0f;
bool bDrawD = false;
GLfloat fTransAX = 0.0f;
GLfloat fOrangeColorX = 0.0f;
GLfloat fWhiteColorX = 0.0f;
GLfloat fGreenColorX = 0.0f;
GLfloat fOrangeColorY = 0.0f;
GLfloat fWhiteColorY = 0.0f;
GLfloat fGreenColorY = 0.0f;
GLfloat fOrangeColorZ = 0.0f;
GLfloat fWhiteColorZ = 0.0f;
GLfloat fGreenColorZ = 0.0f;
bool bDrawPlane = false;
GLfloat fTransPlaneTopX = 0.0f;
GLfloat fTransPlaneMiddleX = 0.0f;
GLfloat fTransPlaneBottomX = 0.0f;
GLfloat fTransPlaneTopY = 0.0f;
GLfloat fTransPlaneMiddleY = 0.0f;
GLfloat fTransPlaneBottomY = 0.0f;
bool bIsPlayed = false;

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

GLfloat gfHeight = 0;
GLfloat gfWidth = 0;
GLfloat fRotx = 0.0f;
GLfloat fRoty = 0.0f;
GLfloat fRotz = 0.0f;

GLenum gGlEnumResult;
GLuint gGlShaderProgramObject;

GLuint vaoI;
GLuint vboIPosition;
GLuint vboIColor;

GLuint vaoN;
GLuint vboNPosition;
GLuint vboNColor;

GLuint vaoD;
GLuint vboDPosition;
GLuint vboDColor;

GLuint vaoA;
GLuint vboAPosition;
GLuint vboAColor;

GLuint vaoPlane;
GLuint vboPlanePosition;
GLuint vboPlaneColor;

GLuint mvpUniform;
mat4 perspectiveProjectionMatrix;


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR szCmdLine, int iCmdShow)
{
	int initialize();
	void display();
	void update();

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
				update();
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
		gfWidth = LOWORD(lParam);
		gfHeight = HIWORD(lParam);
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
	void DrawI();
	void DrawN();
	void DrawD();
	void DrawA();
	void DrawPlane();

	void resize(int, int);
	char szglErrorStr[ERROR_SIZE] = "";
	GLboolean bErrorStatus = GL_TRUE;
	GLuint gGlVertexShaderObject;
	GLuint gGlFragmentShaderObject;

	fTransIX = -10.0f;
	fTransIY = -6.0f;
	fTransNY = 6.0f;
	fTransAX = 10.0f;

	fTransPlaneTopX = -10.0f;
	fTransPlaneMiddleX = -10.0f;
	fTransPlaneBottomX = -10.0f;

	fTransPlaneTopY = 4.0f;
	fTransPlaneMiddleY = 0.0f;
	fTransPlaneBottomY = -4.0f;

	srand((unsigned int)time(NULL));

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
	DrawI();
	DrawN();
	DrawD();
	DrawA();
	DrawPlane();

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
	translationMatrix = translate(fTransIX /*-5.0f*/, 0.0f, -12.0f);

	// do necessary matrix multiplication
	modelViewProjectionMatrix = perspectiveProjectionMatrix * ((translationMatrix * scaleMatrix)*rotationMatrix); //modelViewMatrix = (translationMatrix * scaleMatrix) * rotationMatrix;

	// send necessary metrices to shader in respective uniforms
	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);

	// bind with vao // this will avoid repetdly binding with vbo
	glBindVertexArray(vaoI);

	// similary bind with texture if any

	// draw the necessary scene 
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 4, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 8, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 12, 4);

	// unbind vao
	glBindVertexArray(0);

	// initializing above metrices to identity
	modelViewMatrix = mat4::identity();
	modelViewProjectionMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	rotationMatrix = mat4::identity();
	scaleMatrix = mat4::identity();

	//glTranslatef(fTransIX /*-5.0f*/, 0.0f, -12.0f);
	// do neccessry transformation, rotation, scaling
	translationMatrix = translate(-2.4f, fTransNY/*0.0f*/, -12.0f);

	// do necessary matrix multiplication
	modelViewProjectionMatrix = perspectiveProjectionMatrix * ((translationMatrix * scaleMatrix)*rotationMatrix); //modelViewMatrix = (translationMatrix * scaleMatrix) * rotationMatrix;

	// send necessary metrices to shader in respective uniforms
	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);

	// bind with vao // this will avoid repetdly binding with vbo
	glBindVertexArray(vaoN);

	// similary bind with texture if any

	// draw the necessary scene 
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 4, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 8, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 12, 6);
	glDrawArrays(GL_TRIANGLE_FAN, 18, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 22, 4);

	// unbind vao
	glBindVertexArray(0);

	// initializing above metrices to identity
	modelViewMatrix = mat4::identity();
	modelViewProjectionMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	rotationMatrix = mat4::identity();
	scaleMatrix = mat4::identity();

	// do neccessry transformation, rotation, scaling
	translationMatrix = translate(0.0f, 0.0f, -12.0f);

	// do necessary matrix multiplication
	modelViewProjectionMatrix = perspectiveProjectionMatrix * ((translationMatrix * scaleMatrix)*rotationMatrix); //modelViewMatrix = (translationMatrix * scaleMatrix) * rotationMatrix;

	// send necessary metrices to shader in respective uniforms
	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);

	if (bDrawD == true)
	{
		GLfloat triangleColorVertices[72]
		{
			// 1.0f, 0.5f, 0, // ORANGE fOrangeColorX, fOrangeColorY
			// 1.0f, 1.0f, 1.0f, // White fWhiteColorX,fWhiteColorY, fWhiteColorZ
			// 0.0f, 1.0f, 0, // Green fGreenColorY

			fOrangeColorX, fOrangeColorY, 0,
			fWhiteColorX, fWhiteColorY, fWhiteColorZ,
			fWhiteColorX, fWhiteColorY, fWhiteColorZ,
			fOrangeColorX, fOrangeColorY, 0,
			fOrangeColorX, fOrangeColorY, 0,
			fOrangeColorX, fOrangeColorY, 0,

			0.0f, fGreenColorY, 0,
			0.0f, fGreenColorY, 0,
			0.0f, fGreenColorY, 0,
			0.0f, fGreenColorY, 0,
			fWhiteColorX, fWhiteColorY, fWhiteColorZ,
			fWhiteColorX, fWhiteColorY, fWhiteColorZ,

			fWhiteColorX, fWhiteColorY, fWhiteColorZ,
			fWhiteColorX, fWhiteColorY, fWhiteColorZ,
			fWhiteColorX, fWhiteColorY, fWhiteColorZ,
			fWhiteColorX, fWhiteColorY, fWhiteColorZ,

			fOrangeColorX, fOrangeColorY, 0,
			fWhiteColorX, fWhiteColorY, fWhiteColorZ,
			fWhiteColorX, fWhiteColorY, fWhiteColorZ,
			fOrangeColorX, fOrangeColorY, 0,

			fWhiteColorX, fWhiteColorY, fWhiteColorZ,//fOrangeColorX, fOrangeColorY, 0,
			0.0f, fGreenColorY, 0,
			0.0f, fGreenColorY, 0,
			fWhiteColorX, fWhiteColorY, fWhiteColorZ,
		};
		glBindBuffer(GL_ARRAY_BUFFER, vboDColor);
		glBufferData(GL_ARRAY_BUFFER, sizeof(triangleColorVertices), triangleColorVertices, GL_DYNAMIC_DRAW);
	}

	// bind with vao // this will avoid repetdly binding with vbo
	glBindVertexArray(vaoD);

	// similary bind with texture if any

	// draw the necessary scene 
	glDrawArrays(GL_TRIANGLE_FAN, 0, 6);
	glDrawArrays(GL_TRIANGLE_FAN, 6, 6);
	glDrawArrays(GL_TRIANGLE_FAN, 12, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 16, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 20, 4);

	// unbind vao
	glBindVertexArray(0);

	// initializing above metrices to identity
	modelViewMatrix = mat4::identity();
	modelViewProjectionMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	rotationMatrix = mat4::identity();
	scaleMatrix = mat4::identity();

	// do neccessry transformation, rotation, scaling
	translationMatrix = translate(2.4f, fTransIY/*0.0f*/, -12.0f);

	// do necessary matrix multiplication
	modelViewProjectionMatrix = perspectiveProjectionMatrix * ((translationMatrix * scaleMatrix)*rotationMatrix); //modelViewMatrix = (translationMatrix * scaleMatrix) * rotationMatrix;

	// send necessary metrices to shader in respective uniforms
	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);

	// bind with vao // this will avoid repetdly binding with vbo
	glBindVertexArray(vaoI);

	// similary bind with texture if any

	// draw the necessary scene 
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 4, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 8, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 12, 4);

	// unbind vao
	glBindVertexArray(0);

	// initializing above metrices to identity
	modelViewMatrix = mat4::identity();
	modelViewProjectionMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	rotationMatrix = mat4::identity();
	scaleMatrix = mat4::identity();

	// do neccessry transformation, rotation, scaling
	translationMatrix = translate(fTransAX/*5.0f*/, 0.0f, -12.0f);

	// do necessary matrix multiplication
	modelViewProjectionMatrix = perspectiveProjectionMatrix * ((translationMatrix * scaleMatrix)*rotationMatrix); //modelViewMatrix = (translationMatrix * scaleMatrix) * rotationMatrix;

	// send necessary metrices to shader in respective uniforms
	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);

	// bind with vao // this will avoid repetdly binding with vbo
	glBindVertexArray(vaoA);

	// similary bind with texture if any

	// draw the necessary scene 
	glDrawArrays(GL_TRIANGLE_FAN, 0, 6);
	glDrawArrays(GL_TRIANGLE_FAN, 6, 7);
	glDrawArrays(GL_TRIANGLE_FAN, 13, 7);

	// unbind vao
	glBindVertexArray(0);

	// initializing above metrices to identity
	modelViewMatrix = mat4::identity();
	modelViewProjectionMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	rotationMatrix = mat4::identity();
	scaleMatrix = mat4::identity();

	// do neccessry transformation, rotation, scaling
	translationMatrix = translate(fTransPlaneBottomX, fTransPlaneBottomY, -12.0f);

	// do necessary matrix multiplication
	modelViewProjectionMatrix = perspectiveProjectionMatrix * ((translationMatrix * scaleMatrix)*rotationMatrix); //modelViewMatrix = (translationMatrix * scaleMatrix) * rotationMatrix;

	// send necessary metrices to shader in respective uniforms
	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);

	// bind with vao // this will avoid repetdly binding with vbo
	glBindVertexArray(vaoPlane);

	// similary bind with texture if any

	// draw the necessary scene 
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glDrawArrays(GL_TRIANGLE_FAN, 3, 6);
	glDrawArrays(GL_TRIANGLE_FAN, 9, 14);
	glDrawArrays(GL_TRIANGLE_FAN, 22, 4);

	// unbind vao
	glBindVertexArray(0);

	// initializing above metrices to identity
	modelViewMatrix = mat4::identity();
	modelViewProjectionMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	rotationMatrix = mat4::identity();
	scaleMatrix = mat4::identity();

	// do neccessry transformation, rotation, scaling
	translationMatrix = translate(fTransPlaneTopX, fTransPlaneTopY, -12.0f);

	// do necessary matrix multiplication
	modelViewProjectionMatrix = perspectiveProjectionMatrix * ((translationMatrix * scaleMatrix)*rotationMatrix); //modelViewMatrix = (translationMatrix * scaleMatrix) * rotationMatrix;

	// send necessary metrices to shader in respective uniforms
	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);

	// bind with vao // this will avoid repetdly binding with vbo
	glBindVertexArray(vaoPlane);

	// similary bind with texture if any

	// draw the necessary scene 
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glDrawArrays(GL_TRIANGLE_FAN, 3, 6);
	glDrawArrays(GL_TRIANGLE_FAN, 9, 14);
	glDrawArrays(GL_TRIANGLE_FAN, 22, 4);

	// unbind vao
	glBindVertexArray(0);

	// initializing above metrices to identity
	modelViewMatrix = mat4::identity();
	modelViewProjectionMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	rotationMatrix = mat4::identity();
	scaleMatrix = mat4::identity();

	// do neccessry transformation, rotation, scaling
	translationMatrix = translate(fTransPlaneMiddleX, fTransPlaneMiddleY, -12.0f);

	// do necessary matrix multiplication
	modelViewProjectionMatrix = perspectiveProjectionMatrix * ((translationMatrix * scaleMatrix)*rotationMatrix); //modelViewMatrix = (translationMatrix * scaleMatrix) * rotationMatrix;

	// send necessary metrices to shader in respective uniforms
	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);

	// bind with vao // this will avoid repetdly binding with vbo
	glBindVertexArray(vaoPlane);

	// similary bind with texture if any

	// draw the necessary scene 
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glDrawArrays(GL_TRIANGLE_FAN, 3, 6);
	glDrawArrays(GL_TRIANGLE_FAN, 9, 14);
	glDrawArrays(GL_TRIANGLE_FAN, 22, 4);

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

	if (vboIColor)
	{
		glDeleteBuffers(1, &vboIColor);
	}

	if (vboIPosition)
	{
		glDeleteBuffers(1, &vboIPosition);
	}

	if (vaoI)
	{
		glDeleteVertexArrays(1, &vaoI);
	}

	if (vboAColor)
	{
		glDeleteBuffers(1, &vboAColor);
	}

	if (vboAPosition)
	{
		glDeleteBuffers(1, &vboAPosition);
	}

	if (vaoA)
	{
		glDeleteVertexArrays(1, &vaoA);
	}

	if (vboNColor)
	{
		glDeleteBuffers(1, &vboNColor);
	}

	if (vboNPosition)
	{
		glDeleteBuffers(1, &vboNPosition);
	}

	if (vaoN)
	{
		glDeleteVertexArrays(1, &vaoN);
	}

	if (vboAColor)
	{
		glDeleteBuffers(1, &vboAColor);
	}

	if (vboAPosition)
	{
		glDeleteBuffers(1, &vboAPosition);
	}

	if (vaoA)
	{
		glDeleteVertexArrays(1, &vaoA);
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

void DrawI()
{
	const GLfloat triangleVertices[] =
	{
		-1.0f, 1.0f, 0.0f,
		-1.0f, 0.8f, 0.0f,
		1.0f, 0.8f, 0.0f,
		1.0f, 1.0f, 0.0f,

		-0.2f, 0.8f, 0.0f,
		-0.2f, 0.0f, 0.0f,
		0.2f, 0.0f, 0.0f,
		0.2f, 0.8f, 0.0f,

		-0.2f, 0.0f, 0.0f,
		-0.2f, -0.8f, 0.0f,
		0.2f, -0.8f, 0.0f,
		0.2f, 0.0f, 0.0f,

		-1.0f, -0.8f, 0.0f,
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		1.0f, -0.8f, 0.0f
	};

	const GLfloat triangleColorVertices[] =
	{
		1.0f, 0.5f, 0,
		1.0f, 0.5f, 0,
		1.0f, 0.5f, 0,
		1.0f, 0.5f, 0,

		1.0f, 0.5f, 0,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 0.5f, 0,

		1.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 1.0f,

		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f

	};

	// create vao
	glGenVertexArrays(1, &vaoI);
	glBindVertexArray(vaoI);

	glGenBuffers(1, &vboIPosition);
	glBindBuffer(GL_ARRAY_BUFFER, vboIPosition);
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVertices), triangleVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);

	glGenBuffers(1, &vboIColor);
	glBindBuffer(GL_ARRAY_BUFFER, vboIColor);
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangleColorVertices), triangleColorVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

}

void DrawN()
{
	const GLfloat triangleVertices[] =
	{
		-1.0f, 1.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
		-0.8f, 0.0f, 0.0f,
		-0.8f, 1.0f, 0.0f,

		-1.0f, 0.0f, 0.0f,
		-1.0f, -1.0f, 0.0f,
		-0.8f, -1.0f, 0.0f,
		-0.8f, 0.0f, 0.0f,

		-0.8f, 1.0f, 0.0f,
		-0.8f, 0.7f, 0.0f,
		-0.2f, 0.0f, 0.0f,
		0.2f, 0.0f, 0.0f,

		0.8f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		0.8f, -0.7f, 0.0f,
		0.2f, 0.0f, 0.0f,
		-0.2f, 0.0f, 0.0f,

		0.8f, 1.0f, 0.0f,
		0.8f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 1.0f, 0.0f,

		0.8f, 0.0f, 0.0f,
		0.8f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		1.0f, 0.0f, 0.0f
	};

	const GLfloat triangleColorVertices[] =
	{
		1.0f, 0.5f, 0,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 0.5f, 0,

		1.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 1.0f,

		1.0f, 0.5f, 0.0f,
		1.0f, 0.5f, 0.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,

		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,

		1.0f, 0.5f, 0.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 0.5f, 0.0f,

		1.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 1.0f
	};

	// create vao
	glGenVertexArrays(1, &vaoN);
	glBindVertexArray(vaoN);

	glGenBuffers(1, &vboNPosition);
	glBindBuffer(GL_ARRAY_BUFFER, vboNPosition);
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVertices), triangleVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);

	glGenBuffers(1, &vboNColor);
	glBindBuffer(GL_ARRAY_BUFFER, vboNColor);
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangleColorVertices), triangleColorVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void DrawD()
{
	const GLfloat triangleVertices[] =
	{
		0.5f, 0.8f, 0.0f,
		0.8f, 0.5f, 0.0f,
		1.0f, 0.5f, 0.0f,
		0.5f, 1.0f, 0.0f,
		-1.0f, 1.0f, 0.0f,
		-1.0f, 0.8f, 0.0f,

		0.5f, -0.8f, 0.0f,
		-1.0f, -0.8f, 0.0f,
		-1.0f, -1.0f, 0.0f,
		0.5f, -1.0f, 0.0f,
		1.0f, -0.5f, 0.0f,
		0.8f, -0.5f, 0.0f,

		0.8f, 0.5f, 0.0f,
		0.8f, -0.5f, 0.0f,
		1.0f, -0.5f, 0.0f,
		1.0f, 0.5f, 0.0f,

		-1.0f, 0.8f, 0.0f,
		-1.0f, 0.0f, 0.0f,
		-0.8f, 0.0f, 0.0f,
		-0.8f, 0.8f, 0.0f,

		-1.0f, 0.0f, 0.0f,
		-1.0f, -0.8f, 0.0f,
		-0.8f, -0.8f, 0.0f,
		-0.8f, 0.0f, 0.0f
	};

	GLfloat triangleColorVertices[72];
	for (int i = 0; i < 72; i++)
	{
		triangleColorVertices[0] = 1.0f;
	}
		/*1.0f, 0.5f, 0,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 0.5f, 0,
		1.0f, 0.5f, 0,
		1.0f, 0.5f, 0,

		0.0f, 1.0f, 0,
		0.0f, 1.0f, 0,
		0.0f, 1.0f, 0,
		0.0f, 1.0f, 0,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,

		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,

		1.0f, 0.5f, 0.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 0.5f, 0.0f,

		1.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 1.0f
	};*/

	// create vao
	glGenVertexArrays(1, &vaoD);
	glBindVertexArray(vaoD);

	glGenBuffers(1, &vboDPosition);
	glBindBuffer(GL_ARRAY_BUFFER, vboDPosition);
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVertices), triangleVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);

	glGenBuffers(1, &vboDColor);
	glBindBuffer(GL_ARRAY_BUFFER, vboDColor);
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangleColorVertices), triangleColorVertices, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);

	glBindBuffer(GL_ARRAY_BUFFER, 0);


}

void DrawA()
{
	const GLfloat triangleVertices[] =
	{
		-0.5f, 1.0f, 0.0f,
		-0.8f, 0.0f, 0.0f,
		-0.3f, 0.2f, 0.0f,
		-0.1f, 0.8f, 0.0f,
		0.1f, 0.8f, 0.0f,
		0.5f, 1.0f, 0.0f,

		-0.8f, 0.0f, 0.0f,
		-1.0, -1.0f, 0.0f,
		-0.7f, -1.0f, 0.0f,
		-0.5f, -0.2f, 0.0f,
		0.5f, -0.2f, 0.0f,
		0.3f, 0.2f, 0.0f,
		-0.3f, 0.2f, 0.0f,

		0.8f, 0.0f, 0.0f,
		1.0, -1.0f, 0.0f,
		0.7f, -1.0f, 0.0f,
		0.5f, -0.2f, 0.0f,
		0.3f, 0.2f, 0.0f,
		0.1f, 0.8f, 0.0f,
		0.5f, 1.0f, 0.0f
	};

	const GLfloat triangleColorVertices[] =
	{
		1.0f, 0.5f, 0.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 0.5f, 0.0f,
		1.0f, 0.5f, 0.0f,
		1.0f, 0.5f, 0.0f,

		1.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,

		1.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 0.5f, 0.0f,
		1.0f, 0.5f, 0.0f
	};

	// create vao
	glGenVertexArrays(1, &vaoA);
	glBindVertexArray(vaoA);

	glGenBuffers(1, &vboAPosition);
	glBindBuffer(GL_ARRAY_BUFFER, vboAPosition);
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVertices), triangleVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);

	glGenBuffers(1, &vboAColor);
	glBindBuffer(GL_ARRAY_BUFFER, vboAColor);
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangleColorVertices), triangleColorVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

}

void update()
{
	if (fTransIX < -5.0f)
	{
		fTransIX += 0.001f;
	}

	if (fTransIX >= -5.0f)
	{
		if (fTransNY > 0.0f)
		{
			fTransNY -= 0.001f;
		}
	}

	if (fTransAX <= 4.8f)
	{
		if (fTransIY <= 0.0f)
		{
			fTransIY += 0.001f;
		}
	}

	if (fTransIY >= 0.0f)
	{
		bDrawD = true;
	}

	if (fTransNY <= 0.0f)
	{
		if (fTransAX >= 4.8f)
		{
			fTransAX -= 0.001f;
		}
	}

	if (true == bDrawPlane)
	{
		if (false == bIsPlayed)
		{
			bIsPlayed = true;
			PlaySound(MAKEINTRESOURCE(ID_SONG), GetModuleHandle(NULL), SND_ASYNC | SND_RESOURCE);
		}

		fTransPlaneTopX += 0.0004f;

		if (fTransPlaneTopY >= 0.0f)
		{
			fTransPlaneTopY -= 0.0004f;
		}

		if (fTransPlaneTopX > 6.0f)
		{
			fTransPlaneTopY -= 0.0004f;
		}

		fTransPlaneMiddleX += 0.0004f;
		//fTransPlaneMiddleY += 0.001f;

		fTransPlaneBottomX += 0.0004f;

		if (fTransPlaneBottomY <= 0.0f)
		{
			fTransPlaneBottomY += 0.0004f;
		}

		if (fTransPlaneBottomX > 6.0f)
		{
			fTransPlaneBottomY += 0.0004f;
		}
	}

	if (bDrawD == true)
	{
		if (fOrangeColorX < 1.0f)
		{
			fOrangeColorX += 0.0001f;
		}

		if (fOrangeColorY < 0.5f)
		{
			fOrangeColorY += 0.0001f;
		}


		if (fWhiteColorX < 1.0f)
		{
			fWhiteColorX += 0.0001f;
			fWhiteColorY += 0.0001f;
			fWhiteColorZ += 0.0001f;
		}

		if (fGreenColorY < 1.0f)
		{
			fGreenColorY += 0.0001f;
		}

		if (fGreenColorY >= 1.0f && fWhiteColorX >= 1.0f && fOrangeColorX >= -1.0f)
		{
			bDrawPlane = true;
		}

	}
}

void DrawPlane()
{
	const GLfloat triangleVertices[] =
	{
		// GL_TRIANGLES
		1.0f, 0.0f, 0.0f,
		0.4f, 0.1f, 0.0f,
		0.4f, -0.1f, 0.0f,

		// GL_POLYGON
		0.4f, 0.1f, 0.0f,
		0.3f, 0.2f, 0.0f,
		0.0f, 0.3f, 0.0f,
		0.0f, -0.3f, 0.0f,
		0.3f, -0.2f, 0.0f,
		0.4f, -0.1f, 0.0f,

		0.0f, 0.3f, 0.0f,
		-0.5f, 1.0f, 0.0f,
		-0.6f, 1.0f, 0.0f,
		-0.6f, 0.9f, 0.0f,
		-0.5f, 0.4f, 0.0f,
		-0.6f, 0.4f, 0.0f,
		-0.7f, 0.1f, 0.0f,
		-0.7f, -0.1f, 0.0f,
		-0.6f, -0.4f, 0.0f,
		-0.5f, -0.4f, 0.0f,
		-0.6f, -0.9f, 0.0f,
		-0.6f, -1.0f, 0.0f,
		-0.5f, -1.0f, 0.0f,
		0.0f, -0.3f, 0.0f,

		-0.7f, 0.1f, 0.0f,
		-0.8f, 0.1f, 0.0f,
		-0.8f, -0.1f, 0.0f,
		-0.7f, -0.1f, 0.0f
	};

	const GLfloat triangleColorVertices[] =
	{
		0.7f, 0.8f, 0.9f,
		0.7f, 0.8f, 0.9f,
		0.7f, 0.8f, 0.9f,

		0.7f, 0.8f, 0.9f,
		0.7f, 0.8f, 0.9f,
		0.7f, 0.8f, 0.9f,
		0.7f, 0.8f, 0.9f,
		0.7f, 0.8f, 0.9f,
		0.7f, 0.8f, 0.9f,

		0.7f, 0.8f, 0.9f,
		0.7f, 0.8f, 0.9f,
		0.7f, 0.8f, 0.9f,
		0.7f, 0.8f, 0.9f,
		0.7f, 0.8f, 0.9f,
		0.7f, 0.8f, 0.9f,
		0.7f, 0.8f, 0.9f,
		0.7f, 0.8f, 0.9f,
		0.7f, 0.8f, 0.9f,
		0.7f, 0.8f, 0.9f,
		0.7f, 0.8f, 0.9f,
		0.7f, 0.8f, 0.9f,
		0.7f, 0.8f, 0.9f,
		0.7f, 0.8f, 0.9f,

		0.7f, 0.8f, 0.9f,
		0.7f, 0.8f, 0.9f,
		0.7f, 0.8f, 0.9f,
		0.7f, 0.8f, 0.9f,
	};

	// create vao
	glGenVertexArrays(1, &vaoPlane);
	glBindVertexArray(vaoPlane);

	glGenBuffers(1, &vboPlanePosition);
	glBindBuffer(GL_ARRAY_BUFFER, vboPlanePosition);
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVertices), triangleVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);

	glGenBuffers(1, &vboPlaneColor);
	glBindBuffer(GL_ARRAY_BUFFER, vboPlaneColor);
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangleColorVertices), triangleColorVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	
	/*GLfloat fRandNum1 = 0.0f;
	GLfloat fRandNum2 = 0.0f;
	glColor3f(1.0f, 0.5f, 0.0f);
	glPointSize(4.0f);
	for (int iItr = 0; iItr < 50; iItr++)
	{
		for (int iItr1 = 0; iItr1 < 3; iItr1++)
		{
			if (iItr1 == 0)
			{
				glColor3f(1.0f, 0.5f, 0.0f);
			}
			else if (iItr1 == 1)
			{
				glColor3f(1.0f, 1.0f, 1.0f);
			}
			else if (iItr1 == 2)
			{
				glColor3f(0.0f, 1.0f, 0.0f);
			}


			fRandNum1 = ((GLfloat)rand()) / RAND_MAX * (-2);
			fRandNum2 = ((GLfloat)rand()) / RAND_MAX;
			if (fRandNum1 <= -0.8f && fRandNum1 >= -2.0f && fRandNum2 >= -0.1f && fRandNum2 <= 0.1f)
			{
				glBegin(GL_POINTS);
				glVertex3f(fRandNum1, fRandNum2, 0.0f);
				glEnd();
			}

		}
	}


	glColor3f(1.0f, 1.0f, 1.0f);
	glBegin(GL_LINES);
	glVertex3f(-0.5f, 0.1f, 0.0f);
	glVertex3f(-0.3f, 0.1f, 0.0f);
	glEnd();
	glBegin(GL_LINES);
	glVertex3f(-0.4f, 0.1f, 0.0f);
	glVertex3f(-0.4f, -0.1f, 0.0f);
	glEnd();
	glBegin(GL_LINES);
	glVertex3f(-0.5f, -0.1f, 0.0f);
	glVertex3f(-0.3f, -0.1f, 0.0f);
	glEnd();

	glBegin(GL_LINES);
	glVertex3f(-0.1f, 0.1f, 0.0f);
	glVertex3f(-0.2f, -0.1f, 0.0f);
	glEnd();
	glBegin(GL_LINES);
	glVertex3f(-0.1f, 0.1f, 0.0f);
	glVertex3f(0.0f, -0.1f, 0.0f);
	glEnd();
	glBegin(GL_LINES);
	glVertex3f(-0.15f, 0.0f, 0.0f);
	glVertex3f(-0.05f, 0.0f, 0.0f);
	glEnd();

	glBegin(GL_LINES);
	glVertex3f(0.1f, 0.1f, 0.0f);
	glVertex3f(0.2f, 0.1f, 0.0f);
	glEnd();
	glBegin(GL_LINES);
	glVertex3f(0.1f, 0.1f, 0.0f);
	glVertex3f(0.1f, -0.1f, 0.0f);
	glEnd();
	glBegin(GL_LINES);
	glVertex3f(0.1f, 0.0f, 0.0f);
	glVertex3f(0.2f, 0.0f, 0.0f);
	glEnd();
	*/
}
