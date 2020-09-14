#include <Windows.h>
#include <GL/glew.h>
#include <GL/GL.h>
#include <stdio.h> // FILE
#include "vmath.h"
#include "TwickedSmily.h"

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

enum enSelectedPart
{
	enDefault = 0,
	enLeftBottom = 1,
	enFull,
	enOnlyCenterPixel,
	enFourTiling,
	enRightBottom,
	enLeftTop,
	enRightTop
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

GLuint vaoQuad;
GLuint vboQuadPosition;

GLuint gTextureSmily;
GLuint vboSmilyTexture;

GLuint mvpUniform;
GLuint u_sampler_uniform;
mat4 perspectiveProjectionMatrix;

int giSelectedVal = 0;

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
		TEXT("Twicked smiley"),
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

		case 49:
		case VK_NUMPAD1:
			giSelectedVal = enLeftBottom;
			break;

		case 50:
		case VK_NUMPAD2:
			giSelectedVal = enFull;
			break;

		case 51:
		case VK_NUMPAD3:
			giSelectedVal = enFourTiling;
			break;

		case 52:
		case VK_NUMPAD4:
			giSelectedVal = enOnlyCenterPixel;
			break;

		default:
			giSelectedVal = enDefault;
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
	BOOL LoadTexture(GLuint *pgliTexture, TCHAR imageResourceID[]);

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
		"in vec2 vTexCord;" \
		"uniform mat4 u_mvp_matrix;" \
		"out vec2 out_TexCord;" \
		"void main(void)" \
		"{" \
		"gl_Position = u_mvp_matrix * vPosition;" \
		"out_TexCord = vTexCord;" \
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
		"in vec2 out_TexCord;" \
		"uniform sampler2D u_sampler;" \
		"void main(void)" \
		"{" \
			"FragColor = texture(u_sampler, out_TexCord);"
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
	glBindAttribLocation(gGlShaderProgramObject, AMC_ATTRIBUTE_TEXCORD0, "vTexCord");

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
	u_sampler_uniform = glGetUniformLocation(gGlShaderProgramObject, "u_sampler");

	// glBegin() ... glEnd() // vertices, colors, texture co-ordinates, normals // make their array
	const GLfloat quadVertices[] =
	{
		1, 1, 0,
		-1, 1, 0,
		-1, -1, 0,
		1, -1, 0
	};

