
#include <iostream> // iostream is cache of namespaces
#include <stdio.h> // printf()
#include <stdlib.h> // exit()
#include <memory.h> // memset()
#include <math.h>

#include <X11/Xlib.h> // windows.h // xserver api
#include <X11/Xutil.h> // xvisualinfo structure
#include <X11/XKBlib.h> // keyboard
#include <X11/keysym.h> // key symbol 

#include <GL/glx.h>
#include <GL/gl.h>
#include <GL/glu.h>

using namespace std;

bool gbFullScreen = false;
Display *gpDisplay = NULL;
XVisualInfo *gpXVisualInfo = NULL;
Colormap gColorMap;
Window gWindow;

FILE* gpLogFile = NULL;

static GLXContext gGlxContext; // rendering context
#define NUMBER_OF_VERTICES 5000 * 2
#define PI 3.14f
#define CHAR_HEIGHT 1.5f
#define CHAR_WIDTH 0.05f
#define CHAR_SIZE 0.1f

GLfloat giWindowWidth = 800;
GLfloat giWindowHeight = 600;

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


int main()
{
	void display();
	void update();
	void initialize();
	void resize(int, int);
	void CreateWindow();
	void ToggleFullScreen();
	void uninitialize();
	
	int iWinWidth = giWindowWidth;
	int iWinHeight = giWindowHeight;

	bool bDone = false;

	void MessageBox(char *);

	if((gpLogFile = fopen("Log.txt", "w")) == NULL)
	{
		printf("Log file creation failed");
		//MessageBox((char*)"Log file creation failed");
		return 0;
	}

	CreateWindow();

	initialize();
	
	XEvent event;
	KeySym keysym;
	
	while(bDone == false)
	{
		while(XPending(gpDisplay)) // PeekMessage()
		{
			XNextEvent(gpDisplay, &event);
			switch(event.type)
			{
				case MapNotify:
					break;
					
				case KeyPress:
					keysym = XkbKeycodeToKeysym(gpDisplay, event.xkey.keycode, 0, 0);
					switch(keysym)
					{
						case XK_Escape:
							//uninitialize();
							//exit(0);
							bDone = true;
							break;

						case XK_F:
						case XK_f:
							if(gbFullScreen == false)
							{
								ToggleFullScreen();
								gbFullScreen = true;
							}
							else
							{
								ToggleFullScreen();
								gbFullScreen = false;
							}
							break;
						default:
							break;
					}
					break;
					
				case ButtonPress:
					switch(event.xbutton.button)
					{
						case 1:
							break;
						case 2:
							break;
						case 3:
							break;
						case 4:
							break;
					}
					break;
					
				case MotionNotify:
					break;
						
				case ConfigureNotify:
					iWinWidth = event.xconfigure.width;
					iWinHeight = event.xconfigure.height;
					resize(iWinWidth, iWinHeight);
					break;
				
				case Expose:
					break;
				
				case DestroyNotify:
					break;
				
				case 33:
					//uninitialize();
					//exit(0);
					bDone = true;
					break;
				
				default:
					break;
			}
		}
		update();
		display();
	}
	
	uninitialize();
	return (0);
}

