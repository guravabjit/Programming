#include <Windows.h>
#include <GL/glew.h>
#include <GL/GL.h>
#include <stdio.h> // FILE
#include <cuda_gl_interop.h>
#include <cuda_runtime.h>
#include <helper_cuda.h>
#include <helper_timer.h>
#include "vmath.h"

#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "cudart.lib")

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

int giGraphLine = 0;


// CUDA

int gMeshWidth = 16;
int gMeshHeight = 16;

bool gbPoints = true;
bool gbLines = false;
bool gbLineStrip = false;
bool gbTriangles = false;
bool gbTriangleStrip = false;
bool gbTriangleFan = false;
float gfSize = 1.0f;

//#define MY_ARRAY_SIZE	gMeshWidth * gMeshHeight * 4

#define MY_ARRAY_SIZE	2048 * 2048 * 4

float position[2048][2048][4] = {0}; // initializing to avoid zero out each

struct cudaGraphicsResource *gpGraphicsResource = NULL;
GLuint gpuVbo;
float gfAnimationTime = 0.0f;
bool gbOnGpu = false;
cudaError_t error;


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
	else if (iRet == -6)
	{
		fprintf_s(gpLogFile, "Error in cudaInit() related code.\n");
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
		case 'g':
		case 'G':
			gbOnGpu = true;
			break;
		case 'c':
		case 'C':
			gbOnGpu = false;
			break;
		case 'i':
		case 'I':
			gfSize += 0.5f;
			if(gfSize > 10.0f)
			{
				gfSize = 10.0f;
			}
			break;
		case 'd':
		case 'D':
			gfSize -= 0.5f;
			if(gfSize < 0.5f)
			{
				gfSize = 0.5;
			}
			break; 
		case 'p':
		case 'P':
			gbPoints = true;
			gbLines = false;
			gbLineStrip = false;
			gbTriangles = false;
			gbTriangleFan = false;
			gbTriangleStrip = false;
			break; 
		case 'l':
		case 'L':
			gbLines = true;
			gbPoints = false;
			gbLineStrip = false;
			gbTriangles = false;
			gbTriangleFan = false;
			gbTriangleStrip = false;
			break; 
		case 'k':
		case 'K':
			gbLineStrip = true;
			gbTriangles = false;
			gbTriangleFan = false;
			gbTriangleStrip = false;
			gbPoints = false;
			gbLines = false;
			break; 
		case 't':
		case 'T':
			gbTriangles = true;
			gbTriangleFan = false;
			gbTriangleStrip = false;
			gbPoints = false;
			gbLines = false;
			gbLineStrip = false;
			break; 
		case 'y':
		case 'Y':
			gbTriangleFan = true;
			gbTriangleStrip = false;
			gbPoints = false;
			gbLines = false;
			gbLineStrip = false;
			gbTriangles = false;
			break;
		case 'r':
		case 'R':
			gbTriangleStrip = true;
			gbPoints = false;
			gbLines = false;
			gbLineStrip = false;
			gbTriangles = false;
			gbTriangleFan = false;
			break;  
		case 49:
			gMeshWidth = 16;
			gMeshHeight = 16;
			break;
		case 50:
			gMeshWidth = 32;
			gMeshHeight = 32;
			break;
		case 51:
			gMeshWidth = 64;
			gMeshHeight = 64;
			break;
		case 52:
			gMeshWidth = 128;
			gMeshHeight = 128;
			break;
		case 53:
			gMeshWidth = 512;
			gMeshHeight = 512;
			break;
		case 54:
			gMeshWidth = 1024;
			gMeshHeight = 1024;
			break;
		case 55:
			gMeshWidth = 2048;
			gMeshHeight = 2048;
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
	int cudaInit();
	char szglErrorStr[ERROR_SIZE] = "";
	GLboolean bErrorStatus = GL_TRUE;
	GLuint gGlVertexShaderObject;
	GLuint gGlFragmentShaderObject;

	PIXELFORMATDESCRIPTOR pfd;
	int iPixelFormatIndex = 0;

	if (cudaInit() < 0)
	{
		return -6;
	}

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

	// create vao
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(1, &vboPosition);
	glBindBuffer(GL_ARRAY_BUFFER, vboPosition);
	glBufferData(GL_ARRAY_BUFFER, MY_ARRAY_SIZE * 4, NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);

	glGenBuffers(1, &gpuVbo);
	glBindBuffer(GL_ARRAY_BUFFER, gpuVbo);
	glBufferData(GL_ARRAY_BUFFER, MY_ARRAY_SIZE * 4, NULL, GL_DYNAMIC_DRAW);

	error = cudaGraphicsGLRegisterBuffer(&gpGraphicsResource, gpuVbo, cudaGraphicsRegisterFlagsWriteDiscard);
	if (error != cudaSuccess)
	{
		return -6;
	}


	/*glGenBuffers(1, &vboColor);
	glBindBuffer(GL_ARRAY_BUFFER, vboColor);
	glBufferData(GL_ARRAY_BUFFER, sizeof(colorVertices), colorVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);*/

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
	void launchCpuKernel(unsigned int width, unsigned int height, float time);
	void launchCudaKernel(float4* pos, unsigned int width, unsigned int height, float animTime);

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
	translationMatrix = translate(0.0f, 0.0f, -2.0f);
	rotationMatrix = rotate(90.0f, 0.0f, 1.0f, 0.0f);

	// do necessary matrix multiplication
	modelViewProjectionMatrix = perspectiveProjectionMatrix * ((translationMatrix * scaleMatrix)*rotationMatrix); //modelViewMatrix = (translationMatrix * scaleMatrix) * rotationMatrix;

	// send necessary metrices to shader in respective uniforms
	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);

	// bind with vao // this will avoid repetdly binding with vbo
	glBindVertexArray(vao);

	if (gbOnGpu)
	{
		glBindBuffer(GL_ARRAY_BUFFER, gpuVbo);

		error = cudaGraphicsMapResources(1, &gpGraphicsResource);
		if (error != cudaSuccess)
		{
			return ;
		}

		float4 *pPosition = { NULL };
		size_t byteCount = 0;
		error = cudaGraphicsResourceGetMappedPointer((void**)&pPosition, &byteCount, gpGraphicsResource);
		if (error != cudaSuccess)
		{
			return;
		}

		launchCudaKernel(pPosition, gMeshWidth, gMeshHeight, gfAnimationTime);

		error = cudaGraphicsUnmapResources(1, &gpGraphicsResource, 0);
	}
	else
	{
		glBindBuffer(GL_ARRAY_BUFFER, vboPosition);
		launchCpuKernel(gMeshWidth, gMeshHeight, gfAnimationTime);
		
		glBufferData(GL_ARRAY_BUFFER, (MY_ARRAY_SIZE/*gMeshWidth * gMeshHeight*/ * 4)-1, position+1, GL_DYNAMIC_DRAW);

	}

	
	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 4, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);

	// similary bind with texture if any

	GLuint iShape = GL_POINTS;
	//u the necessary scene 
	
	if(gbPoints == true)
	{
		glPointSize(gfSize);
		iShape = GL_POINTS;
	}
	else if(gbLines == true)
	{
		glLineWidth(gfSize);
		iShape = GL_LINES;
	}
	else if(gbLineStrip == true)
	{
		glLineWidth(gfSize);
		iShape = GL_LINE_STRIP;
	}
	else if(gbTriangles == true)
	{
		iShape = GL_TRIANGLES;
	}
	else if(gbTriangleStrip == true)
	{
		iShape = GL_TRIANGLE_STRIP;
	}
	else if(gbTriangleFan == true)
	{
		iShape = GL_TRIANGLE_FAN;
	}
	
	//glPointSize(2.0f);
	//glDrawArrays(GL_POINTS, 0, gMeshWidth * gMeshHeight);

	glDrawArrays(iShape, 0, gMeshWidth * gMeshHeight);

	// unbind vao
	glBindVertexArray(0);

	// un-use the program
	glUseProgram(0);

	gfAnimationTime += 0.009f;

	SwapBuffers(ghDC);
}

