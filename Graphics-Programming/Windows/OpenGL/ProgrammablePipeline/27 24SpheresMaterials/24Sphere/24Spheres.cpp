#include <Windows.h>
#include <GL/glew.h>
#include <GL/GL.h>
#include <stdio.h> // FILE
#include "vmath.h"
#include "Sphere.h"

#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib,"Sphere.lib")

#define WIN_WIDTH	800
#define WIN_HEIGHT	600
#define ERROR_SIZE 1024

using namespace vmath;

enum RotationAxis
{
	enRA_X,
	enRA_Y,
	enRA_Z
};

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

typedef struct Materials
{

	GLfloat materialAmbient[4];
	GLfloat materialDiffuse[4];
	GLfloat materialSpecular[4];
	GLfloat materialShininess;

}MaterialsSphere;

MaterialsSphere materials[24] = {
{
{ 0.0215f, 0.1745f, 0.0215f, 1.0f },//ambient;
		{ 0.07568f, 0.61424f, 0.07568f, 1.0f}, //diffuse
		{ 0.633f, 0.727811f, 0.633f, 1.0f},//specular
 0.6f * 128//shininess
},

	{ //6 Brass
		{ 0.329412f, 0.223529f, 0.27451f, 1.0f },//ambient;
		{ 0.78392f, 0.568627f, 0.113725f, 1.0f }, //diffuse
		{ 0.992157f, 0.941176f, 0.807843f, 1.0f },//specular
		0.21794872f * 128//shininess
	},
	{ //12 Black
		{ 0.0f, 0.0f, 0.0f, 1.0f },//ambient;
		{ 0.0f, 0.0f, 0.0f, 1.0f }, //diffuse
		{ 0.50f, 0.50f, 0.50f, 1.0f },//specular
		0.25f * 128//shininess
	},
	{ //18 Black
		{ 0.02f, 0.02f, 0.02f, 1.0f },//ambient;
		{ 0.01f, 0.01f, 0.01f, 1.0f }, //diffuse
		{ 0.4f, 0.4f, 0.4f, 1.0f },//specular
		0.078125f * 128//shininess
	},
	{ //1 Jade
		{0.135f, 0.2225f, 0.1575f, 1.0f},//ambient;
		{0.54f, 0.89f, 0.63f, 1.0f}, //diffuse
		{0.316228f, 0.316228f, 0.316228f, 1.0f},//specular
		 0.1f * 128//shininess
	},
	{ //7 Bronze
		{ 0.2125f, 0.1275f, 0.054f, 1.0f },//ambient;
		{ 0.714f, 0.4284f, 0.18144f, 1.0f }, //diffuse
		{ 0.393548f, 0.271906f, 0.166721f, 1.0f },//specular
		0.2f * 128//shininess
	},
	{ //13 Cyan
		{ 0.0f, 0.1f, 0.06f, 1.0f },//ambient;
		{ 0.0f, 0.50980392f, 0.50980392f, 1.0f }, //diffuse
		{ 0.50196078f, 0.50196078f, 0.50196078f, 1.0f },//specular
		0.25 * 128//shininess
	},
		{ //19 Cyan
		{ 0.0f, 0.05f, 0.05f, 1.0f },//ambient;
		{ 0.4f, 0.5f, 0.5f, 1.0f }, //diffuse
		{ 0.04f, 0.7f, 0.7f, 1.0f },//specular
		0.078125f * 128//shininess
	},
{ //2 Obsidian
		{ 0.05375f, 0.5f, 0.06625f, 1.0f },//ambient;
		{ 0.18275f, 0.17f, 0.22525f, 1.0f }, //diffuse
		{ 0.332741f, 0.328634f, 0.346435f, 1.0f },//specular
		0.3f * 128//shininess
},

	{ //8 Chrome
		{ 0.25f, 0.25f, 0.25f, 1.0f },//ambient;
		{ 0.4f, 0.4f, 0.4f, 1.0f }, //diffuse
		{ 0.774597f, 0.774597f, 0.774597f, 1.0f },//specular
		0.6f * 128//shininess
	},
	{ //14 Green
		{ 0.0f, 0.0f, 0.0f, 1.0f },//ambient;
		{ 0.1f, 0.35f, 0.1f, 1.0f }, //diffuse
		{ 0.45f, 0.55f, 0.45f, 1.0f },//specular
		0.25f * 128//shininess
	},
	{ //20 Green
		{ 0.0f, 0.05f, 0.00f, 1.0f },//ambient;
		{ 0.4f, 0.5f, 0.4f, 1.0f }, //diffuse
		{ 0.04f, 0.7f, 0.04f, 1.0f },//specular
		0.078125f * 128//shininess
	},


	{ //3 Pearl
		{ 0.25f, 0.20725f, 0.20725f, 1.0f },//ambient;
		{ 1.0f, 0.829f, 0.829f, 1.0f }, //diffuse
		{ 0.296648f, 0.296648f, 0.296648f, 1.0f },//specular
		0.88f * 128//shininess
	},
	{ //9 Copper
		{ 0.19125f, 0.0735f, 0.0225f, 1.0f },//ambient;
		{ 0.7038f, 0.27048f, 0.0828f, 1.0f }, //diffuse
		{ 0.25677f, 0.137622f, 0.086014f, 1.0f },//specular
		0.1f * 128//shininess
	},
	{ //15 Red
		{ 0.0f, 0.0f, 0.0f, 1.0f },//ambient;
		{ 0.5f, 0.0f, 0.0f, 1.0f }, //diffuse
		{ 0.7f, 0.6f, 0.6f, 1.0f },//specular
		0.25f * 128//shininess
	},
	{ //21 Red
		{ 0.05f, 0.0f, 0.0f, 1.0f },//ambient;
		{ 0.5f, 0.4f, 0.4f, 1.0f }, //diffuse
		{ 0.7f, 0.04f, 0.04f, 1.0f },//specular
		0.078125f * 128//shininess
	},
	{ //4 Ruby
		{ 0.1745f, 0.01175f, 0.01175f, 1.0f },//ambient;
		{ 0.61424f, 0.04136f, 0.04136f, 1.0f }, //diffuse
		{ 0.727811f, 0.626959f, 0.626959f, 1.0f },//specular
		0.6f * 128//shininess
	},
			{ //10 Gold
		{ 0.24725f, 0.1995f, 0.0745f, 1.0f },//ambient;
		{ 0.75164f, 0.60648f, 0.22648f, 1.0f }, //diffuse
		{ 0.628281f, 0.555802f, 0.366065f, 1.0f },//specular
		0.4 * 128//shininess
			},
		{ //16 White
		{ 0.0f, 0.0f, 0.0f, 1.0f },//ambient;
		{ 0.55f, 0.55f, 0.55f, 1.0f }, //diffuse
		{ 0.70f, 0.70f, 0.70f, 1.0f },//specular
		0.25f * 128//shininess
		},

	{ //22 White
		{ 0.05f, 0.05f, 0.05f, 1.0f },//ambient;
		{ 0.5f, 0.5f, 0.5f, 1.0f }, //diffuse
		{ 0.7f, 0.7f, 0.7f, 1.0f },//specular
		0.078125f * 128//shininess
	},
	{ //5 Turquoise
		{ 0.1f, 0.18725f, 0.1745f, 1.0f },//ambient;
		{ 0.396f, 0.74151f, 0.69102f, 1.0f }, //diffuse
		{ 0.297254f, 0.30829f, 0.306678f, 1.0f },//specular
		0.1f * 128//shininess
	},




	{ //11 Silver
		{ 0.19225f, 0.19225f, 0.19225f, 1.0f },//ambient;
		{ 0.50745f, 0.50745f, 0.50745f, 1.0f }, //diffuse
		{ 0.508273f, 0.508273f, 0.508273f, 1.0f },//specular
		0.4f * 128//shininess
	},





	{ //17 Yellow Plastic
		{ 0.0f, 0.0f, 0.0f, 1.0f },//ambient;
		{ 0.5f, 0.5f, 0.0f, 1.0f }, //diffuse
		{ 0.60f, 0.60f, 0.50f, 1.0f },//specular
		0.25f * 128//shininess
	},




	{ //23 Yellow Rubber
		{ 0.05f, 0.05f, 0.0f, 1.0f },//ambient;
		{ 0.5f, 0.5f, 0.4f, 1.0f }, //diffuse
		{ 0.7f, 0.7f, 0.04f, 1.0f },//specular
		0.078125f * 128//shininess
	}
};

