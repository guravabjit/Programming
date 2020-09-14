#include <Windows.h>
#include <GL/glew.h>
#include <GL/GL.h>
#include <stdio.h> // FILE
#include "vmath.h"
#include "Interleaved.h"

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

GLfloat lightAmbient[] = { 0.25f, 0.25f, 0.25f, 1.0f };
GLfloat lightDiffuse[] = { 1.0f,1.0f,1.0f,1.0f };
GLfloat lightSpecular[] = { 1.0f,1.0f,1.0f,1.0f };
GLfloat lightPosition[] = { 0.0f,0.0f,10.0f,1.0f };

GLfloat materialAmbient[] = { 0.0f,0.0f,0.0f,1.0f };
GLfloat materialDiffuse[] = { 1.0f,1.0f,1.0f,1.0f };
GLfloat materialSpecular[] = { 1.0f,1.0f,1.0f,1.0f };
GLfloat materialShininess = 128.0f;	//50.0f

GLuint vaoInterleaved;
GLuint vboInterleaved;

GLuint mUniform;
GLuint vUniform;
GLuint pUniform;

GLuint laUniform = 0;
GLuint kaUniform = 0;

GLuint ldUniform = 0;
GLuint kdUniform = 0;

GLuint lsUniform = 0;
GLuint ksUniform = 0;

GLuint materialShininessUniform = 0;
GLuint lightPositionUniform = 0;
GLuint lightingEnabledUniform;

GLuint samplerUniform;
mat4 perspectiveProjectionMatrix;

GLenum gGlEnumResult;
GLuint gGlShaderProgramObject;

GLuint gTextureStone;
GLfloat angleCube;

