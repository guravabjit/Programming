#include <Windows.h>
#include <GL/glew.h>
#include <GL/GL.h>
#include <stdio.h> // FILE
#include "vmath.h"
#include "Sphere.h"
#include "SolarSystem.h"

#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib,"Sphere.lib")

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
float angleRot = 0.0f;
FILE *gpLogFile = NULL;

DWORD gdwStyle = 0;
WINDOWPLACEMENT gwpPrev = { sizeof(WINDOWPLACEMENT) };

struct nodeStackData
{
	mat4 modelMatrix;
	mat4 viewMatrix;
	mat4 projectionMatrix;
};

int iPushIndex = 0;
struct nodeStackData modelViewProjectionStack[32];

void pushInStack(struct nodeStackData*);
void popFromStack(struct nodeStackData*);

float angleRotDay;
float angleRotYear;
float angleRotMoon;

GLenum gGlEnumResult;
GLuint gGlShaderProgramObjectV;
GLuint gGlShaderProgramObjectF;

GLuint vaoSphere;
GLuint vboSpherePosition;
GLuint vboTexture;
GLuint vboSphereNormal;
GLuint vboSphereElements;

GLuint viewUniform;
GLuint modelUniform;
GLuint projectionUniform;
mat4 perspectiveProjectionMatrix;

GLuint viewUniformV;
GLuint modelUniformV;
GLuint projectionUniformV;

float sphere_vertices[1146];
float sphere_normals[1146];
float sphere_textures[764];
unsigned short sphere_elements[2280];

int gNumVertices;
int gNumElements;
GLuint lKeyPressedUniform;
GLuint lKeyPressedUniformV;

GLuint laUniform;
GLuint kaUniform;

GLuint ldUniform;
GLuint kdUniform;

GLuint lsUniform;
GLuint ksUniform;
GLuint materialShininessUniform;

GLuint lightPositionUniform;



GLuint laUniformV;
GLuint kaUniformV;

GLuint ldUniformV;
GLuint kdUniformV;

GLuint lsUniformV;
GLuint ksUniformV;
GLuint materialShininessUniformV;
GLuint u_sampler_uniformV;

GLuint lightPositionUniformV;

BOOL gbLighting = FALSE;
BOOL gbPerVer = TRUE;
BOOL gbPerFrag = FALSE;

float gLightAmbient[] = { 0.0f, 0.0f, 0.0f, 0.0f };
float gLightDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
float gLightSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
float gLightPosition[] = { 100.0f, 100.0f, 100.0f, 1.0f };

float gMaterialAmbient[] = { 0.0f, 0.0f, 0.0f, 0.0f };
float gMaterialDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
float gMaterialSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
float gMaterialShininess = 128.0f;//50.0f; // 128.0f

int giYear = 0;
int giDay = 0;

