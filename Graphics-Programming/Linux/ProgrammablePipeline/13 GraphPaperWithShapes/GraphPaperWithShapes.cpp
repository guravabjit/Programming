
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
#define NUMBER_OF_CIRCLE_LINES			6000
#define NUMBER_OF_CIRCLE_VERTICES		NUMBER_OF_CIRCLE_LINES*2*3
#define NUMBER_OF_TRIANGLE_VERTICES		3*3
#define NUMBER_OF_QUAD_VERTICES			4*3
#define NUMBER_OF_GRAPH_VERTICES		1024//(21*2*3)+2 // 21 number of lines *2 verticle and horizontal *3 line +2 middle lines
#define PI 3.14

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


GLuint vaoGraph;
GLuint vboGraphPosition;
GLuint vboGraphColor;

GLuint vaoTri;
GLuint vboTriPosition;
GLuint vboTriColor;

GLuint vaoQuad;
GLuint vboQuadPosition;
GLuint vboQuadColor;

GLuint vaoCircle;
GLuint vboCirlcePosition;
GLuint vboCirlceColor;

int giNumberOfGraphLines = 0;

GLuint mvpUniform;
mat4 perspectiveProjectionMatrix;

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

	if (vboGraphPosition)
	{
		glDeleteBuffers(1, &vboGraphPosition);
	}

	if (vboGraphColor)
	{
		glDeleteBuffers(1, &vboGraphColor);
	}

	if (vaoGraph)
	{
		glDeleteVertexArrays(1, &vaoGraph);
	}

	if (vboCirlcePosition)
	{
		glDeleteBuffers(1, &vboCirlcePosition);
	}

	if (vboCirlceColor)
	{
		glDeleteBuffers(1, &vboCirlceColor);
	}

	if (vaoCircle)
	{
		glDeleteVertexArrays(1, &vaoCircle);
	}

	if (vboTriPosition)
	{
		glDeleteBuffers(1, &vboTriPosition);
	}

	if (vboTriColor)
	{
		glDeleteBuffers(1, &vboTriColor);
	}

	if (vaoTri)
	{
		glDeleteVertexArrays(1, &vaoTri);
	}

	if (vboQuadPosition)
	{
		glDeleteBuffers(1, &vboQuadPosition);
	}

	if (vboQuadColor)
	{
		glDeleteBuffers(1, &vboQuadColor);
	}

	if (vaoQuad)
	{
		glDeleteVertexArrays(1, &vaoQuad);
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
		uninitialize();
		exit(0);
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
        strcpy(szglErrorStr, "Error in fragment shader object code : ");
        bErrorStatus = GetGlErrorStatus(enGlGET_Shader, gGlFragmentShaderObject, szglErrorStr);
        if (GL_FALSE == bErrorStatus)
        {
                fprintf(gpLogFile, "%s.\n", szglErrorStr);
		uninitialize();
		exit(0);
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
		uninitialize();
		exit(0);
        }

        // post linking retreving uniform location
        mvpUniform = glGetUniformLocation(gGlShaderProgramObject, "u_mvp_matrix");

		
        // glBegin() ... glEnd() // vertices, colors, texture co-ordinates, normals // make their array

	GLfloat graphVertices[NUMBER_OF_GRAPH_VERTICES] = { 0 };
	GLfloat colorVertices[NUMBER_OF_GRAPH_VERTICES] = { 0 };
	int iArrIndex = 0;
	float fDiff = 1.0f / 20.0f;
	for (float fItr = -1.0f; fItr <= 1.05f; fItr += fDiff)
	{
		colorVertices[iArrIndex] = 0;
		colorVertices[iArrIndex+1] = 0;
		colorVertices[iArrIndex+2] = 1;

		graphVertices[iArrIndex++] = -1.0f;
		graphVertices[iArrIndex++] = fItr;
		graphVertices[iArrIndex++] = 0.0f;

		colorVertices[iArrIndex] = 0;
		colorVertices[iArrIndex + 1] = 0;
		colorVertices[iArrIndex + 2] = 1;

		graphVertices[iArrIndex++] = 1.0f;
		graphVertices[iArrIndex++] = fItr;
		graphVertices[iArrIndex++] = 0.0f;

		colorVertices[iArrIndex] = 0;
		colorVertices[iArrIndex + 1] = 0;
		colorVertices[iArrIndex + 2] = 1;

		graphVertices[iArrIndex++] = fItr;
		graphVertices[iArrIndex++] = -1.0;
		graphVertices[iArrIndex++] = 0.0f;

		colorVertices[iArrIndex] = 0;
		colorVertices[iArrIndex + 1] = 0;
		colorVertices[iArrIndex + 2] = 1;

		graphVertices[iArrIndex++] = fItr;
		graphVertices[iArrIndex++] = 1.0;
		graphVertices[iArrIndex++] = 0.0f;
	}

	colorVertices[iArrIndex] = 1;
	colorVertices[iArrIndex + 1] = 0;
	colorVertices[iArrIndex + 2] = 0;

	graphVertices[iArrIndex++] = -1.0f;
	graphVertices[iArrIndex++] = 0.0f;
	graphVertices[iArrIndex++] = 0.0f;

	colorVertices[iArrIndex] = 1;
	colorVertices[iArrIndex + 1] = 0;
	colorVertices[iArrIndex + 2] = 0;

	graphVertices[iArrIndex++] = 1.0f;
	graphVertices[iArrIndex++] = 0.0f;
	graphVertices[iArrIndex++] = 0.0f;

	colorVertices[iArrIndex] = 0;
	colorVertices[iArrIndex + 1] = 1;
	colorVertices[iArrIndex + 2] = 0;

	graphVertices[iArrIndex++] = 0.0f;
	graphVertices[iArrIndex++] = -1.0f;
	graphVertices[iArrIndex++] = 0.0f;

	colorVertices[iArrIndex] = 0;
	colorVertices[iArrIndex + 1] = 1;
	colorVertices[iArrIndex + 2] = 0;

	graphVertices[iArrIndex++] = 0.0f;
	graphVertices[iArrIndex++] = 1.0f;
	graphVertices[iArrIndex++] = 0.0f;

	giNumberOfGraphLines = iArrIndex / 2;

	// create vao
	glGenVertexArrays(1, &vaoGraph);
	glBindVertexArray(vaoGraph);
	glGenBuffers(1, &vboGraphPosition);
	glBindBuffer(GL_ARRAY_BUFFER, vboGraphPosition);
	glBufferData(GL_ARRAY_BUFFER, sizeof(graphVertices), graphVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);

	glLineWidth(2.0f);

	glGenBuffers(1, &vboGraphColor);
	glBindBuffer(GL_ARRAY_BUFFER, vboGraphColor);
	glBufferData(GL_ARRAY_BUFFER, sizeof(colorVertices), colorVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

// triangle vao
	GLfloat triVertices[] = 
	{ 
		0.0f, 0.5f, 0.0f,
		-0.5f, -0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
	
	};
	GLfloat triColorVertices[] = 
	{
		0.5, 1.0, 1.0,
		0.5, 1.0, 1.0,
		0.5, 1.0, 1.0,
	};
	glGenVertexArrays(1, &vaoTri);
	glBindVertexArray(vaoTri);
	glGenBuffers(1, &vboTriPosition);
	glBindBuffer(GL_ARRAY_BUFFER, vboTriPosition);
	glBufferData(GL_ARRAY_BUFFER, sizeof(triVertices), triVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);

	glGenBuffers(1, &vboTriColor);
	glBindBuffer(GL_ARRAY_BUFFER, vboTriColor);
	glBufferData(GL_ARRAY_BUFFER, sizeof(triColorVertices), triColorVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	// end tri vao
	
	// quad vao
	GLfloat quadVertices[] =
	{
		-0.5f, 0.5f, 0.0f,
		-0.5f, -0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
		0.5f, 0.5f, 0.0f
	};
	GLfloat quadColorVertices[] =
	{
		1.0, 1.0, 0.0,
		1.0, 1.0, 0.0,
		1.0, 1.0, 0.0,
		1.0, 1.0, 0.0
	};
	glGenVertexArrays(1, &vaoQuad);
	glBindVertexArray(vaoQuad);
	glGenBuffers(1, &vboQuadPosition);
	glBindBuffer(GL_ARRAY_BUFFER, vboQuadPosition);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);

	glGenBuffers(1, &vboQuadColor);
	glBindBuffer(GL_ARRAY_BUFFER, vboQuadColor);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadColorVertices), quadColorVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	// end quad vao

	// circle vao
	GLfloat circleVertices[NUMBER_OF_CIRCLE_LINES*2*3] = {0};
	GLfloat circleColorVertices[NUMBER_OF_CIRCLE_LINES * 2 * 3] = { 0 };
	
	iArrIndex = 0;
	// circle vertices
	for (int iItr = 0; iItr < NUMBER_OF_CIRCLE_LINES; iItr++)
	{
		circleColorVertices[iArrIndex] = 1.0;
		circleColorVertices[iArrIndex + 1] = 0.5;
		circleColorVertices[iArrIndex + 2] = 0.0;
		circleVertices[iArrIndex++] = (GLfloat)cos((5.0f * PI * iItr) / NUMBER_OF_CIRCLE_LINES);
		circleVertices[iArrIndex++] = (GLfloat)sin((5.0f * PI * iItr) / NUMBER_OF_CIRCLE_LINES);
		circleVertices[iArrIndex++] = 0.0f;

		iItr++;

		circleColorVertices[iArrIndex] = 1.0;
		circleColorVertices[iArrIndex + 1] = 0.5;
		circleColorVertices[iArrIndex + 2] = 0.0;
		circleVertices[iArrIndex++] = (GLfloat)cos((5.0f * PI * iItr) / NUMBER_OF_CIRCLE_LINES);
		circleVertices[iArrIndex++] = (GLfloat)sin((5.0f * PI * iItr) / NUMBER_OF_CIRCLE_LINES);
		circleVertices[iArrIndex++] = 0.0f;
	}
	// end of circle

	glGenVertexArrays(1, &vaoCircle);
	glBindVertexArray(vaoCircle);
	glGenBuffers(1, &vboCirlcePosition);
	glBindBuffer(GL_ARRAY_BUFFER, vboCirlcePosition);
	glBufferData(GL_ARRAY_BUFFER, sizeof(circleVertices), circleVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);

	glGenBuffers(1, &vboCirlceColor);
	glBindBuffer(GL_ARRAY_BUFFER, vboCirlceColor);
	glBufferData(GL_ARRAY_BUFFER, sizeof(circleColorVertices), circleColorVertices, GL_STATIC_DRAW);
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
	
	// usual display code
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
	translationMatrix = translate(0.0f, 0.0f, -3.0f);
	
	// do necessary matrix multiplication
	modelViewProjectionMatrix = perspectiveProjectionMatrix * ((translationMatrix * scaleMatrix)*rotationMatrix); //modelViewMatrix = (translationMatrix * scaleMatrix) * rotationMatrix;

	// send necessary metrices to shader in respective uniforms
	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);

	// bind with vao // this will avoid repetdly binding with vbo
		glBindVertexArray(vaoGraph);
	// texture binding
	glDrawArrays(GL_LINES, 0, giNumberOfGraphLines);
	glBindVertexArray(0);
	
	// initializing above metrices to identity
	modelViewMatrix = mat4::identity();
	modelViewProjectionMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	rotationMatrix = mat4::identity();
	scaleMatrix = mat4::identity();

	// do neccessry transformation, rotation, scaling
	translationMatrix = translate(-0.5f, 0.5f, -3.0f);
	scaleMatrix = scale(0.5f, 0.5f, 0.5f);

	// do necessary matrix multiplication
	modelViewProjectionMatrix = perspectiveProjectionMatrix * ((translationMatrix * scaleMatrix)*rotationMatrix); //modelViewMatrix = (translationMatrix * scaleMatrix) * rotationMatrix;

	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);

	glBindVertexArray(vaoTri);
	// texture binding
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glBindVertexArray(0);

	// initializing above metrices to identity
	modelViewMatrix = mat4::identity();
	modelViewProjectionMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	rotationMatrix = mat4::identity();
	scaleMatrix = mat4::identity();

	// do neccessry transformation, rotation, scaling
	translationMatrix = translate(0.5f, 0.5f, -3.0f);
	scaleMatrix = scale(0.5f, 0.5f, 0.5f);

	// do necessary matrix multiplication
	modelViewProjectionMatrix = perspectiveProjectionMatrix * ((translationMatrix * scaleMatrix)*rotationMatrix); //modelViewMatrix = (translationMatrix * scaleMatrix) * rotationMatrix;

	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);

	glBindVertexArray(vaoQuad);
	// texture binding
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glBindVertexArray(0);


	// initializing above metrices to identity
	modelViewMatrix = mat4::identity();
	modelViewProjectionMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	rotationMatrix = mat4::identity();
	scaleMatrix = mat4::identity();

	// do neccessry transformation, rotation, scaling
	translationMatrix = translate(-0.5f, -0.5f, -3.0f);
	scaleMatrix = scale(0.5f, 0.5f, 0.5f);

	// do necessary matrix multiplication
	modelViewProjectionMatrix = perspectiveProjectionMatrix * ((translationMatrix * scaleMatrix)*rotationMatrix); //modelViewMatrix = (translationMatrix * scaleMatrix) * rotationMatrix;

	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjectionMatrix);

	glBindVertexArray(vaoCircle);
	// texture binding
	glDrawArrays(GL_LINES, 0, NUMBER_OF_CIRCLE_LINES);
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