bool gbAnimation = false;
bool gbLighting = false;

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
		case 'l':
		case 'L':
			if (gbLighting == false)
			{
				gbLighting = true;
			}
			else if (gbLighting == true)
			{
				gbLighting = false;
			}
			break;
		case 'a':
		case 'A':
			if (gbAnimation == false)
			{
				gbAnimation = true;
			}
			else if (gbAnimation == true)
			{
				gbAnimation = false;
			}
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
		"in vec4 vPosition;\n" \
		"in vec4 vColor;\n" \
		"in vec3 vNormal;\n" \
		"in vec2 vTexCoord;\n" \
		"uniform mat4 u_m_matrix;\n" \
		"uniform mat4 u_v_matrix;\n" \
		"uniform mat4 u_p_matrix;\n" \
		"uniform int u_lkeypressed;\n" \
		"uniform vec4 u_light_position;\n" \
		"out vec3 tNorm;\n" \
		"out vec2 out_texcoord;\n" \
		"out vec3 light_direction;\n" \
		"out vec3 viewer_vector;\n" \
		"out vec4 out_color;\n" \
		"void main(void)\n" \
		"{\n" \
			"if(u_lkeypressed == 1)\n" \
			"{\n" \
				"vec4 eye_coordinates = u_v_matrix * u_m_matrix * vPosition;\n" \
				"tNorm = mat3(u_v_matrix * u_m_matrix) * vNormal;\n" \
				"light_direction = vec3(u_light_position - eye_coordinates);\n" \
				"viewer_vector = -eye_coordinates.xyz;\n" \
			"}\n" \
			"gl_Position = u_p_matrix * u_v_matrix* u_m_matrix * vPosition;\n" \
			"out_color = vColor;\n" \
			"out_texcoord = vTexCoord;\n" \
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
		"in vec3 tNorm;\n" \
		"in vec3 light_direction;\n" \
		"in vec3 viewer_vector;\n" \
		"in vec2 out_texcoord;\n" \
		"in vec4 out_color;\n" \
		"uniform int u_lkeypressed;\n" \
		"uniform vec3 u_la;\n" \
		"uniform vec3 u_ka;\n" \
		"uniform vec3 u_ld;\n" \
		"uniform vec3 u_kd;\n" \
		"uniform vec3 u_ls;\n" \
		"uniform vec3 u_ks;\n" \
		"uniform float u_material_shininess;\n" \
		"uniform sampler2D u_sampler;\n" \
		"out vec4 FragColor;\n" \
		"void main(void)\n" \
		"{\n" \
			"vec3 phong_ads_light;\n" \
			"if(u_lkeypressed == 1)\n" \
			"{\n" \
				"vec3 normalized_tNorm = normalize(tNorm);\n" \
				"vec3 normalized_light_direction = normalize(light_direction);\n" \
				"vec3 normalized_viewer_vector = normalize(viewer_vector);\n" \
				"float tn_dot_ld = max(dot(normalized_light_direction, normalized_tNorm), 0.0);\n" \
				"vec3 reflection_vector = reflect(-normalized_light_direction, normalized_tNorm);\n" \
				"vec3 ambient = u_la * u_ka;\n" \
				"vec3 diffuse = u_ld * u_kd * tn_dot_ld;\n" \
				"vec3 specular = u_ls * u_ks * pow(max(dot(reflection_vector, normalized_viewer_vector), 0.0), u_material_shininess);\n" \
				"phong_ads_light = ambient + diffuse + specular;\n" \
			"}\n" \
			"else\n" \
			"{\n" \
				"phong_ads_light = vec3(1.0, 1.0, 1.0);\n" \
			"}\n" \
			"FragColor = texture(u_sampler, out_texcoord) * out_color * vec4(phong_ads_light, 1.0);\n" \
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
	glBindAttribLocation(gGlShaderProgramObject, AMC_ATTRIBUTE_COLOR, "vColor");
	glBindAttribLocation(gGlShaderProgramObject, AMC_ATTRIBUTE_NORMAL, "vNormal");
	glBindAttribLocation(gGlShaderProgramObject, AMC_ATTRIBUTE_TEXCORD0, "vTexCoord");

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
	mUniform = glGetUniformLocation(gGlShaderProgramObject, "u_m_matrix");
	vUniform = glGetUniformLocation(gGlShaderProgramObject, "u_v_matrix");
	pUniform = glGetUniformLocation(gGlShaderProgramObject, "u_p_matrix");
	
	laUniform = glGetUniformLocation(gGlShaderProgramObject, "u_la");
	kaUniform = glGetUniformLocation(gGlShaderProgramObject, "u_ka");
	
	ldUniform = glGetUniformLocation(gGlShaderProgramObject, "u_ld");
	kdUniform = glGetUniformLocation(gGlShaderProgramObject, "u_kd");

	lsUniform = glGetUniformLocation(gGlShaderProgramObject, "u_ls");
	ksUniform = glGetUniformLocation(gGlShaderProgramObject, "u_ks");
	
	lightingEnabledUniform = glGetUniformLocation(gGlShaderProgramObject, "u_lkeypressed");
	materialShininessUniform = glGetUniformLocation(gGlShaderProgramObject, "u_material_shininess");
	lightPositionUniform = glGetUniformLocation(gGlShaderProgramObject, "u_light_position");
	samplerUniform = glGetUniformLocation(gGlShaderProgramObject, "u_sampler");

	// glBegin() ... glEnd() // vertices, colors, texture co-ordinates, normals // make their array

	const GLfloat quadVertices[] =
	{
		// Top face
		1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		-1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
		-1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,

		// Bottom face
		1.0f, -1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f,
		-1.0f, -1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f,
		1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,

		// Front face
		1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		-1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
		-1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
		1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,

		// Back face
		1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,
		-1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
		-1.0f, -1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,
		1.0f, -1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,

		// Right face
		1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
		1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
		1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,

		// Left face
		-1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
		-1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
		-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
		-1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f
	};

	// create vao
	glGenVertexArrays(1, &vaoInterleaved);
	glBindVertexArray(vaoInterleaved);
	glGenBuffers(1, &vboInterleaved);
	glBindBuffer(GL_ARRAY_BUFFER, vboInterleaved);
	
	glBufferData(GL_ARRAY_BUFFER, 24 * 11 * sizeof(GLfloat), quadVertices, GL_STATIC_DRAW);

	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);

	glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);

	glVertexAttribPointer(AMC_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(AMC_ATTRIBUTE_NORMAL);

	glVertexAttribPointer(AMC_ATTRIBUTE_TEXCORD0, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (void*)(9 * sizeof(GLfloat)));
	glEnableVertexAttribArray(AMC_ATTRIBUTE_TEXCORD0);
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

	glEnable(GL_TEXTURE_2D);
	LoadTexture(&gTextureStone, MAKEINTRESOURCE(ID_BITMAP_STONE));

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
	void update();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(gGlShaderProgramObject);

	// declaration of metrices
	mat4 modelMatrix;
	mat4 viewMatrix;
	mat4 projectionMatrix;
	mat4 translateMatrix;
	mat4 rotationMatrix;
	mat4 scaleMatrix;
	
	// initializing above metrices to identity
	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	projectionMatrix = mat4::identity();
	translateMatrix = mat4::identity();
	rotationMatrix = mat4::identity();
	scaleMatrix = mat4::identity();

	// do neccessry transformation, rotation, scaling
	translateMatrix = translate(0.0f, 0.0f, -6.0f);
	scaleMatrix = scale(0.75f, 0.75f, 0.75f);
	rotationMatrix = rotate(angleCube, 1.0f, 0.0f, 0.0f);
	rotationMatrix *= rotate(angleCube, 0.0f, 1.0f, 0.0f);
	rotationMatrix *= rotate(angleCube, 0.0f, 0.0f, 1.0f);

	// do necessary matrix multiplication
	//modelMatrix = (translateMatrix * scaleMatrix)*rotationMatrix;
	modelMatrix = modelMatrix * translateMatrix;
	modelMatrix = modelMatrix * scaleMatrix;
	modelMatrix = modelMatrix * rotationMatrix;
	projectionMatrix = projectionMatrix * perspectiveProjectionMatrix; //modelViewMatrix = (translateMatrix * scaleMatrix) * rotationMatrix;

	// send necessary metrices to shader in respective uniforms
	glUniformMatrix4fv(mUniform, 1, GL_FALSE, modelMatrix);
	glUniformMatrix4fv(vUniform, 1, GL_FALSE, viewMatrix);
	glUniformMatrix4fv(pUniform, 1, GL_FALSE, projectionMatrix);

	if (gbLighting == true)
	{
		glUniform1i(lightingEnabledUniform, 1);
		glUniform3fv(laUniform, 1, lightAmbient);
		glUniform3fv(ldUniform, 1, lightDiffuse);
		glUniform3fv(lsUniform, 1, lightSpecular);
		glUniform3fv(kaUniform, 1, materialAmbient);
		glUniform3fv(kdUniform, 1, materialDiffuse);
		glUniform3fv(ksUniform, 1, materialSpecular);
		glUniform1f(materialShininessUniform, materialShininess);
		glUniform4fv(lightPositionUniform, 1, lightPosition);
	}
	else
	{
		glUniform1i(lightingEnabledUniform, 0);
	}

	// bind with vao // this will avoid repetdly binding with vbo
	glBindVertexArray(vaoInterleaved);

	// similary bind with texture if any
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTextureStone);
	glUniform1i(samplerUniform, 0);

	// draw the necessary scene 
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 4, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 8, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 12, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 16, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 20, 4);

	glBindTexture(GL_TEXTURE_2D, 0);

	// unbind vao
	glBindVertexArray(0);

	// un-use the program
	glUseProgram(0);

	update();

	SwapBuffers(ghDC);
}

void uninitialize()
{
	void ToggleFullScreen();

	if (gbIsFullScreen == true)
	{
		ToggleFullScreen();
	}

	if (vaoInterleaved)
	{
		glDeleteVertexArrays(1, &vaoInterleaved);
		vaoInterleaved = 0;
	}

	if (vboInterleaved)
	{
		glDeleteBuffers(1, &vboInterleaved);
		vboInterleaved = 0;
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

void update()
{
	if (gbAnimation)
	{
		angleCube = angleCube + /*1.0f*/0.02f;
		if (angleCube == 360.0f)
		{
			angleCube = 0.0f;
		}
	}
}