void CreateWindow()
{
	void uninitialize();
	
	XSetWindowAttributes winAttribs;
	int defaultScreen;
	int defaultDepth;
	int styleMask;
	static int iFrameBufferAttributes[] = {
		GLX_RGBA,
		GLX_DOUBLEBUFFER, True,
		GLX_RED_SIZE, 8,
		GLX_GREEN_SIZE, 8,
		GLX_BLUE_SIZE, 8,
		GLX_ALPHA_SIZE, 8,
		GLX_DEPTH_SIZE, 24,
		None
	}; // 1 size for single buffer, 8 size for double dbuffer
	
	gpDisplay = XOpenDisplay(NULL);
	if(gpDisplay == NULL)
	{
		printf("Error : Unable to open X display.\n Exitting now...\n");
		uninitialize();
		exit(1);
	}
	
	defaultScreen = XDefaultScreen(gpDisplay);
	defaultDepth = DefaultDepth(gpDisplay, defaultScreen);
	
/*	gpXVisualInfo = (XVisualInfo*)malloc(sizeof(XVisualInfo));
	if(gpXVisualInfo == NULL)
	{
		printf("malloc xvf failed\n");
		uninitialize();
		exit(1);
	}
	
	XMatchVisualInfo(gpDisplay,
					defaultScreen,
					defaultDepth,
					TrueColor,
					gpXVisualInfo);
	if(gpXVisualInfo == NULL)
	{
		printf("xmvi xvf failed\n");
		uninitialize();
		exit(1);
	}*/

	gpXVisualInfo = glXChooseVisual(gpDisplay, defaultScreen, iFrameBufferAttributes);
	if(NULL == gpXVisualInfo)
	{
		printf("glxChooseVisual returns NULL\n");
		uninitialize();
		exit(1);
	}

	
	winAttribs.border_pixel = 0;
	winAttribs.background_pixmap = 0;
	winAttribs.colormap = XCreateColormap(gpDisplay,
										RootWindow(gpDisplay, gpXVisualInfo->screen),
										gpXVisualInfo->visual,
										AllocNone);
										
	gColorMap = winAttribs.colormap;
	
	winAttribs.background_pixel = BlackPixel(gpDisplay, defaultScreen);
	
	winAttribs.event_mask = ExposureMask | VisibilityChangeMask | ButtonPressMask | KeyPressMask | PointerMotionMask | StructureNotifyMask;
	styleMask = CWBorderPixel | CWBackPixel | CWEventMask | CWColormap;
	
	gWindow = XCreateWindow(gpDisplay,
							RootWindow(gpDisplay, gpXVisualInfo->screen),
							0,
							0,
							giWindowWidth,
							giWindowHeight,
							0,
							gpXVisualInfo->depth,
							InputOutput,
							gpXVisualInfo->visual,
							styleMask,
							&winAttribs);
							
	if(!gWindow)
	{
		printf("error gwindow\n");
		uninitialize();
		exit(1);
	}
	
	XStoreName(gpDisplay, gWindow, "Graph");
	
	Atom windowManagerDelete = XInternAtom(gpDisplay, "WM_DELETE_WINDOW", True);
	XSetWMProtocols(gpDisplay, gWindow, &windowManagerDelete, 1);
	
	XMapWindow(gpDisplay, gWindow);
	
}

void ToggleFullScreen()
{
	Atom wm_state;
	Atom fullscreen;
	XEvent xev={0};
	
	wm_state = XInternAtom(gpDisplay, "_NET_WM_STATE", False);
	memset(&xev, 0, sizeof(xev));
	
	xev.type = ClientMessage;
	xev.xclient.window = gWindow;
	xev.xclient.message_type = wm_state;
	xev.xclient.format = 32;
	xev.xclient.data.l[0] = gbFullScreen ? 0 : 1;
	fullscreen = XInternAtom(gpDisplay, "_NET_WM_STATE_FULLSCREEN", False);
	xev.xclient.data.l[1] = fullscreen;
	
	XSendEvent(gpDisplay,
				RootWindow(gpDisplay, gpXVisualInfo->screen),
				False,
				StructureNotifyMask,
				&xev);
}

void uninitialize()
{
	if(true == gbFullScreen)
	{
		ToggleFullScreen();
	}

	GLXContext currentglXContext = glXGetCurrentContext();
	if(NULL != currentglXContext && currentglXContext == gGlxContext)
	{
		glXMakeCurrent(gpDisplay, 0, 0);
	}

	if(gGlxContext)
	{
		glXDestroyContext(gpDisplay, gGlxContext);
	}


	if(gWindow)
	{
		XDestroyWindow(gpDisplay, gWindow);
	}
	
	if(gColorMap)
	{
		XFreeColormap(gpDisplay, gColorMap);
	}
	
	if(gpXVisualInfo)
	{
		free(gpXVisualInfo);
		gpXVisualInfo = NULL;
	}
	
	if(gpDisplay)
	{
		XCloseDisplay(gpDisplay);
		gpDisplay = NULL;
	}
}