void uninitialize()
{
	void ToggleFullScreen();

	if (gpGraphicsResource)
	{
		cudaGraphicsUnregisterResource(gpGraphicsResource);
		gpGraphicsResource = NULL;
	}

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

int cudaInit()
{
	int iDevCnt = 0;

	error = cudaGetDeviceCount(&iDevCnt);
	if (error != cudaSuccess)
	{
		return -1;
	}
	else if (iDevCnt == 0)
	{
		return -2;
	}
	else
	{
		cudaSetDevice(0);
	}
	return 0;
}

void launchCpuKernel(unsigned int width, unsigned int height, float time)
{
	
	
	
	for (int i = 0; i < width; i++)
	{
		for (int j = 0; j < height; j++)
		{
			for (int k = 0; k < 4; k++)
			{
				float u = i / (float)width;
				float v = j / (float)width;

				u = (u*2.0f) - 1.0f;
				v = (v*2.0f) - 1.0f;
				float freq = 4.0f;
				float w = sin(freq * u * time) * cos(freq * v * time) * 0.5f;

				if (k == 0)
				{
					position[i][j][k] = u;
				}
				else if (k == 1)
				{
					position[i][j][k] = w;
				}
				else if (k == 2)
				{
					position[i][j][k] = v;
				}
				else if (k == 3)
				{
					position[i][j][k] = 1.0;
				}
			}
		}
	}
	/*if(width < 2048)
	{
		position[i][j][k] = 1.0f;
	}*/
}