GLuint gTextureSun;
GLuint gTextureEarth;
GLuint gTextureMoon;

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
		case 'Q':
		case 'q':
			DestroyWindow(hWnd);
			break;

		case 'Y':
			giYear += 3;
			break;

		case 'y':
			giYear -= 3;
			break;

		case 'D':
			giDay += 3;
			break;

		case 'd':
			giDay -= 3;
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
		case 'V':
		case 'v':
			if (gbPerVer == TRUE)
			{
				gbPerVer = FALSE;
				gbPerFrag = TRUE;
			}
			else
			{
				gbPerVer = TRUE;
				gbPerFrag = FALSE;
			}
			break;

		case 'F':
		case 'f':
			if (gbPerFrag == TRUE)
			{
				gbPerFrag = FALSE;
				gbPerVer = TRUE;
			}
			else
			{
				gbPerFrag = TRUE;
				gbPerVer = FALSE;
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
	int PerFragLight();
	int PerVerLight();

	void resize(int, int);
	char szglErrorStr[ERROR_SIZE] = "";
	GLboolean bErrorStatus = GL_TRUE;

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

	if (0 != PerVerLight())
	{
		return -5;
	}

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

	glGenBuffers(1, &vboTexture);
	glBindBuffer(GL_ARRAY_BUFFER, vboTexture);
	glBufferData(GL_ARRAY_BUFFER, sizeof(sphere_textures), sphere_textures, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_TEXCORD0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_TEXCORD0);
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

	glEnable(GL_TEXTURE_2D);
	LoadTexture(&gTextureSun, MAKEINTRESOURCE(ID_BITMAP_SUN));
	LoadTexture(&gTextureEarth, MAKEINTRESOURCE(ID_BITMAP_EARTH));
	LoadTexture(&gTextureMoon, MAKEINTRESOURCE(ID_BITMAP_MOON));


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

	void DrawSun(GLfloat x, GLfloat z, GLfloat s);
	void DrawEarth(GLfloat x, GLfloat z, GLfloat s);
	void DrawMoon(GLfloat x, GLfloat z, GLfloat s);


	mat4 viewMatrix = mat4::identity();
	mat4 modelMatrix = mat4::identity();
	mat4 projectionMatrix = mat4::identity();

	modelMatrix = translate(0.0f, 0.0f, -6.0f);
	projectionMatrix = perspectiveProjectionMatrix;

	if (gbPerVer)
	{
		//PerVerLight();
		glUseProgram(gGlShaderProgramObjectV);
	}

	if (gbPerFrag)
	{
		//PerFragLight();
		glUseProgram(gGlShaderProgramObjectF);
	}

	glUniformMatrix4fv(viewUniformV, 1, GL_FALSE, viewMatrix);
	glUniformMatrix4fv(modelUniformV, 1, GL_FALSE, modelMatrix);
	glUniformMatrix4fv(projectionUniformV, 1, GL_FALSE, projectionMatrix);

	if (gbLighting == TRUE)
	{
		if (gbPerVer)
		{
			glUniform1i(lKeyPressedUniformV, 1);

			glUniform3fv(laUniformV, 1, gLightAmbient);
			glUniform3fv(kaUniformV, 1, gMaterialAmbient);

			glUniform3fv(ldUniformV, 1, gLightDiffuse); // white color / light color
			glUniform3fv(kdUniformV, 1, gMaterialDiffuse); // gray color / material color

			glUniform3fv(lsUniformV, 1, gLightSpecular);
			glUniform3fv(ksUniformV, 1, gMaterialSpecular);

			glUniform4fv(lightPositionUniformV, 1, gLightPosition); // light position

			glUniform1f(materialShininessUniformV, gMaterialShininess);
		}
		if (gbPerFrag)
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
	}
	else
	{
		if (gbPerVer)
		{
			glUniform1i(lKeyPressedUniformV, 0);
		}
		if (gbPerFrag)
		{
			glUniform1i(lKeyPressedUniform, 0);
		}
	}

	// *** bind vao ***
	glBindVertexArray(vaoSphere);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTextureSun);
	glUniform1i(u_sampler_uniformV, 0);

	// *** draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);

	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);

	// *** unbind vao ***
	glBindVertexArray(0);

	if (gbLighting == TRUE)
	{
		if (gbPerVer)
		{
			glUniform1i(lKeyPressedUniformV, 1);

			glUniform3fv(laUniformV, 1, gLightAmbient);
			glUniform3fv(kaUniformV, 1, gMaterialAmbient);

			glUniform3fv(ldUniformV, 1, gLightDiffuse); // white color / light color
			glUniform3fv(kdUniformV, 1, gMaterialDiffuse); // gray color / material color

			glUniform3fv(lsUniformV, 1, gLightSpecular);
			glUniform3fv(ksUniformV, 1, gMaterialSpecular);

			glUniform4fv(lightPositionUniformV, 1, gLightPosition); // light position

			glUniform1f(materialShininessUniformV, gMaterialShininess);
		}
		if (gbPerFrag)
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
	}
	else
	{
		if (gbPerVer)
		{
			glUniform1i(lKeyPressedUniformV, 0);
		}
		if (gbPerFrag)
		{
			glUniform1i(lKeyPressedUniform, 0);
		}
	}

	modelMatrix *= rotate((GLfloat)giYear, 0.0f, 1.0f, 0.0f);
	modelMatrix *= translate(2.0f, 0.0f, 0.0f);
	modelMatrix *= rotate((GLfloat)giDay, 0.0f, 1.0f, 0.0f);
	modelMatrix *= scale(0.5f, 0.5f, 0.5f);


	glUniformMatrix4fv(viewUniformV, 1, GL_FALSE, viewMatrix);
	glUniformMatrix4fv(modelUniformV, 1, GL_FALSE, modelMatrix);
	glUniformMatrix4fv(projectionUniformV, 1, GL_FALSE, projectionMatrix);

	// *** bind vao ***
	glBindVertexArray(vaoSphere);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTextureEarth);
	glUniform1i(u_sampler_uniformV, 0);

	// *** draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);

	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);

	// *** unbind vao ***
	glBindVertexArray(0);


	if (gbLighting == TRUE)
	{
		if (gbPerVer)
		{
			glUniform1i(lKeyPressedUniformV, 1);

			glUniform3fv(laUniformV, 1, gLightAmbient);
			glUniform3fv(kaUniformV, 1, gMaterialAmbient);

			glUniform3fv(ldUniformV, 1, gLightDiffuse); // white color / light color
			glUniform3fv(kdUniformV, 1, gMaterialDiffuse); // gray color / material color

			glUniform3fv(lsUniformV, 1, gLightSpecular);
			glUniform3fv(ksUniformV, 1, gMaterialSpecular);

			glUniform4fv(lightPositionUniformV, 1, gLightPosition); // light position

			glUniform1f(materialShininessUniformV, gMaterialShininess);
		}
		if (gbPerFrag)
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
	}
	else
	{
		if (gbPerVer)
		{
			glUniform1i(lKeyPressedUniformV, 0);
		}
		if (gbPerFrag)
		{
			glUniform1i(lKeyPressedUniform, 0);
		}
	}

	modelMatrix *= rotate((GLfloat)giYear, 0.0f, 1.0f, 0.0f);
	modelMatrix *= translate(1.0f, 0.0f, 0.0f);
	modelMatrix *= rotate((GLfloat)giDay, 0.0f, 1.0f, 0.0f);
	modelMatrix *= scale(0.25f, 0.25f, 0.25f);


	glUniformMatrix4fv(viewUniformV, 1, GL_FALSE, viewMatrix);
	glUniformMatrix4fv(modelUniformV, 1, GL_FALSE, modelMatrix);
	glUniformMatrix4fv(projectionUniformV, 1, GL_FALSE, projectionMatrix);


	// *** bind vao ***
	glBindVertexArray(vaoSphere);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTextureMoon);
	glUniform1i(u_sampler_uniformV, 0);

	// *** draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);

	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);

	// *** unbind vao ***
	glBindVertexArray(0);


	//DrawSun(0.0f, -3.0f, 0.5f);
	//DrawEarth(1.0f, -3.0f, 0.2f);
	//DrawMoon(1.2f, -3.0f, 0.1f);

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
	angleRot += 0.01f;
	if (angleRot > 360.0f)
	{
		angleRot = 0.0f;
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
	viewUniform = glGetUniformLocation(gGlShaderProgramObjectF, "u_v_matrix");
	modelUniform = glGetUniformLocation(gGlShaderProgramObjectF, "u_m_matrix");
	projectionUniform = glGetUniformLocation(gGlShaderProgramObjectF, "u_p_matrix");

	lKeyPressedUniform = glGetUniformLocation(gGlShaderProgramObjectF, "u_l_key_pressed");

	laUniform = glGetUniformLocation(gGlShaderProgramObjectF, "u_la");
	kaUniform = glGetUniformLocation(gGlShaderProgramObjectF, "u_ka");

	ldUniform = glGetUniformLocation(gGlShaderProgramObjectF, "u_ld");
	kdUniform = glGetUniformLocation(gGlShaderProgramObjectF, "u_kd");

	lsUniform = glGetUniformLocation(gGlShaderProgramObjectF, "u_ls");
	ksUniform = glGetUniformLocation(gGlShaderProgramObjectF, "u_ks");

	lightPositionUniform = glGetUniformLocation(gGlShaderProgramObjectF, "u_light_position");

	materialShininessUniform = glGetUniformLocation(gGlShaderProgramObjectF, "u_materialShininess");

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
		"in vec2 vTexCord;" \
		"\n" \
		"uniform mat4 u_m_matrixV;" \
		"uniform mat4 u_v_matrixV;" \
		"uniform mat4 u_p_matrixV;" \
		"\n" \
		"uniform vec3 u_laV;" \
		"uniform vec3 u_kaV;" \
		"\n" \
		"uniform vec3 u_ldV;" \
		"uniform vec3 u_kdV;" \
		"\n" \
		"uniform vec3 u_lsV;" \
		"uniform vec3 u_ksV;" \
		"uniform float u_materialShininessV;" \
		"\n" \
		"uniform int u_l_key_pressedV;" \
		"uniform vec4 u_light_positionV;" \
		"\n" \
		"out vec3 out_fong_ads_lightV;" \
		"out vec2 out_TexCord;" \
		"\n" \
		"void main(void)" \
		"{" \
		"\n" \
		"if(u_l_key_pressedV == 1)" \
		"{" \
		"\n" \
		"vec4 eyeCoordinates = u_v_matrixV * u_m_matrixV * vPosition;" \
		"\n" \
		/*"mat3 normalMatrix = mat3(transpose(inverse(u_mv_matrix)));" \*/
		/*"mat3 normalMatrix = mat3(u_mv_matrix);" \*/
		"vec3 transposeNormal = normalize(mat3(u_v_matrixV * u_m_matrixV) * vNormal);" \
		"\n" \
		/*"vec3 source_light_direction = vec3(u_light_position.x - eyeCoordinates.x, u_light_position.y - eyeCoordinates.y, u_light_position.z - eyeCoordinates.z);" \*/
		"vec3 source_light_direction = normalize(vec3(u_light_positionV - eyeCoordinates));" \
		"\n" \
		"float transposeNormalDotLightDir = max(dot(source_light_direction, transposeNormal), 0.0);" \
		"\n" \
		"vec3 reflectionVector = reflect(-source_light_direction, transposeNormal);" \
		"\n" \
		"vec3 viewerVector = normalize(vec3(-eyeCoordinates.xyz));" \
		"\n" \
		"vec3 ambient = u_laV * u_kaV;" \
		"\n" \
		"vec3 diffuse = u_ldV * u_kdV * transposeNormalDotLightDir;" \
		"\n" \
		"vec3 specular = u_lsV * u_ksV * pow(max(dot(reflectionVector, viewerVector), 0.0), u_materialShininessV);" \
		"\n" \
		"out_fong_ads_lightV = ambient + diffuse + specular;" \
		"\n" \
		"}" \
		"else" \
		"{" \
		"out_fong_ads_lightV = vec3(1.0, 1.0, 1.0);" \
		"}" \
		"gl_Position = u_p_matrixV * u_v_matrixV * u_m_matrixV * vPosition;" \
		"out_TexCord = vTexCord;" \
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
		"uniform sampler2D u_sampler;" \
		"in vec2 out_TexCord;" \
		"void main(void)" \
		"{"	\
			"fragColor = texture(u_sampler, out_TexCord) * vec4(out_fong_ads_lightV, 1.0);" \
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
	glBindAttribLocation(gGlShaderProgramObjectV, AMC_ATTRIBUTE_TEXCORD0, "vTexCord");
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
	viewUniformV = glGetUniformLocation(gGlShaderProgramObjectV, "u_v_matrixV");
	modelUniformV = glGetUniformLocation(gGlShaderProgramObjectV, "u_m_matrixV");
	projectionUniformV = glGetUniformLocation(gGlShaderProgramObjectV, "u_p_matrixV");

	lKeyPressedUniformV = glGetUniformLocation(gGlShaderProgramObjectV, "u_l_key_pressedV");

	laUniformV = glGetUniformLocation(gGlShaderProgramObjectV, "u_laV");
	kaUniformV = glGetUniformLocation(gGlShaderProgramObjectV, "u_kaV");

	ldUniformV = glGetUniformLocation(gGlShaderProgramObjectV, "u_ldV");
	kdUniformV = glGetUniformLocation(gGlShaderProgramObjectV, "u_kdV");

	lsUniformV = glGetUniformLocation(gGlShaderProgramObjectV, "u_lsV");
	ksUniformV = glGetUniformLocation(gGlShaderProgramObjectV, "u_ksV");

	lightPositionUniformV = glGetUniformLocation(gGlShaderProgramObjectV, "u_light_positionV");

	materialShininessUniformV = glGetUniformLocation(gGlShaderProgramObjectV, "u_materialShininessV");

	u_sampler_uniformV = glGetUniformLocation(gGlShaderProgramObjectV, "u_sampler");

	return 0;
}