void initialize()
{
	void uninitialize();

	gGlxContext = glXCreateContext(gpDisplay, gpXVisualInfo, NULL, True);

	glXMakeCurrent(gpDisplay, gWindow, gGlxContext);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepth(1.0f);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	// usual openGL initialization code
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
}

void resize(int iWidth, int iHeight)
{
	// usual resize code
	glViewport(0, 0, (GLsizei)iWidth, (GLsizei)iHeight);

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

void display(void)
{
	// usual display code
	
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	void DrawI();
	void DrawN();
	void DrawD();
	void DrawA();
	void DrawPlane();

	glClear(GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	
	if (fTransIX < -5.0f)
	{
		fTransIX += 0.0001f;
	}
	
	glLoadIdentity();
	glTranslatef(fTransIX /*-5.0f*/, 0.0f, -12.0f);
	DrawI();
	
	if (fTransIX >= -5.0f)
	{
		if (fTransNY > 0.0f)
		{
			fTransNY -= 0.0001f;
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
			fTransIY += 0.0001f;
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
			fTransAX -= 0.0001f;
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
			//PlaySound(MAKEINTRESOURCE(ID_SONG), GetModuleHandle(NULL), SND_ASYNC | SND_RESOURCE);
		}
		
		fTransPlaneTopX += 0.00004f;

		if(fTransPlaneTopY >= 0.0f)
		{
			fTransPlaneTopY -= 0.00004f;
		}
		
		if (fTransPlaneTopX > 6.0f)
		{
			fTransPlaneTopY -= 0.00004f;
		}
		
		fTransPlaneMiddleX += 0.00004f;
		//fTransPlaneMiddleY += 0.001f;

		fTransPlaneBottomX += 0.00004f;

		if(fTransPlaneBottomY <= 0.0f)
		{
			fTransPlaneBottomY += 0.00004f;
		}

		if (fTransPlaneBottomX > 6.0f)
		{
			fTransPlaneBottomY += 0.00004f;
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


	glXSwapBuffers(gpDisplay, gWindow); // double buffer
}

void update()
{
	
}

int MessageBox(char *pszText)
{
	Display *pDisplay;
	Window window;
	XEvent event;
	int iScreen;

	pDisplay = XOpenDisplay(NULL);
	if (NULL == pDisplay)
	{
		printf("XOpenDisplay() failed from MessageBox()\n");
		exit(1);
	}

	iScreen = DefaultScreen(pDisplay);
	window = XCreateSimpleWindow(pDisplay,
			RootWindow(pDisplay, iScreen),
		       	10, // border
		       	10,
		       	500,
		       	200,
		       	1,
			BlackPixel(pDisplay, iScreen),
			WhitePixel(pDisplay, iScreen));
	
	XSelectInput(pDisplay, window, ExposureMask | KeyPressMask);
	XMapWindow(pDisplay, window);

	while(1)
	{
		XNextEvent(pDisplay, &event);
		switch(event.type)
		{
			case Expose:
			{
				XFillRectangle(pDisplay, window, DefaultGC(pDisplay, iScreen), 20, 20, 10, 10);
				XDrawString(pDisplay, window, DefaultGC(pDisplay, iScreen), 10, 50, pszText, strlen(pszText));
			}
			break;
			
			case KeyPress:
			case ButtonPress:
				break;
   		}
	}

////	XDestroyWindow(pDisplay,window);
//	XCloseDisplay(pDisplay);
	return 0;
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
			fOrangeColorX += 0.00001f;
		}

		if (fOrangeColorY < 0.5f)
		{
			fOrangeColorY += 0.00001f;
		}

		
		if (fWhiteColorX < 1.0f)
		{
			fWhiteColorX += 0.00001f;
			fWhiteColorY += 0.00001f;
			fWhiteColorZ += 0.00001f;
		}		

		if (fGreenColorY < 1.0f)
		{
			fGreenColorY += 0.00001f;
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
