
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
#include <SOIL/SOIL.h>
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


GLuint gTextureStone;
GLfloat angleCube;

bool gbAnimation = false;
bool gbLighting = false;


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
						case XK_L:
						case XK_l:
							if (gbLighting == false)
							{
								gbLighting = true;
							}
							else if (gbLighting == true)
							{
								gbLighting = false;
							}
							break;
						case XK_a:
						case XK_A:
							if (gbAnimation == false)
							{
								gbAnimation = true;
							}
							else if (gbAnimation == true)
							{
								gbAnimation = false;
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
	GLuint LoadTexture(GLuint &Texture, char*);
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
	glBindAttribLocation(gGlShaderProgramObject, AMC_ATTRIBUTE_COLOR, "vColor");
	glBindAttribLocation(gGlShaderProgramObject, AMC_ATTRIBUTE_NORMAL, "vNormal");
	glBindAttribLocation(gGlShaderProgramObject, AMC_ATTRIBUTE_TEXCORD0, "vTexCoord");

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
	LoadTexture(gTextureStone, (char*)"./stone.bmp");

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

	glXSwapBuffers(gpDisplay, gWindow); // double buffer
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

GLuint LoadTexture(GLuint &Texture, char* path)
{
	int iImgWidth, iImgHeight;
	//GLuint Texture;

	unsigned char* imageData = SOIL_load_image(path, &iImgWidth, &iImgHeight, 0, SOIL_LOAD_RGB);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glBindTexture(GL_TEXTURE_2D, Texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	//gluBuild2DMipmaps(GL_TEXTURE_2D, 3, iImgWidth, iImgHeight, GL_BGR_EXT, GL_UNSIGNED_BYTE, imageData);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, iImgWidth, iImgHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, imageData);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
		
	SOIL_free_image_data(imageData);
	return Texture;	
}


