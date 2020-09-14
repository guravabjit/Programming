
#include <iostream> // iostream is cache of namespaces
#include <stdio.h> // printf()
#include <stdlib.h> // exit()
#include <memory.h> // memset()

#include <X11/Xlib.h> // windows.h // xserver api
#include <X11/Xutil.h> // xvisualinfo structure
#include <X11/XKBlib.h> // keyboard
#include <X11/keysym.h> // key symbol 

#include <GL/glew.h>

#include <GL/glx.h>
#include <GL/gl.h>
#include "vmath.h"

#define ERROR_SIZE 1024

using namespace std;
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

GLenum gGlEnumResult;
GLuint gGlShaderProgramObject;

GLuint vao;
GLuint vboPosition;
GLuint vboColor;
GLuint mvpUniform;
mat4 perspectiveProjectionMatrix;

GLuint vboPointElement;
int giPointElements = 0;

GLuint vboLinesElement;
int giLinesElements = 0;

GLuint vboSquareElement;
int giSquareElements = 0;

GLuint vboDiagonalSquareElement;
int giDigonalSquareElements = 0;

GLuint vboFanElement;
int giFanElements = 0;

GLuint vaoClrSqr;
GLuint vboColorSquare;
GLuint vboColorSquareElement;
int giColorSquareElements = 0;


bool gbFullScreen = false;
Display *gpDisplay = NULL;
XVisualInfo *gpXVisualInfo = NULL;
Colormap gColorMap;
Window gWindow;

typedef GLXContext(*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);
glXCreateContextAttribsARBProc glXCreateContextAttribsARB = NULL;
GLXFBConfig gglxFBConfig;
//glxContext gglxContext;

int giWindowWidth = 800;
int giWindowHeight = 600;

FILE* gpLogFile = NULL;

static GLXContext gGlxContext; // rendering context

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
			update();
			display();
		}
	}
	
	uninitialize();
	return (0);
}

