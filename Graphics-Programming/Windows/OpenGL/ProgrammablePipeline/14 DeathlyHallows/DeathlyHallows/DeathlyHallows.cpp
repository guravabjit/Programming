#include <Windows.h>
#include <GL/glew.h>
#include <GL/GL.h>
#include <stdio.h> // FILE
#include "vmath.h"

#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "Winmm.lib")

#define WIN_WIDTH	800
#define WIN_HEIGHT	600
#define ERROR_SIZE 1024
#define NUMBER_OF_VERTICES 5000 * 2
#define PI 3.14

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

GLuint vaoTriangle;
GLuint vboTriangle;

GLuint vaoCircle;
GLuint vboCircle;

GLuint vaoLine;
GLuint vboLine;

GLuint mvpUniform;
mat4 perspectiveProjectionMatrix;

GLfloat fRotx = 0.0f;
GLfloat fRoty = 0.0f;
GLfloat fRotz = 0.0f;
GLfloat gfTransLiney = 0.0f;
GLfloat gfTransTrix = 0.0f;
GLfloat gfTransTriy = 0.0f;
GLfloat gfTransCirx = 0.0f;
GLfloat gfTransCiry = 0.0f;
GLfloat gfCenterYForCircle = 0.0f;
bool gbLineRot = true;
bool gbTriRot1 = true;
bool gbTriRot2 = true;
bool gbStopCirRot = true;

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
		TEXT("Deathly Hallows"),
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

	PlaySound(TEXT("./deathlyhallow.wav"), NULL, SND_ASYNC | SND_FILENAME);
	gfTransTriy = -2.0f;
	gfTransTrix = -2.0f;
	gfTransCirx = 2.0f;
	gfTransCiry = -2.0f;
	gfTransLiney = 2.0f;

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
		"uniform mat4 u_mvp_matrix;" \
		"void main(void)" \
		"{" \
		"gl_Position = u_mvp_matrix * vPosition;" \
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
		"out vec4 FragColor;" \
		"void main(void)" \
		"{" \
		"FragColor = vec4(1.0,1.0,1.0,1.0);"
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
	const GLfloat triangleVertices[] =
	{
		0, 1, 0,
		-1, -1, 0,
		1, -1, 0
	};

	// create vao
	glGenVertexArrays(1, &vaoTriangle);
	glBindVertexArray(vaoTriangle);
	glLineWidth(2.0f);
	glGenBuffers(1, &vboTriangle);
	glBindBuffer(GL_ARRAY_BUFFER, vboTriangle);
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVertices), triangleVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLfloat fBase = 2.0f;
	GLfloat fHeight = 2.0f;
	GLfloat fSides = sqrtf(fHeight * fHeight + (fBase / 2) * (fBase / 2));
	GLfloat fArea = (fBase * fHeight) / 2.0f;
	GLfloat fPerimeter = (fBase + fSides + fSides);
	GLfloat fRadius = (2.0f * fArea / fPerimeter);
	GLfloat fCenterX = 0.0f;
	gfCenterYForCircle = 1.0f - fRadius;
	int iIndex = 0;
	GLfloat circleVertices[NUMBER_OF_VERTICES * 3] = { 0 };
	for (int iItr = 0; iItr < NUMBER_OF_VERTICES; iItr++)
	{
		//circleVertices[iItr] = (GLfloat)(fRadius * cos((5.0f * (PI + 1) * iItr) / NUMBER_OF_VERTICES));
		//circleVertices[iItr+1] = (GLfloat)(fRadius * sin((5.0f * (PI + 1) * iItr) / NUMBER_OF_VERTICES));
		//circleVertices[iItr+2] = 0.0f;

		circleVertices[iIndex++] = (GLfloat)cos((5.0f * PI * iItr) / NUMBER_OF_VERTICES);
		circleVertices[iIndex++] = (GLfloat)sin((5.0f * PI * iItr) / NUMBER_OF_VERTICES);
		circleVertices[iIndex++] = 0.0f;

		iItr++;

		circleVertices[iIndex++] = (GLfloat)cos((5.0f * PI * iItr) / NUMBER_OF_VERTICES);
		circleVertices[iIndex++] = (GLfloat)sin((5.0f * PI * iItr) / NUMBER_OF_VERTICES);
		circleVertices[iIndex++] = 0.0f;

	}
	glGenVertexArrays(1, &vaoCircle);
	glBindVertexArray(vaoCircle);
	glPointSize(1.0);
	glGenBuffers(1, &vboCircle);
	glBindBuffer(GL_ARRAY_BUFFER, vboCircle);
	glBufferData(GL_ARRAY_BUFFER, sizeof(circleVertices), circleVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	const GLfloat lineVertices[] =
	{
		0.0f, 1.0f, 0.0f,
		0.0f, -1.0f, 0.0f
	};
	glGenVertexArrays(1, &vaoLine);
	glBindVertexArray(vaoLine);
	glGenBuffers(1, &vboLine);
	glBindBuffer(GL_ARRAY_BUFFER, vboLine);
	glBufferData(GL_ARRAY_BUFFER, sizeof(lineVertices), lineVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
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
	translationMatrix = translate(gfTransTrix, gfTransTriy, -3.0f);
	rotationMatrix = rotate(fRotx, 0.0f, 1.0f, 0.0f);
	scaleMatrix = scale(0.8f, 0.8f, 0.8f);

	// do necessary matrix multiplication
	modelViewProjectionMatrix = perspectiveProjectionMatrix * ((translationMatrix * scaleMatrix)*rotationMatrix); //modelViewMatrix = (translationMatrix * scaleMatrix) * rotationMatrix;

	// send necessary metrices to shader in respective uniforms
	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);

	// bind with vao // this will avoid repetdly binding with vbo
	glBindVertexArray(vaoTriangle);

	// similary bind with texture if any

	// draw the necessary scene 
	glDrawArrays(GL_LINE_LOOP, 0, 3);

	// unbind vao
	glBindVertexArray(0);

	// initializing above metrices to identity
	modelViewMatrix = mat4::identity();
	modelViewProjectionMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	rotationMatrix = mat4::identity();
	scaleMatrix = mat4::identity();

	// do neccessry transformation, rotation, scaling
	translationMatrix = translate(gfTransCirx, gfTransCiry, -3.0f);
	rotationMatrix = rotate(fRotx, 1.0f, 0.0f, 0.0f);
	scaleMatrix = scale(0.5f, 0.5f, 0.5f);

	// do necessary matrix multiplication
	modelViewProjectionMatrix = perspectiveProjectionMatrix * ((translationMatrix * scaleMatrix)*rotationMatrix); //modelViewMatrix = (translationMatrix * scaleMatrix) * rotationMatrix;

	// send necessary metrices to shader in respective uniforms
	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);

	// bind with vao // this will avoid repetdly binding with vbo
	glBindVertexArray(vaoCircle);

	// similary bind with texture if any

	// draw the necessary scene 
	glDrawArrays(GL_LINES, 0, NUMBER_OF_VERTICES);

	// unbind vao
	glBindVertexArray(0);

	if (gbStopCirRot && gbTriRot2 && gbTriRot1)
	{
		// initializing above metrices to identity
		modelViewMatrix = mat4::identity();
		modelViewProjectionMatrix = mat4::identity();
		translationMatrix = mat4::identity();
		rotationMatrix = mat4::identity();
		scaleMatrix = mat4::identity();

		// do neccessry transformation, rotation, scaling
		translationMatrix = translate(0.0f, gfTransLiney, -3.0f);
		//rotationMatrix = rotate(fRotx, 1.0f, 0.0f, 0.0f);
		//scaleMatrix = scale(0.75f, 0.75f, 0.75f);

		// do necessary matrix multiplication
		modelViewProjectionMatrix = perspectiveProjectionMatrix * ((translationMatrix * scaleMatrix)*rotationMatrix); //modelViewMatrix = (translationMatrix * scaleMatrix) * rotationMatrix;

		// send necessary metrices to shader in respective uniforms
		glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);

		// bind with vao // this will avoid repetdly binding with vbo
		glBindVertexArray(vaoLine);

		// similary bind with texture if any

		// draw the necessary scene 
		glDrawArrays(GL_LINES, 0, 2);

		// unbind vao
		glBindVertexArray(0);
		
	}

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

	if (vboTriangle)
	{
		glDeleteBuffers(1, &vboTriangle);
	}

	if (vboLine)
	{
		glDeleteBuffers(1, &vboCircle);
	}

	if (vboCircle)
	{
		glDeleteBuffers(1, &vboCircle);
	}

	if (vaoTriangle)
	{
		glDeleteVertexArrays(1, &vaoTriangle);
	}

	if (vaoLine)
	{
		glDeleteVertexArrays(1, &vaoLine);
	}

	if (vaoCircle)
	{
		glDeleteVertexArrays(1, &vaoCircle);
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

void update()
{
	fRotx += 0.1f;
	fRoty += 0.1f;
	fRotz += 0.1f;

	if (fRotx > 360.0f)
	{
		fRotx = 0.0f;
	}
	if (fRoty > 360.0f)
	{
		fRoty = 0.0f;
	}
	if (fRotz > 360.0f)
	{
		fRotz = 0.0f;
	}

	if (gfTransCirx > 0.0f)
	{
		gfTransCirx -= 0.00003f;
		gbStopCirRot = false;
	}
	else
	{
		gbStopCirRot = true;
		gfTransCirx = 0.0f;
	}

	if (gfTransTrix < 0.0f)
	{
		gfTransTrix += 0.00003f;
		gbTriRot1 = false;
	}
	else
	{
		gbTriRot1 = true;
		gfTransTrix = 0.0f;
	}

	if (gfTransTriy < 0.0f)
	{
		gbTriRot2 = false;
		gfTransTriy += 0.00003f;
	}
	else
	{
		gbTriRot2 = true;
		gfTransTriy = 0.0f;
	}

	if (/*gfTransCiry < 0.0f && */gfTransCiry < -gfCenterYForCircle)
	{
		gfTransCiry += 0.00003f;
	}
	else
	{
		gfTransCiry = -gfCenterYForCircle;
	}


	if (gbStopCirRot && gbTriRot2 && gbTriRot1)
	{
		if (gfTransLiney > 0.0f)
		{
			gfTransLiney -= 0.00003f;
			gbLineRot = false;
		}
		else
		{
			gbLineRot = true;
			gfTransLiney = 0.0f;
		}
	}
}

