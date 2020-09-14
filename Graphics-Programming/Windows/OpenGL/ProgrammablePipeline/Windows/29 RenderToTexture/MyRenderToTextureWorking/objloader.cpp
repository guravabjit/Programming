//Header Files
#include <windows.h>
#include <iostream>
#include <stdio.h>
#include <gl/glew.h> // IMPORTANT : THIS FILE MUST BE INCLUDED BEFORE <gl/GL.h >  & <gl/GLU.h>
#include <gl/GL.h>
#include <math.h>
#include <vector>
#include "uf/vmath.h"
#include "uf/utilheader.h"
#include "uf/loadshader.h"
#include "uf/objloader.h"
#include "UF/resource.h"

//Linking with Opengl Library (This is for linker)
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "opengl32.lib")

//Macros
#define WIN_WIDTH 800
#define WIN_HEIGHT 600


// global Variable declarations
bool gbFullscreen 		= false;
DWORD dwStyle;
WINDOWPLACEMENT wpPrev 	= { sizeof(WINDOWPLACEMENT) };
HWND ghwnd 				= NULL;
HDC ghdc 				= NULL;
HGLRC ghrc 				= NULL;
bool gbActiveWindow 	= false;


GLenum result;

GLuint vs;
GLuint fs;
GLuint program1;

GLuint mUniform;
GLuint vUniform;
GLuint pUniform;
mat4 PerspectiveProjectionMatrix;

GLfloat rAngle;

GLuint lKeyPressedUniform;

GLuint laUniform;
GLuint kaUniform;

GLuint ldUniform;
GLuint kdUniform;

GLuint lsUniform;
GLuint ksUniform;
GLuint materialShininessUniform;

GLuint lightPositionUniform;

BOOL gbLighting = FALSE;

float gLightAmbient[] = { 0.0f, 0.0f, 0.0f, 0.0f };
float gLightDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
float gLightSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
float gLightPosition[] = { 100.0f, 100.0f, 100.0f, 1.0f };

float gMaterialAmbient[] = { 0.0f, 0.0f, 0.0f, 0.0f };
float gMaterialDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
float gMaterialSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
float gMaterialShininess = 128.0f;//50.0f; // 128.0f

GLuint vboMarbleTexture;
GLuint u_sampler_uniform;
GLuint gTextureMarble;




GLuint program2;
GLuint vaoCube;
GLuint vboCubePosition;
GLuint vboCubeTexture;
GLfloat angleCube;

GLuint mUniformCube;
GLuint vUniformCube;
GLuint pUniformCube;
GLuint u_sampler_uniformCube;




GLuint fbo;
GLuint color_texture;
GLuint depth_texture;



GLuint gWindowWidth;
GLuint gWindowHeight;

//Function declaration
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