typedef struct Light_Material
{
	float fLightAmbient[4];
	float fLightDiffuse[4];
	float fLightSpecular[4];
	float fLightPosition[4];

	float fMaterialAmbient[4];
	float fMaterialDiffuse[4];
	float fMaterialSpecular[4];

	float fMaterialShyniness[1];

	GLuint lKeyPressedUniform;
	GLuint viewUniform;
	GLuint modelUniform;
	GLuint projectionUniform;
	GLuint laUniform;
	GLuint kaUniform;
	GLuint ldUniform;
	GLuint kdUniform;
	GLuint lsUniform;
	GLuint ksUniform;
	GLuint lightPositionUniform;
	GLuint materialShininessUniform;

	GLuint lKeyPressedUniformV;
	GLuint viewUniformV;
	GLuint modelUniformV;
	GLuint projectionUniformV;
	GLuint laUniformV;
	GLuint kaUniformV;
	GLuint ldUniformV;
	GLuint kdUniformV;
	GLuint lsUniformV;
	GLuint ksUniformV;
	GLuint lightPositionUniformV;
	GLuint materialShininessUniformV;

}LIGHT_MATERIAL;


float gW = 1.0f;
float gH = 1.0f;
float gX = 0.0f;
float gY = 0.0f;
int gTotalNumberOfRows = 6;
int gTotalNumberOfColumns = 4;

LIGHT_MATERIAL gLightMaterial[24];
GLuint sphereNumberUninformV;
GLuint sphereNumberUninform;

RotationAxis giRotationAxis = enRA_X;
GLfloat gfAngleXAxisRot = 0.0f;
GLfloat gfAngleYAxisRot = 0.0f;
GLfloat gfAngleZAxisRot = 0.0f;


LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
GLboolean GetGlErrorStatus(GLGetErrorType, GLuint, char*);

HWND ghWnd = NULL;
HDC ghDC = NULL;
HGLRC ghGlRC = NULL;

bool gbIsActiveWindow = false;
bool gbIsFullScreen = false;
bool gbLighting = false;
bool gbAnimation = false;
bool gbPerVer = false;
bool gbPerFrag = true;

FILE *gpLogFile = NULL;

DWORD gdwStyle = 0;
WINDOWPLACEMENT gwpPrev = { sizeof(WINDOWPLACEMENT) };


GLenum gGlEnumResult;
GLuint gGlShaderProgramObjectV;
GLuint gGlShaderProgramObjectF;

GLuint vaoSphere;
GLuint vboSpherePosition;
GLuint vboSphereNormal;
GLuint vboSphereElements;

GLuint mvpUniform;
mat4 perspectiveProjectionMatrix;

float sphere_vertices[1146];
float sphere_normals[1146];
float sphere_textures[764];
unsigned short sphere_elements[2280];

