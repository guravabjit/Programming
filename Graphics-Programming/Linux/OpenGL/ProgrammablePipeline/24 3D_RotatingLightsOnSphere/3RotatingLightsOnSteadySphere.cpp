
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
#include "Sphere.h"

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


struct nodeStackData
{
	mat4 modelMatrix;
	mat4 viewMatrix;
	mat4 projectionMatrix;
};

int iPushIndex = 0;
struct nodeStackData modelViewProjectionStack[32];


typedef struct LIGHT
{
	GLfloat mfLightAmbient[4];
	GLfloat mfLightDiffuse[4];
	GLfloat mfLightPosition[4];
	GLfloat mfLightSpecular[4];

	GLfloat mfMaterialAmbient[4];
	GLfloat mfMaterialDiffuse[4];
	GLfloat mfMaterialSpecular[4];
	GLfloat mfMaterialShininess;
	
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

	GLuint lightPositionUniformV;

} LIGHT, *LPLIGHT;

LIGHT gLights[3];

GLenum gGlEnumResult;
GLuint gGlShaderProgramObject;

GLuint vaoQuad;
GLuint vboQuadPosition;
GLuint vboQuadNormal;

GLfloat gfAngleOne = 0.0f;
GLfloat gfAngleTwo = 0.0f;
GLfloat gfAngleThree = 0.0f;


GLuint viewUniform;
GLuint modelUniform;
GLuint projectionUniform;
mat4 perspectiveProjectionMatrix;

GLuint lKeyPressedUniform;
GLuint laUniform;
GLuint kaUniform;

GLuint ldUniform;
GLuint kdUniform;

GLuint lsUniform;
GLuint ksUniform;
GLuint materialShininessUniform;

GLuint lightPositionUniform;


Bool gbLighting = False;
Bool gbCubeAnimation = False;

bool gbFullScreen = false;
Display *gpDisplay = NULL;
XVisualInfo *gpXVisualInfo = NULL;
Colormap gColorMap;
Window gWindow;

float gLightAmbient[] = { 0.0f, 0.0f, 0.0f, 0.0f };
float gLightDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
float gLightSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
float gLightPosition[] = { 100.0f, 100.0f, 100.0f, 1.0f };

float gMaterialAmbient[] = { 0.0f, 0.0f, 0.0f, 0.0f };
float gMaterialDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
float gMaterialSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
float gMaterialShininess = 128.0f; //50.0f; // 128.0f

typedef GLXContext(*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);
glXCreateContextAttribsARBProc glXCreateContextAttribsARB = NULL;
GLXFBConfig gglxFBConfig;
//glxContext gglxContext;

int giWindowWidth = 800;
int giWindowHeight = 600;

FILE* gpLogFile = NULL;

float sphere_vertices[1146];
float sphere_normals[1146];
float sphere_textures[764];
unsigned short sphere_elements[2280];

int gNumVertices;
int gNumElements;