	// create vao
	glGenVertexArrays(1, &vaoQuad);
	glBindVertexArray(vaoQuad);
	glGenBuffers(1, &vboQuadPosition);
	glBindBuffer(GL_ARRAY_BUFFER, vboQuadPosition);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
	//glBufferData(GL_ARRAY_BUFFER, 16 * 4 * 3, NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// create vao
	glGenBuffers(1, &vboSmilyTexture);
	glBindBuffer(GL_ARRAY_BUFFER, vboSmilyTexture);
	glBufferData(GL_ARRAY_BUFFER, 4 * 2 * 4, NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_TEXCORD0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_TEXCORD0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

	glEnable(GL_TEXTURE_2D);
	LoadTexture(&gTextureSmily, MAKEINTRESOURCE(ID_BITMAP_SMILY));

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
	GLfloat quadTexCord[] =
	{
		1, 1,
		0, 1,
		0, 0,
		1, 0
	};

	GLfloat quadTexCordForTiling[] =
	{
		1, 1,
		0, 1,
		0, 0,
		1, 0
	};
	
	switch (giSelectedVal)
	{
	case enLeftBottom:
		quadTexCord[0] = 0.5f;
		quadTexCord[1] = 0.5f;
		quadTexCord[2] = 0.0f;
		quadTexCord[3] = 0.5f;
		quadTexCord[4] = 0.0f;
		quadTexCord[5] = 0.0f;
		quadTexCord[6] = 0.5f;
		quadTexCord[7] = 0.0f;
		break;
	case enRightBottom:
		quadTexCord[0] = 1.0f;
		quadTexCord[1] = 0.5f;
		quadTexCord[2] = 0.5f;
		quadTexCord[3] = 0.5f;
		quadTexCord[4] = 0.5f;
		quadTexCord[5] = 0.0f;
		quadTexCord[6] = 1.0f;
		quadTexCord[7] = 0.0f;
		break;
	case enLeftTop:
		quadTexCord[0] = 0.5f;
		quadTexCord[1] = 1.0f;
		quadTexCord[2] = 0.0f;
		quadTexCord[3] = 1.0f;
		quadTexCord[4] = 0.0f;
		quadTexCord[5] = 0.5f;
		quadTexCord[6] = 0.5f;
		quadTexCord[7] = 0.5f;
		break;
	case enRightTop:
		quadTexCord[0] = 1.0f;
		quadTexCord[1] = 1.0f;
		quadTexCord[2] = 0.5f;
		quadTexCord[3] = 1.0f;
		quadTexCord[4] = 0.5f;
		quadTexCord[5] = 0.5f;
		quadTexCord[6] = 1.0f;
		quadTexCord[7] = 0.5f;
		break;
	case enFull:
		quadTexCord[0] = 1.0f;
		quadTexCord[1] = 1.0f;
		quadTexCord[2] = 0.0f;
		quadTexCord[3] = 1.0f;
		quadTexCord[4] = 0.0f;
		quadTexCord[5] = 0.0f;
		quadTexCord[6] = 1.0f;
		quadTexCord[7] = 0.0f;
		break;
	case enFourTiling:

		quadTexCord[0] = 2.0f;
		quadTexCord[1] = 2.0f;
		quadTexCord[2] = 0.0f;
		quadTexCord[3] = 2.0f;
		quadTexCord[4] = 0.0f;
		quadTexCord[5] = 0.0f;
		quadTexCord[6] = 2.0f;
		quadTexCord[7] = 0.0f;

		break;
	case enOnlyCenterPixel:
		quadTexCord[0] = 0.5f;
		quadTexCord[1] = 0.5f;
		quadTexCord[2] = 0.5f;
		quadTexCord[3] = 0.5f;
		quadTexCord[4] = 0.5f;
		quadTexCord[5] = 0.5f;
		quadTexCord[6] = 0.5f;
		quadTexCord[7] = 0.5f;
		break;
	default:
		quadTexCord[0] = 0.0f;
		quadTexCord[1] = 0.0f;
		quadTexCord[2] = 0.0f;
		quadTexCord[3] = 0.0f;
		quadTexCord[4] = 0.0f;
		quadTexCord[5] = 0.0f;
		quadTexCord[6] = 0.0f;
		quadTexCord[7] = 0.0f;
		break;
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(gGlShaderProgramObject);

	// declaration of metrices
	mat4 modelViewMatrix;
	mat4 modelViewProjectionMatrix;
	mat4 translationMatrix;

	modelViewMatrix = mat4::identity();
	modelViewProjectionMatrix = mat4::identity();
	translationMatrix = mat4::identity();

	
	// do neccessry transformation, rotation, scaling
	modelViewMatrix = translate(0.0f, 0.0f, -3.0f);

	// do necessary matrix multiplication
	modelViewProjectionMatrix = modelViewProjectionMatrix * perspectiveProjectionMatrix;
	modelViewProjectionMatrix = modelViewProjectionMatrix * modelViewMatrix;
	modelViewProjectionMatrix = modelViewProjectionMatrix * translationMatrix;

	// send necessary metrices to shader in respective uniforms
	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);

	// bind with vao // this will avoid repetdly binding with vbo
	glBindVertexArray(vaoQuad);

	// similary bind with texture if any
	glBindBuffer(GL_ARRAY_BUFFER, vboSmilyTexture);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadTexCord), quadTexCord, GL_DYNAMIC_DRAW);
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTextureSmily);
	//if(giSelectedVal != )
	glUniform1i(u_sampler_uniform, 0);

	// draw the necessary scene 
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	// unbind vao
	glBindVertexArray(0);

	// un-use the program
	glUseProgram(0);

	SwapBuffers(ghDC);
}