void DrawSun(GLfloat x, GLfloat z, GLfloat s)
{
	
	// declaration of metrices
	mat4 modelMatrix;
	mat4 viewMatrix;
	mat4 projectionMatrix;
	mat4 translationMatrix;
	mat4 rotationMatrix;
	mat4 modelViewProjectionMatrix;
	mat4 scaleMatrix;

	// initializing above metrices to identity
	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	modelViewProjectionMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	rotationMatrix = mat4::identity();
	projectionMatrix = mat4::identity();
	scaleMatrix = mat4::identity();

	struct nodeStackData tempStackData;


	// do neccessry transformation, rotation, scaling
	//translationMatrix = translate(x, 0.0f, z);

	
	rotationMatrix = rotate(90.0f, 1.0f, 0.0f, 0.0f);
	tempStackData.modelMatrix = rotationMatrix;
	tempStackData.viewMatrix = viewMatrix;
	tempStackData.projectionMatrix = projectionMatrix;

	pushInStack(&tempStackData);

	//scaleMatrix = scale(s, s, s);

	// do necessary matrix multiplication
	/*modelViewProjectionMatrix = modelViewProjectionMatrix * perspectiveProjectionMatrix;
	modelViewProjectionMatrix = modelViewProjectionMatrix * modelViewMatrix;
	modelViewProjectionMatrix = modelViewProjectionMatrix * translationMatrix;
	modelViewProjectionMatrix = modelViewProjectionMatrix * rotationMatrix;*/
	modelMatrix = (translationMatrix * scaleMatrix) * rotationMatrix;
	projectionMatrix = perspectiveProjectionMatrix;

	// send necessary metrices to shader in respective uniforms

	glUniformMatrix4fv(viewUniformV, 1, GL_FALSE, viewMatrix);
	glUniformMatrix4fv(modelUniformV, 1, GL_FALSE, modelMatrix);
	glUniformMatrix4fv(projectionUniformV, 1, GL_FALSE, projectionMatrix);

	// *** bind vao ***
	glBindVertexArray(vaoSphere);

	// *** draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);

	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);

	// *** unbind vao ***
	glBindVertexArray(0);


	popFromStack(&tempStackData);

	modelMatrix = tempStackData.modelMatrix;
	viewMatrix = tempStackData.viewMatrix;
	projectionMatrix = tempStackData.projectionMatrix;


	x = 1.0f;
	z = -3.0f;
	s = 0.2f;

	
	// do neccessry transformation, rotation, scaling
	rotationMatrix = rotate(angleRotYear, 0.0f, 1.0f, 0.0f);
	tempStackData.modelMatrix = rotationMatrix;
	tempStackData.viewMatrix = viewMatrix;
	tempStackData.projectionMatrix = projectionMatrix;
	pushInStack(&tempStackData);

	translationMatrix = translate(x, 0.0f, z);
	modelMatrix = translationMatrix * rotationMatrix;
	
	rotationMatrix *= rotate(90.0f, 1.0f, 0.0f, 0.0f);
	rotationMatrix *= rotate(angleRotDay, 1.0f, 0.0f, 0.0f);

	//scaleMatrix = scale(s, s, s);

	// *** bind vao ***
	glBindVertexArray(vaoSphere);

	// *** draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);

	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);

	// *** unbind vao ***
	glBindVertexArray(0);

	popFromStack(&tempStackData);

	modelMatrix = tempStackData.modelMatrix;
	viewMatrix = tempStackData.viewMatrix;
	projectionMatrix = tempStackData.projectionMatrix;

	tempStackData.modelMatrix = translationMatrix;
	tempStackData.viewMatrix = viewMatrix;
	tempStackData.projectionMatrix = projectionMatrix;
	pushInStack(&tempStackData);

	// do neccessry transformation, rotation, scaling
	rotationMatrix = rotate(angleRotMoon, 0.0f, 1.0f, 0.0f);
	translationMatrix = translate(x, 0.0f, z);
	rotationMatrix *= rotate(90.0f, 1.0f, 0.0f, 0.0f);
	rotationMatrix *= rotate(angleRotDay, 1.0f, 0.0f, 0.0f);


	// do neccessry transformation, rotation, scaling
	//scaleMatrix = scale(s, s, s);

	// *** bind vao ***
	glBindVertexArray(vaoSphere);

	// *** draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);

	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);

	// *** unbind vao ***
	glBindVertexArray(0);

	popFromStack(&tempStackData);

	modelMatrix = tempStackData.modelMatrix;
	viewMatrix = tempStackData.viewMatrix;
	projectionMatrix = tempStackData.projectionMatrix;


}

