#include <Windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include <stdio.h>
#include <math.h>

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")

HDC ghDC = NULL;
HGLRC ghGlRc = NULL;
bool gbIsActiveWindow = false;
bool gbIsFullScreen = false;
HWND ghWnd = NULL;
FILE* gpLogFile = NULL;
WINDOWPLACEMENT gwpPrev = { sizeof(WINDOWPLACEMENT) };
DWORD gdwStyle = 0;
GLfloat giHeight = 0;
GLfloat giWidth = 0;
GLfloat fRotx = 0.0f;
GLfloat fRoty = 0.0f;
GLfloat fRotz = 0.0f;

#define WIN_WIDTH 800
#define WIN_HEIGHT 600
#define NUMBER_OF_VERTICES 5000 * 2
#define PI 3.14

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
void ToggleFullScreen();

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR szCmdLine, int iCmdShow)
{
	int initialize();
	void display();

	bool bIsDone = false;
	int iRet = 0;
	TCHAR szAppName[] = TEXT("RDH");
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
		TEXT("Rotating Deathly Hallows"),
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
		giWidth = LOWORD(lParam);
		giHeight = HIWORD(lParam);
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
	glClear(GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(0.0f, 0.0f, -3.0f);

	glColor3f(1.0f, 0.5f, 0.0f);
	glLineWidth(2.0f);

	glRotatef(fRotx, 0.0f, 1.0f, 0.0f);

	glBegin(GL_LINE_LOOP);

	glVertex3f(0, 1, 0.0f);
	glVertex3f(-1, -1, 0.0f);
	glVertex3f(1, -1, 0.0f);

	glEnd();

	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, -3.0f);

	glRotatef(fRoty, 1.0f, 0.0f, 0.0f);
	glBegin(GL_LINES);

	glVertex3f(0.0f, 1.0f, 0.0f);
	glVertex3f(0.0f, -1.0f, 0.0f);

	glEnd();

	GLfloat fBase = 2.0f;
	GLfloat fHeight = 2.0f;
	GLfloat fSides = sqrtf(fHeight * fHeight + (fBase / 2) * (fBase / 2));
	GLfloat fArea = (fBase * fHeight) / 2.0f;
	GLfloat fPerimeter = (fBase + fSides + fSides);
	GLfloat fRadius = (2.0f * fArea / fPerimeter);
	GLfloat fCenterX = 0.0f;
	GLfloat fCenterY = 1.0f - fRadius;

	glLoadIdentity();
	glTranslatef(0.0f, -fCenterY, -3.0f);

	glRotatef(fRotz, 0.0f, 0.0f, 1.0);
	glBegin(GL_POINTS);
	glPointSize(3.0);
	for (int iItr = 0; iItr < NUMBER_OF_VERTICES; iItr++)
	{
		glVertex3f((GLfloat)(fRadius * cos((5.0f * (PI + 1) * iItr) / NUMBER_OF_VERTICES)), (GLfloat)(fRadius * sin((5.0f * (PI + 1) * iItr) / NUMBER_OF_VERTICES)), 0.0f);
	}
	glEnd();

	fRotx+=0.1f;
	fRoty+=0.1f;
	fRotz+=0.1f;

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