GLuint gVao_sphere;
GLuint gVbo_sphere_position;
GLuint gVbo_sphere_normal;
GLuint gVbo_sphere_element;

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
						case XK_A:
						case XK_a:
							if (gbCubeAnimation == True)
							{
								gbCubeAnimation = False;
							}
							else
							{
								gbCubeAnimation = True;
							}
							break;

						case XK_L:
						case XK_l:
							if (gbLighting == True)
							{
								gbLighting = False;
							}
							else
							{
								gbLighting = True;
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

	if (vboQuadNormal)
	{
		glDeleteBuffers(1, &vboQuadNormal);
	}

	if (vboQuadPosition)
	{
		glDeleteBuffers(1, &vboQuadPosition);
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
	void FillLightDetails();
	void uninitialize();
	void resize(int, int);
	GLboolean GetGlErrorStatus(GLGetErrorType enErrorType, GLuint uiErrorObject, char* pszErrorStrPrefix);;

	char szglErrorStr[ERROR_SIZE] = "";
	GLboolean bErrorStatus = GL_TRUE;
	GLuint gGlVertexShaderObject;
	GLuint gGlFragmentShaderObject;

	FillLightDetails();

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
		"in vec3 vNormal;" \
		"\n" \
		"uniform int u_l_key_pressed;" \
		"uniform mat4 u_m_matrix;" \
		"uniform mat4 u_v_matrix;" \
		"uniform mat4 u_p_matrix;" \
		"uniform vec4 u_Rlight_position;" \
		"uniform vec4 u_Blight_position;" \
		"uniform vec4 u_Glight_position;" \
		"\n" \
		"out vec3 transposeNormal;" \
		"out vec3 source_Rlight_direction;" \
		"out vec3 source_Blight_direction;" \
		"out vec3 source_Glight_direction;" \
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
		"source_Rlight_direction = vec3(u_Rlight_position - eyeCoordinates);" \
		"source_Blight_direction = vec3(u_Blight_position - eyeCoordinates);" \
		"source_Glight_direction = vec3(u_Glight_position - eyeCoordinates);" \
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
		"uniform vec3 u_Rla;" \
		"uniform vec3 u_Rka;" \
		"\n" \
		"uniform vec3 u_Rld;" \
		"uniform vec3 u_Rkd;" \
		"\n" \
		"uniform vec3 u_Rls;" \
		"uniform vec3 u_Rks;" \
		"uniform float u_RmaterialShininess;" \
		"\n" \
		"uniform vec3 u_Bla;" \
		"uniform vec3 u_Bka;" \
		"\n" \
		"uniform vec3 u_Bld;" \
		"uniform vec3 u_Bkd;" \
		"\n" \
		"uniform vec3 u_Bls;" \
		"uniform vec3 u_Bks;" \
		"uniform float u_BmaterialShininess;" \
		"\n" \
		"uniform vec3 u_Gla;" \
		"uniform vec3 u_Gka;" \
		"\n" \
		"uniform vec3 u_Gld;" \
		"uniform vec3 u_Gkd;" \
		"\n" \
		"uniform vec3 u_Gls;" \
		"uniform vec3 u_Gks;" \
		"uniform float u_GmaterialShininess;" \
		"\n" \
		"uniform int u_l_key_pressed;" \
		"\n" \
		"vec3 fong_ads_Rlight;" \
		"vec3 fong_ads_Blight;" \
		"vec3 fong_ads_Glight;" \
		"in vec3 transposeNormal;" \
		"in vec3 source_Rlight_direction;" \
		"in vec3 source_Blight_direction;" \
		"in vec3 source_Glight_direction;" \
		"in vec3 viewerVector;" \
		"vec4 fragRColor;" \
		"vec4 fragBColor;" \
		"vec4 fragGColor;" \
		"out vec4 fragColor;" \
		"void BlueColor();" \
		"void RedColor();" \
		"void GreenColor();" \
		"void main(void)" \
		"{"	\
			"if(u_l_key_pressed == 1)" \
			"{" \
				"RedColor();\n" \
				"BlueColor();\n" \
				"GreenColor();\n" \
				"fragColor = fragRColor + fragGColor + fragBColor;" \
			"}" \
			"else" \
			"{" \
				"fragColor = vec4(1.0, 1.0, 1.0, 1.0);" \
			"}" \
		"}" \
		"void RedColor()" \
		"{\n" \
			"vec3 normTransposeNormal = normalize(transposeNormal);" \
			"vec3 normSource_light_direction = normalize(source_Rlight_direction);" \
			"vec3 normviewerVector = normalize(viewerVector);" \
			"float transposeNormalDotLightDir = max(dot(normSource_light_direction, normTransposeNormal), 0.0);" \
			"\n" \
			"vec3 reflectionVector = reflect(-normSource_light_direction, normTransposeNormal);" \
			"vec3 ambient = u_Rla * u_Rka;" \
			"\n" \
			"vec3 diffuse = u_Rld * u_Rkd * transposeNormalDotLightDir;" \
			"\n" \
			"vec3 specular = u_Rls * u_Rks * pow(max(dot(reflectionVector, normviewerVector), 0.0), u_RmaterialShininess);" \
			"\n" \
			"fong_ads_Rlight = ambient + diffuse + specular;" \
			"fragRColor = vec4(fong_ads_Rlight, 1.0);" \
		"}\n" \
		"void BlueColor()" \
		"{\n" \
			"vec3 normTransposeNormal = normalize(transposeNormal);" \
			"vec3 normSource_light_direction = normalize(source_Blight_direction);" \
			"vec3 normviewerVector = normalize(viewerVector);" \
			"float transposeNormalDotLightDir = max(dot(normSource_light_direction, normTransposeNormal), 0.0);" \
			"\n" \
			"vec3 reflectionVector = reflect(-normSource_light_direction, normTransposeNormal);" \
			"vec3 ambient = u_Bla * u_Bka;" \
			"\n" \
			"vec3 diffuse = u_Bld * u_Bkd * transposeNormalDotLightDir;" \
			"\n" \
			"vec3 specular = u_Bls * u_Bks * pow(max(dot(reflectionVector, normviewerVector), 0.0), u_BmaterialShininess);" \
			"\n" \
			"fong_ads_Blight = ambient + diffuse + specular;" \
			"fragBColor = vec4(fong_ads_Blight, 1.0);" \
		"}\n" \
		"void GreenColor()" \
		"{\n" \
			"vec3 normTransposeNormal = normalize(transposeNormal);" \
			"vec3 normSource_light_direction = normalize(source_Glight_direction);" \
			"vec3 normviewerVector = normalize(viewerVector);" \
			"float transposeNormalDotLightDir = max(dot(normSource_light_direction, normTransposeNormal), 0.0);" \
			"\n" \
			"vec3 reflectionVector = reflect(-normSource_light_direction, normTransposeNormal);" \
			"vec3 ambient = u_Gla * u_Gka;" \
			"\n" \
			"vec3 diffuse = u_Gld * u_Gkd * transposeNormalDotLightDir;" \
			"\n" \
			"vec3 specular = u_Gls * u_Gks * pow(max(dot(reflectionVector, normviewerVector), 0.0), u_GmaterialShininess);" \
			"\n" \
			"fong_ads_Glight = ambient + diffuse + specular;" \
			"fragGColor = vec4(fong_ads_Glight, 1.0);" \
		"}\n" \


	};



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
	glBindAttribLocation(gGlShaderProgramObject, AMC_ATTRIBUTE_NORMAL, "vNormal");

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
	viewUniform = glGetUniformLocation(gGlShaderProgramObject, "u_v_matrix");
	modelUniform = glGetUniformLocation(gGlShaderProgramObject, "u_m_matrix");
	projectionUniform = glGetUniformLocation(gGlShaderProgramObject, "u_p_matrix");

	lKeyPressedUniform = glGetUniformLocation(gGlShaderProgramObject, "u_l_key_pressed");

	gLights[0].laUniform = glGetUniformLocation(gGlShaderProgramObject, "u_Rla");
	gLights[0].kaUniform = glGetUniformLocation(gGlShaderProgramObject, "u_Rka");

	gLights[0].ldUniform = glGetUniformLocation(gGlShaderProgramObject, "u_Rld");
	gLights[0].kdUniform = glGetUniformLocation(gGlShaderProgramObject, "u_Rkd");

	gLights[0].lsUniform = glGetUniformLocation(gGlShaderProgramObject, "u_Rls");
	gLights[0].ksUniform = glGetUniformLocation(gGlShaderProgramObject, "u_Rks");

	gLights[0].lightPositionUniform = glGetUniformLocation(gGlShaderProgramObject, "u_Rlight_position");

	gLights[0].materialShininessUniform = glGetUniformLocation(gGlShaderProgramObject, "u_RmaterialShininess");


	gLights[1].laUniform = glGetUniformLocation(gGlShaderProgramObject, "u_Bla");
	gLights[1].kaUniform = glGetUniformLocation(gGlShaderProgramObject, "u_Bka");

	gLights[1].ldUniform = glGetUniformLocation(gGlShaderProgramObject, "u_Bld");
	gLights[1].kdUniform = glGetUniformLocation(gGlShaderProgramObject, "u_Bkd");

	gLights[1].lsUniform = glGetUniformLocation(gGlShaderProgramObject, "u_Bls");
	gLights[1].ksUniform = glGetUniformLocation(gGlShaderProgramObject, "u_Bks");

	gLights[1].lightPositionUniform = glGetUniformLocation(gGlShaderProgramObject, "u_Blight_position");

	gLights[1].materialShininessUniform = glGetUniformLocation(gGlShaderProgramObject, "u_BmaterialShininess");


	gLights[2].laUniform = glGetUniformLocation(gGlShaderProgramObject, "u_Gla");
	gLights[2].kaUniform = glGetUniformLocation(gGlShaderProgramObject, "u_Gka");

	gLights[2].ldUniform = glGetUniformLocation(gGlShaderProgramObject, "u_Gld");
	gLights[2].kdUniform = glGetUniformLocation(gGlShaderProgramObject, "u_Gkd");

	gLights[2].lsUniform = glGetUniformLocation(gGlShaderProgramObject, "u_Gls");
	gLights[2].ksUniform = glGetUniformLocation(gGlShaderProgramObject, "u_Gks");

	gLights[2].lightPositionUniform = glGetUniformLocation(gGlShaderProgramObject, "u_Glight_position");

	gLights[2].materialShininessUniform = glGetUniformLocation(gGlShaderProgramObject, "u_GmaterialShininess");

        // glBegin() ... glEnd() // vertices, colors, texture co-ordinates, normals // make their array
	getSphereVertexData(sphere_vertices, sphere_normals, sphere_textures, sphere_elements);
	gNumVertices = getNumberOfSphereVertices();
	gNumElements = getNumberOfSphereElements();
	
	// create vao
	glGenVertexArrays(1, &gVao_sphere);
	glBindVertexArray(gVao_sphere);

	glGenBuffers(1, &gVbo_sphere_position);
	glBindBuffer(GL_ARRAY_BUFFER, gVbo_sphere_position);
	glBufferData(GL_ARRAY_BUFFER, sizeof(sphere_vertices), sphere_vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// normal vbo
	glGenBuffers(1, &gVbo_sphere_normal);
	glBindBuffer(GL_ARRAY_BUFFER, gVbo_sphere_normal);
	glBufferData(GL_ARRAY_BUFFER, sizeof(sphere_normals), sphere_normals, GL_STATIC_DRAW);

	glVertexAttribPointer(AMC_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(AMC_ATTRIBUTE_NORMAL);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// element vbo
	glGenBuffers(1, &gVbo_sphere_element);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_sphere_element);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(sphere_elements), sphere_elements, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// unbind vao
	glBindVertexArray(0);

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
	void pushInStack(struct nodeStackData*);
	void popFromStack(struct nodeStackData*);

	// usual display code
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
        glUseProgram(gGlShaderProgramObject);

        // declaration of metrices
	mat4 modelMatrix;
	mat4 viewMatrix;
	mat4 projectionMatrix;
	mat4 translationMatrix;
	mat4 rotationMatrix;
	mat4 modelViewProjectionMatrix;
	mat4 scaleMatrix;

	struct nodeStackData tempStackData;

	// initializing above metrices to identity
	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	modelViewProjectionMatrix = mat4::identity();
	translationMatrix = mat4::identity();
	rotationMatrix = mat4::identity();
	projectionMatrix = mat4::identity();
	scaleMatrix = mat4::identity();

	// do neccessry transformation, rotation, scaling
	translationMatrix = translate(0.0f, 0.0f, -3.0f);

	// do necessary matrix multiplication
	modelMatrix = (translationMatrix * scaleMatrix) * rotationMatrix;
	projectionMatrix = perspectiveProjectionMatrix;

	// send necessary metrices to shader in respective uniforms
	glUniformMatrix4fv(viewUniform, 1, GL_FALSE, viewMatrix);
	glUniformMatrix4fv(modelUniform, 1, GL_FALSE, modelMatrix);
	glUniformMatrix4fv(projectionUniform, 1, GL_FALSE, projectionMatrix);

	if (gbLighting == True)
	{
			tempStackData.modelMatrix = modelMatrix;
			tempStackData.viewMatrix = viewMatrix;
			tempStackData.projectionMatrix = projectionMatrix;

			pushInStack(&tempStackData);
			rotationMatrix = rotate(gfAngleOne, 1.0f, 0.0f, 0.0f);
			glUniformMatrix4fv(viewUniform, 1, GL_FALSE, viewMatrix);
			glUniformMatrix4fv(modelUniform, 1, GL_FALSE, modelMatrix);
			glUniformMatrix4fv(projectionUniform, 1, GL_FALSE, projectionMatrix);

			glUniform1i(lKeyPressedUniform, 1);

			glUniform3fv(gLights[0].laUniform, 1, gLights[0].mfLightAmbient);
			glUniform3fv(gLights[0].kaUniform, 1, gLights[0].mfMaterialAmbient);

			glUniform3fv(gLights[0].ldUniform, 1, gLights[0].mfLightDiffuse); // white color / light color
			glUniform3fv(gLights[0].kdUniform, 1, gLights[0].mfMaterialDiffuse); // gray color / material color

			glUniform3fv(gLights[0].lsUniform, 1, gLights[0].mfLightSpecular);
			glUniform3fv(gLights[0].ksUniform, 1, gLights[0].mfMaterialSpecular);

			gLights[0].mfLightPosition[0] = 0.0f;
			gLights[0].mfLightPosition[1] = (GLfloat)sin(gfAngleOne) * 100;
			gLights[0].mfLightPosition[2] = (GLfloat)cos(gfAngleOne) * 100;;
			gLights[0].mfLightPosition[3] = 1.0f;

			//gLights[0].mfLightPosition[1] = gfAngleOne;
			glUniform4fv(gLights[0].lightPositionUniform, 1, gLights[0].mfLightPosition); // light position

			glUniform1f(gLights[0].materialShininessUniform, gLights[0].mfMaterialShininess);

			popFromStack(&tempStackData);

			modelMatrix = tempStackData.modelMatrix;
			viewMatrix = tempStackData.viewMatrix;
			projectionMatrix = tempStackData.projectionMatrix;

			

			modelMatrix = (translationMatrix * scaleMatrix) * rotationMatrix;
			projectionMatrix = perspectiveProjectionMatrix;

			tempStackData.modelMatrix = modelMatrix;
			tempStackData.viewMatrix = viewMatrix;
			tempStackData.projectionMatrix = projectionMatrix;

			pushInStack(&tempStackData);

			rotationMatrix = rotate(gfAngleTwo, 0.0f, 1.0f, 0.0f);


			glUniform3fv(gLights[1].laUniform, 1, gLights[1].mfLightAmbient);
			glUniform3fv(gLights[1].kaUniform, 1, gLights[1].mfMaterialAmbient);

			glUniform3fv(gLights[1].ldUniform, 1, gLights[1].mfLightDiffuse); // white color / light color
			glUniform3fv(gLights[1].kdUniform, 1, gLights[1].mfMaterialDiffuse); // gray color / material color

			glUniform3fv(gLights[1].lsUniform, 1, gLights[1].mfLightSpecular);
			glUniform3fv(gLights[1].ksUniform, 1, gLights[1].mfMaterialSpecular);

			//gLights[1].mfLightPosition[0] = gfAngleTwo;
			gLights[1].mfLightPosition[1] = 0.0f;
			gLights[1].mfLightPosition[0] = (GLfloat)sin(gfAngleTwo) * 100;
			gLights[1].mfLightPosition[2] = (GLfloat)cos(gfAngleTwo) * 100;;
			gLights[1].mfLightPosition[3] = 1.0f;

			glUniform4fv(gLights[1].lightPositionUniform, 1, gLights[1].mfLightPosition); // light position

			glUniform1f(gLights[1].materialShininessUniform, gLights[1].mfMaterialShininess);

			popFromStack(&tempStackData);

			modelMatrix = tempStackData.modelMatrix;
			viewMatrix = tempStackData.viewMatrix;
			projectionMatrix = tempStackData.projectionMatrix;

			modelMatrix = (translationMatrix * scaleMatrix) * rotationMatrix;
			projectionMatrix = perspectiveProjectionMatrix;

			tempStackData.modelMatrix = modelMatrix;
			tempStackData.viewMatrix = viewMatrix;
			tempStackData.projectionMatrix = projectionMatrix;

			pushInStack(&tempStackData);


			rotationMatrix = rotate(gfAngleTwo, 0.0f, 0.0f, 1.0f);
			
			glUniform3fv(gLights[2].laUniform, 1, gLights[2].mfLightAmbient);
			glUniform3fv(gLights[2].kaUniform, 1, gLights[2].mfMaterialAmbient);

			glUniform3fv(gLights[2].ldUniform, 1, gLights[2].mfLightDiffuse); // white color / light color
			glUniform3fv(gLights[2].kdUniform, 1, gLights[2].mfMaterialDiffuse); // gray color / material color

			glUniform3fv(gLights[2].lsUniform, 1, gLights[2].mfLightSpecular);
			glUniform3fv(gLights[2].ksUniform, 1, gLights[2].mfMaterialSpecular);

			//gLights[2].mfLightPosition[0] = gfAngleThree;
			gLights[2].mfLightPosition[2] = 0.0f;
			gLights[2].mfLightPosition[0] = (GLfloat)sin(gfAngleThree) * 100;
			gLights[2].mfLightPosition[1] = (GLfloat)cos(gfAngleThree) * 100;;
			gLights[2].mfLightPosition[3] = 1.0f;

			glUniform4fv(gLights[2].lightPositionUniform, 1, gLights[2].mfLightPosition); // light position

			glUniform1f(gLights[2].materialShininessUniform, gLights[2].mfMaterialShininess);
			popFromStack(&tempStackData);

			modelMatrix = tempStackData.modelMatrix;
			viewMatrix = tempStackData.viewMatrix;
			projectionMatrix = tempStackData.projectionMatrix;
	}
	else
	{
			glUniform1i(lKeyPressedUniform, 0);
			glUniformMatrix4fv(viewUniform, 1, GL_FALSE, viewMatrix);
			glUniformMatrix4fv(modelUniform, 1, GL_FALSE, modelMatrix);
			glUniformMatrix4fv(projectionUniform, 1, GL_FALSE, projectionMatrix);

	}

	// *** bind vao ***
	glBindVertexArray(gVao_sphere);

	// *** draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_sphere_element);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);

	// *** unbind vao ***
	glBindVertexArray(0);

        // un-use the program
        glUseProgram(0);

	glXSwapBuffers(gpDisplay, gWindow); // double buffer
}

void update()
{
	if(gbCubeAnimation == True)
	{
		gfAngleOne += 0.005f;
		gfAngleTwo += 0.005f;
		gfAngleThree += 0.005f;

		if (gfAngleOne > 180.0f)
		{
			gfAngleOne = 0.0f;
		}

		if (gfAngleTwo > 180.0f)
		{
			gfAngleTwo = 0.0f;
		}

		if (gfAngleThree > 360.0f)
		{
			gfAngleThree = 0.0f;
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



void FillLightDetails()
{
	gLights[0].mfLightAmbient[0] = 0.0f;
	gLights[0].mfLightAmbient[1] = 0.0f;
	gLights[0].mfLightAmbient[2] = 0.0f;
	gLights[0].mfLightAmbient[3] = 1.0f;

	gLights[0].mfLightDiffuse[0] = 1.0f;
	gLights[0].mfLightDiffuse[1] = 0.0f;
	gLights[0].mfLightDiffuse[2] = 0.0f;
	gLights[0].mfLightDiffuse[3] = 1.0f;

	gLights[0].mfLightSpecular[0] = 1.0f;
	gLights[0].mfLightSpecular[1] = 0.0f;
	gLights[0].mfLightSpecular[2] = 0.0f;
	gLights[0].mfLightSpecular[3] = 1.0f;

	gLights[0].mfLightPosition[0] = 0.0f;
	gLights[0].mfLightPosition[1] = 0.0f;
	gLights[0].mfLightPosition[2] = 0.0f;
	gLights[0].mfLightPosition[3] = 0.0f;

	gLights[0].mfMaterialAmbient[0] = 0.0f;
	gLights[0].mfMaterialAmbient[1] = 0.0f;
	gLights[0].mfMaterialAmbient[2] = 0.0f;
	gLights[0].mfMaterialAmbient[3] = 1.0f;

	gLights[0].mfMaterialDiffuse[0] = 1.0f;
	gLights[0].mfMaterialDiffuse[1] = 1.0f;
	gLights[0].mfMaterialDiffuse[2] = 1.0f;
	gLights[0].mfMaterialDiffuse[3] = 1.0f;

	gLights[0].mfMaterialSpecular[0] = 1.0f;
	gLights[0].mfMaterialSpecular[1] = 1.0f;
	gLights[0].mfMaterialSpecular[2] = 1.0f;
	gLights[0].mfMaterialSpecular[3] = 1.0f;

	gLights[0].mfMaterialShininess = 128.0f;


	gLights[1].mfLightAmbient[0] = 0.0f;
	gLights[1].mfLightAmbient[1] = 0.0f;
	gLights[1].mfLightAmbient[2] = 0.0f;
	gLights[1].mfLightAmbient[3] = 1.0f;

	gLights[1].mfLightDiffuse[0] = 0.0f;
	gLights[1].mfLightDiffuse[1] = 1.0f;
	gLights[1].mfLightDiffuse[2] = 0.0f;
	gLights[1].mfLightDiffuse[3] = 1.0f;

	gLights[1].mfLightSpecular[0] = 0.0f;
	gLights[1].mfLightSpecular[1] = 1.0f;
	gLights[1].mfLightSpecular[2] = 0.0f;
	gLights[1].mfLightSpecular[3] = 1.0f;

	gLights[1].mfLightPosition[0] = 0.0f;
	gLights[1].mfLightPosition[1] = 0.0f;
	gLights[1].mfLightPosition[2] = 0.0f;
	gLights[1].mfLightPosition[3] = 0.0f;

	gLights[1].mfMaterialAmbient[0] = 0.0f;
	gLights[1].mfMaterialAmbient[1] = 0.0f;
	gLights[1].mfMaterialAmbient[2] = 0.0f;
	gLights[1].mfMaterialAmbient[3] = 1.0f;

	gLights[1].mfMaterialDiffuse[0] = 1.0f;
	gLights[1].mfMaterialDiffuse[1] = 1.0f;
	gLights[1].mfMaterialDiffuse[2] = 1.0f;
	gLights[1].mfMaterialDiffuse[3] = 1.0f;

	gLights[1].mfMaterialSpecular[0] = 1.0f;
	gLights[1].mfMaterialSpecular[1] = 1.0f;
	gLights[1].mfMaterialSpecular[2] = 1.0f;
	gLights[1].mfMaterialSpecular[3] = 1.0f;

	gLights[1].mfMaterialShininess = 128.0f;


	gLights[2].mfLightAmbient[0] = 0.0f;
	gLights[2].mfLightAmbient[1] = 0.0f;
	gLights[2].mfLightAmbient[2] = 0.0f;
	gLights[2].mfLightAmbient[3] = 1.0f;

	gLights[2].mfLightDiffuse[0] = 0.0f;
	gLights[2].mfLightDiffuse[1] = 0.0f;
	gLights[2].mfLightDiffuse[2] = 1.0f;
	gLights[2].mfLightDiffuse[3] = 1.0f;

	gLights[2].mfLightSpecular[0] = 0.0f;
	gLights[2].mfLightSpecular[1] = 0.0f;
	gLights[2].mfLightSpecular[2] = 1.0f;
	gLights[2].mfLightSpecular[3] = 1.0f;

	gLights[2].mfLightPosition[0] = 0.0f;
	gLights[2].mfLightPosition[1] = 0.0f;
	gLights[2].mfLightPosition[2] = 0.0f;
	gLights[2].mfLightPosition[3] = 0.0f;

	gLights[2].mfMaterialAmbient[0] = 0.0f;
	gLights[2].mfMaterialAmbient[1] = 0.0f;
	gLights[2].mfMaterialAmbient[2] = 0.0f;
	gLights[2].mfMaterialAmbient[3] = 1.0f;

	gLights[2].mfMaterialDiffuse[0] = 1.0f;
	gLights[2].mfMaterialDiffuse[1] = 1.0f;
	gLights[2].mfMaterialDiffuse[2] = 1.0f;
	gLights[2].mfMaterialDiffuse[3] = 1.0f;

	gLights[2].mfMaterialSpecular[0] = 1.0f;
	gLights[2].mfMaterialSpecular[1] = 1.0f;
	gLights[2].mfMaterialSpecular[2] = 1.0f;
	gLights[2].mfMaterialSpecular[3] = 1.0f;

	gLights[2].mfMaterialShininess = 128.0f;
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