void DrawEarth(GLfloat x, GLfloat z, GLfloat s)
{

	// declaration of metrices
	mat4 modelMatrix;
	mat4 viewMatrix;
	mat4 projectionMatrix;
	mat4 translationMatrix;
	mat4 rotationMatrix;
	mat4 modelViewProjectionMatrix;
	mat4 scaleMatrix;

	// initializing above metrices to identity
	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	modelViewProjectionMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	rotationMatrix = mat4::identity();
	projectionMatrix = mat4::identity();
	scaleMatrix = mat4::identity();

	struct nodeStackData tempStackData;


	tempStackData.modelMatrix = translationMatrix;
	tempStackData.viewMatrix = viewMatrix;
	tempStackData.projectionMatrix = projectionMatrix;
	pushInStack(&tempStackData);

	// do neccessry transformation, rotation, scaling
	rotationMatrix = rotate(angleRotYear, 0.0f, 1.0f, 0.0f);
	translationMatrix = translate(x, 0.0f, z);
	rotationMatrix *= rotate(90.0f, 1.0f, 0.0f, 0.0f);
	rotationMatrix *= rotate(angleRotDay, 1.0f, 0.0f, 0.0f);
	
	scaleMatrix = scale(s, s, s);

	// do necessary matrix multiplication
	/*modelViewProjectionMatrix = modelViewProjectionMatrix * perspectiveProjectionMatrix;
	modelViewProjectionMatrix = modelViewProjectionMatrix * modelViewMatrix;
	modelViewProjectionMatrix = modelViewProjectionMatrix * translationMatrix;
	modelViewProjectionMatrix = modelViewProjectionMatrix * rotationMatrix;*/
	modelMatrix = (translationMatrix * scaleMatrix) * rotationMatrix;
	projectionMatrix = perspectiveProjectionMatrix;

	// send necessary metrices to shader in respective uniforms

	if (gbLighting == TRUE)
	{
		if (gbPerVer)
		{
			glUniformMatrix4fv(viewUniformV, 1, GL_FALSE, viewMatrix);
			glUniformMatrix4fv(modelUniformV, 1, GL_FALSE, modelMatrix);
			glUniformMatrix4fv(projectionUniformV, 1, GL_FALSE, projectionMatrix);

			glUniform1i(lKeyPressedUniformV, 1);

			glUniform3fv(laUniformV, 1, gLightAmbient);
			glUniform3fv(kaUniformV, 1, gMaterialAmbient);

			glUniform3fv(ldUniformV, 1, gLightDiffuse); // white color / light color
			glUniform3fv(kdUniformV, 1, gMaterialDiffuse); // gray color / material color

			glUniform3fv(lsUniformV, 1, gLightSpecular);
			glUniform3fv(ksUniformV, 1, gMaterialSpecular);

			glUniform4fv(lightPositionUniformV, 1, gLightPosition); // light position

			glUniform1f(materialShininessUniformV, gMaterialShininess);
		}
		if (gbPerFrag)
		{
			glUniformMatrix4fv(viewUniform, 1, GL_FALSE, viewMatrix);
			glUniformMatrix4fv(modelUniform, 1, GL_FALSE, modelMatrix);
			glUniformMatrix4fv(projectionUniform, 1, GL_FALSE, projectionMatrix);

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
	}
	else
	{
		if (gbPerVer)
		{
			glUniformMatrix4fv(viewUniformV, 1, GL_FALSE, viewMatrix);
			glUniformMatrix4fv(modelUniformV, 1, GL_FALSE, modelMatrix);
			glUniformMatrix4fv(projectionUniformV, 1, GL_FALSE, projectionMatrix);

			glUniform1i(lKeyPressedUniformV, 0);
		}
		if (gbPerFrag)
		{
			glUniformMatrix4fv(viewUniformV, 1, GL_FALSE, viewMatrix);
			glUniformMatrix4fv(modelUniformV, 1, GL_FALSE, modelMatrix);
			glUniformMatrix4fv(projectionUniformV, 1, GL_FALSE, projectionMatrix);

			glUniform1i(lKeyPressedUniform, 0);
		}
	}

	
	// *** bind vao ***
	glBindVertexArray(vaoSphere);

	// *** draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);

	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);

	// *** unbind vao ***
	glBindVertexArray(0);
	popFromStack(&tempStackData);

	modelMatrix = tempStackData.modelMatrix;
	viewMatrix = tempStackData.viewMatrix;
	projectionMatrix = tempStackData.projectionMatrix;


}

