#include <Windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include "DynamicIndia.h"

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "Winmm.lib")

HDC ghDC = NULL;
HGLRC ghGlRc = NULL;
bool gbIsActiveWindow = false;
bool gbIsFullScreen = false;
HWND ghWnd = NULL;
FILE* gpLogFile = NULL;
WINDOWPLACEMENT gwpPrev = { sizeof(WINDOWPLACEMENT) };
DWORD gdwStyle = 0;
GLfloat gfHeight = 0;
GLfloat gfWidth = 0;
GLfloat fRotx = 0.0f;
GLfloat fRoty = 0.0f;
GLfloat fRotz = 0.0f;

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

#define WIN_WIDTH 800
#define WIN_HEIGHT 600
#define NUMBER_OF_VERTICES 5000 * 2
#define PI 3.14f
#define CHAR_HEIGHT 1.5f
#define CHAR_WIDTH 0.05f
#define CHAR_SIZE 0.1f

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
void ToggleFullScreen();

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR szCmdLine, int iCmdShow)
{
	int initialize();
	void display();

	bool bIsDone = false;
	int iRet = 0;
	TCHAR szAppName[] = TEXT("SI");
	WNDCLASSEX wndclass;
	HWND hWnd;
	MSG msg;

	if (fopen_s(&gpLogFile, "Log.txt", "w") != 0)
	{
		MessageBox(NULL, TEXT("Log file creation failed"), TEXT("Error"), MB_OK);
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
		TEXT("Static Indias"),
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

		case 'F':
		case 'f':
			//ToggleFullScreen();
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

	PIXELFORMATDESCRIPTOR pfd;
	int iPixelFormatIndex = 0;

	memset((void*)&pfd, NULL, sizeof(PIXELFORMATDESCRIPTOR));

	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
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

	ghGlRc = wglCreateContext(ghDC);
	if (NULL == ghGlRc)
	{
		return -3;
	}

	if (wglMakeCurrent(ghDC, ghGlRc) == FALSE)
	{
		return -4;
	}

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	resize(WIN_WIDTH, WIN_HEIGHT);

	ToggleFullScreen();

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

	return 0;
}

void resize(int iWidth, int iHeight)
{
	glViewport(0, 0, (GLsizei)iWidth, (GLsizei)iHeight);
	//glViewport(giVPx, giVPy, (GLsizei)iWidth, (GLsizei)iHeight);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	if (0 == iHeight)
	{
		iHeight = 1;
	}

	gluPerspective(45.0f,
		(GLfloat)iWidth / (GLfloat)iHeight,
		0.1f,
		100.0f
	);

}

void display()
{
	void DrawI();
	void DrawN();
	void DrawD();
	void DrawA();
	void DrawPlane();

	glClear(GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	
	if (fTransIX < -5.0f)
	{
		fTransIX += 0.001f;
	}
	
	glLoadIdentity();
	glTranslatef(fTransIX /*-5.0f*/, 0.0f, -12.0f);
	DrawI();
	
	if (fTransIX >= -5.0f)
	{
		if (fTransNY > 0.0f)
		{
			fTransNY -= 0.001f;
		}
	}
	
	glLoadIdentity();
	glTranslatef(-2.4f, fTransNY/*0.0f*/, -12.0f);
	DrawN();

	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, -12.0f);
	DrawD();

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

	glLoadIdentity();
	glTranslatef(2.4f, fTransIY/*0.0f*/, -12.0f);
	DrawI();
	
	if (fTransNY <= 0.0f)
	{
		if (fTransAX >= 4.8f)
		{
			fTransAX -= 0.001f;
		}
	}

	glLoadIdentity();
	glTranslatef(fTransAX/*5.0f*/, 0.0f, -12.0f);
	DrawA();

	if (true == bDrawPlane)
	{
		if (false == bIsPlayed)
		{
			bIsPlayed = true;
			PlaySound(MAKEINTRESOURCE(ID_SONG), GetModuleHandle(NULL), SND_ASYNC | SND_RESOURCE);
		}
		
		fTransPlaneTopX += 0.0004f;

		if(fTransPlaneTopY >= 0.0f)
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

		if(fTransPlaneBottomY <= 0.0f)
		{
			fTransPlaneBottomY += 0.0004f;
		}

		if (fTransPlaneBottomX > 6.0f)
		{
			fTransPlaneBottomY += 0.0004f;
		}

		glLoadIdentity();
		glTranslatef(fTransPlaneBottomX /*-10.0f*/, fTransPlaneBottomY /*-5.0f*/, -12.0f);

		DrawPlane();

		glLoadIdentity();
		glTranslatef(fTransPlaneTopX/*10.0f*/, fTransPlaneTopY/*5.0f*/, -12.0f);

		DrawPlane();


		glLoadIdentity();
		glTranslatef(fTransPlaneMiddleX /*-10.0f*/, fTransPlaneMiddleY/*0.0f*/, -12.0f);

		DrawPlane();
	}

	SwapBuffers(ghDC);
}

void uninitialize()
{
	if (gbIsFullScreen == true)
	{
		ToggleFullScreen();
	}

	if (wglGetCurrentContext() == ghGlRc)
	{
		wglMakeCurrent(NULL, NULL);
		if (ghGlRc)
		{
			wglDeleteContext(ghGlRc);
			ghGlRc = NULL;
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

void DrawI()
{
	/* logic 1
	glLineWidth(2.0f);

	float fFirstX = 2.0f;
	float fFirstY = 1.0f;
	
	glBegin(GL_LINE_LOOP);
	
	fFirstX += (CHAR_SIZE*4);
	glVertex3f(-fFirstX, fFirstY, 0.0f);

	fFirstY -= CHAR_SIZE*2;
	glVertex3f(-fFirstX, fFirstY, 0.0f);

	fFirstX += (CHAR_SIZE * 2.5);
	glVertex3f(-fFirstX, fFirstY, 0.0f);

	fFirstY -= CHAR_HEIGHT;
	glVertex3f(-fFirstX, fFirstY, 0.0f);

	fFirstX -= (CHAR_SIZE*2.5);
	glVertex3f(-fFirstX, fFirstY, 0.0f);

	fFirstY -= CHAR_SIZE*2;
	glVertex3f(-fFirstX, fFirstY, 0.0f);

	fFirstX += CHAR_SIZE*7;
	glVertex3f(-fFirstX, fFirstY, 0.0f);

	fFirstY += CHAR_SIZE*2;
	glVertex3f(-fFirstX, fFirstY, 0.0f);

	fFirstX -= (CHAR_SIZE*2.5);
	glVertex3f(-fFirstX, fFirstY, 0.0f);

	fFirstY += CHAR_HEIGHT;
	glVertex3f(-fFirstX, fFirstY, 0.0f);

	fFirstX += (CHAR_SIZE*2.5);
	glVertex3f(-fFirstX, fFirstY, 0.0f);

	fFirstY += CHAR_SIZE*2;
	glVertex3f(-fFirstX, fFirstY, 0.0f);

	glEnd();*/

	// logic 2
	/*glLineWidth(1.0f);
	
	glBegin(GL_LINE_LOOP);

	glVertex3f(-1.0f, 1.0f, 0.0f);
	glVertex3f(-1.0f, 0.8f, 0.0f);
	glVertex3f(-0.5f, 0.8f, 0.0f);
	glVertex3f(-0.5f, -0.8f, 0.0f);
	glVertex3f(-1.0f, -0.8f, 0.0f);
	glVertex3f(-1.0f, -1.0f, 0.0f);
	glVertex3f(1.0f, -1.0f, 0.0f);
	glVertex3f(1.0f, -0.8f, 0.0f);
	glVertex3f(0.5f, -0.8f, 0.0f);
	glVertex3f(0.5f, 0.8f, 0.0f);
	glVertex3f(1.0f, 0.8f, 0.0f);
	glVertex3f(1.0f, 1.0f, 0.0f);*/

	/*glColor3f(255, 128, 0);
	glVertex3f(-0.5f, 0.5f, 0.0f);
	glVertex3f(-0.5f, 0.4f, 0.0f);
	glVertex3f(-0.2f, 0.4f, 0.0f);
	glVertex3f(-0.2f, -0.5f, 0.0f);
	glVertex3f(-0.5f, -0.5f, 0.0f);
	glVertex3f(-0.5f, -0.6f, 0.0f);
	glVertex3f(0.5f, -0.6f, 0.0f);
	glVertex3f(0.5f, -0.5f, 0.0f);
	glVertex3f(0.2f, -0.5f, 0.0f);
	glVertex3f(0.2f, 0.4f, 0.0f);
	glVertex3f(0.5f, 0.4f, 0.0f);
	glVertex3f(0.5f, 0.5f, 0.0f);
	

	glEnd();*/

	glColor3f(1.0f, 0.5f, 0);
	glBegin(GL_TRIANGLE_FAN);
	glVertex3f(-1.0f, 1.0f, 0.0f);
	glVertex3f(-1.0f, 0.8f, 0.0f);
	glVertex3f(1.0f, 0.8f, 0.0f);
	glVertex3f(1.0f, 1.0f, 0.0f);
	glEnd();

	
	
	glColor3f(1.0f, 0.5f, 0);
	glBegin(GL_TRIANGLE_FAN);
	glVertex3f(-0.2f, 0.8f, 0.0f);
	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex3f(-0.2f, 0.0f, 0.0f);
	glVertex3f(0.2f, 0.0f, 0.0f);
	glColor3f(1.0f, 0.5f, 0);
	glVertex3f(0.2f, 0.8f, 0.0f);
	glEnd();

	
	glColor3f(1.0f, 1.0f, 1.0f);
	glBegin(GL_TRIANGLE_FAN);
	glVertex3f(-0.2f, 0.0f, 0.0f);
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(-0.2f, -0.8f, 0.0f);
	glVertex3f(0.2f, -0.8f, 0.0f);
	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex3f(0.2f, 0.0f, 0.0f);
	glEnd();

	glColor3f(0.0f, 1.0f, 0.0f);
	glBegin(GL_TRIANGLE_FAN);
	glVertex3f(-1.0f, -0.8f, 0.0f);
	glVertex3f(-1.0f, -1.0f, 0.0f);
	glVertex3f(1.0f, -1.0f, 0.0f);
	glVertex3f(1.0f, -0.8f, 0.0f);
	glEnd();
}

void DrawN()
{
	/*glLineWidth(1.0f);
	glBegin(GL_LINE_LOOP);

	glVertex3f(-1.0f, 1.0f, 0.0f);
	glVertex3f(-1.0f, -1.0f, 0.0f);
	glVertex3f(-0.8f, -1.0f, 0.0f);
	glVertex3f(-0.8f, 0.6f, 0.0f);
	glVertex3f(0.8f, -1.0f, 0.0f);
	glVertex3f(1.0f, -1.0f, 0.0f);
	glVertex3f(1.0f, 1.0f, 0.0f);
	glVertex3f(0.8f, 1.0f, 0.0f);
	glVertex3f(0.8f, -0.6f, 0.0f);
	glVertex3f(-0.8f, 1.0f, 0.0f);

	glEnd();*/

	/*glLineWidth(1.0f);
	glBegin(GL_LINE_LOOP);
	glVertex3f(-0.5f, 0.5f, 0.0f);
	glVertex3f(-0.5f, -0.6f, 0.0f);
	glVertex3f(-0.4f, -0.6f, 0.0f);
	glVertex3f(-0.4f, 0.3f, 0.0f);
	glVertex3f(0.4f, -0.6f, 0.0f);
	glVertex3f(0.5f, -0.6f, 0.0f);
	glVertex3f(0.5f, 0.5f, 0.0f);
	glVertex3f(0.4f, 0.5f, 0.0f);
	glVertex3f(0.4f, -0.4f, 0.0f);
	glVertex3f(-0.4f, 0.5f, 0.0f);
	glEnd();*/

	
	glBegin(GL_TRIANGLE_FAN);

	glColor3f(1.0f, 0.5f, 0);
	glVertex3f(-1.0f, 1.0f, 0.0f);
	
	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex3f(-1.0f, 0.0f, 0.0f);
	glVertex3f(-0.8f, 0.0f, 0.0f);

	glColor3f(1.0f, 0.5f, 0);
	glVertex3f(-0.8f, 1.0f, 0.0f);

	glEnd();

	
	glBegin(GL_TRIANGLE_FAN);
	
	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex3f(-1.0f, 0.0f, 0.0f);

	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(-1.0f, -1.0f, 0.0f);
	glVertex3f(-0.8f, -1.0f, 0.0f);

	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex3f(-0.8f, 0.0f, 0.0f);

	glEnd();


	glBegin(GL_TRIANGLE_FAN);

	glColor3f(1.0f, 0.5f, 0.0f);
	glVertex3f(-0.8f, 1.0f, 0.0f);
	//glVertex3f(-1.0f, 1.0f, 0.0f);
	glVertex3f(-0.8f, 0.7f, 0.0f);

	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex3f(-0.2f, 0.0f, 0.0f);
	glVertex3f(0.2f, 0.0f, 0.0f);
	glEnd();

	glBegin(GL_TRIANGLE_FAN);

	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(0.8f, -1.0f, 0.0f);
	glVertex3f(1.0f, -1.0f, 0.0f);

	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex3f(1.0f, 0.0f, 0.0f);

	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(0.8f, -0.7f, 0.0f);
	
	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex3f(0.2f, 0.0f, 0.0f);
	glVertex3f(-0.2f, 0.0f, 0.0f);

	glEnd();


	glBegin(GL_TRIANGLE_FAN);

	glColor3f(1.0f, 0.5f, 0.0f);
	glVertex3f(0.8f, 1.0f, 0.0f);

	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex3f(0.8f, 0.0f, 0.0f);
	glVertex3f(1.0f, 0.0f, 0.0f);

	glColor3f(1.0f, 0.5f, 0.0f);
	glVertex3f(1.0f, 1.0f, 0.0f);
	glEnd();

	glBegin(GL_TRIANGLE_FAN);

	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex3f(0.8f, 0.0f, 0.0f);

	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(0.8f, -1.0f, 0.0f);
	glVertex3f(1.0f, -1.0f, 0.0f);

	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex3f(1.0f, 0.0f, 0.0f);
	glEnd();
}

void DrawD()
{
	/*glLineWidth(1.0f);
	glBegin(GL_LINE_LOOP);
	glVertex3f(-1.0f, 1.0f, 0.0f);
	glVertex3f(-1.0f, -1.0f, 0.0f);
	glVertex3f(0.8f, -1.0f, 0.0f);
	glVertex3f(1.0f, -0.8f, 0.0f);
	glVertex3f(1.0f, 0.8f, 0.0f);
	glVertex3f(0.8f, 1.0f, 0.0f);
	glEnd();

	glBegin(GL_LINE_LOOP);
	glVertex3f(-0.9f, 0.9f, 0.0f);
	glVertex3f(-0.9f, -0.9f, 0.0f);
	glVertex3f(0.7f, -0.9f, 0.0f);
	glVertex3f(0.9f, -0.7f, 0.0f);
	glVertex3f(0.9f, 0.7f, 0.0f);
	glVertex3f(0.7f, 0.9f, 0.0f);
	glEnd();
	*/

	/*glLineWidth(1.0f);
	glBegin(GL_LINE_LOOP);
	glVertex3f(-0.5f, 0.5f, 0.0f);
	glVertex3f(-0.5f, -0.6f, 0.0f);
	glVertex3f(0.35f, -0.6f, 0.0f);
	glVertex3f(0.5f, -0.4f, 0.0f);
	glVertex3f(0.5f, 0.3f, 0.0f);
	glVertex3f(0.35f, 0.5f, 0.0f);
	glEnd();

	glBegin(GL_LINE_LOOP);
	glVertex3f(-0.35f, 0.35f, 0.0f);
	glVertex3f(-0.35f, -0.45f, 0.0f);
	glVertex3f(0.2f, -0.45f, 0.0f);
	glVertex3f(0.35f, -0.25f, 0.0f);
	glVertex3f(0.35f, 0.15f, 0.0f);
	glVertex3f(0.2f, 0.35f, 0.0f);
	glEnd();*/

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

	glBegin(GL_TRIANGLE_FAN);
	glColor3f(fOrangeColorX/*1.0f*/, fOrangeColorY/*0.5f*/, 0);
	glVertex3f(0.5f, 0.8f, 0.0f);
	
	glColor3f(fWhiteColorX/*1.0f*/, fWhiteColorY/*1.0f*/, fWhiteColorZ/*1.0f*/);
	glVertex3f(0.8f, 0.5f, 0.0f);
	glVertex3f(1.0f, 0.5f, 0.0f);

	glColor3f(fOrangeColorX/*1.0f*/, fOrangeColorY/*0.5f*/, 0);
	glVertex3f(0.5f, 1.0f, 0.0f);
	glVertex3f(-1.0f, 1.0f, 0.0f);
	glVertex3f(-1.0f, 0.8f, 0.0f);
	glEnd();


	glBegin(GL_TRIANGLE_FAN);
	glColor3f(fGreenColorX/*0.0f*/, fGreenColorY/*1.0f*/, fGreenColorZ/*0.0f*/);
	glVertex3f(0.5f, -0.8f, 0.0f);
	glVertex3f(-1.0f, -0.8f, 0.0f);
	glVertex3f(-1.0f, -1.0f, 0.0f);
	glVertex3f(0.5f, -1.0f, 0.0f);

	glColor3f(fWhiteColorX/*1.0f*/, fWhiteColorY/*1.0f*/, fWhiteColorZ/*1.0f*/);
	glVertex3f(1.0f, -0.5f, 0.0f);
	glVertex3f(0.8f, -0.5f, 0.0f);
	glEnd();


	glColor3f(fWhiteColorX/*1.0f*/, fWhiteColorY/*1.0f*/, fWhiteColorZ/*1.0f*/);
	glBegin(GL_TRIANGLE_FAN);
	glVertex3f(0.8f, 0.5f, 0.0f);
	glVertex3f(0.8f, -0.5f, 0.0f);
	glVertex3f(1.0f, -0.5f, 0.0f);
	glVertex3f(1.0f, 0.5f, 0.0f);
	glEnd();

	
	glBegin(GL_TRIANGLE_FAN);
	
	glColor3f(fOrangeColorX/*1.0f*/, fOrangeColorY/*0.5f*/, 0.0f);
	glVertex3f(-1.0f, 0.8f, 0.0f);
	
	glColor3f(fWhiteColorX/*1.0f*/, fWhiteColorY/*1.0f*/, fWhiteColorZ/*1.0f*/);
	glVertex3f(-1.0f, 0.0f, 0.0f);
	glVertex3f(-0.8f, 0.0f, 0.0f);

	glColor3f(fOrangeColorX/*1.0f*/, fOrangeColorY/*0.5f*/, 0.0f);
	glVertex3f(-0.8f, 0.8f, 0.0f);
	glEnd();


	glBegin(GL_TRIANGLE_FAN);

	glColor3f(fWhiteColorX/*1.0f*/, fWhiteColorY/*1.0f*/, fWhiteColorZ/*1.0f*/);
	glVertex3f(-1.0f, 0.0f, 0.0f);

	glColor3f(fGreenColorX/*0.0f*/, fGreenColorY/*1.0f*/, fGreenColorZ/*0.0f*/);
	glVertex3f(-1.0f, -0.8f, 0.0f);
	glVertex3f(-0.8f, -0.8f, 0.0f);

	glColor3f(fWhiteColorX/*1.0f*/, fWhiteColorX/*1.0f*/, fWhiteColorX/*1.0f*/);
	glVertex3f(-0.8f, 0.0f, 0.0f);
	glEnd();

}

void DrawA()
{
	/*glLineWidth(1.0f);
	glBegin(GL_LINE_LOOP);
	glVertex3f(-0.5f, 1.0f, 0.0f);
	glVertex3f(-1.5f, -1.0f, 0.0f);
	glVertex3f(-0.7f, -1.0f, 0.0f);
	glVertex3f(-0.5f, -0.3f, 0.0f);
	glVertex3f(0.5f, -0.3f, 0.0f);
	glVertex3f(0.7f, -1.0f, 0.0f);
	glVertex3f(1.5f, -1.0f, 0.0f);
	glVertex3f(0.5f, 1.0f, 0.0f);
	glEnd();

	glBegin(GL_LINE_LOOP);
	glVertex3f(-0.2f, 0.5f, 0.0f);
	glVertex3f(-0.5f, 0.0f, 0.0f);
	glVertex3f(0.5f, 0.0f, 0.0f);
	glVertex3f(0.2f, 0.5f, 0.0f);
	glEnd();*/

	/*glLineWidth(1.0f);
	glBegin(GL_LINE_LOOP);
	glVertex3f(-0.25f, 0.5f, 0.0f);
	glVertex3f(-0.7f, -0.6f, 0.0f);
	glVertex3f(-0.4f, -0.6f, 0.0f);
	glVertex3f(-0.25f, -0.15f, 0.0f);
	glVertex3f(0.25f, -0.15f, 0.0f);
	glVertex3f(0.4f, -0.6f, 0.0f);
	glVertex3f(0.7f, -0.6f, 0.0f);
	glVertex3f(0.25f, 0.5f, 0.0f);
	glEnd();

	glBegin(GL_LINE_LOOP);
	glVertex3f(-0.1f, 0.3f, 0.0f);
	glVertex3f(-0.2f, 0.0f, 0.0f);
	glVertex3f(0.2f, 0.0f, 0.0f);
	glVertex3f(0.1f, 0.3f, 0.0f);
	glEnd();*/

	glBegin(GL_TRIANGLE_FAN);
	glColor3f(1.0f, 0.5f, 0.0f);
	glVertex3f(-0.5f, 1.0f, 0.0f);

	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex3f(-0.8f, 0.0f, 0.0f);
	glVertex3f(-0.3f, 0.2f, 0.0f);

	glColor3f(1.0f, 0.5f, 0.0f);
	glVertex3f(-0.1f, 0.8f, 0.0f);
	glVertex3f(0.1f, 0.8f, 0.0f);
	glVertex3f(0.5f, 1.0f, 0.0f);
	glEnd();

	glBegin(GL_TRIANGLE_FAN);
	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex3f(-0.8f, 0.0f, 0.0f);

	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(-1.0, -1.0f, 0.0f);
	glVertex3f(-0.7f, -1.0f, 0.0f);

	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex3f(-0.5f, -0.2f, 0.0f);
	glVertex3f(0.5f, -0.2f, 0.0f);
	glVertex3f(0.3f, 0.2f, 0.0f);
	glVertex3f(-0.3f, 0.2f, 0.0f);
	glEnd();

	glBegin(GL_TRIANGLE_FAN);
	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex3f(0.8f, 0.0f, 0.0f);

	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(1.0, -1.0f, 0.0f);
	glVertex3f(0.7f, -1.0f, 0.0f);

	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex3f(0.5f, -0.2f, 0.0f);
	glVertex3f(0.3f, 0.2f, 0.0f);

	glColor3f(1.0f, 0.5f, 0.0f);
	glVertex3f(0.1f, 0.8f, 0.0f);
	glVertex3f(0.5f, 1.0f, 0.0f);
	glEnd();
}

void DrawPlane()
{
	/*
	glColor3f(0.7f, 0.8f, 0.9f);

	glBegin(GL_QUADS);
	glVertex3f(-0.5f, 0.2f, 0.0f);
	glVertex3f(-0.7f, -0.2f, 0.0f);
	glVertex3f(0.5f, -0.2f, 0.0f);
	glVertex3f(0.7f, 0.2f, 0.0f);
	glEnd();

	glColor3f(1.0f, 1.0f, 1.0f);
	glBegin(GL_LINES);
	glVertex3f(-0.4f, 0.1f, 0.0f);
	glVertex3f(-0.2f, 0.1f, 0.0f);
	glEnd();
	glBegin(GL_LINES);
	glVertex3f(-0.3f, 0.1f, 0.0f);
	glVertex3f(-0.3f, -0.1f, 0.0f);
	glEnd();
	glBegin(GL_LINES);
	glVertex3f(-0.4f, -0.1f, 0.0f);
	glVertex3f(-0.2f, -0.1f, 0.0f);
	glEnd();

	glBegin(GL_LINES);
	glVertex3f(0.0f, 0.1f, 0.0f);
	glVertex3f(-0.1f, -0.1f, 0.0f);
	glEnd();
	glBegin(GL_LINES);
	glVertex3f(0.0f, 0.1f, 0.0f);
	glVertex3f(0.1f, -0.1f, 0.0f);
	glEnd();
	glBegin(GL_LINES);
	glVertex3f(-0.05f, 0.0f, 0.0f);
	glVertex3f(0.05f, 0.0f, 0.0f);
	glEnd();

	glBegin(GL_LINES);
	glVertex3f(0.2f, 0.1f, 0.0f);
	glVertex3f(0.3f, 0.1f, 0.0f);
	glEnd();
	glBegin(GL_LINES);
	glVertex3f(0.2f, 0.1f, 0.0f);
	glVertex3f(0.2f, -0.1f, 0.0f);
	glEnd();
	glBegin(GL_LINES);
	glVertex3f(0.2f, 0.0f, 0.0f);
	glVertex3f(0.3f, 0.0f, 0.0f);
	glEnd();

	glColor3f(0.7f, 0.8f, 0.9f);
	glBegin(GL_TRIANGLES);
	glVertex3f(-0.5f, 0.2f, 0.0f);
	glVertex3f(0.0f, 0.2f, 0.0f);
	glVertex3f(-0.7f, 0.5f, 0.0f);
	glEnd();

	glBegin(GL_TRIANGLES);
	glVertex3f(0.5f, -0.2f, 0.0f);
	glVertex3f(0.0f, -0.2f, 0.0f);
	glVertex3f(0.7f, -0.5f, 0.0f);
	glEnd();

	glBegin(GL_TRIANGLES);
	glVertex3f(-0.7f, -0.2f, 0.0f);
	glVertex3f(-1.2f, -0.15f, 0.0f);
	glVertex3f(-0.85f, -0.5f, 0.0f);
	glEnd();

	glBegin(GL_QUADS);
	glColor3f(1.0f, 0.5f, 0.0f);
	glVertex3f(-1.2f, -0.15f, 0.0f);
	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex3f(-1.4f, -0.35f, 0.0f);
	glVertex3f(-1.3f, -0.45f, 0.0f);
	glColor3f(1.0f, 0.5f, 0.0f);
	glVertex3f(-1.1f, -0.25f, 0.0f);
	glEnd();

	glBegin(GL_QUADS);
	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex3f(-1.1f, -0.25f, 0.0f);
	glVertex3f(-1.3f, -0.45f, 0.0f);
	glVertex3f(-1.15f, -0.6f, 0.0f);
	glVertex3f(-0.95f, -0.40f, 0.0f);
	glEnd();

	glBegin(GL_QUADS);
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(-0.95f, -0.4f, 0.0f);
	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex3f(-1.15f, -0.6f, 0.0f);
	glVertex3f(-1.05f, -0.7f, 0.0f);
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(-0.85f, -0.5f, 0.0f);
	glEnd();
	*/

	glColor3f(0.7f, 0.8f, 0.9f);
	
	glBegin(GL_TRIANGLES);
	glVertex3f(1.0f, 0.0f, 0.0f);
	glVertex3f(0.4f, 0.1f, 0.0f);
	glVertex3f(0.4f, -0.1f, 0.0f);
	glEnd();

	glBegin(GL_POLYGON);
	glVertex3f(0.4f, 0.1f, 0.0f);
	glVertex3f(0.3f, 0.2f, 0.0f);
	glVertex3f(0.0f, 0.3f, 0.0f);
	glVertex3f(0.0f, -0.3f, 0.0f);
	glVertex3f(0.3f, -0.2f, 0.0f);
	glVertex3f(0.4f, -0.1f, 0.0f);
	glEnd();

	glBegin(GL_POLYGON);
	glVertex3f(0.0f, 0.3f, 0.0f);
	glVertex3f(- 0.5f, 1.0f, 0.0f);
	glVertex3f(- 0.6f, 1.0f, 0.0f);
	glVertex3f(- 0.6f, 0.9f, 0.0f);
	glVertex3f(- 0.5f, 0.4f, 0.0f);
	glVertex3f(- 0.6f, 0.4f, 0.0f);
	glVertex3f(- 0.7f, 0.1f, 0.0f);
	glVertex3f(- 0.7f, -0.1f, 0.0f);
	glVertex3f(- 0.6f, -0.4f, 0.0f);
	glVertex3f(- 0.5f, -0.4f, 0.0f);
	glVertex3f(- 0.6f, -0.9f, 0.0f);
	glVertex3f(- 0.6f, -1.0f, 0.0f);
	glVertex3f(- 0.5f, -1.0f, 0.0f);
	glVertex3f(0.0f, -0.3f, 0.0f);
	glEnd();

	glBegin(GL_QUADS);
	glVertex3f(-0.7f, 0.1f, 0.0f);
	glVertex3f(-0.8f, 0.1f, 0.0f);
	glVertex3f(-0.8f, -0.1f, 0.0f);
	glVertex3f(-0.7f, -0.1f, 0.0f);
	glEnd();

	/*
	glColor3f(0.0f, 0.0f, 0.0f);
	glBegin(GL_QUADS);
	glVertex3f(-0.8f, 0.1f, 0.0f);
	glVertex3f(-2.0f, 0.1f, 0.0f);
	glVertex3f(-2.0f, -0.1f, 0.0f);
	glVertex3f(-0.8f, -0.1f, 0.0f);
	glEnd();
	*/
	GLfloat fRandNum1 = 0.0f;
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

}