//Main Function - WinMain()
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
	// function declarations
	int initialize(void);
	void display(void);
	void update(void);

	// variable declarations
	WNDCLASSEX wndClass;
	HWND hwnd;
	MSG msg;
	TCHAR szAppName[] = TEXT("MyApp");
	bool bDone = false;
	int iRet = 0;

	if (fopen_s(&gpFile, "Log.txt", "w") != 0)
	{
		MessageBox(NULL, 
			TEXT("Log File Can not be Created"), 
			TEXT("Error"), MB_OK | MB_TOPMOST | MB_ICONSTOP);
		exit(0);
	}
	else
	{
		fprintf(gpFile, "Log file is Sucessfully created \n");
	}

	// code
	// initialization of WNDCLASSEX
	wndClass.cbSize 		= sizeof(WNDCLASSEX);
	wndClass.style 			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wndClass.cbClsExtra 	= 0;
	wndClass.cbWndExtra 	= 0;
	wndClass.lpfnWndProc 	= WndProc;
	wndClass.hInstance 		= hInstance;
	wndClass.hIcon 			= LoadIcon(NULL, IDI_APPLICATION);
	wndClass.hCursor 		= LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground 	= (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndClass.lpszClassName 	= szAppName;
	wndClass.lpszMenuName 	= NULL;
	wndClass.hIconSm		= LoadIcon(NULL, IDI_APPLICATION);

	// register above class
	RegisterClassEx(&wndClass);

	// get mid point dimensions
	int screenX = ((GetSystemMetrics(SM_CXSCREEN)/2) - (WIN_WIDTH  / 2));
	int screenY = ((GetSystemMetrics(SM_CYSCREEN)/2) - (WIN_HEIGHT / 2));

	// create windowex
	hwnd = CreateWindowEx(WS_EX_APPWINDOW,
		szAppName,
		TEXT(".obj Loader"),
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
		screenX,
		screenY,
		WIN_WIDTH,
		WIN_HEIGHT,
		NULL,
		NULL,
		hInstance,
		NULL);

	ghwnd = hwnd;

	iRet = initialize(); // here we should check the value of iRet for errors

	if (iRet == -1)
	{
		fprintf(gpFile, "ChoosePixelFormat() failed\n");
		DestroyWindow(hwnd);
		exit(0);
	}
	else if (iRet == -2)
	{
		fprintf(gpFile, "SetPixelFormat() failed\n");
		DestroyWindow(hwnd);
		exit(0);
	}
	else if (iRet == -3)
	{
		fprintf(gpFile, "wglCreateContext() failed\n");
		DestroyWindow(hwnd);
		exit(0);
	}
	else if (iRet == -4)
	{
		fprintf(gpFile, "wglMakeCurrentContext() failed\n");
		DestroyWindow(hwnd);
		exit(0);
	}
	else
	{
		fprintf(gpFile, "Initialization Sucessfull !n");
	}

	// Show Window
	ShowWindow(hwnd, iCmdShow);

	//Here instead of calling UpdateWindow(), call below 2 new functions
	SetForegroundWindow(hwnd);
	SetFocus(hwnd);

	// Game loop
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
			//Game Loop - Here the rendering happens
			if (gbActiveWindow == true)
			{
				// Here call update()

			}
			// render function
			display();
		}
	}

	return((int)msg.wParam);
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	//Function declarations
	void ToggleFullscreen(void);
	void resize(int, int);
	void uninitialize(void);

	// code
	switch (iMsg)
	{
	case WM_SETFOCUS:
		gbActiveWindow = true;
		break;

	case WM_KILLFOCUS:
		gbActiveWindow = false;
		break;

	case WM_SIZE:
		gWindowWidth = LOWORD(lParam);
		gWindowHeight = HIWORD(lParam);
		resize(LOWORD(lParam), HIWORD(lParam));
		break;

	case WM_ERASEBKGND:
		return (0);

	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;
	case WM_CHAR:
		switch (wParam)
		{
		case 'F':
		case 'f':
			ToggleFullscreen();
			break;
		case 'L':
		case 'l':
			if (gbLighting == TRUE)
			{
				gbLighting = FALSE;
			}
			else
			{
				gbLighting = TRUE;
			}
			break;
		}
		break;
	case WM_KEYDOWN:
		switch (LOWORD(wParam))
		{
			case VK_ESCAPE:
				DestroyWindow(hwnd);
				break;
			default:
				break;
		}
		break;

	case WM_DESTROY:
		uninitialize();
		PostQuitMessage(0);
		break;

	default:
		break;
	}

	return (DefWindowProc(hwnd, iMsg, wParam, lParam));
}

void ToggleFullscreen(void)
{
	//code
	MONITORINFO mi;

	if (gbFullscreen == false)
	{
		dwStyle = GetWindowLong(ghwnd, GWL_STYLE);

		if (dwStyle & WS_OVERLAPPEDWINDOW)
		{
			mi = { sizeof(MONITORINFO) };
			if (GetWindowPlacement(ghwnd, &wpPrev)
				&& GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY), &mi))
			{
				SetWindowLong(ghwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);

				SetWindowPos(ghwnd,
					HWND_TOP,
					mi.rcMonitor.left,
					mi.rcMonitor.top,
					mi.rcMonitor.right - mi.rcMonitor.left,
					mi.rcMonitor.bottom - mi.rcMonitor.top,
					SWP_NOZORDER | SWP_FRAMECHANGED);
			}

		}

		ShowCursor(FALSE);
		gbFullscreen = true;
	}
	else
	{
		SetWindowLong(ghwnd,
			GWL_STYLE,
			dwStyle | WS_OVERLAPPEDWINDOW);

		SetWindowPlacement(ghwnd, &wpPrev);
		SetWindowPos(ghwnd,
			HWND_TOP,
			0,
			0,
			0,
			0,
			SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER);

		ShowCursor(TRUE);
		gbFullscreen = false;
	}
}