void displayByViewChangeForTiling()
{
	GLfloat quadTexCord[] =
	{
		1, 1,
		0, 1,
		0, 0,
		1, 0
	};

	switch (giSelectedVal)
	{
		 case enLeftBottom:
			 quadTexCord[0] = 0.5f;
			 quadTexCord[1] = 0.5f;
			 quadTexCord[2] = 0.0f;
			 quadTexCord[3] = 0.5f;
			 quadTexCord[4] = 0.0f;
			 quadTexCord[5] = 0.0f;
			 quadTexCord[6] = 0.5f;
			 quadTexCord[7] = 0.0f;
			 break;
		 case enRightBottom:
			 quadTexCord[0] = 1.0f;
			 quadTexCord[1] = 0.5f;
			 quadTexCord[2] = 0.5f;
			 quadTexCord[3] = 0.5f;
			 quadTexCord[4] = 0.5f;
			 quadTexCord[5] = 0.0f;
			 quadTexCord[6] = 1.0f;
			 quadTexCord[7] = 0.0f;
			 break;
		 case enLeftTop:
			 quadTexCord[0] = 0.5f;
			 quadTexCord[1] = 1.0f;
			 quadTexCord[2] = 0.0f;
			 quadTexCord[3] = 1.0f;
			 quadTexCord[4] = 0.0f;
			 quadTexCord[5] = 0.5f;
			 quadTexCord[6] = 0.5f;
			 quadTexCord[7] = 0.5f;
			 break;
		 case enRightTop:
			 quadTexCord[0] = 1.0f;
			 quadTexCord[1] = 1.0f;
			 quadTexCord[2] = 0.5f;
			 quadTexCord[3] = 1.0f;
			 quadTexCord[4] = 0.5f;
			 quadTexCord[5] = 0.5f;
			 quadTexCord[6] = 1.0f;
			 quadTexCord[7] = 0.5f;
			 break;
		 case enFull:
			 quadTexCord[0] = 1.0f;
			 quadTexCord[1] = 1.0f;
			 quadTexCord[2] = 0.0f;
			 quadTexCord[3] = 1.0f;
			 quadTexCord[4] = 0.0f;
			 quadTexCord[5] = 0.0f;
			 quadTexCord[6] = 1.0f;
			 quadTexCord[7] = 0.0f;
			 break;
		 case enFourTiling:
			 
			 quadTexCord[0] = 1.0f;
			 quadTexCord[1] = 1.0f;
			 quadTexCord[2] = 0.0f;
			 quadTexCord[3] = 1.0f;
			 quadTexCord[4] = 0.0f;
			 quadTexCord[5] = 0.0f;
			 quadTexCord[6] = 1.0f;
			 quadTexCord[7] = 0.0f;
			 
			 break;
		 case enOnlyCenterPixel:
			 quadTexCord[0] = 0.5f;
			 quadTexCord[1] = 0.5f;
			 quadTexCord[2] = 0.5f;
			 quadTexCord[3] = 0.5f;
			 quadTexCord[4] = 0.5f;
			 quadTexCord[5] = 0.5f;
			 quadTexCord[6] = 0.5f;
			 quadTexCord[7] = 0.5f;
			 break;
		 default:
			 quadTexCord[0] = 0.0f;
			 quadTexCord[1] = 0.0f;
			 quadTexCord[2] = 0.0f;
			 quadTexCord[3] = 0.0f;
			 quadTexCord[4] = 0.0f;
			 quadTexCord[5] = 0.0f;
			 quadTexCord[6] = 0.0f;
			 quadTexCord[7] = 0.0f;
			 break;
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(gGlShaderProgramObject);

	// declaration of metrices
	mat4 modelViewMatrix;
	mat4 modelViewProjectionMatrix;
	mat4 translationMatrix;

	modelViewMatrix = mat4::identity();
	modelViewProjectionMatrix = mat4::identity();
	translationMatrix = mat4::identity();

	if (giSelectedVal == enFourTiling)
	{
		// do neccessry transformation, rotation, scaling
		modelViewMatrix = translate(-1.0f, 1.0f, -7.0f);

		// do necessary matrix multiplication
		modelViewProjectionMatrix = modelViewProjectionMatrix * perspectiveProjectionMatrix;
		modelViewProjectionMatrix = modelViewProjectionMatrix * modelViewMatrix;
		modelViewProjectionMatrix = modelViewProjectionMatrix * translationMatrix;

		// send necessary metrices to shader in respective uniforms
		glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);

		// bind with vao // this will avoid repetdly binding with vbo
		glBindVertexArray(vaoQuad);

		// similary bind with texture if any
		glBindBuffer(GL_ARRAY_BUFFER, vboSmilyTexture);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadTexCord), quadTexCord, GL_DYNAMIC_DRAW);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gTextureSmily);
		glUniform1i(u_sampler_uniform, 0);

		// draw the necessary scene 
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);


		modelViewMatrix = mat4::identity();
		modelViewProjectionMatrix = mat4::identity();
		translationMatrix = mat4::identity();

		// do neccessry transformation, rotation, scaling
		modelViewMatrix = translate(1.0f, 1.0f, -7.0f);

		// do necessary matrix multiplication
		modelViewProjectionMatrix = modelViewProjectionMatrix * perspectiveProjectionMatrix;
		modelViewProjectionMatrix = modelViewProjectionMatrix * modelViewMatrix;
		modelViewProjectionMatrix = modelViewProjectionMatrix * translationMatrix;

		// send necessary metrices to shader in respective uniforms
		glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);

		// bind with vao // this will avoid repetdly binding with vbo
		glBindVertexArray(vaoQuad);

		// similary bind with texture if any
		glBindBuffer(GL_ARRAY_BUFFER, vboSmilyTexture);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadTexCord), quadTexCord, GL_DYNAMIC_DRAW);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gTextureSmily);
		glUniform1i(u_sampler_uniform, 0);

		// draw the necessary scene 
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);


		modelViewMatrix = mat4::identity();
		modelViewProjectionMatrix = mat4::identity();
		translationMatrix = mat4::identity();

		// do neccessry transformation, rotation, scaling
		modelViewMatrix = translate(-1.0f, -1.0f, -7.0f);

		// do necessary matrix multiplication
		modelViewProjectionMatrix = modelViewProjectionMatrix * perspectiveProjectionMatrix;
		modelViewProjectionMatrix = modelViewProjectionMatrix * modelViewMatrix;
		modelViewProjectionMatrix = modelViewProjectionMatrix * translationMatrix;

		// send necessary metrices to shader in respective uniforms
		glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);

		// bind with vao // this will avoid repetdly binding with vbo
		glBindVertexArray(vaoQuad);

		// similary bind with texture if any
		glBindBuffer(GL_ARRAY_BUFFER, vboSmilyTexture);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadTexCord), quadTexCord, GL_DYNAMIC_DRAW);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gTextureSmily);
		glUniform1i(u_sampler_uniform, 0);

		// draw the necessary scene 
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

		modelViewMatrix = mat4::identity();
		modelViewProjectionMatrix = mat4::identity();
		translationMatrix = mat4::identity();

		// do neccessry transformation, rotation, scaling
		modelViewMatrix = translate(1.0f, -1.0f, -7.0f);

		// do necessary matrix multiplication
		modelViewProjectionMatrix = modelViewProjectionMatrix * perspectiveProjectionMatrix;
		modelViewProjectionMatrix = modelViewProjectionMatrix * modelViewMatrix;
		modelViewProjectionMatrix = modelViewProjectionMatrix * translationMatrix;

		// send necessary metrices to shader in respective uniforms
		glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);

		// bind with vao // this will avoid repetdly binding with vbo
		glBindVertexArray(vaoQuad);

		// similary bind with texture if any
		glBindBuffer(GL_ARRAY_BUFFER, vboSmilyTexture);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadTexCord), quadTexCord, GL_DYNAMIC_DRAW);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gTextureSmily);
		glUniform1i(u_sampler_uniform, 0);

		// draw the necessary scene 
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	}
	else
	{
		// do neccessry transformation, rotation, scaling
		modelViewMatrix = translate(0.0f, 0.0f, -3.0f);

		// do necessary matrix multiplication
		modelViewProjectionMatrix = modelViewProjectionMatrix * perspectiveProjectionMatrix;
		modelViewProjectionMatrix = modelViewProjectionMatrix * modelViewMatrix;
		modelViewProjectionMatrix = modelViewProjectionMatrix * translationMatrix;

		// send necessary metrices to shader in respective uniforms
		glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);

		// bind with vao // this will avoid repetdly binding with vbo
		glBindVertexArray(vaoQuad);

		// similary bind with texture if any
		glBindBuffer(GL_ARRAY_BUFFER, vboSmilyTexture);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadTexCord), quadTexCord, GL_DYNAMIC_DRAW);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gTextureSmily);
		glUniform1i(u_sampler_uniform, 0);

		// draw the necessary scene 
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	}

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



	if (vboSmilyTexture)
	{
		glDeleteBuffers(1, &vboSmilyTexture);
	}


	if (vaoQuad)
	{
		glDeleteVertexArrays(1, &vaoQuad);
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

BOOL LoadTexture(GLuint *pgliTexture, TCHAR imageResourceID[])
{
	HBITMAP hBitmap;
	BITMAP bmp;
	BOOL bState = FALSE;

	hBitmap = (HBITMAP)LoadImage(GetModuleHandle(NULL),
		imageResourceID,
		IMAGE_BITMAP,
		0,
		0,
		LR_CREATEDIBSECTION);

	if (hBitmap)
	{
		bState = TRUE;
		GetObject(hBitmap, sizeof(BITMAP), &bmp);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
		glGenTextures(1, pgliTexture);
		glBindTexture(GL_TEXTURE_2D, *pgliTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

		//gluBuild2DMipmaps(GL_TEXTURE_2D, 3, bmp.bmWidth, bmp.bmHeight, GL_BGR_EXT, GL_UNSIGNED_BYTE, bmp.bmBits);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, bmp.bmWidth, bmp.bmHeight, 0, GL_BGR, GL_UNSIGNED_BYTE, bmp.bmBits);
		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
		DeleteObject(hBitmap);
	}
	return bState;
}