void CreateWindow()
{
	void uninitialize();

	GLXFBConfig *pglxFBConfig = NULL;
	GLXFBConfig bestglxFBConfig;
	XVisualInfo *pTempXVisualInfo = NULL;
	int iNumberOfFBConfigs = 0;

	XSetWindowAttributes winAttribs;
	int defaultScreen;
	int defaultDepth;
	int styleMask;
	static int iFrameBufferAttributes[] = {
		GLX_X_RENDERABLE, True,
		GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
		GLX_RENDER_TYPE, GLX_RGBA_BIT,
		GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
		GLX_RED_SIZE, 8,
		GLX_GREEN_SIZE, 8,
		GLX_BLUE_SIZE, 8,
		GLX_ALPHA_SIZE, 8,
		GLX_DEPTH_SIZE, 24,
		GLX_STENCIL_SIZE, 8,
		GLX_DOUBLEBUFFER, True,
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

	/*gpXVisualInfo = glXChooseVisual(gpDisplay, defaultScreen, iFrameBufferAttributes);
	if(NULL == gpXVisualInfo)
	{
		printf("glxChooseVisual returns NULL\n");
		uninitialize();
		exit(1);
	}*/

	pglxFBConfig = glXChooseFBConfig(gpDisplay, defaultScreen, iFrameBufferAttributes, &iNumberOfFBConfigs);
	printf("There are %d matching fbconfigs\n",iNumberOfFBConfigs);
	
	int iBestFrameBufferConfig = -1;
	int iBestNumberOfSamples = -1;
	int iWorstFrameBufferConfig = -1;
	int iWorstNumberOfSamples = 999;
	int sampleBuffers, samples;

	for(int i = 0; i < iNumberOfFBConfigs; i++)
	{
		pTempXVisualInfo = glXGetVisualFromFBConfig(gpDisplay, pglxFBConfig[i]);
		if(pTempXVisualInfo)
		{
			glXGetFBConfigAttrib(gpDisplay,pglxFBConfig[i], GLX_SAMPLE_BUFFERS, &sampleBuffers);
			glXGetFBConfigAttrib(gpDisplay,pglxFBConfig[i], GLX_SAMPLES, &samples);
			printf("Matching Framebuffer Config=%d : Visual ID=0x%lu : SAMPLE_BUFFERS=%d : SAMPLES=%d\n",i,pTempXVisualInfo->visualid,sampleBuffers,samples);

			if(iBestFrameBufferConfig < 0 || sampleBuffers && samples > iBestNumberOfSamples)
			{
				iBestFrameBufferConfig = i;
				iBestNumberOfSamples = samples;
			}
			if(iWorstFrameBufferConfig < 0 || (!sampleBuffers) || samples < iWorstNumberOfSamples)
			{
				iWorstFrameBufferConfig = i;
				iWorstNumberOfSamples = samples;
			}
			XFree(pTempXVisualInfo);
		}
	}

	bestglxFBConfig = pglxFBConfig[iBestFrameBufferConfig];
	gglxFBConfig = bestglxFBConfig;
	XFree(pglxFBConfig);

	gpXVisualInfo = glXGetVisualFromFBConfig(gpDisplay, bestglxFBConfig);
	printf("Chosen Visual ID=0x%lu\n",gpXVisualInfo->visualid );

/*        glxContextCreateContextAttribARB = (glxContextCreateContextAttribARBProc)glXGetProcAddressARB((GLubyte*)"glxCreateContextAttribsARB");
        if(NULL == glxContextCreateContextAttribARB)
        {
                printf("failed to get address of glxCreateContextAttribsARB\n");
                uninitialize();
                exit(0);
        }

        GLint pAttribs[] = {
                        GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
                        GLX_CONTEXT_MINOR_VERSION_ARB, 5,
                        GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
		//	GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
                        0 };

	if(!gGlxContext)
	{
        	gGlxContext = glxContextCreateContextAttribARB(gpDisplay, gglxFBConfig, 0, True, pAttribs);
	}
	
	if(!glXIsDirect(gpDisplay, gGlxContext))
	{
		printf("Obtained context is not HW context\n");
	}
	else
	{
		printf("Obtained context is HW context\n");
	}

        glXMakeCurrent(gpDisplay, gWindow, gGlxContext);

	gGlEnumResult = glewInit();
	if (GLEW_OK != gGlEnumResult)
	{
		fprintf(gpLogFile, "glewInit() failed.\n");
	//	return -5;
	}

*/
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
	
	XStoreName(gpDisplay, gWindow, "Ortho");
	
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

	if (vboPosition)
	{
		glDeleteBuffers(1, &vboPosition);
	}

	if (vboColor)
	{
		glDeleteBuffers(1, &vboColor);
	}

	if (vao)
	{
		glDeleteVertexArrays(1, &vao);
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
	void resize(int, int);
	GLboolean GetGlErrorStatus(GLGetErrorType enErrorType, GLuint uiErrorObject, char* pszErrorStrPrefix);;
	
	char szglErrorStr[ERROR_SIZE] = "";
	GLboolean bErrorStatus = GL_TRUE;
	GLuint gGlVertexShaderObject;
	GLuint gGlFragmentShaderObject;

	//gGlxContext = glXCreateContext(gpDisplay, gpXVisualInfo, NULL, True);
        glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)glXGetProcAddressARB((GLubyte*)"glXCreateContextAttribsARB");
        if(NULL == glXCreateContextAttribsARB)
        {
                printf("failed to get address of glxCreateContextAttribsARB\n");
                uninitialize();
                exit(0);
        }

        GLint pAttribs[] = {
                        GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
                        GLX_CONTEXT_MINOR_VERSION_ARB, 6,
                        GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
                //      GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
                        0 };

	gGlxContext = glXCreateContextAttribsARB(gpDisplay, gglxFBConfig, 0, True, pAttribs);
        if(!gGlxContext)
        {
		printf("Failed To Create GLX 4.6 context. Hence Using Old-Style GLX Context\n");
        	GLint pAttribs[] = {
                        GLX_CONTEXT_MAJOR_VERSION_ARB, 1,
                        GLX_CONTEXT_MINOR_VERSION_ARB, 0,
                        0 };
                gGlxContext = glXCreateContextAttribsARB(gpDisplay, gglxFBConfig, 0, True, pAttribs);
        }
        
        if(!glXIsDirect(gpDisplay, gGlxContext))
        {
                printf("Obtained context is not HW context\n");
        }
        else
        {
                printf("Obtained context is HW context\n");
        }

        glXMakeCurrent(gpDisplay, gWindow, gGlxContext);

        gGlEnumResult = glewInit();
        if (GLEW_OK != gGlEnumResult)
        {
                fprintf(gpLogFile, "glewInit() failed.\n");
        //      return -5;
        }



	glXMakeCurrent(gpDisplay, gWindow, gGlxContext);


        gGlEnumResult = glewInit();
        if (GLEW_OK != gGlEnumResult)
        {
                fprintf(gpLogFile, "glewInit() failed.\n");
		uninitialize();
		exit(0);
        }

	// define vertex shader object
	gGlVertexShaderObject = glCreateShader(GL_VERTEX_SHADER);

	// write vertex shader code
	const GLchar *pszVertexShaderSourceCode =
		"#version 450 core" \
		"\n" \
		"in vec4 vPosition;" \
		"in vec4 vColor;" \
		"uniform mat4 u_mvp_matrix;" \
		"out vec4 out_Color;" \
		"void main(void)" \
		"{" \
		"gl_Position = u_mvp_matrix * vPosition;" \
		"out_Color = vColor;" \
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
	strcpy(szglErrorStr, "Error in vertex shader object code : ");
	bErrorStatus = GetGlErrorStatus(enGlGET_Shader, gGlVertexShaderObject, szglErrorStr);
	if (GL_FALSE == bErrorStatus)
	{
		fprintf(gpLogFile, "%s.\n", szglErrorStr);
		return ;
	}

	// define fragment shader object
	gGlFragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);

	// write fragment shader code
	const GLchar *pszFragmentShaderSourceCode[] = {
		"#version 450 core" \
		"\n" \
		"in vec4 out_Color;" \
		"out vec4 fragColor;" \
		"void main(void)" \
		"{"	\
		"fragColor = out_Color;" \
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
	strcpy(szglErrorStr,  "Error in fragment shader object code : ");
	bErrorStatus = GetGlErrorStatus(enGlGET_Shader, gGlFragmentShaderObject, szglErrorStr);
	if (GL_FALSE == bErrorStatus)
	{
		fprintf(gpLogFile, "%s.\n", szglErrorStr);
		return ;
	}

	// create shader program object
	gGlShaderProgramObject = glCreateProgram();

	// attach vertex shader to shader program
	glAttachShader(gGlShaderProgramObject, gGlVertexShaderObject);

	// attach fragment shader to shader program
	glAttachShader(gGlShaderProgramObject, gGlFragmentShaderObject);

	// prelinking binding to vertex attributes
	glBindAttribLocation(gGlShaderProgramObject, AMC_ATTRIBUTE_POSITION, "vPosition");

	// link the shader program
	glLinkProgram(gGlShaderProgramObject);

	// Error code checking function by AG
	memset(szglErrorStr, 0, ERROR_SIZE);
	strcpy(szglErrorStr, "Error in program object : ");
	bErrorStatus = GetGlErrorStatus(enGlGET_Program, gGlShaderProgramObject, szglErrorStr);
	if (GL_FALSE == bErrorStatus)
	{
		fprintf(gpLogFile, "%s.\n", szglErrorStr);
		return ;
	}

	// post linking retreving uniform location
	mvpUniform = glGetUniformLocation(gGlShaderProgramObject, "u_mvp_matrix");

	// glBegin() ... glEnd() // vertices, colors, texture co-ordinates, normals // make their array
	float vertices[] = {
		-1.0f, 1.0f, 0.0f,
		-0.5f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.5f, 1.0f, 0.0f,

		-1.0f, 0.5f, 0.0f,
		-0.5f, 0.5f, 0.0f,
		0.0f, 0.5f, 0.0f,
		0.5f, 0.5f, 0.0f,

		-1.0f, 0.0f, 0.0f,
		-0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		0.5f, 0.0f, 0.0f,

		-1.0f, -0.5f, 0.0f,
		-0.5f, -0.5f, 0.0f,
		0.0f, -0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
	};

	float fColorSquareVertices[] = {
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 1.0f,

		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 1.0f,

		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 1.0f,

		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 1.0f,

	};

	float colorVertices[48] = { 0.0f };
	for (int iItr = 0; iItr < 48; iItr++)
	{
		colorVertices[iItr] = 1.0f;
	}

	// create vao
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(1, &vboPosition);
	glBindBuffer(GL_ARRAY_BUFFER, vboPosition);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);

	// GL_POINTS indices
	int iPointIndices[16];
	giPointElements = 16;
	for (int iItr = 0; iItr < 16; iItr++)
	{
		iPointIndices[iItr] = iItr;
	}
	glGenBuffers(1, &vboPointElement);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboPointElement);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(iPointIndices), iPointIndices, GL_STATIC_DRAW);
	
	int iLineIndices[30];
	int iIndex = 0;
	iLineIndices[iIndex++] = 0;
	iLineIndices[iIndex++] = 12;
	iLineIndices[iIndex++] = 1;
	iLineIndices[iIndex++] = 13;
	iLineIndices[iIndex++] = 2;
	iLineIndices[iIndex++] = 14;

	iLineIndices[iIndex++] = 0;
	iLineIndices[iIndex++] = 3;
	iLineIndices[iIndex++] = 4;
	iLineIndices[iIndex++] = 7;
	iLineIndices[iIndex++] = 8;
	iLineIndices[iIndex++] = 11;

	iLineIndices[iIndex++] = 4;
	iLineIndices[iIndex++] = 1;
	iLineIndices[iIndex++] = 8;
	iLineIndices[iIndex++] = 2;
	iLineIndices[iIndex++] = 12;
	iLineIndices[iIndex++] = 3;
	iLineIndices[iIndex++] = 13;
	iLineIndices[iIndex++] = 7;
	iLineIndices[iIndex++] = 14;
	iLineIndices[iIndex++] = 11;

	giLinesElements = iIndex;
	glGenBuffers(1, &vboLinesElement);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboLinesElement);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(iLineIndices), iLineIndices, GL_STATIC_DRAW);

	int iSquareIndices[30];
	iIndex = 0;
	iSquareIndices[iIndex++] = 0;
	iSquareIndices[iIndex++] = 12;
	iSquareIndices[iIndex++] = 1;
	iSquareIndices[iIndex++] = 13;
	iSquareIndices[iIndex++] = 2;
	iSquareIndices[iIndex++] = 14;
	iSquareIndices[iIndex++] = 3;
	iSquareIndices[iIndex++] = 15;

	iSquareIndices[iIndex++] = 0;
	iSquareIndices[iIndex++] = 3;
	iSquareIndices[iIndex++] = 4;
	iSquareIndices[iIndex++] = 7;
	iSquareIndices[iIndex++] = 8;
	iSquareIndices[iIndex++] = 11;
	iSquareIndices[iIndex++] = 12;
	iSquareIndices[iIndex++] = 15;

	giSquareElements = iIndex;
	glGenBuffers(1, &vboSquareElement);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSquareElement);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(iSquareIndices), iSquareIndices, GL_STATIC_DRAW);


	int iDiagonalSquareIndices[30];
	iIndex = 0;
	iDiagonalSquareIndices[iIndex++] = 0;
	iDiagonalSquareIndices[iIndex++] = 12;
	iDiagonalSquareIndices[iIndex++] = 1;
	iDiagonalSquareIndices[iIndex++] = 13;
	iDiagonalSquareIndices[iIndex++] = 2;
	iDiagonalSquareIndices[iIndex++] = 14;
	iDiagonalSquareIndices[iIndex++] = 3;
	iDiagonalSquareIndices[iIndex++] = 15;

	iDiagonalSquareIndices[iIndex++] = 0;
	iDiagonalSquareIndices[iIndex++] = 3;
	iDiagonalSquareIndices[iIndex++] = 4;
	iDiagonalSquareIndices[iIndex++] = 7;
	iDiagonalSquareIndices[iIndex++] = 8;
	iDiagonalSquareIndices[iIndex++] = 11;
	iDiagonalSquareIndices[iIndex++] = 12;
	iDiagonalSquareIndices[iIndex++] = 15;

	iDiagonalSquareIndices[iIndex++] = 4;
	iDiagonalSquareIndices[iIndex++] = 1;
	iDiagonalSquareIndices[iIndex++] = 8;
	iDiagonalSquareIndices[iIndex++] = 2;
	iDiagonalSquareIndices[iIndex++] = 12;
	iDiagonalSquareIndices[iIndex++] = 3;
	iDiagonalSquareIndices[iIndex++] = 13;
	iDiagonalSquareIndices[iIndex++] = 7;
	iDiagonalSquareIndices[iIndex++] = 14;
	iDiagonalSquareIndices[iIndex++] = 11;

	giDigonalSquareElements = iIndex;
	glGenBuffers(1, &vboDiagonalSquareElement);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboDiagonalSquareElement);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(iDiagonalSquareIndices), iDiagonalSquareIndices, GL_STATIC_DRAW);


	int iFanIndices[30];
	iIndex = 0;
	iFanIndices[iIndex++] = 0;
	iFanIndices[iIndex++] = 12;
	iFanIndices[iIndex++] = 0;
	iFanIndices[iIndex++] = 13;
	iFanIndices[iIndex++] = 0;
	iFanIndices[iIndex++] = 14;
	iFanIndices[iIndex++] = 0;
	iFanIndices[iIndex++] = 15;
	iFanIndices[iIndex++] = 0;
	iFanIndices[iIndex++] = 11;
	iFanIndices[iIndex++] = 0;
	iFanIndices[iIndex++] = 7;
	iFanIndices[iIndex++] = 0;
	iFanIndices[iIndex++] = 3;
	iFanIndices[iIndex++] = 12;
	iFanIndices[iIndex++] = 15;
	iFanIndices[iIndex++] = 15;
	iFanIndices[iIndex++] = 3;

	giFanElements = iIndex;
	glGenBuffers(1, &vboFanElement);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboFanElement);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(iFanIndices), iFanIndices, GL_STATIC_DRAW);

	glGenBuffers(1, &vboColor);
	glBindBuffer(GL_ARRAY_BUFFER, vboColor);
	glBufferData(GL_ARRAY_BUFFER, sizeof(colorVertices), colorVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);

	glBindBuffer(GL_ARRAY_BUFFER, 0);


	glGenVertexArrays(1, &vaoClrSqr);
	glBindVertexArray(vaoClrSqr);
	glGenBuffers(1, &vboPosition);
	glBindBuffer(GL_ARRAY_BUFFER, vboPosition);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);

	int iColorSquareIndices[300];
	iIndex = 0;
	iColorSquareIndices[iIndex++] = 1;
	iColorSquareIndices[iIndex++] = 0;
	iColorSquareIndices[iIndex++] = 4;

	iColorSquareIndices[iIndex++] = 1;
	iColorSquareIndices[iIndex++] = 4;
	iColorSquareIndices[iIndex++] = 5;

	iColorSquareIndices[iIndex++] = 5;
	iColorSquareIndices[iIndex++] = 4;
	iColorSquareIndices[iIndex++] = 8;

	iColorSquareIndices[iIndex++] = 5;
	iColorSquareIndices[iIndex++] = 8;
	iColorSquareIndices[iIndex++] = 9;

	iColorSquareIndices[iIndex++] = 9;
	iColorSquareIndices[iIndex++] = 8;
	iColorSquareIndices[iIndex++] = 12;

	iColorSquareIndices[iIndex++] = 9;
	iColorSquareIndices[iIndex++] = 12;
	iColorSquareIndices[iIndex++] = 13;

	iColorSquareIndices[iIndex++] = 2;
	iColorSquareIndices[iIndex++] = 1;
	iColorSquareIndices[iIndex++] = 5;

	iColorSquareIndices[iIndex++] = 2;
	iColorSquareIndices[iIndex++] = 5;
	iColorSquareIndices[iIndex++] = 6;

	iColorSquareIndices[iIndex++] = 6;
	iColorSquareIndices[iIndex++] = 5;
	iColorSquareIndices[iIndex++] = 9;

	iColorSquareIndices[iIndex++] = 6;
	iColorSquareIndices[iIndex++] = 9;
	iColorSquareIndices[iIndex++] = 10;

	iColorSquareIndices[iIndex++] = 10;
	iColorSquareIndices[iIndex++] = 9;
	iColorSquareIndices[iIndex++] = 13;

	iColorSquareIndices[iIndex++] = 10;
	iColorSquareIndices[iIndex++] = 13;
	iColorSquareIndices[iIndex++] = 14;


	iColorSquareIndices[iIndex++] = 3;
	iColorSquareIndices[iIndex++] = 2;
	iColorSquareIndices[iIndex++] = 6;

	iColorSquareIndices[iIndex++] = 3;
	iColorSquareIndices[iIndex++] = 6;
	iColorSquareIndices[iIndex++] = 7;

	iColorSquareIndices[iIndex++] = 7;
	iColorSquareIndices[iIndex++] = 6;
	iColorSquareIndices[iIndex++] = 10;

	iColorSquareIndices[iIndex++] = 7;
	iColorSquareIndices[iIndex++] = 10;
	iColorSquareIndices[iIndex++] = 11;

	iColorSquareIndices[iIndex++] = 11;
	iColorSquareIndices[iIndex++] = 10;
	iColorSquareIndices[iIndex++] = 14;

	iColorSquareIndices[iIndex++] = 11;
	iColorSquareIndices[iIndex++] = 14;
	iColorSquareIndices[iIndex++] = 15;


	/*iColorSquareIndices[iIndex++] = 0;
	iColorSquareIndices[iIndex++] = 3;
	iColorSquareIndices[iIndex++] = 4;
	iColorSquareIndices[iIndex++] = 7;
	iColorSquareIndices[iIndex++] = 8;
	iColorSquareIndices[iIndex++] = 11;
	iColorSquareIndices[iIndex++] = 12;
	iColorSquareIndices[iIndex++] = 15;*/

	giColorSquareElements = iIndex;
	glGenBuffers(1, &vboColorSquareElement);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboColorSquareElement);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(iColorSquareIndices), iColorSquareIndices, GL_STATIC_DRAW);

	glGenBuffers(1, &vboColorSquare);
	glBindBuffer(GL_ARRAY_BUFFER, vboColorSquare);
	glBufferData(GL_ARRAY_BUFFER, sizeof(fColorSquareVertices), fColorSquareVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);
	glBindBuffer(GL_ARRAY_BUFFER, 0);


	
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	glClearDepth(1.0f);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glShadeModel(GL_SMOOTH);
	// usual openGL initialization code
	//
	resize(giWindowHeight, giWindowWidth);
}