int initialize(void)
{
	//function declarations
	void resize(int, int);
	void uninitialize(void);
	BOOL LoadTexture(GLuint *pgliTexture, TCHAR imageResourceID[]);

	// variable declarations
	static PIXELFORMATDESCRIPTOR pfd;
	int iPixelFormatIndex;

	// code
	// Initialize pfd structure
	ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));
	pfd.nSize 		= sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion 	= 1;
	pfd.dwFlags 	= PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType 	= PFD_TYPE_RGBA;
	pfd.cColorBits 	= 32;
	pfd.cRedBits 	= 8;
	pfd.cGreenBits 	= 8;
	pfd.cBlueBits 	= 8;
	pfd.cAlphaBits 	= 8;
	pfd.cDepthBits 	= 24;

	ghdc = GetDC(ghwnd);

	iPixelFormatIndex = ChoosePixelFormat(ghdc, &pfd);
	//return index is always 1 based means it zero get return is failure
	if (iPixelFormatIndex == 0)
	{
		return (-1);
	}

	if (SetPixelFormat(ghdc, iPixelFormatIndex, &pfd) == FALSE)
	{
		return (-2);
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

	//GLEW intialization code for GLSL
	// IMPORTANT : it must be here.means after creating OpenGL Context
	// before using any opengl function
	result = glewInit();
	if (result != GLEW_OK)
	{
		fprintf(gpFile, "glewInit error.Exiting now\n");
		uninitialize();
	}

	// *** SHADER BLOCK ***
	GLint iProgramLinkStatus = 0;
	GLint iInfoLogLength = 0;
	GLchar *szInfoLog = NULL;

	// *** VERTEX SHADER ***
	vs = LoadShaders("./MEDIA/shaders/vertexshader.vs", GL_VERTEX_SHADER);
	if (!vs)
	{
		uninitialize();
		exit(0);
	}

	// *** FRAGMENT SHADER ***
	fs = LoadShaders("./MEDIA/shaders/fragmentshader.fs", GL_FRAGMENT_SHADER);
	if (!fs)
	{
		uninitialize();
		exit(0);
	}

	// *** SHADER program1 ***
	// create 
	program1 = glCreateProgram();

	glAttachShader(program1, vs);
	glAttachShader(program1, fs);

	// prelinking binding to vertex attribute //
	glBindAttribLocation(program1, AMC_ATTRIBUTE_POSITION, "vPosition");
	glBindAttribLocation(program1, AMC_ATTRIBUTE_NORMALS, "vNormal");
	glBindAttribLocation(program1, AMC_ATTRIBUTE_NORMALS, "vTexCord");
	
	// Link the shader program1
	glLinkProgram(program1);
	glGetProgramiv(program1, GL_LINK_STATUS, &iProgramLinkStatus);
	if (iProgramLinkStatus == GL_FALSE)
	{
		glGetProgramiv(program1, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength > 0)
		{
			szInfoLog = (char *)malloc(iInfoLogLength);
			if (szInfoLog != NULL)
			{
				GLsizei written;
				glGetProgramInfoLog(program1, iInfoLogLength, &written, szInfoLog);
				fprintf(gpFile, "Shader Program Link Log : %s\n", szInfoLog);
				free(szInfoLog);
				uninitialize();
				DestroyWindow(NULL);
				exit(0);
			}
		}
	}

	// postlinking receiving uniform location //
	mUniform = glGetUniformLocation(program1, "u_m_matrix");
	vUniform = glGetUniformLocation(program1, "u_v_matrix");
	pUniform = glGetUniformLocation(program1, "u_p_matrix");
	u_sampler_uniform = glGetUniformLocation(program1, "u_sampler");

	lKeyPressedUniform = glGetUniformLocation(program1, "u_l_key_pressed");

	laUniform = glGetUniformLocation(program1, "u_la");
	kaUniform = glGetUniformLocation(program1, "u_ka");

	ldUniform = glGetUniformLocation(program1, "u_ld");
	kdUniform = glGetUniformLocation(program1, "u_kd");

	lsUniform = glGetUniformLocation(program1, "u_ls");
	ksUniform = glGetUniformLocation(program1, "u_ks");

	lightPositionUniform = glGetUniformLocation(program1, "u_light_position");

	materialShininessUniform = glGetUniformLocation(program1, "u_materialShininess");


	// Load Obj Model And Prepare To Draw
	LoadObj("./MEDIA/objects/utahteapot.obj");
	//LoadObj("./MEDIA/objects/examplecube.obj");
	//LoadObj("./MEDIA/objects/spaceplane.obj");

	glEnable(GL_TEXTURE_2D);
	LoadTexture(&gTextureMarble, MAKEINTRESOURCE(ID_BITMAP_MARBLE));


	iProgramLinkStatus = 0;
	iInfoLogLength = 0;
	szInfoLog = NULL;


	// *** VERTEX SHADER ***
	vs = 0;
	vs = LoadShaders("./MEDIA/shaders/vertexshaderCube.vs", GL_VERTEX_SHADER);
	if (!vs)
	{
		uninitialize();
		exit(0);
	}

	// *** FRAGMENT SHADER ***
	fs = 0;
	fs = LoadShaders("./MEDIA/shaders/fragmentshaderCube.fs", GL_FRAGMENT_SHADER);
	if (!fs)
	{
		uninitialize();
		exit(0);
	}

	// *** SHADER program1 ***
	// create 
	program2 = glCreateProgram();

	glAttachShader(program2, vs);
	glAttachShader(program2, fs);

	glBindAttribLocation(program2, AMC_ATTRIBUTE_POSITION, "vPositionCube");
	glBindAttribLocation(program2, AMC_ATTRIBUTE_TEXCOORD0, "vTexCordCube");
	
	// Link the shader program1
	glLinkProgram(program2);
	glGetProgramiv(program2, GL_LINK_STATUS, &iProgramLinkStatus);
	if (iProgramLinkStatus == GL_FALSE)
	{
		glGetProgramiv(program2, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength > 0)
		{
			szInfoLog = (char *)malloc(iInfoLogLength);
			if (szInfoLog != NULL)
			{
				GLsizei written;
				glGetProgramInfoLog(program2, iInfoLogLength, &written, szInfoLog);
				fprintf(gpFile, "Shader Program Link Log : %s\n", szInfoLog);
				free(szInfoLog);
				uninitialize();
				DestroyWindow(NULL);
				exit(0);
			}
		}
	}

	// postlinking receiving uniform location //
	mUniformCube = glGetUniformLocation(program2, "u_m_matrixCube");
	vUniformCube = glGetUniformLocation(program2, "u_v_matrixCube");
	pUniformCube = glGetUniformLocation(program2, "u_p_matrixCube");
	u_sampler_uniformCube = glGetUniformLocation(program2, "u_samplerCube");

	const GLfloat quadVertices[] =
	{
		1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, 1.0f,
		1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f, -1.0f, 1.0f,
		-1.0f, -1.0f, -1.0f,
	};

	// create vao
	glGenVertexArrays(1, &vaoCube);
	glBindVertexArray(vaoCube);
	glGenBuffers(1, &vboCubePosition);
	glBindBuffer(GL_ARRAY_BUFFER, vboCubePosition);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	const GLfloat quadTexVertices[] =
	{
		1.0f, 1.0f,
		0.0f, 1.0f,
		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f,
		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f,
		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f,
		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f,
		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f,
		0.0f, 0.0f,
		1.0f, 0.0f
	};

	glGenBuffers(1, &vboCubeTexture);
	glBindBuffer(GL_ARRAY_BUFFER, vboCubeTexture);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadTexVertices), quadTexVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_TEXCOORD0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_TEXCOORD0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);


	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	glGenTextures(1, &color_texture);
	glBindTexture(GL_TEXTURE_2D, color_texture);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB8, 512, 512);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenTextures(1, &depth_texture);
	glBindTexture(GL_TEXTURE_2D, depth_texture);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32F, 512, 512);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, color_texture, 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth_texture, 0);

	static const GLenum draw_buffers[] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, draw_buffers);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	// clear the screen by opengl color
	glClearColor(0.0f, 0.0f, 1.0f, 0.0f);

	//depth
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	PerspectiveProjectionMatrix = mat4::identity();

	// intialize the values
	rAngle = 0.0f;

	//warm up call to resize
	resize(WIN_WIDTH, WIN_HEIGHT); //function calls

	return(0);
}

