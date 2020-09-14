
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
int giWindowWidth = 800;
int giWindowHeight = 600;

FILE* gpLogFile = NULL;

static GLXContext gGlxContext; // rendering context

bool gbLight = false;

struct Light
{
	GLfloat gfLightAmbient[4];
	GLfloat gfLightDiffuse[4];
	GLfloat gfLightPosition[4];
	GLfloat gfLightSpecular[4];

};

Light gLights[3];

/*GLfloat gfLightAmbientZero[] = { 0.0f, 0.0f, 0.0f, 1.0f };
GLfloat gfLightDiffuseZero[] = { 1.0f, 0.0f, 0.0f, 1.0f };
GLfloat gfLightPositionZero[] = { 0.0f, 0.0f, 0.0f, 1.0f }; // 1.0, 1.0, 1.0, 1.0
GLfloat gfLightSpecularZero[] = { 1.0f, 0.0f, 0.0f, 1.0f };

GLfloat gfLightAmbientOne[] = { 0.0f, 0.0f, 0.0f, 1.0f };
GLfloat gfLightDiffuseOne[] = { 0.0f, 1.0f, 0.0f, 1.0f };
GLfloat gfLightPositionOne[] = { 0.0f, 0.0f, 0.0f, 1.0f }; // 1.0, 1.0, 1.0, 1.0
GLfloat gfLightSpecularOne[] = { 0.0f, 1.0f, 0.0f, 1.0f };

GLfloat gfLightAmbientThree[] = { 0.0f, 0.0f, 0.0f, 1.0f };
GLfloat gfLightDiffuseThree[] = { 0.0f, 0.0f, 1.0f, 1.0f };
GLfloat gfLightPositionThree[] = { 0.0f, 0.0f, 0.0f, 1.0f }; // 1.0, 1.0, 1.0, 1.0
GLfloat gfLightSpecularThree[] = { 0.0f, 0.0f, 1.0f, 1.0f };*/

GLfloat gfMaterialAmbient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
GLfloat gfMaterialDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat gfMaterialSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat gfMaterialShyniness[] = { 128.0f }; // 50.0

GLfloat gfAngleOne = 0.0f;
GLfloat gfAngleTwo = 0.0f;
GLfloat gfAngleThree = 0.0f;

