
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

GLfloat gfLightAmbient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
GLfloat gfLightDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat gfLightPosition[] = { 1.0f, 1.0f, 1.0f, 1.0f }; // 0.0, 3.0, 3.0, 1.0

GLfloat gfLight_Model_Ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
GLfloat gfLight_Model_Local_Viewer[] = { 0.0f };

GLfloat gfAngleXAxisRot = 0.0f;
GLfloat gfAngleYAxisRot = 0.0f;
GLfloat gfAngleZAxisRot = 0.0f;

GLfloat gfMaterialAmbient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
GLfloat gfMaterialDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat gfMaterialSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat gfMaterialShyniness[] = { 128.0f }; // 50.0

GLUquadric *gpQuadric[24] = { NULL };

GLint giKeyPress = 0;

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

						case XK_x:
						case XK_X:
							gfAngleXAxisRot = 0.0f;
							giKeyPress = 1;
							break;

						case XK_y:
						case XK_Y:
							gfAngleYAxisRot = 0.0f;
							giKeyPress = 2;
							break;

						case XK_z:
						case XK_Z:
							gfAngleZAxisRot = 0.0f;
							giKeyPress = 3;
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
	glEnable(GL_AUTO_NORMAL);
	glEnable(GL_NORMALIZE);

	glLightfv(GL_LIGHT0, GL_AMBIENT, gfLightAmbient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, gfLightDiffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, gfLightPosition);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, gfLight_Model_Ambient);
	glLightModelfv(GL_LIGHT_MODEL_LOCAL_VIEWER, gfLight_Model_Local_Viewer);
	glEnable(GL_LIGHT0);

	for (int iItr = 0; iItr < 24; iItr++)
	{
		gpQuadric[iItr] = gluNewQuadric();
	}

	glClearColor(0.25f, 0.25f, 0.25f, 1.0f);


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

	/*gluPerspective(45.0f,
		(GLfloat)iWidth / (GLfloat)iHeight,
		0.1f,
		100.0f
	);*/

	if (iWidth < iHeight)
	{
		glOrtho(0.0f, 15.5f, 0.0f, 15.5f * ((GLfloat)iHeight / (GLfloat)iWidth), -10.0f, 10.0f);
	}
	else
	{
		glOrtho(0.0f, 15.5f * ((GLfloat)iWidth / (GLfloat)iHeight), 0.0f, 15.5f, -10.0f, 10.0f);
	}

}

void display(void)
{
	// usual display code
	void Draw24Spheres();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(0.0f, 0.0f, -1.0f);
	
	if (1 == giKeyPress)
	{
		glRotatef(gfAngleXAxisRot, 1.0f, 0.0f, 0.0f);
		gfLightPosition[0] = 0.0f;
		gfLightPosition[1] = gfAngleXAxisRot;
		gfLightPosition[2] = 0.0f;
		gfLightPosition[3] = 1.0f;
	}
	else if (2 == giKeyPress)
	{
		glRotatef(gfAngleYAxisRot, 0.0f, 1.0f, 0.0f);
		gfLightPosition[0] = gfAngleYAxisRot;
		gfLightPosition[1] = 0.0f;
		gfLightPosition[2] = 0.0f;
		gfLightPosition[3] = 1.0f;
	}
	else if (3 == giKeyPress)
	{
		glRotatef(gfAngleZAxisRot, 0.0f, 0.0f, 1.0f);
		gfLightPosition[0] = gfAngleZAxisRot;
		gfLightPosition[1] = 0.0f;
		gfLightPosition[2] = 0.0f;
		gfLightPosition[3] = 1.0f; 
	}

	glLightfv(GL_LIGHT0, GL_POSITION, gfLightPosition);

	Draw24Spheres();
	

	glXSwapBuffers(gpDisplay, gWindow); // double buffer
}