void resize(int width, int height)
{
	// code
	if (height == 0)
	{
		height = 1;
	}

	glViewport(0, 0, (GLsizei)width, (GLsizei)height);

	PerspectiveProjectionMatrix = perspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);

}

void display(void)
{
	//code
	
	// declaration of matrices //
	mat4 modelMatrix;
	mat4 viewMatrix;
	mat4 projectionMatrix;
	mat4 translationMatrix;
	mat4 rotationMatrix;

	
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, 512, 512);
	
	
	glUseProgram(program1);

	// initialization above matrices identity //
	modelMatrix 			= mat4::identity();
	viewMatrix = mat4::identity();
	projectionMatrix 	= mat4::identity();
	translationMatrix 			= mat4::identity();
	rotationMatrix				= mat4::identity();

	//translationMatrix 			= translate(0.0f, -7.5f, -30.0f);
	translationMatrix 			= translate(0.0f, 0.0f, -40.0f);
	modelMatrix 			= translationMatrix;
	rotationMatrix				= rotate(-90.0f, 1.0f, 0.0f, 0.0f);
	modelMatrix				= modelMatrix * rotationMatrix;
	rotationMatrix				= rotate(rAngle, rAngle, rAngle);
	modelMatrix				= modelMatrix * rotationMatrix;
	projectionMatrix 	= PerspectiveProjectionMatrix;// * modelViewMatrix;

	glUniformMatrix4fv(mUniform, 1, GL_FALSE, modelMatrix);
	glUniformMatrix4fv(vUniform, 1, GL_FALSE, viewMatrix);
	glUniformMatrix4fv(pUniform, 1, GL_FALSE, projectionMatrix);

	if (gbLighting == TRUE)
	{
		glUniform1i(lKeyPressedUniform, 1);

		glUniform3fv(laUniform, 1, gLightAmbient);
		glUniform3fv(kaUniform, 1, gMaterialAmbient);

		glUniform3fv(ldUniform, 1, gLightDiffuse); // white color / light color
		glUniform3fv(kdUniform, 1, gMaterialDiffuse); // gray color / material color

		glUniform3fv(lsUniform, 1, gLightSpecular);
		glUniform3fv(ksUniform, 1, gMaterialSpecular);

		glUniform4fv(lightPositionUniform, 1, gLightPosition); // light position

		glUniform1f(materialShininessUniform, gMaterialShininess);
	}
	else
	{
		glUniform1i(lKeyPressedUniform, 0);
	}

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTextureMarble);
	glUniform1i(u_sampler_uniform, 0);

	RenderObject();

	glUseProgram(0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glViewport(0, 0, gWindowWidth, gWindowHeight);
	glUseProgram(program2);

	// initialization above matrices identity //
	modelMatrix 			= mat4::identity();
	viewMatrix = mat4::identity();
	projectionMatrix 	= mat4::identity();
	translationMatrix 			= mat4::identity();
	rotationMatrix				= mat4::identity();

	translationMatrix 			= translate(0.0f, 0.0f, -5.0f);
	modelMatrix 			= translationMatrix;
	rotationMatrix				= rotate(rAngle, rAngle, rAngle);
	modelMatrix				= modelMatrix * rotationMatrix;
	projectionMatrix 	= PerspectiveProjectionMatrix;// * modelViewMatrix;

	glUniformMatrix4fv(mUniformCube, 1, GL_FALSE, modelMatrix);
	glUniformMatrix4fv(vUniformCube, 1, GL_FALSE, viewMatrix);
	glUniformMatrix4fv(pUniformCube, 1, GL_FALSE, projectionMatrix);

	glBindVertexArray(vaoCube);

	/*glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTextureMarble);
	glUniform1i(u_sampler_uniformCube, 0);*/
	
	glBindTexture(GL_TEXTURE_2D, color_texture);

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 4, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 8, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 12, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 16, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 20, 4);


	glUseProgram(0);

	// Start Using shader program1 object //