GLUquadric *gpQuadric = NULL;

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

	gLights[0].gfLightAmbient[0] = 0.0f;
	gLights[0].gfLightAmbient[1] = 0.0f;
	gLights[0].gfLightAmbient[2] = 0.0f;
	gLights[0].gfLightAmbient[3] = 1.0f;

	gLights[0].gfLightDiffuse[0] = 1.0f;
	gLights[0].gfLightDiffuse[1] = 0.0f;
	gLights[0].gfLightDiffuse[2] = 0.0f;
	gLights[0].gfLightDiffuse[3] = 1.0f;

	gLights[0].gfLightSpecular[0] = 1.0f;
	gLights[0].gfLightSpecular[1] = 0.0f;
	gLights[0].gfLightSpecular[2] = 0.0f;
	gLights[0].gfLightSpecular[3] = 1.0f;

	gLights[0].gfLightPosition[0] = 0.0f;
	gLights[0].gfLightPosition[1] = 0.0f;
	gLights[0].gfLightPosition[2] = 0.0f;
	gLights[0].gfLightPosition[3] = 0.0f;


	gLights[1].gfLightAmbient[0] = 0.0f;
	gLights[1].gfLightAmbient[1] = 0.0f;
	gLights[1].gfLightAmbient[2] = 0.0f;
	gLights[1].gfLightAmbient[3] = 1.0f;

	gLights[1].gfLightDiffuse[0] = 0.0f;
	gLights[1].gfLightDiffuse[1] = 1.0f;
	gLights[1].gfLightDiffuse[2] = 0.0f;
	gLights[1].gfLightDiffuse[3] = 1.0f;

	gLights[1].gfLightSpecular[0] = 0.0f;
	gLights[1].gfLightSpecular[1] = 1.0f;
	gLights[1].gfLightSpecular[2] = 0.0f;
	gLights[1].gfLightSpecular[3] = 1.0f;

	gLights[1].gfLightPosition[0] = 0.0f;
	gLights[1].gfLightPosition[1] = 0.0f;
	gLights[1].gfLightPosition[2] = 0.0f;
	gLights[1].gfLightPosition[3] = 0.0f;

	gLights[2].gfLightAmbient[0] = 0.0f;
	gLights[2].gfLightAmbient[1] = 0.0f;
	gLights[2].gfLightAmbient[2] = 0.0f;
	gLights[2].gfLightAmbient[3] = 1.0f;
	
	gLights[2].gfLightDiffuse[0] = 0.0f;
	gLights[2].gfLightDiffuse[1] = 0.0f;
	gLights[2].gfLightDiffuse[2] = 1.0f;
	gLights[2].gfLightDiffuse[3] = 1.0f;

	gLights[2].gfLightSpecular[0] = 0.0f;
	gLights[2].gfLightSpecular[1] = 0.0f;
	gLights[2].gfLightSpecular[2] = 1.0f;
	gLights[2].gfLightSpecular[3] = 1.0f;

	gLights[2].gfLightPosition[0] = 0.0f;
	gLights[2].gfLightPosition[1] = 0.0f;
	gLights[2].gfLightPosition[2] = 0.0f;
	gLights[2].gfLightPosition[3] = 0.0f;

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
						case XK_L:
						case XK_l:
							if (false == gbLight)
							{
								gbLight = true;
								glEnable(GL_LIGHTING);
							}
							else
							{
								gbLight = false;
								glDisable(GL_LIGHTING);
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
	void resize(int iWidth, int iHeight);

	gGlxContext = glXCreateContext(gpDisplay, gpXVisualInfo, NULL, True);

	glXMakeCurrent(gpDisplay, gWindow, gGlxContext);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepth(1.0f);

	glShadeModel(GL_SMOOTH);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	// usual openGL initialization code
	glLightfv(GL_LIGHT0, GL_AMBIENT, gLights[0].gfLightAmbient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, gLights[0].gfLightDiffuse);
	//glLightfv(GL_LIGHT0, GL_POSITION, gLights[0].gfLightPosition);
	glLightfv(GL_LIGHT0, GL_SPECULAR, gLights[0].gfLightSpecular);
	glEnable(GL_LIGHT0);

	glLightfv(GL_LIGHT1, GL_AMBIENT, gLights[1].gfLightAmbient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, gLights[1].gfLightDiffuse);
	//glLightfv(GL_LIGHT1, GL_POSITION, gLights[1].gfLightPosition);
	glLightfv(GL_LIGHT1, GL_SPECULAR, gLights[1].gfLightSpecular);
	glEnable(GL_LIGHT1);

	glLightfv(GL_LIGHT2, GL_AMBIENT, gLights[2].gfLightAmbient);
	glLightfv(GL_LIGHT2, GL_DIFFUSE, gLights[2].gfLightDiffuse);
	//glLightfv(GL_LIGHT2, GL_POSITION, gLights[1].gfLightPosition);
	glLightfv(GL_LIGHT2, GL_SPECULAR, gLights[2].gfLightSpecular);
	glEnable(GL_LIGHT2);

	glMaterialfv(GL_FRONT, GL_AMBIENT, gfMaterialAmbient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, gfMaterialDiffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, gfMaterialSpecular);
	glMaterialfv(GL_FRONT, GL_SHININESS, gfMaterialShyniness);


	resize(giWindowWidth, giWindowHeight);

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

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(0.0f, 0.0f, -1.0f);
	glPushMatrix();

	gluLookAt(0.0f, 0.0f, 3.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f);

	glPushMatrix();
	glRotatef(gfAngleOne,  1.0f, 0.0f, 0.0f); // x rotation
	gLights[0].gfLightPosition[1] = gfAngleOne; // as above is x rotation we can change, y or z value of light position : Here we are changing y
	glLightfv(GL_LIGHT0, GL_POSITION, gLights[0].gfLightPosition);
	glPopMatrix();

	glPushMatrix();
	glRotatef(gfAngleTwo, 0.0f, 1.0f, 0.0f); // y rotation
	gLights[1].gfLightPosition[0] = gfAngleTwo; // as above is y rotation we can change, x or z value of light position : Here we are changing x
	glLightfv(GL_LIGHT1, GL_POSITION, gLights[1].gfLightPosition);
	glPopMatrix();

	glPushMatrix();
	glRotatef(gfAngleThree, 0.0f, 0.0f, 1.0f); // z rotation
	gLights[2].gfLightPosition[0] = gfAngleThree; // as above is z rotation we can change, x or y value of light position : Here we are changing x
	glLightfv(GL_LIGHT2, GL_POSITION, gLights[2].gfLightPosition);
	glPopMatrix();
	glPopMatrix();
	
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	gpQuadric = gluNewQuadric();

	if (NULL != gpQuadric)
	{
		gluSphere(gpQuadric, 0.2f, 30, 30);
		gluDeleteQuadric(gpQuadric);
		gpQuadric = NULL;
	}
	else
	{
		fprintf(gpLogFile, "gpQuadric is null.");
	}
	

	glXSwapBuffers(gpDisplay, gWindow); // double buffer
}

void update()
{
	gfAngleOne += 0.04f;
	gfAngleTwo += 0.04f;
	gfAngleThree += 0.04f;

	if (gfAngleOne > 360.0f)
	{
		gfAngleOne = 0.0f;
	}

	if (gfAngleTwo > 360.0f)
	{
		gfAngleTwo = 0.0f;
	}

	if (gfAngleThree > 360.0f)
	{
		gfAngleThree = 0.0f;
	}
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