void update()
{
	if (1 == giKeyPress)
	{
		gfAngleXAxisRot += 0.05f;
		gfAngleYAxisRot = 0.0f;
		gfAngleZAxisRot = 0.0f;
	}
	else if (2 == giKeyPress)
	{
		gfAngleYAxisRot += 0.05f;
		gfAngleXAxisRot = 0.0f;
		gfAngleZAxisRot = 0.0f;
	}
	else if (3 == giKeyPress)
	{
		gfAngleZAxisRot += 0.05f;
		gfAngleXAxisRot = 0.0f;
		gfAngleYAxisRot = 0.0f;
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

void Draw24Spheres()
{
	// Sphere 1
	gfMaterialAmbient[0] = 0.0215f;
	gfMaterialAmbient[1] = 0.1745f;
	gfMaterialAmbient[2] = 0.0215f;
	gfMaterialAmbient[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_AMBIENT, gfMaterialAmbient);

	gfMaterialDiffuse[0] = 0.07568f;
	gfMaterialDiffuse[1] = 0.61424f;
	gfMaterialDiffuse[2] = 0.075685f;
	gfMaterialDiffuse[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_DIFFUSE, gfMaterialDiffuse);

	gfMaterialSpecular[0] = 0.633f;
	gfMaterialSpecular[1] = 0.727811f;
	gfMaterialSpecular[2] = 0.633f;
	gfMaterialSpecular[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_SPECULAR, gfMaterialSpecular);

	gfMaterialShyniness[0] = 0.6f * 128.0f;
	glMaterialfv(GL_FRONT, GL_SHININESS, gfMaterialShyniness);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	glTranslatef(2.5f, 14.0f, 0.0f);

	gluSphere(gpQuadric[0], 1.0f, 30, 30);

	// Sphere 2
	gfMaterialAmbient[0] = 0.135f;
	gfMaterialAmbient[1] = 0.2225f;
	gfMaterialAmbient[2] = 0.1575f;
	gfMaterialAmbient[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_AMBIENT, gfMaterialAmbient);

	gfMaterialDiffuse[0] = 0.54f;
	gfMaterialDiffuse[1] = 0.89f;
	gfMaterialDiffuse[2] = 0.63f;
	gfMaterialDiffuse[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_DIFFUSE, gfMaterialDiffuse);

	gfMaterialSpecular[0] = 0.316228f;
	gfMaterialSpecular[1] = 0.316228f;
	gfMaterialSpecular[2] = 0.316228f;
	gfMaterialSpecular[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_SPECULAR, gfMaterialSpecular);

	gfMaterialShyniness[0] = 0.1f * 128.0f;
	glMaterialfv(GL_FRONT, GL_SHININESS, gfMaterialShyniness);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(2.5f, 11.5f, 0.0f);

	gluSphere(gpQuadric[1], 1.0f, 30, 30);


	// Sphere 3
	gfMaterialAmbient[0] = 0.05375f;
	gfMaterialAmbient[1] = 0.05f;
	gfMaterialAmbient[2] = 0.06625f;
	gfMaterialAmbient[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_AMBIENT, gfMaterialAmbient);

	gfMaterialDiffuse[0] = 0.18275f;
	gfMaterialDiffuse[1] = 0.17f;
	gfMaterialDiffuse[2] = 0.22525f;
	gfMaterialDiffuse[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_DIFFUSE, gfMaterialDiffuse);

	gfMaterialSpecular[0] = 0.332741f;
	gfMaterialSpecular[1] = 0.328634f;
	gfMaterialSpecular[2] = 0.346435f;
	gfMaterialSpecular[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_SPECULAR, gfMaterialSpecular);

	gfMaterialShyniness[0] = 0.3f * 128.0f;
	glMaterialfv(GL_FRONT, GL_SHININESS, gfMaterialShyniness);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(2.5f, 9.0f, 0.0f);

	gluSphere(gpQuadric[2], 1.0f, 30, 30);



	// Sphere 4
	gfMaterialAmbient[0] = 0.25f;
	gfMaterialAmbient[1] = 0.20725f;
	gfMaterialAmbient[2] = 0.20725f;
	gfMaterialAmbient[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_AMBIENT, gfMaterialAmbient);

	gfMaterialDiffuse[0] = 1.0f;
	gfMaterialDiffuse[1] = 0.829f;
	gfMaterialDiffuse[2] = 0.829f;
	gfMaterialDiffuse[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_DIFFUSE, gfMaterialDiffuse);

	gfMaterialSpecular[0] = 0.296648f;
	gfMaterialSpecular[1] = 0.296648f;
	gfMaterialSpecular[2] = 0.296648f;
	gfMaterialSpecular[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_SPECULAR, gfMaterialSpecular);

	gfMaterialShyniness[0] = 0.088f * 128.0f;
	glMaterialfv(GL_FRONT, GL_SHININESS, gfMaterialShyniness);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(2.5f, 6.5f, 0.0f);

	gluSphere(gpQuadric[3], 1.0f, 30, 30);


	// Sphere 5
	gfMaterialAmbient[0] = 0.1745f;
	gfMaterialAmbient[1] = 0.01175f;
	gfMaterialAmbient[2] = 0.01175f;
	gfMaterialAmbient[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_AMBIENT, gfMaterialAmbient);

	gfMaterialDiffuse[0] = 0.61424f;
	gfMaterialDiffuse[1] = 0.04136f;
	gfMaterialDiffuse[2] = 0.04136f;
	gfMaterialDiffuse[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_DIFFUSE, gfMaterialDiffuse);

	gfMaterialSpecular[0] = 0.727811f;
	gfMaterialSpecular[1] = 0.626959f;
	gfMaterialSpecular[2] = 0.626959f;
	gfMaterialSpecular[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_SPECULAR, gfMaterialSpecular);

	gfMaterialShyniness[0] = 0.6f * 128.0f;
	glMaterialfv(GL_FRONT, GL_SHININESS, gfMaterialShyniness);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(2.5f, 4.0f, 0.0f);

	gluSphere(gpQuadric[4], 1.0f, 30, 30);



	// Sphere 6
	gfMaterialAmbient[0] = 0.1f;
	gfMaterialAmbient[1] = 0.18725f;
	gfMaterialAmbient[2] = 0.1745f;
	gfMaterialAmbient[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_AMBIENT, gfMaterialAmbient);

	gfMaterialDiffuse[0] = 0.396f;
	gfMaterialDiffuse[1] = 0.74151f;
	gfMaterialDiffuse[2] = 0.69102f;
	gfMaterialDiffuse[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_DIFFUSE, gfMaterialDiffuse);

	gfMaterialSpecular[0] = 0.297254f;
	gfMaterialSpecular[1] = 0.30829f;
	gfMaterialSpecular[2] = 0.306678f;
	gfMaterialSpecular[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_SPECULAR, gfMaterialSpecular);

	gfMaterialShyniness[0] = 0.1f * 128.0f;
	glMaterialfv(GL_FRONT, GL_SHININESS, gfMaterialShyniness);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(2.5f, 1.5f, 0.0f);

	gluSphere(gpQuadric[5], 1.0f, 30, 30);



	// Sphere 7
	gfMaterialAmbient[0] = 0.329412f;
	gfMaterialAmbient[1] = 0.223529f;
	gfMaterialAmbient[2] = 0.027451f;
	gfMaterialAmbient[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_AMBIENT, gfMaterialAmbient);

	gfMaterialDiffuse[0] = 0.780392f;
	gfMaterialDiffuse[1] = 0.568627f;
	gfMaterialDiffuse[2] = 0.113725f;
	gfMaterialDiffuse[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_DIFFUSE, gfMaterialDiffuse);

	gfMaterialSpecular[0] = 0.992157f;
	gfMaterialSpecular[1] = 0.941176f;
	gfMaterialSpecular[2] = 0.807843f;
	gfMaterialSpecular[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_SPECULAR, gfMaterialSpecular);

	gfMaterialShyniness[0] = 0.21794872f * 128.0f;
	glMaterialfv(GL_FRONT, GL_SHININESS, gfMaterialShyniness);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(10.0f, 14.0f, 0.0f);

	gluSphere(gpQuadric[6], 1.0f, 30, 30);

	// Sphere 8
	gfMaterialAmbient[0] = 0.2125f;
	gfMaterialAmbient[1] = 0.1275f;
	gfMaterialAmbient[2] = 0.054f;
	gfMaterialAmbient[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_AMBIENT, gfMaterialAmbient);

	gfMaterialDiffuse[0] = 0.714f;
	gfMaterialDiffuse[1] = 0.4284f;
	gfMaterialDiffuse[2] = 0.1844f;
	gfMaterialDiffuse[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_DIFFUSE, gfMaterialDiffuse);

	gfMaterialSpecular[0] = 0.393548f;
	gfMaterialSpecular[1] = 0.271906f;
	gfMaterialSpecular[2] = 0.166721f;
	gfMaterialSpecular[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_SPECULAR, gfMaterialSpecular);

	gfMaterialShyniness[0] = 0.2f * 128.0f;
	glMaterialfv(GL_FRONT, GL_SHININESS, gfMaterialShyniness);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(10.0f, 11.5f, 0.0f);

	gluSphere(gpQuadric[7], 1.0f, 30, 30);


	// Sphere 9
	gfMaterialAmbient[0] = 0.25f;
	gfMaterialAmbient[1] = 0.25f;
	gfMaterialAmbient[2] = 0.25f;
	gfMaterialAmbient[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_AMBIENT, gfMaterialAmbient);

	gfMaterialDiffuse[0] = 0.4f;
	gfMaterialDiffuse[1] = 0.4f;
	gfMaterialDiffuse[2] = 0.4f;
	gfMaterialDiffuse[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_DIFFUSE, gfMaterialDiffuse);

	gfMaterialSpecular[0] = 0.774597f;
	gfMaterialSpecular[1] = 0.774597f;
	gfMaterialSpecular[2] = 0.774597f;
	gfMaterialSpecular[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_SPECULAR, gfMaterialSpecular);

	gfMaterialShyniness[0] = 0.6f * 128.0f;
	glMaterialfv(GL_FRONT, GL_SHININESS, gfMaterialShyniness);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(10.0f, 9.0f, 0.0f);

	gluSphere(gpQuadric[8], 1.0f, 30, 30);



	// Sphere 10
	gfMaterialAmbient[0] = 0.19125f;
	gfMaterialAmbient[1] = 0.0735f;
	gfMaterialAmbient[2] = 0.0225f;
	gfMaterialAmbient[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_AMBIENT, gfMaterialAmbient);

	gfMaterialDiffuse[0] = 0.7038f;
	gfMaterialDiffuse[1] = 0.27048f;
	gfMaterialDiffuse[2] = 0.0828f;
	gfMaterialDiffuse[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_DIFFUSE, gfMaterialDiffuse);

	gfMaterialSpecular[0] = 0.256777f;
	gfMaterialSpecular[1] = 0.137622f;
	gfMaterialSpecular[2] = 0.086014f;
	gfMaterialSpecular[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_SPECULAR, gfMaterialSpecular);

	gfMaterialShyniness[0] = 0.1f * 128.0f;
	glMaterialfv(GL_FRONT, GL_SHININESS, gfMaterialShyniness);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(10.0f, 6.5f, 0.0f);

	gluSphere(gpQuadric[9], 1.0f, 30, 30);


	// Sphere 11
	gfMaterialAmbient[0] = 0.24725f;
	gfMaterialAmbient[1] = 0.1995f;
	gfMaterialAmbient[2] = 0.0745f;
	gfMaterialAmbient[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_AMBIENT, gfMaterialAmbient);

	gfMaterialDiffuse[0] = 0.75164f;
	gfMaterialDiffuse[1] = 0.606148f;
	gfMaterialDiffuse[2] = 0.22648f;
	gfMaterialDiffuse[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_DIFFUSE, gfMaterialDiffuse);

	gfMaterialSpecular[0] = 0.628281f;
	gfMaterialSpecular[1] = 0.555802f;
	gfMaterialSpecular[2] = 0.366065f;
	gfMaterialSpecular[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_SPECULAR, gfMaterialSpecular);

	gfMaterialShyniness[0] = 0.4f * 128.0f;
	glMaterialfv(GL_FRONT, GL_SHININESS, gfMaterialShyniness);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(10.0f, 4.0f, 0.0f);

	gluSphere(gpQuadric[10], 1.0f, 30, 30);



	// Sphere 12
	gfMaterialAmbient[0] = 0.19225f;
	gfMaterialAmbient[1] = 0.19225f;
	gfMaterialAmbient[2] = 0.19225f;
	gfMaterialAmbient[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_AMBIENT, gfMaterialAmbient);

	gfMaterialDiffuse[0] = 0.50754f;
	gfMaterialDiffuse[1] = 0.50754f;
	gfMaterialDiffuse[2] = 0.50754f;
	gfMaterialDiffuse[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_DIFFUSE, gfMaterialDiffuse);

	gfMaterialSpecular[0] = 0.508273f;
	gfMaterialSpecular[1] = 0.508273f;
	gfMaterialSpecular[2] = 0.508273f;
	gfMaterialSpecular[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_SPECULAR, gfMaterialSpecular);

	gfMaterialShyniness[0] = 0.4f * 128.0f;
	glMaterialfv(GL_FRONT, GL_SHININESS, gfMaterialShyniness);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(10.0f, 1.5f, 0.0f);

	gluSphere(gpQuadric[11], 1.0f, 30, 30);





	// Sphere 13
	gfMaterialAmbient[0] = 0.0f;
	gfMaterialAmbient[1] = 0.0f;
	gfMaterialAmbient[2] = 0.0f;
	gfMaterialAmbient[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_AMBIENT, gfMaterialAmbient);

	gfMaterialDiffuse[0] = 0.01f;
	gfMaterialDiffuse[1] = 0.01f;
	gfMaterialDiffuse[2] = 0.01f;
	gfMaterialDiffuse[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_DIFFUSE, gfMaterialDiffuse);

	gfMaterialSpecular[0] = 0.50f;
	gfMaterialSpecular[1] = 0.50f;
	gfMaterialSpecular[2] = 0.50f;
	gfMaterialSpecular[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_SPECULAR, gfMaterialSpecular);

	gfMaterialShyniness[0] = 0.25f * 128.0f;
	glMaterialfv(GL_FRONT, GL_SHININESS, gfMaterialShyniness);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(18.5f, 14.0f, 0.0f);

	gluSphere(gpQuadric[12], 1.0f, 30, 30);

	// Sphere 14
	gfMaterialAmbient[0] = 0.0f;
	gfMaterialAmbient[1] = 0.1f;
	gfMaterialAmbient[2] = 0.06f;
	gfMaterialAmbient[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_AMBIENT, gfMaterialAmbient);

	gfMaterialDiffuse[0] = 0.0f;
	gfMaterialDiffuse[1] = 0.50980392f;
	gfMaterialDiffuse[2] = 0.50980392f;
	gfMaterialDiffuse[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_DIFFUSE, gfMaterialDiffuse);

	gfMaterialSpecular[0] = 0.50196078f;
	gfMaterialSpecular[1] = 0.50196078f;
	gfMaterialSpecular[2] = 0.50196078f;
	gfMaterialSpecular[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_SPECULAR, gfMaterialSpecular);

	gfMaterialShyniness[0] = 0.25f * 128.0f;
	glMaterialfv(GL_FRONT, GL_SHININESS, gfMaterialShyniness);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(18.5f, 11.5f, 0.0f);

	gluSphere(gpQuadric[13], 1.0f, 30, 30);


	// Sphere 15
	gfMaterialAmbient[0] = 0.0f;
	gfMaterialAmbient[1] = 0.0f;
	gfMaterialAmbient[2] = 0.0f;
	gfMaterialAmbient[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_AMBIENT, gfMaterialAmbient);

	gfMaterialDiffuse[0] = 0.1f;
	gfMaterialDiffuse[1] = 0.35f;
	gfMaterialDiffuse[2] = 0.1f;
	gfMaterialDiffuse[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_DIFFUSE, gfMaterialDiffuse);

	gfMaterialSpecular[0] = 0.45f;
	gfMaterialSpecular[1] = 0.55f;
	gfMaterialSpecular[2] = 0.45f;
	gfMaterialSpecular[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_SPECULAR, gfMaterialSpecular);

	gfMaterialShyniness[0] = 0.25f * 128.0f;
	glMaterialfv(GL_FRONT, GL_SHININESS, gfMaterialShyniness);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(18.5f, 9.0f, 0.0f);

	gluSphere(gpQuadric[14], 1.0f, 30, 30);



	// Sphere 16
	gfMaterialAmbient[0] = 0.0f;
	gfMaterialAmbient[1] = 0.0f;
	gfMaterialAmbient[2] = 0.0f;
	gfMaterialAmbient[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_AMBIENT, gfMaterialAmbient);

	gfMaterialDiffuse[0] = 0.5f;
	gfMaterialDiffuse[1] = 0.0f;
	gfMaterialDiffuse[2] = 0.0f;
	gfMaterialDiffuse[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_DIFFUSE, gfMaterialDiffuse);

	gfMaterialSpecular[0] = 0.7f;
	gfMaterialSpecular[1] = 0.6f;
	gfMaterialSpecular[2] = 0.6f;
	gfMaterialSpecular[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_SPECULAR, gfMaterialSpecular);

	gfMaterialShyniness[0] = 0.25f * 128.0f;
	glMaterialfv(GL_FRONT, GL_SHININESS, gfMaterialShyniness);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(18.5f, 6.5f, 0.0f);

	gluSphere(gpQuadric[15], 1.0f, 30, 30);


	// Sphere 17
	gfMaterialAmbient[0] = 0.0f;
	gfMaterialAmbient[1] = 0.0f;
	gfMaterialAmbient[2] = 0.0f;
	gfMaterialAmbient[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_AMBIENT, gfMaterialAmbient);

	gfMaterialDiffuse[0] = 0.55f;
	gfMaterialDiffuse[1] = 0.55f;
	gfMaterialDiffuse[2] = 0.55f;
	gfMaterialDiffuse[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_DIFFUSE, gfMaterialDiffuse);

	gfMaterialSpecular[0] = 0.70f;
	gfMaterialSpecular[1] = 0.70f;
	gfMaterialSpecular[2] = 0.70f;
	gfMaterialSpecular[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_SPECULAR, gfMaterialSpecular);

	gfMaterialShyniness[0] = 0.25f * 128.0f;
	glMaterialfv(GL_FRONT, GL_SHININESS, gfMaterialShyniness);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(18.5f, 4.0f, 0.0f);

	gluSphere(gpQuadric[16], 1.0f, 30, 30);



	// Sphere 18
	gfMaterialAmbient[0] = 0.0f;
	gfMaterialAmbient[1] = 0.0f;
	gfMaterialAmbient[2] = 0.0f;
	gfMaterialAmbient[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_AMBIENT, gfMaterialAmbient);

	gfMaterialDiffuse[0] = 0.5f;
	gfMaterialDiffuse[1] = 0.5f;
	gfMaterialDiffuse[2] = 0.0f;
	gfMaterialDiffuse[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_DIFFUSE, gfMaterialDiffuse);

	gfMaterialSpecular[0] = 0.60f;
	gfMaterialSpecular[1] = 0.60f;
	gfMaterialSpecular[2] = 0.50f;
	gfMaterialSpecular[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_SPECULAR, gfMaterialSpecular);

	gfMaterialShyniness[0] = 0.25f * 128.0f;
	glMaterialfv(GL_FRONT, GL_SHININESS, gfMaterialShyniness);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(18.5f, 1.5f, 0.0f);

	gluSphere(gpQuadric[17], 1.0f, 30, 30);





	// Sphere 19
	gfMaterialAmbient[0] = 0.02f;
	gfMaterialAmbient[1] = 0.02f;
	gfMaterialAmbient[2] = 0.02f;
	gfMaterialAmbient[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_AMBIENT, gfMaterialAmbient);

	gfMaterialDiffuse[0] = 0.01f;
	gfMaterialDiffuse[1] = 0.01f;
	gfMaterialDiffuse[2] = 0.01f;
	gfMaterialDiffuse[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_DIFFUSE, gfMaterialDiffuse);

	gfMaterialSpecular[0] = 0.40f;
	gfMaterialSpecular[1] = 0.40f;
	gfMaterialSpecular[2] = 0.40f;
	gfMaterialSpecular[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_SPECULAR, gfMaterialSpecular);

	gfMaterialShyniness[0] = 0.078125f * 128.0f;
	glMaterialfv(GL_FRONT, GL_SHININESS, gfMaterialShyniness);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(25.0f, 14.0f, 0.0f);

	gluSphere(gpQuadric[18], 1.0f, 30, 30);

	// Sphere 20
	gfMaterialAmbient[0] = 0.0f;
	gfMaterialAmbient[1] = 0.05f;
	gfMaterialAmbient[2] = 0.05f;
	gfMaterialAmbient[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_AMBIENT, gfMaterialAmbient);

	gfMaterialDiffuse[0] = 0.4f;
	gfMaterialDiffuse[1] = 0.5f;
	gfMaterialDiffuse[2] = 0.5f;
	gfMaterialDiffuse[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_DIFFUSE, gfMaterialDiffuse);

	gfMaterialSpecular[0] = 0.04f;
	gfMaterialSpecular[1] = 0.7f;
	gfMaterialSpecular[2] = 0.7f;
	gfMaterialSpecular[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_SPECULAR, gfMaterialSpecular);

	gfMaterialShyniness[0] = 0.078125f * 128.0f;
	glMaterialfv(GL_FRONT, GL_SHININESS, gfMaterialShyniness);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(25.0f, 11.5f, 0.0f);

	gluSphere(gpQuadric[19], 1.0f, 30, 30);


	// Sphere 21
	gfMaterialAmbient[0] = 0.0f;
	gfMaterialAmbient[1] = 0.05f;
	gfMaterialAmbient[2] = 0.0f;
	gfMaterialAmbient[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_AMBIENT, gfMaterialAmbient);

	gfMaterialDiffuse[0] = 0.4f;
	gfMaterialDiffuse[1] = 0.5f;
	gfMaterialDiffuse[2] = 0.4f;
	gfMaterialDiffuse[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_DIFFUSE, gfMaterialDiffuse);

	gfMaterialSpecular[0] = 0.04f;
	gfMaterialSpecular[1] = 0.7f;
	gfMaterialSpecular[2] = 0.04f;
	gfMaterialSpecular[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_SPECULAR, gfMaterialSpecular);

	gfMaterialShyniness[0] = 0.078125f * 128.0f;
	glMaterialfv(GL_FRONT, GL_SHININESS, gfMaterialShyniness);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(25.0f, 9.0f, 0.0f);

	gluSphere(gpQuadric[20], 1.0f, 30, 30);



	// Sphere 22
	gfMaterialAmbient[0] = 0.05f;
	gfMaterialAmbient[1] = 0.0f;
	gfMaterialAmbient[2] = 0.0f;
	gfMaterialAmbient[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_AMBIENT, gfMaterialAmbient);

	gfMaterialDiffuse[0] = 0.5f;
	gfMaterialDiffuse[1] = 0.4f;
	gfMaterialDiffuse[2] = 0.4f;
	gfMaterialDiffuse[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_DIFFUSE, gfMaterialDiffuse);

	gfMaterialSpecular[0] = 0.7f;
	gfMaterialSpecular[1] = 0.04f;
	gfMaterialSpecular[2] = 0.04f;
	gfMaterialSpecular[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_SPECULAR, gfMaterialSpecular);

	gfMaterialShyniness[0] = 0.078125f * 128.0f;
	glMaterialfv(GL_FRONT, GL_SHININESS, gfMaterialShyniness);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(25.0f, 6.5f, 0.0f);

	gluSphere(gpQuadric[21], 1.0f, 30, 30);


	// Sphere 23
	gfMaterialAmbient[0] = 0.05f;
	gfMaterialAmbient[1] = 0.05f;
	gfMaterialAmbient[2] = 0.05f;
	gfMaterialAmbient[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_AMBIENT, gfMaterialAmbient);

	gfMaterialDiffuse[0] = 0.5f;
	gfMaterialDiffuse[1] = 0.5f;
	gfMaterialDiffuse[2] = 0.5f;
	gfMaterialDiffuse[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_DIFFUSE, gfMaterialDiffuse);

	gfMaterialSpecular[0] = 0.70f;
	gfMaterialSpecular[1] = 0.70f;
	gfMaterialSpecular[2] = 0.70f;
	gfMaterialSpecular[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_SPECULAR, gfMaterialSpecular);

	gfMaterialShyniness[0] = 0.078125f * 128.0f;
	glMaterialfv(GL_FRONT, GL_SHININESS, gfMaterialShyniness);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(25.0f, 4.0f, 0.0f);

	gluSphere(gpQuadric[22], 1.0f, 30, 30);



	// Sphere 18
	gfMaterialAmbient[0] = 0.05f;
	gfMaterialAmbient[1] = 0.05f;
	gfMaterialAmbient[2] = 0.0f;
	gfMaterialAmbient[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_AMBIENT, gfMaterialAmbient);

	gfMaterialDiffuse[0] = 0.5f;
	gfMaterialDiffuse[1] = 0.5f;
	gfMaterialDiffuse[2] = 0.4f;
	gfMaterialDiffuse[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_DIFFUSE, gfMaterialDiffuse);

	gfMaterialSpecular[0] = 0.7f;
	gfMaterialSpecular[1] = 0.7f;
	gfMaterialSpecular[2] = 0.04f;
	gfMaterialSpecular[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_SPECULAR, gfMaterialSpecular);

	gfMaterialShyniness[0] = 0.078125f * 128.0f;
	glMaterialfv(GL_FRONT, GL_SHININESS, gfMaterialShyniness);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(25.0f, 1.5f, 0.0f);

	gluSphere(gpQuadric[23], 1.0f, 30, 30);

}