int gNumVertices;
int gNumElements;

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
			ToggleFullScreen();
			break;
		}
		break;
	case WM_CHAR:
		switch (wParam)
		{

		case 'A':
		case 'a':
			if (gbAnimation == true)
			{
				gbAnimation = false;
			}
			else
			{
				gbAnimation = true;
			}
			break;

		case 'Q':
		case 'q':
			DestroyWindow(hWnd);
			break;
		case 'L':
		case 'l':
			if (gbLighting == true)
			{
				gbLighting = false;
			}
			else
			{
				gbLighting = true;
			}
			break;
		case 'V':
		case 'v':
			/*if (gbPerVer == true)
			{
				gbPerVer = false;
				gbPerFrag = true;
			}
			else
			{
				gbPerVer = true;
				gbPerFrag = false;
			}*/
			break;

		case 'F':
		case 'f':
			if (gbPerFrag == true)
			{
				gbPerFrag = false;
				//gbPerVer = true;
			}
			else
			{
				gbPerFrag = true;
				//gbPerVer = false;
			}
			break;
		case 'x':
		case 'X':
			gfAngleXAxisRot = 90.0f;
			giRotationAxis = enRA_X;
			break;

		case 'y':
		case 'Y':
			gfAngleYAxisRot = 90.0f;
			giRotationAxis = enRA_Y;
			break;

		case 'z':
		case 'Z':
			gfAngleZAxisRot = 90.0f;
			giRotationAxis = enRA_Z;
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
	int PerFragLight();
	int PerVerLight();

	void resize(int, int);
	void initialize24SphereLightMaterial();

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

	/*if (0 != PerVerLight())
	{
		return -5;
	}*/

	if (0 != PerFragLight())
	{
		return -5;
	}

	// glBegin() ... glEnd() // vertices, colors, texture co-ordinates, normals // make their array
	getSphereVertexData(sphere_vertices, sphere_normals, sphere_textures, sphere_elements);
	gNumVertices = getNumberOfSphereVertices();
	gNumElements = getNumberOfSphereElements();

	// vao
	glGenVertexArrays(1, &vaoSphere);
	glBindVertexArray(vaoSphere);

	// position vbo
	glGenBuffers(1, &vboSpherePosition);
	glBindBuffer(GL_ARRAY_BUFFER, vboSpherePosition);
	glBufferData(GL_ARRAY_BUFFER, sizeof(sphere_vertices), sphere_vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// normal vbo
	glGenBuffers(1, &vboSphereNormal);
	glBindBuffer(GL_ARRAY_BUFFER, vboSphereNormal);
	glBufferData(GL_ARRAY_BUFFER, sizeof(sphere_normals), sphere_normals, GL_STATIC_DRAW);

	glVertexAttribPointer(AMC_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(AMC_ATTRIBUTE_NORMAL);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// element vbo
	glGenBuffers(1, &vboSphereElements);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(sphere_elements), sphere_elements, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// unbind vao
	glBindVertexArray(0);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepth(1.0f);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	resize(WIN_WIDTH, WIN_HEIGHT);

	return 0;
}

void initialize24SphereLightMaterial()
{
	void FillMaterial(int iItr);
	int iItr = 0;
	int iIndex = 0;

	for (iItr = 0; iItr < 24; iItr++)
	{
		iIndex = 0;
		gLightMaterial[iItr].fLightAmbient[iIndex++] = 0.0f;
		gLightMaterial[iItr].fLightAmbient[iIndex++] = 0.0f;
		gLightMaterial[iItr].fLightAmbient[iIndex++] = 0.0f;
		gLightMaterial[iItr].fLightAmbient[iIndex++] = 1.0f;

		iIndex = 0;
		gLightMaterial[iItr].fLightDiffuse[iIndex++] = 1.0f;
		gLightMaterial[iItr].fLightDiffuse[iIndex++] = 1.0f;
		gLightMaterial[iItr].fLightDiffuse[iIndex++] = 1.0f;
		gLightMaterial[iItr].fLightDiffuse[iIndex++] = 1.0f;

		iIndex = 0;
		gLightMaterial[iItr].fLightSpecular[iIndex++] = 1.0f;
		gLightMaterial[iItr].fLightSpecular[iIndex++] = 1.0f;
		gLightMaterial[iItr].fLightSpecular[iIndex++] = 1.0f;
		gLightMaterial[iItr].fLightSpecular[iIndex++] = 1.0f;

		iIndex = 0;
		gLightMaterial[iItr].fLightPosition[iIndex++] = 1.0f;
		gLightMaterial[iItr].fLightPosition[iIndex++] = 1.0f;
		gLightMaterial[iItr].fLightPosition[iIndex++] = 1.0f;
		gLightMaterial[iItr].fLightPosition[iIndex++] = 1.0f;

		FillMaterial(iItr);
		/*iIndex = 0;
		gLightMaterial[iItr].fMaterialAmbient[iIndex++] = 0.0f;
		gLightMaterial[iItr].fMaterialAmbient[iIndex++] = 0.0f;
		gLightMaterial[iItr].fMaterialAmbient[iIndex++] = 0.0f;
		gLightMaterial[iItr].fMaterialAmbient[iIndex++] = 1.0f;

		iIndex = 0;
		gLightMaterial[iItr].fMaterialDiffuse[iIndex++] = 1.0f;
		gLightMaterial[iItr].fMaterialDiffuse[iIndex++] = 1.0f;
		gLightMaterial[iItr].fMaterialDiffuse[iIndex++] = 1.0f;
		gLightMaterial[iItr].fMaterialDiffuse[iIndex++] = 1.0f;

		iIndex = 0;
		gLightMaterial[iItr].fMaterialSpecular[iIndex++] = 1.0f;
		gLightMaterial[iItr].fMaterialSpecular[iIndex++] = 1.0f;
		gLightMaterial[iItr].fMaterialSpecular[iIndex++] = 1.0f;
		gLightMaterial[iItr].fMaterialSpecular[iIndex++] = 1.0f;*/

		iIndex = 0;
		gLightMaterial[iItr].fMaterialShyniness[iIndex++] = 128.0f;
	}

	
	
}

void resize(int iWidth, int iHeight)
{
	glViewport(0, 0, (GLsizei)iWidth, (GLsizei)iHeight);

	perspectiveProjectionMatrix = perspective(45.0f,
		(GLfloat)iWidth / (GLfloat)iHeight,
		0.1f,
		100.0f);

	gW = (GLfloat)iWidth / (GLfloat)gTotalNumberOfRows;
	gH = (GLfloat)iHeight / (GLfloat)gTotalNumberOfRows;
	gX = (iWidth - (gW * gTotalNumberOfColumns)) / 2;
	gY = (iHeight - (gH * gTotalNumberOfRows)) / 2;

}

void display()
{
	void Draw24Spheres();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/*if (gbPerVer)
	{
		//PerVerLight();
		glUseProgram(gGlShaderProgramObjectV);
	}*/

	if (gbPerFrag)
	{
		//PerFragLight();
		glUseProgram(gGlShaderProgramObjectF);
	}

	Draw24Spheres();
	
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

	if (vboSphereElements)
	{
		glDeleteBuffers(1, &vboSphereElements);
	}

	if (vboSphereNormal)
	{
		glDeleteBuffers(1, &vboSphereNormal);
	}

	if (vboSpherePosition)
	{
		glDeleteBuffers(1, &vboSpherePosition);
	}

	if (vaoSphere)
	{
		glDeleteVertexArrays(1, &vaoSphere);
	}

	GLsizei shaderCount = 0;
	GLsizei shaderNumber = 0;

	if (gGlShaderProgramObjectF)
	{
		glUseProgram(gGlShaderProgramObjectF);

		// ask program how many shader attached to it
		glGetProgramiv(gGlShaderProgramObjectF, GL_ATTACHED_SHADERS, &shaderCount);
		if (shaderCount > 0)
		{
			GLuint *pShaders = (GLuint*)malloc(sizeof(GLuint) * shaderCount);
			if (pShaders)
			{
				glGetAttachedShaders(gGlShaderProgramObjectF, shaderCount, &shaderCount, pShaders);
				for (shaderNumber = 0; shaderNumber < shaderCount; shaderNumber++)
				{
					glDetachShader(gGlShaderProgramObjectF, pShaders[shaderNumber]);

					// delete shader
					glDeleteShader(pShaders[shaderNumber]);
					pShaders[shaderNumber] = 0;
				}
				free(pShaders);
			}
		}
		glDeleteProgram(gGlShaderProgramObjectF);
		gGlShaderProgramObjectF = 0;
		glUseProgram(0);
	}

	if (gGlShaderProgramObjectV)
	{
		glUseProgram(gGlShaderProgramObjectV);

		// ask program how many shader attached to it
		glGetProgramiv(gGlShaderProgramObjectV, GL_ATTACHED_SHADERS, &shaderCount);
		if (shaderCount > 0)
		{
			GLuint *pShaders = (GLuint*)malloc(sizeof(GLuint) * shaderCount);
			if (pShaders)
			{
				glGetAttachedShaders(gGlShaderProgramObjectV, shaderCount, &shaderCount, pShaders);
				for (shaderNumber = 0; shaderNumber < shaderCount; shaderNumber++)
				{
					glDetachShader(gGlShaderProgramObjectV, pShaders[shaderNumber]);

					// delete shader
					glDeleteShader(pShaders[shaderNumber]);
					pShaders[shaderNumber] = 0;
				}
				free(pShaders);
			}
		}
		glDeleteProgram(gGlShaderProgramObjectV);
		gGlShaderProgramObjectV = 0;
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
	if (gbAnimation)
	{
		if (enRA_X == giRotationAxis)
		{
			gfAngleXAxisRot += 0.005f;
			gfAngleYAxisRot = 0.0f;
			gfAngleZAxisRot = 0.0f;

		}
		else if (enRA_Y == giRotationAxis)
		{
			gfAngleYAxisRot += 0.005f;
			gfAngleXAxisRot = 0.0f;
			gfAngleZAxisRot = 0.0f;
		}
		else if (enRA_Z == giRotationAxis)
		{
			gfAngleZAxisRot += 0.005f;
			gfAngleXAxisRot = 0.0f;
			gfAngleYAxisRot = 0.0f;
		}
	}
}

void Draw24Spheres()
{
	void drawSphere(float trnasX, float trnasY, float trnasZ, int iIndex, float x, float y, float widht, float height);

	int iIndex = 0;

	float trnasX;
	float trnasY;
	float trnasZ;

	trnasX = -6.0f;
	trnasY = 3.7f;
	trnasZ = -10.0f;
	for (int iItr1 = 0; iItr1 < 4; iItr1++)
	{
		for (int iItr = 0; iItr < 6; iItr++)
		{
			drawSphere(trnasX, trnasY, trnasZ, iIndex, gX+(iItr1*gW), gY+(iItr*gH), gW , gH);
			iIndex++;
			trnasY -= 1.5f;
		}
		trnasY = 3.7f;
		trnasX += 4.0f;
	}
}

void drawSphere(float trnasX, float trnasY, float trnasZ, int iIndex, float x, float y, float widht, float height)
{
	// declaration of metrices
	mat4 translationMatrix;
	mat4 rotationMatrix;
	mat4 scaleMatrix;

	mat4 modelMatrix;
	mat4 viewMatrix;
	mat4 projectionMatrix;
	perspectiveProjectionMatrix = mat4::identity();
	glViewport((GLint)x, (GLint)y, (GLsizei)widht, (GLsizei)height);
	perspectiveProjectionMatrix = perspective(60.0f,
		(GLfloat)widht / (GLfloat)height,
		0.1f,
		100.0f);


	// initializing above metrices to identity
	translationMatrix = mat4::identity();
	rotationMatrix = mat4::identity();
	scaleMatrix = mat4::identity();

	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	projectionMatrix = mat4::identity();

	// do neccessry transformation, rotation, scaling
	//translationMatrix = translate(trnasX, trnasY, trnasZ);
	translationMatrix = translate(0.0f, 0.0f, -3.0f);
	//scaleMatrix = scale(0.5f, 0.5f, 0.5f);
	float fLightPosition[4] = { 0.0f, 0.0f, 2.0f, 1.0f };
	if (enRA_X == giRotationAxis)
	{
		rotationMatrix = rotate(gfAngleXAxisRot, 1.0f, 0.0f, 0.0f);
		gLightMaterial[iIndex].fLightPosition[0] = 0.0f;
		gLightMaterial[iIndex].fLightPosition[1] = (GLfloat)sin(gfAngleXAxisRot) * 100;
		gLightMaterial[iIndex].fLightPosition[2] = (GLfloat)cos(gfAngleXAxisRot) * 100;
		gLightMaterial[iIndex].fLightPosition[3] = 1.0f;

		fLightPosition[0] = 0.0f;
		fLightPosition[1] = (GLfloat)sin(gfAngleXAxisRot) * 100;
		fLightPosition[2] = (GLfloat)cos(gfAngleXAxisRot) * 100;;
		fLightPosition[3] = 1.0f;
	}
	else if (enRA_Y == giRotationAxis)
	{
		rotationMatrix = rotate(gfAngleYAxisRot, 0.0f, 1.0f, 0.0f);
		gLightMaterial[iIndex].fLightPosition[0] = (GLfloat)sin(gfAngleYAxisRot)*100;
		gLightMaterial[iIndex].fLightPosition[1] = 0.0f;
		gLightMaterial[iIndex].fLightPosition[2] = (GLfloat)cos(gfAngleYAxisRot) * 100;
		gLightMaterial[iIndex].fLightPosition[3] = 1.0f;

		fLightPosition[1] = 0.0f;
		fLightPosition[0] = (GLfloat)sin(gfAngleYAxisRot) * 100;
		fLightPosition[2] = (GLfloat)cos(gfAngleYAxisRot) * 100;
		fLightPosition[3] = 1.0f;
	}
	else if (enRA_Z == giRotationAxis)
	{
		rotationMatrix = rotate(gfAngleZAxisRot, 0.0f, 0.0f, 1.0f);
		gLightMaterial[iIndex].fLightPosition[0] = (GLfloat)sin(gfAngleZAxisRot)*100;
		gLightMaterial[iIndex].fLightPosition[1] = (GLfloat)cos(gfAngleZAxisRot) * 100;
		gLightMaterial[iIndex].fLightPosition[2] = 0.0f;
		gLightMaterial[iIndex].fLightPosition[3] = 1.0f;

		fLightPosition[2] = 0.0f;
		fLightPosition[0] = (GLfloat)sin(gfAngleZAxisRot) * 100;
		fLightPosition[1] = (GLfloat)cos(gfAngleZAxisRot) * 100;;
		fLightPosition[3] = 1.0f;
	}

	// do necessary matrix multiplication
	//modelMatrix = (translationMatrix * scaleMatrix) * rotationMatrix;
	modelMatrix = translationMatrix;
	projectionMatrix = perspectiveProjectionMatrix;



	// send necessary metrices to shader in respective uniforms
	
	if (gbLighting == true)
	{
		if (gbPerVer)
		{
			glUniformMatrix4fv(gLightMaterial[iIndex].viewUniformV, 1, GL_FALSE, viewMatrix);
			glUniformMatrix4fv(gLightMaterial[iIndex].modelUniformV, 1, GL_FALSE, modelMatrix);
			glUniformMatrix4fv(gLightMaterial[iIndex].projectionUniformV, 1, GL_FALSE, projectionMatrix);

			glUniform1i(sphereNumberUninformV, iIndex);
			glUniform1i(gLightMaterial[iIndex].lKeyPressedUniformV, 1);

			glUniform3fv(gLightMaterial[iIndex].laUniformV, 1, gLightMaterial[/*iIndex*/0].fLightAmbient);
			glUniform3fv(gLightMaterial[iIndex].kaUniformV, 1, gLightMaterial[iIndex].fMaterialAmbient);

			glUniform3fv(gLightMaterial[iIndex].ldUniformV, 1, gLightMaterial[/*iIndex*/0].fLightDiffuse); // white color / light color
			glUniform3fv(gLightMaterial[iIndex].kdUniformV, 1, gLightMaterial[iIndex].fMaterialDiffuse); // gray color / material color

			glUniform3fv(gLightMaterial[iIndex].lsUniformV, 1, gLightMaterial[/*iIndex*/0].fLightSpecular);
			glUniform3fv(gLightMaterial[iIndex].ksUniformV, 1, gLightMaterial[iIndex].fMaterialSpecular);

			glUniform4fv(gLightMaterial[iIndex].lightPositionUniformV, 1, gLightMaterial[iIndex].fLightPosition); // light position

			glUniform1f(gLightMaterial[iIndex].materialShininessUniformV, gLightMaterial[iIndex].fMaterialSpecular[0]);
		}

		if (gbPerFrag)
		{
			float fLightAmbient[4] = {0.0f, 0.0f, 0.0f, 1.0f};
			float fLightDiffuse[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
			float fLightSpecular[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
			


			glUniformMatrix4fv(gLightMaterial[/*iIndex*/0].viewUniform, 1, GL_FALSE, viewMatrix);
			glUniformMatrix4fv(gLightMaterial[/*iIndex*/0].modelUniform, 1, GL_FALSE, modelMatrix);
			glUniformMatrix4fv(gLightMaterial[/*iIndex*/0].projectionUniform, 1, GL_FALSE, projectionMatrix);

			glUniform1i(sphereNumberUninform, /*iIndex*/0);
			glUniform1i(gLightMaterial[/*iIndex*/0].lKeyPressedUniform, 1);

			glUniform3fv(gLightMaterial[/*iIndex*/0].laUniform, 1, fLightAmbient);//gLightMaterial[/*iIndex*/0].fLightAmbient);
			glUniform3fv(gLightMaterial[/*iIndex*/0].kaUniform, 1, materials[iIndex].materialAmbient); //gLightMaterial[iIndex].fMaterialAmbient);

			glUniform3fv(gLightMaterial[/*iIndex*/0].ldUniform, 1, fLightDiffuse);//gLightMaterial[/*iIndex*/0].fLightDiffuse); // white color / light color
			glUniform3fv(gLightMaterial[/*iIndex*/0].kdUniform, 1, materials[iIndex].materialDiffuse);//gLightMaterial[iIndex].fMaterialDiffuse); // gray color / material color

			glUniform3fv(gLightMaterial[/*iIndex*/0].lsUniform, 1, fLightSpecular); //gLightMaterial[/*iIndex*/0].fLightSpecular);
			glUniform3fv(gLightMaterial[/*iIndex*/0].ksUniform, 1, materials[iIndex].materialSpecular);//gLightMaterial[iIndex].fMaterialSpecular);

			glUniform4fv(gLightMaterial[/*iIndex*/0].lightPositionUniform, 1, fLightPosition);//gLightMaterial[iIndex].fLightPosition); // light position

			glUniform1f(gLightMaterial[/*iIndex*/0].materialShininessUniform, materials[iIndex].materialShininess);// [iIndex].fMaterialSpecular[0]);
		}
	}
	else
	{
		if (gbPerVer)
		{
			glUniformMatrix4fv(gLightMaterial[iIndex].viewUniformV, 1, GL_FALSE, viewMatrix);
			glUniformMatrix4fv(gLightMaterial[iIndex].modelUniformV, 1, GL_FALSE, modelMatrix);
			glUniformMatrix4fv(gLightMaterial[iIndex].projectionUniformV, 1, GL_FALSE, projectionMatrix);

			glUniform1i(sphereNumberUninformV, iIndex);
			glUniform1i(gLightMaterial[iIndex].lKeyPressedUniformV, 0);
		}
		if (gbPerFrag)
		{
			glUniformMatrix4fv(gLightMaterial[/*iIndex*/0].viewUniform, 1, GL_FALSE, viewMatrix);
			glUniformMatrix4fv(gLightMaterial[/*iIndex*/0].modelUniform, 1, GL_FALSE, modelMatrix);
			glUniformMatrix4fv(gLightMaterial[/*iIndex*/0].projectionUniform, 1, GL_FALSE, projectionMatrix);

			glUniform1i(sphereNumberUninform, /*iIndex*/0);
			glUniform1i(gLightMaterial[/*iIndex*/0].lKeyPressedUniform, 0);
		}
	}

	// *** bind vao ***
	glBindVertexArray(vaoSphere);

	// *** draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);

	// *** unbind vao ***
	glBindVertexArray(0);

}

void FillMaterial(int iItr)
{
	switch (iItr)
	{
		case 0:
			gLightMaterial[iItr].fMaterialAmbient[0] = 0.0215f;
			gLightMaterial[iItr].fMaterialAmbient[1] = 0.1745f;
			gLightMaterial[iItr].fMaterialAmbient[2] = 0.0215f;
			gLightMaterial[iItr].fMaterialAmbient[3] = 1.0f;

			gLightMaterial[iItr].fMaterialDiffuse[0] = 0.07568f;
			gLightMaterial[iItr].fMaterialDiffuse[1] = 0.61424f;
			gLightMaterial[iItr].fMaterialDiffuse[2] = 0.075685f;
			gLightMaterial[iItr].fMaterialDiffuse[3] = 1.0f;

			gLightMaterial[iItr].fMaterialSpecular[0] = 0.633f;
			gLightMaterial[iItr].fMaterialSpecular[1] = 0.727811f;
			gLightMaterial[iItr].fMaterialSpecular[2] = 0.633f;
			gLightMaterial[iItr].fMaterialSpecular[3] = 1.0f;
			break;

		case 1:
			gLightMaterial[iItr].fMaterialAmbient[0] = 0.135f;
			gLightMaterial[iItr].fMaterialAmbient[1] = 0.2225f;
			gLightMaterial[iItr].fMaterialAmbient[2] = 0.1575f;
			gLightMaterial[iItr].fMaterialAmbient[3] = 1.0f;

			gLightMaterial[iItr].fMaterialDiffuse[0] = 0.54f;
			gLightMaterial[iItr].fMaterialDiffuse[1] = 0.89f;
			gLightMaterial[iItr].fMaterialDiffuse[2] = 0.63f;
			gLightMaterial[iItr].fMaterialDiffuse[3] = 1.0f;

			gLightMaterial[iItr].fMaterialSpecular[0] = 0.316228f;
			gLightMaterial[iItr].fMaterialSpecular[1] = 0.316228f;
			gLightMaterial[iItr].fMaterialSpecular[2] = 0.316228f;
			gLightMaterial[iItr].fMaterialSpecular[3] = 1.0f;
			break;

		default:
			gLightMaterial[iItr].fMaterialAmbient[0] = 0.0215f;
			gLightMaterial[iItr].fMaterialAmbient[1] = 0.1745f;
			gLightMaterial[iItr].fMaterialAmbient[2] = 0.0215f;
			gLightMaterial[iItr].fMaterialAmbient[3] = 1.0f;

			gLightMaterial[iItr].fMaterialDiffuse[0] = 0.07568f;
			gLightMaterial[iItr].fMaterialDiffuse[1] = 0.61424f;
			gLightMaterial[iItr].fMaterialDiffuse[2] = 0.075685f;
			gLightMaterial[iItr].fMaterialDiffuse[3] = 1.0f;

			gLightMaterial[iItr].fMaterialSpecular[0] = 0.633f;
			gLightMaterial[iItr].fMaterialSpecular[1] = 0.727811f;
			gLightMaterial[iItr].fMaterialSpecular[2] = 0.633f;
			gLightMaterial[iItr].fMaterialSpecular[3] = 1.0f;
			break;
	}
}

int PerFragLight()
{
	char szglErrorStr[ERROR_SIZE] = "";
	GLboolean bErrorStatus = GL_TRUE;
	GLuint gGlVertexShaderObject;
	GLuint gGlFragmentShaderObject;

	// define vertex shader object
	gGlVertexShaderObject = glCreateShader(GL_VERTEX_SHADER);

	// write vertex shader code
	const GLchar *pszVertexShaderSourceCode =
		"#version 450 core" \
		"\n" \
		"in vec4 vPosition;" \
		"in vec3 vNormal;" \
		"\n" \
		"uniform int u_l_key_pressed;" \
		"uniform int u_sphereNumber;" \
		"uniform mat4 u_m_matrix;" \
		"uniform mat4 u_v_matrix;" \
		"uniform mat4 u_p_matrix;" \
		"uniform vec4 u_light_position;" \
		"\n" \
		"out vec3 transposeNormal;" \
		"out vec3 source_light_direction;" \
		"out vec3 viewerVector;" \
		"\n" \
		"void main(void)" \
		"{" \
		"\n" \
		"if(u_l_key_pressed == 1)" \
		"{" \
		"\n" \
		"vec4 eyeCoordinates = u_v_matrix * u_m_matrix * vPosition;" \
		"\n" \
		/*"mat3 normalMatrix = mat3(transpose(inverse(u_mv_matrix)));" \*/
		/*"mat3 normalMatrix = mat3(u_mv_matrix);" \*/
		"transposeNormal = mat3(u_v_matrix * u_m_matrix) * vNormal;" \
		"\n" \
		/*"vec3 source_light_direction = vec3(u_light_position.x - eyeCoordinates.x, u_light_position.y - eyeCoordinates.y, u_light_position.z - eyeCoordinates.z);" \*/
		"source_light_direction = vec3(u_light_position - eyeCoordinates);" \
		"\n" \
		"viewerVector = vec3(-eyeCoordinates.xyz);" \
		"\n" \
		"}" \
		"gl_Position = u_p_matrix * u_v_matrix * u_m_matrix * vPosition;" \
		/*"out_Color = vColor;" \*/
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
		"uniform vec3 u_la;" \
		"uniform vec3 u_ka;" \
		"\n" \
		"uniform vec3 u_ld;" \
		"uniform vec3 u_kd;" \
		"\n" \
		"uniform vec3 u_ls;" \
		"uniform vec3 u_ks;" \
		"uniform float u_materialShininess;" \
		"\n" \
		"uniform int u_l_key_pressed;" \
		"uniform int u_sphereNumber;" \
		"\n" \
		"vec3 out_fong_ads_light;" \
		"in vec3 transposeNormal;" \
		"in vec3 source_light_direction;" \
		"in vec3 viewerVector;" \
		"out vec4 fragColor;" \
		"void main(void)" \
		"{"	\
			"if(u_l_key_pressed == 1)" \
			"{" \
				"vec3 normTransposeNormal = normalize(transposeNormal);" \
				"vec3 normSource_light_direction = normalize(source_light_direction);" \
				"vec3 normviewerVector = normalize(viewerVector);" \

				"float transposeNormalDotLightDir = max(dot(normSource_light_direction, normTransposeNormal), 0.0);" \
				"\n" \
				"vec3 reflectionVector = reflect(-normSource_light_direction, normTransposeNormal);" \
				"vec3 ambient = u_la * u_ka;" \
				"\n" \
				"vec3 diffuse = u_ld * u_kd * transposeNormalDotLightDir;" \
				"\n" \
				"vec3 specular = u_ls * u_ks * pow(max(dot(reflectionVector, normviewerVector), 0.0), u_materialShininess);" \
				"\n" \
				"out_fong_ads_light = ambient + diffuse + specular;" \
				"fragColor = vec4(out_fong_ads_light, 1.0);" \
			"}" \
			"else" \
			"{" \
				"fragColor = vec4(1.0, 1.0, 1.0, 1.0);" \
			"}" \
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
	gGlShaderProgramObjectF = glCreateProgram();

	// attach vertex shader to shader program
	glAttachShader(gGlShaderProgramObjectF, gGlVertexShaderObject);

	// attach fragment shader to shader program
	glAttachShader(gGlShaderProgramObjectF, gGlFragmentShaderObject);

	// prelinking binding to vertex attributes
	glBindAttribLocation(gGlShaderProgramObjectF, AMC_ATTRIBUTE_POSITION, "vPosition");
	glBindAttribLocation(gGlShaderProgramObjectF, AMC_ATTRIBUTE_NORMAL, "vNormal");

	// link the shader program
	glLinkProgram(gGlShaderProgramObjectF);

	// Error code checking function by AG
	memset(szglErrorStr, 0, ERROR_SIZE);
	strcpy_s(szglErrorStr, ERROR_SIZE, "Error in program object : ");
	bErrorStatus = GetGlErrorStatus(enGlGET_Program, gGlShaderProgramObjectF, szglErrorStr);
	if (GL_FALSE == bErrorStatus)
	{
		fprintf(gpLogFile, "%s.\n", szglErrorStr);
		return -5;
	}

	// post linking retreving uniform location
	
	char str[255];
	sphereNumberUninform = glGetUniformLocation(gGlShaderProgramObjectF, "u_sphereNumber");
	//for (int iItr = 0; iItr < 24; iItr++)
	int iItr = 0;
	{
		memset(str, 0, 255);
		sprintf_s(str, "u_v_matrix[%d]", iItr);
		gLightMaterial[iItr].viewUniform = glGetUniformLocation(gGlShaderProgramObjectF, "u_v_matrix");

		memset(str, 0, 255);
		sprintf_s(str, "u_m_matrix[%d]", iItr);
		gLightMaterial[iItr].modelUniform = glGetUniformLocation(gGlShaderProgramObjectF, "u_m_matrix");
		
		memset(str, 0, 255);
		sprintf_s(str, "u_p_matrix[%d]", iItr);
		gLightMaterial[iItr].projectionUniform = glGetUniformLocation(gGlShaderProgramObjectF, "u_p_matrix");

		memset(str, 0, 255);
		sprintf_s(str, "u_l_key_pressed[%d]", iItr);
		gLightMaterial[iItr].lKeyPressedUniform = glGetUniformLocation(gGlShaderProgramObjectF, "u_l_key_pressed");

		memset(str, 0, 255);
		sprintf_s(str, "u_la[%d]", iItr);
		gLightMaterial[iItr].laUniform = glGetUniformLocation(gGlShaderProgramObjectF, "u_la");

		memset(str, 0, 255);
		sprintf_s(str, "u_ka[%d]", iItr);
		gLightMaterial[iItr].kaUniform = glGetUniformLocation(gGlShaderProgramObjectF, "u_ka");

		memset(str, 0, 255);
		sprintf_s(str, "u_ld[%d]", iItr);
		gLightMaterial[iItr].ldUniform = glGetUniformLocation(gGlShaderProgramObjectF, "u_ld");

		memset(str, 0, 255);
		sprintf_s(str, "u_kd[%d]", iItr);
		gLightMaterial[iItr].kdUniform = glGetUniformLocation(gGlShaderProgramObjectF, "u_kd");

		memset(str, 0, 255);
		sprintf_s(str, "u_ls[%d]", iItr);
		gLightMaterial[iItr].lsUniform = glGetUniformLocation(gGlShaderProgramObjectF, "u_ls");

		memset(str, 0, 255);
		sprintf_s(str, "u_ks[%d]", iItr);
		gLightMaterial[iItr].ksUniform = glGetUniformLocation(gGlShaderProgramObjectF, "u_ks");

		memset(str, 0, 255);
		sprintf_s(str, "u_light_position[%d]", iItr);
		gLightMaterial[iItr].lightPositionUniform = glGetUniformLocation(gGlShaderProgramObjectF, "u_light_position");

		memset(str, 0, 255);
		sprintf_s(str, "u_materialShininess");
		gLightMaterial[iItr].materialShininessUniform = glGetUniformLocation(gGlShaderProgramObjectF, str);
	}

	return 0;
}

int PerVerLight()
{
	char szglErrorStr[ERROR_SIZE] = "";
	GLboolean bErrorStatus = GL_TRUE;
	GLuint gGlVertexShaderObject;
	GLuint gGlFragmentShaderObject;

	// define vertex shader object
	gGlVertexShaderObject = glCreateShader(GL_VERTEX_SHADER);

	// write vertex shader code
	const GLchar *pszVertexShaderSourceCode =
		"#version 450 core" \
		"\n" \
		"in vec4 vPosition;" \
		"in vec3 vNormal;" \
		"\n" \
		"uniform mat4 u_m_matrixV[24];" \
		"uniform mat4 u_v_matrixV[24];" \
		"uniform mat4 u_p_matrixV[24];" \
		"\n" \
		"uniform vec3 u_laV[24];" \
		"uniform vec3 u_kaV[24];" \
		"\n" \
		"uniform vec3 u_ldV[24];" \
		"uniform vec3 u_kdV[24];" \
		"\n" \
		"uniform vec3 u_lsV[24];" \
		"uniform vec3 u_ksV[24];" \
		"uniform float u_materialShininessV;" \
		"\n" \
		"uniform int u_l_key_pressedV[24];" \
		"uniform vec4 u_light_positionV[24];" \
		"uniform int u_sphereNumberV;" \
		"\n" \
		"out vec3 out_fong_ads_lightV;" \
		"\n" \
		"void main(void)" \
		"{" \
		"\n" \
		"if(u_l_key_pressedV[u_sphereNumberV] == 1)" \
		"{" \
		"\n" \
		"vec4 eyeCoordinates = u_v_matrixV[u_sphereNumberV] * u_m_matrixV[u_sphereNumberV] * vPosition;" \
		"\n" \
		/*"mat3 normalMatrix = mat3(transpose(inverse(u_mv_matrix)));" \*/
		/*"mat3 normalMatrix = mat3(u_mv_matrix);" \*/
		"vec3 transposeNormal = normalize(mat3(u_v_matrixV[u_sphereNumberV] * u_m_matrixV[u_sphereNumberV]) * vNormal);" \
		"\n" \
		/*"vec3 source_light_direction = vec3(u_light_position.x - eyeCoordinates.x, u_light_position.y - eyeCoordinates.y, u_light_position.z - eyeCoordinates.z);" \*/
		"vec3 source_light_direction = normalize(vec3(u_light_positionV[u_sphereNumberV] - eyeCoordinates));" \
		"\n" \
		"float transposeNormalDotLightDir = max(dot(source_light_direction, transposeNormal), 0.0);" \
		"\n" \
		"vec3 reflectionVector = reflect(-source_light_direction, transposeNormal);" \
		"\n" \
		"vec3 viewerVector = normalize(vec3(-eyeCoordinates.xyz));" \
		"\n" \
		"vec3 ambient = u_laV[u_sphereNumberV] * u_kaV[u_sphereNumberV];" \
		"\n" \
		"vec3 diffuse = u_ldV[u_sphereNumberV] * u_kdV[u_sphereNumberV] * transposeNormalDotLightDir;" \
		"\n" \
		"vec3 specular = u_lsV[u_sphereNumberV] * u_ksV[u_sphereNumberV] * pow(max(dot(reflectionVector, viewerVector), 0.0), u_materialShininessV);" \
		"\n" \
		"out_fong_ads_lightV = ambient + diffuse + specular;" \
		"if(out_fong_ads_lightV == vec3(0.0, 0.0, 0.0)){out_fong_ads_lightV=vec3(1.0, 0.0, 0.0);}"
		"\n" \
		"}" \
		"else" \
		"{" \
		"out_fong_ads_lightV = vec3(1.0, 1.0, 1.0);" \
		"}" \
		"gl_Position = u_p_matrixV[u_sphereNumberV] * u_v_matrixV[u_sphereNumberV] * u_m_matrixV[u_sphereNumberV] * vPosition;" \
		/*"out_Color = vColor;" \*/
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
		"in vec3 out_fong_ads_lightV;" \
		"out vec4 fragColor;" \
		"void main(void)" \
		"{"	\
			"fragColor = vec4(out_fong_ads_lightV, 1.0);" \
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
	gGlShaderProgramObjectV = glCreateProgram();

	// attach vertex shader to shader program
	glAttachShader(gGlShaderProgramObjectV, gGlVertexShaderObject);

	// attach fragment shader to shader program
	glAttachShader(gGlShaderProgramObjectV, gGlFragmentShaderObject);

	// prelinking binding to vertex attributes
	glBindAttribLocation(gGlShaderProgramObjectV, AMC_ATTRIBUTE_POSITION, "vPosition");
	glBindAttribLocation(gGlShaderProgramObjectV, AMC_ATTRIBUTE_NORMAL, "vNormal");

	// link the shader program
	glLinkProgram(gGlShaderProgramObjectV);

	// Error code checking function by AG
	memset(szglErrorStr, 0, ERROR_SIZE);
	strcpy_s(szglErrorStr, ERROR_SIZE, "Error in program object : ");
	bErrorStatus = GetGlErrorStatus(enGlGET_Program, gGlShaderProgramObjectV, szglErrorStr);
	if (GL_FALSE == bErrorStatus)
	{
		fprintf(gpLogFile, "%s.\n", szglErrorStr);
		return -5;
	}

	// post linking retreving uniform location
	sphereNumberUninformV = glGetUniformLocation(gGlShaderProgramObjectV, "u_sphereNumberV");
	char str[255];

	for (int iItr = 0; iItr < 24; iItr++)
	{
		memset(str, 0, 255);
		sprintf_s(str, "u_v_matrixV[%d]",iItr);
		gLightMaterial[iItr].viewUniformV = glGetUniformLocation(gGlShaderProgramObjectV, str);

		memset(str, 0, 255);
		sprintf_s(str, "u_m_matrixV[%d]", iItr);
		gLightMaterial[iItr].modelUniformV = glGetUniformLocation(gGlShaderProgramObjectV, str);

		memset(str, 0, 255);
		sprintf_s(str, "u_p_matrixV[%d]", iItr);
		gLightMaterial[iItr].projectionUniformV = glGetUniformLocation(gGlShaderProgramObjectV, str);

		memset(str, 0, 255);
		sprintf_s(str, "u_l_key_pressedV[%d]", iItr);
		gLightMaterial[iItr].lKeyPressedUniformV = glGetUniformLocation(gGlShaderProgramObjectV, str);

		memset(str, 0, 255);
		sprintf_s(str, "u_laV[%d]", iItr);
		gLightMaterial[iItr].laUniformV = glGetUniformLocation(gGlShaderProgramObjectV, str);

		memset(str, 0, 255);
		sprintf_s(str, "u_kaV[%d]", iItr);
		gLightMaterial[iItr].kaUniformV = glGetUniformLocation(gGlShaderProgramObjectV, str);

		memset(str, 0, 255);
		sprintf_s(str, "u_ldV[%d]", iItr);
		gLightMaterial[iItr].ldUniformV = glGetUniformLocation(gGlShaderProgramObjectV, str);

		memset(str, 0, 255);
		sprintf_s(str, "u_kdV[%d]", iItr);
		gLightMaterial[iItr].kdUniformV = glGetUniformLocation(gGlShaderProgramObjectV, str);

		memset(str, 0, 255);
		sprintf_s(str, "u_lsV[%d]", iItr);
		gLightMaterial[iItr].lsUniformV = glGetUniformLocation(gGlShaderProgramObjectV, str);

		memset(str, 0, 255);
		sprintf_s(str, "u_ksV[%d]", iItr);
		gLightMaterial[iItr].ksUniformV = glGetUniformLocation(gGlShaderProgramObjectV, str);

		memset(str, 0, 255);
		sprintf_s(str, "u_light_positionV[%d]", iItr);
		gLightMaterial[iItr].lightPositionUniformV = glGetUniformLocation(gGlShaderProgramObjectV, str);

		memset(str, 0, 255);
		sprintf_s(str, "u_materialShininessV");
		gLightMaterial[iItr].materialShininessUniformV = glGetUniformLocation(gGlShaderProgramObjectV, str);
	}
	
	return 0;
}