/*	glUseProgram(program1);


	// initialization above matrices identity //
	modelMatrix 			= mat4::identity();
	viewMatrix = mat4::identity();
	projectionMatrix 	= mat4::identity();
	translationMatrix 			= mat4::identity();
	rotationMatrix				= mat4::identity();

	translationMatrix 			= translate(0.0f, -7.5f, -30.0f);
	modelMatrix 			= translationMatrix;
	rotationMatrix				= rotate(-90.0f, 1.0f, 0.0f, 0.0f);
	modelMatrix				= modelMatrix * rotationMatrix;
	rotationMatrix				= rotate(rAngle, rAngle, rAngle);
	modelMatrix				= modelMatrix * rotationMatrix;
	projectionMatrix 	= PerspectiveProjectionMatrix;// * modelViewMatrix;

	glUniformMatrix4fv(mUniform, 1, GL_FALSE, modelMatrix);
	glUniformMatrix4fv(vUniform, 1, GL_FALSE, viewMatrix);
	glUniformMatrix4fv(pUniform, 1, GL_FALSE, projectionMatrix);

	if (gbLighting == TRUE)
	{
		glUniform1i(lKeyPressedUniform, 1);

		glUniform3fv(laUniform, 1, gLightAmbient);
		glUniform3fv(kaUniform, 1, gMaterialAmbient);

		glUniform3fv(ldUniform, 1, gLightDiffuse); // white color / light color
		glUniform3fv(kdUniform, 1, gMaterialDiffuse); // gray color / material color

		glUniform3fv(lsUniform, 1, gLightSpecular);
		glUniform3fv(ksUniform, 1, gMaterialSpecular);

		glUniform4fv(lightPositionUniform, 1, gLightPosition); // light position

		glUniform1f(materialShininessUniform, gMaterialShininess);
	}
	else
	{
		glUniform1i(lKeyPressedUniform, 0);
	}

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTextureMarble);
	glUniform1i(u_sampler_uniform, 0);

	RenderObject();

	glUseProgram(0);
*/
	SwapBuffers(ghdc);
	
	rAngle += 0.05f;
	if (rAngle >= 360.0f)
		rAngle = 0.0f;
}