void resize(int iWidth, int iHeight)
{
	// usual resize code
	glViewport(0, 0, (GLsizei)iWidth, (GLsizei)iHeight);

	if (0 == iHeight)
	{
		iHeight = 1;
	}

     perspectiveProjectionMatrix = perspective(45.0f,
		(GLfloat)iWidth / (GLfloat)iHeight,
		0.1f,
		100.0f);

}

void display(void)
{

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(gGlShaderProgramObject);

	// declaration of metrices
	mat4 modelViewMatrix;
	mat4 translationMatrix;
	mat4 rotationMatrix;
	mat4 scaleMatrix;
	mat4 modelViewProjectionMatrix;

	// initializing above metrices to identity
	modelViewMatrix = mat4::identity();
	modelViewProjectionMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	rotationMatrix = mat4::identity();
	scaleMatrix = mat4::identity();

	// do neccessry transformation, rotation, scaling
	translationMatrix = translate(-3.0f, 1.0f, -7.0f);

	// do necessary matrix multiplication
	modelViewProjectionMatrix = perspectiveProjectionMatrix * ((translationMatrix * scaleMatrix)*rotationMatrix); //modelViewMatrix = (translationMatrix * scaleMatrix) * rotationMatrix;

	// send necessary metrices to shader in respective uniforms
	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);

	// bind with vao // this will avoid repetdly binding with vbo
	glBindVertexArray(vao);

	// similary bind with texture if any
	glPointSize(2.5f);
	// draw the necessary scene 
	//glDrawArrays(GL_POINTS, 0, 16);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboPointElement);
	glDrawElements(GL_POINTS, giPointElements, GL_UNSIGNED_INT, 0/*  */);

	// unbind vao
	glBindVertexArray(0);

	// initializing above metrices to identity
	modelViewMatrix = mat4::identity();
	modelViewProjectionMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	rotationMatrix = mat4::identity();
	scaleMatrix = mat4::identity();

	// do neccessry transformation, rotation, scaling
	translationMatrix = translate(0.0f, 1.0f, -7.0f);

	// do necessary matrix multiplication
	modelViewProjectionMatrix = perspectiveProjectionMatrix * ((translationMatrix * scaleMatrix)*rotationMatrix); //modelViewMatrix = (translationMatrix * scaleMatrix) * rotationMatrix;

	// send necessary metrices to shader in respective uniforms
	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);

	// bind with vao // this will avoid repetdly binding with vbo
	glBindVertexArray(vao);

	// similary bind with texture if any

	// draw the necessary scene 
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboLinesElement);
	glDrawElements(GL_LINES, giLinesElements, GL_UNSIGNED_INT, 0/*  */);

	// unbind vao
	glBindVertexArray(0);

	// initializing above metrices to identity
	modelViewMatrix = mat4::identity();
	modelViewProjectionMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	rotationMatrix = mat4::identity();
	scaleMatrix = mat4::identity();

	// do neccessry transformation, rotation, scaling
	translationMatrix = translate(2.5f, 1.0f, -7.0f);

	// do necessary matrix multiplication
	modelViewProjectionMatrix = perspectiveProjectionMatrix * ((translationMatrix * scaleMatrix)*rotationMatrix); //modelViewMatrix = (translationMatrix * scaleMatrix) * rotationMatrix;

	// send necessary metrices to shader in respective uniforms
	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);

	// bind with vao // this will avoid repetdly binding with vbo
	glBindVertexArray(vao);

	// similary bind with texture if any

	// draw the necessary scene 
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSquareElement);
	glDrawElements(GL_LINES, giSquareElements, GL_UNSIGNED_INT, 0/*  */);

	// unbind vao
	glBindVertexArray(0);

	// initializing above metrices to identity
	modelViewMatrix = mat4::identity();
	modelViewProjectionMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	rotationMatrix = mat4::identity();
	scaleMatrix = mat4::identity();

	// do neccessry transformation, rotation, scaling
	translationMatrix = translate(-3.0f, -1.0f, -7.0f);

	// do necessary matrix multiplication
	modelViewProjectionMatrix = perspectiveProjectionMatrix * ((translationMatrix * scaleMatrix)*rotationMatrix); //modelViewMatrix = (translationMatrix * scaleMatrix) * rotationMatrix;

	// send necessary metrices to shader in respective uniforms
	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);

	// bind with vao // this will avoid repetdly binding with vbo
	glBindVertexArray(vao);

	// similary bind with texture if any

	// draw the necessary scene 
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboDiagonalSquareElement);
	glDrawElements(GL_LINES, giDigonalSquareElements, GL_UNSIGNED_INT, 0/*  */);

	// unbind vao
	glBindVertexArray(0);

	// initializing above metrices to identity
	modelViewMatrix = mat4::identity();
	modelViewProjectionMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	rotationMatrix = mat4::identity();
	scaleMatrix = mat4::identity();

	// do neccessry transformation, rotation, scaling
	translationMatrix = translate(0.0f, -1.0f, -7.0f);

	// do necessary matrix multiplication
	modelViewProjectionMatrix = perspectiveProjectionMatrix * ((translationMatrix * scaleMatrix)*rotationMatrix); //modelViewMatrix = (translationMatrix * scaleMatrix) * rotationMatrix;

	// send necessary metrices to shader in respective uniforms
	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);

	// bind with vao // this will avoid repetdly binding with vbo
	glBindVertexArray(vao);

	// similary bind with texture if any

	// draw the necessary scene 
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboFanElement);
	glDrawElements(GL_LINES, giFanElements, GL_UNSIGNED_INT, 0/*  */);

	// unbind vao
	glBindVertexArray(0);

	// initializing above metrices to identity
	modelViewMatrix = mat4::identity();
	modelViewProjectionMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	rotationMatrix = mat4::identity();
	scaleMatrix = mat4::identity();

	// do neccessry transformation, rotation, scaling
	translationMatrix = translate(2.5f, -1.0f, -7.0f);

	// do necessary matrix multiplication
	modelViewProjectionMatrix = perspectiveProjectionMatrix * ((translationMatrix * scaleMatrix)*rotationMatrix); //modelViewMatrix = (translationMatrix * scaleMatrix) * rotationMatrix;

	// send necessary metrices to shader in respective uniforms
	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);

	//glPolygonMode(GL_POLYGON_OFFSET_FILL, GL_FILL);
	// bind with vao // this will avoid repetdly binding with vbo
	glBindVertexArray(vaoClrSqr);

	// similary bind with texture if any

	// draw the necessary scene 
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboColorSquareElement);
	//glDrawElements(GL_TRIANGLES, giColorSquareElements, GL_UNSIGNED_INT, 0/*  */);
	glDrawElements(GL_LINES, giColorSquareElements, GL_UNSIGNED_INT, 0/*  */);

	// unbind vao
	glBindVertexArray(0);

	// un-use the program
	glUseProgram(0);
	

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
                strcat(pszErrorStrPrefix, "Error enum not found\n");
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
                                strcat(pszErrorStrPrefix, szInfoLog);
                        }
                        else
                        {
                                strcat(pszErrorStrPrefix, "malloc() failed\n");
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