void DrawMoon(GLfloat x, GLfloat z, GLfloat s)
{
	


	// declaration of metrices
	mat4 modelMatrix;
	mat4 viewMatrix;
	mat4 projectionMatrix;
	mat4 translationMatrix;
	mat4 rotationMatrix;
	mat4 modelViewProjectionMatrix;
	mat4 scaleMatrix;

	// initializing above metrices to identity
	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	modelViewProjectionMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	rotationMatrix = mat4::identity();
	projectionMatrix = mat4::identity();
	scaleMatrix = mat4::identity();

	struct nodeStackData tempStackData;


	tempStackData.modelMatrix = translationMatrix;
	tempStackData.viewMatrix = viewMatrix;
	tempStackData.projectionMatrix = projectionMatrix;
	pushInStack(&tempStackData);

	// do neccessry transformation, rotation, scaling
	rotationMatrix = rotate(angleRotMoon, 0.0f, 1.0f, 0.0f);
	translationMatrix = translate(x, 0.0f, z);
	rotationMatrix *= rotate(90.0f, 1.0f, 0.0f, 0.0f);
	rotationMatrix *= rotate(angleRotDay, 1.0f, 0.0f, 0.0f);


	// do neccessry transformation, rotation, scaling
	scaleMatrix = scale(s, s, s);

	// do necessary matrix multiplication
	/*modelViewProjectionMatrix = modelViewProjectionMatrix * perspectiveProjectionMatrix;
	modelViewProjectionMatrix = modelViewProjectionMatrix * modelViewMatrix;
	modelViewProjectionMatrix = modelViewProjectionMatrix * translationMatrix;
	modelViewProjectionMatrix = modelViewProjectionMatrix * rotationMatrix;*/
	modelMatrix = (translationMatrix * scaleMatrix) * rotationMatrix;
	projectionMatrix = perspectiveProjectionMatrix;

	// send necessary metrices to shader in respective uniforms

	if (gbLighting == TRUE)
	{
		if (gbPerVer)
		{
			glUniformMatrix4fv(viewUniformV, 1, GL_FALSE, viewMatrix);
			glUniformMatrix4fv(modelUniformV, 1, GL_FALSE, modelMatrix);
			glUniformMatrix4fv(projectionUniformV, 1, GL_FALSE, projectionMatrix);

			glUniform1i(lKeyPressedUniformV, 1);

			glUniform3fv(laUniformV, 1, gLightAmbient);
			glUniform3fv(kaUniformV, 1, gMaterialAmbient);

			glUniform3fv(ldUniformV, 1, gLightDiffuse); // white color / light color
			glUniform3fv(kdUniformV, 1, gMaterialDiffuse); // gray color / material color

			glUniform3fv(lsUniformV, 1, gLightSpecular);
			glUniform3fv(ksUniformV, 1, gMaterialSpecular);

			glUniform4fv(lightPositionUniformV, 1, gLightPosition); // light position

			glUniform1f(materialShininessUniformV, gMaterialShininess);
		}
		if (gbPerFrag)
		{
			glUniformMatrix4fv(viewUniform, 1, GL_FALSE, viewMatrix);
			glUniformMatrix4fv(modelUniform, 1, GL_FALSE, modelMatrix);
			glUniformMatrix4fv(projectionUniform, 1, GL_FALSE, projectionMatrix);

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
	}
	else
	{
		if (gbPerVer)
		{
			glUniformMatrix4fv(viewUniformV, 1, GL_FALSE, viewMatrix);
			glUniformMatrix4fv(modelUniformV, 1, GL_FALSE, modelMatrix);
			glUniformMatrix4fv(projectionUniformV, 1, GL_FALSE, projectionMatrix);

			glUniform1i(lKeyPressedUniformV, 0);
		}
		if (gbPerFrag)
		{
			glUniformMatrix4fv(viewUniformV, 1, GL_FALSE, viewMatrix);
			glUniformMatrix4fv(modelUniformV, 1, GL_FALSE, modelMatrix);
			glUniformMatrix4fv(projectionUniformV, 1, GL_FALSE, projectionMatrix);

			glUniform1i(lKeyPressedUniform, 0);
		}
	}

	

	// *** bind vao ***
	glBindVertexArray(vaoSphere);

	// *** draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);

	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);

	// *** unbind vao ***
	glBindVertexArray(0);

	popFromStack(&tempStackData);

	modelMatrix = tempStackData.modelMatrix;
	viewMatrix = tempStackData.viewMatrix;
	projectionMatrix = tempStackData.projectionMatrix;


}


void pushInStack(struct nodeStackData* pData)
{
	modelViewProjectionStack[iPushIndex].modelMatrix = pData->modelMatrix;
	modelViewProjectionStack[iPushIndex].projectionMatrix = pData->projectionMatrix;
	modelViewProjectionStack[iPushIndex].viewMatrix = pData->viewMatrix;
	iPushIndex++;
}

void popFromStack(struct nodeStackData* pData)
{
	pData->modelMatrix = modelViewProjectionStack[iPushIndex].modelMatrix;
	pData->projectionMatrix = modelViewProjectionStack[iPushIndex].projectionMatrix;
	pData->viewMatrix = modelViewProjectionStack[iPushIndex].viewMatrix;
	iPushIndex--;
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