void uninitialize(void)
{
	//code
	//check whether fullscreen or not if it is, then restore to
	// normal size & then proceed for uninitialization
	if (gbFullscreen == true)
	{
		// code
		SetWindowLong(ghwnd,
			GWL_STYLE,
			dwStyle | WS_OVERLAPPEDWINDOW);

		SetWindowPlacement(ghwnd, &wpPrev);
		SetWindowPos(ghwnd,
			HWND_TOP,
			0,
			0,
			0,
			0,
			SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER);


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
		ReleaseDC(ghwnd, ghdc);
		ghdc = NULL;
	}

	if (gpFile)
	{
		fprintf(gpFile, "Log file closed Succesfully");
		fclose(gpFile);
		gpFile = NULL;
	}

	if (vbo_position)
	{
		glDeleteBuffers(1, &vbo_position);
		vbo_position = 0;
	}

	if (vbo_element)
	{
		glDeleteBuffers(1, &vbo_element);
		vbo_element = 0;
	}

	if (vao)
	{
		glDeleteVertexArrays(1, &vao);
		vao = 0;
	}

	// safe Release //
	if (program1)
	{
		GLsizei ShaderCount;
		GLsizei ShaderNumber;

		glUseProgram(program1);

		// ask program how many shaders are attached to you
		glGetProgramiv(program1, GL_ATTACHED_SHADERS, &ShaderCount);
		GLuint *pShader = (GLuint *)malloc(sizeof(GLuint) * ShaderCount);
		if (pShader)
		{
			// give me attached shaders
			glGetAttachedShaders(program1, ShaderCount, &ShaderCount, pShader);
			for (ShaderNumber = 0; ShaderNumber <= ShaderCount; ShaderNumber++)
			{
				// DetachShader //
				glDetachShader(program1, pShader[ShaderNumber]);
				glDeleteShader(pShader[ShaderNumber]);
				pShader[ShaderNumber] = 0;
			}// end for loop

			free(pShader);
		} // end of if block of pShader

		glDeleteProgram(program1);
		program1 = 0;
		glUseProgram(0);
	} // end if block of program//

} // END OF UNINITIALIZE

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
