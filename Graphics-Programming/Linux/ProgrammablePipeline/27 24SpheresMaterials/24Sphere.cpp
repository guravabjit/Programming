
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

typedef struct Materials
{

	GLfloat materialAmbient[4];
	GLfloat materialDiffuse[4];
	GLfloat materialSpecular[4];
	GLfloat materialShininess;

}MaterialsSphere;

enum RotationAxis
{
	enRA_X,
	enRA_Y,
	enRA_Z
};

typedef struct Light_Position
{
	float fLightAmbient[4];
	float fLightDiffuse[4];
	float fLightSpecular[4];
	float fLightPosition[4];

	float fMaterialAmbient[4];
	float fMaterialDiffuse[4];
	float fMaterialSpecular[4];

	float fMaterialShyniness[1];

	GLuint lKeyPressedUniform;
	GLuint viewUniform;
	GLuint modelUniform;
	GLuint projectionUniform;
	GLuint laUniform;
	GLuint kaUniform;
	GLuint ldUniform;
	GLuint kdUniform;
	GLuint lsUniform;
	GLuint ksUniform;
	GLuint lightPositionUniform;
	GLuint materialShininessUniform;

	GLuint lKeyPressedUniformV;
	GLuint viewUniformV;
	GLuint modelUniformV;
	GLuint projectionUniformV;
	GLuint laUniformV;
	GLuint kaUniformV;
	GLuint ldUniformV;
	GLuint kdUniformV;
	GLuint lsUniformV;
	GLuint ksUniformV;
	GLuint lightPositionUniformV;
	GLuint materialShininessUniformV;

}LIGHT_POSITION;

LIGHT_POSITION gLightMaterial[24];

float gW = 1.0f;
float gH = 1.0f;
float gX = 0.0f;
float gY = 0.0f;
int gTotalNumberOfRows = 6;
int gTotalNumberOfColumns = 4;

RotationAxis giRotationAxis = enRA_X;
GLfloat gfAngleXAxisRot = 0.0f;
GLfloat gfAngleYAxisRot = 0.0f;
GLfloat gfAngleZAxisRot = 0.0f;


MaterialsSphere materials[24] = {
{
{ 0.0215f, 0.1745f, 0.0215f, 1.0f },//ambient;
		{ 0.07568f, 0.61424f, 0.07568f, 1.0f}, //diffuse
		{ 0.633f, 0.727811f, 0.633f, 1.0f},//specular
 0.6f * 128//shininess
},

	{ //6 Brass
		{ 0.329412f, 0.223529f, 0.27451f, 1.0f },//ambient;
		{ 0.78392f, 0.568627f, 0.113725f, 1.0f }, //diffuse
		{ 0.992157f, 0.941176f, 0.807843f, 1.0f },//specular
		0.21794872f * 128//shininess
	},
	{ //12 Black
		{ 0.0f, 0.0f, 0.0f, 1.0f },//ambient;
		{ 0.0f, 0.0f, 0.0f, 1.0f }, //diffuse
		{ 0.50f, 0.50f, 0.50f, 1.0f },//specular
		0.25f * 128//shininess
	},
	{ //18 Black
		{ 0.02f, 0.02f, 0.02f, 1.0f },//ambient;
		{ 0.01f, 0.01f, 0.01f, 1.0f }, //diffuse
		{ 0.4f, 0.4f, 0.4f, 1.0f },//specular
		0.078125f * 128//shininess
	},
	{ //1 Jade
		{0.135f, 0.2225f, 0.1575f, 1.0f},//ambient;
		{0.54f, 0.89f, 0.63f, 1.0f}, //diffuse
		{0.316228f, 0.316228f, 0.316228f, 1.0f},//specular
		 0.1f * 128//shininess
	},
	{ //7 Bronze
		{ 0.2125f, 0.1275f, 0.054f, 1.0f },//ambient;
		{ 0.714f, 0.4284f, 0.18144f, 1.0f }, //diffuse
		{ 0.393548f, 0.271906f, 0.166721f, 1.0f },//specular
		0.2f * 128//shininess
	},
	{ //13 Cyan
		{ 0.0f, 0.1f, 0.06f, 1.0f },//ambient;
		{ 0.0f, 0.50980392f, 0.50980392f, 1.0f }, //diffuse
		{ 0.50196078f, 0.50196078f, 0.50196078f, 1.0f },//specular
		0.25 * 128//shininess
	},
		{ //19 Cyan
		{ 0.0f, 0.05f, 0.05f, 1.0f },//ambient;
		{ 0.4f, 0.5f, 0.5f, 1.0f }, //diffuse
		{ 0.04f, 0.7f, 0.7f, 1.0f },//specular
		0.078125f * 128//shininess
	},
{ //2 Obsidian
		{ 0.05375f, 0.5f, 0.06625f, 1.0f },//ambient;
		{ 0.18275f, 0.17f, 0.22525f, 1.0f }, //diffuse
		{ 0.332741f, 0.328634f, 0.346435f, 1.0f },//specular
		0.3f * 128//shininess
},

	{ //8 Chrome
		{ 0.25f, 0.25f, 0.25f, 1.0f },//ambient;
		{ 0.4f, 0.4f, 0.4f, 1.0f }, //diffuse
		{ 0.774597f, 0.774597f, 0.774597f, 1.0f },//specular
		0.6f * 128//shininess
	},
	{ //14 Green
		{ 0.0f, 0.0f, 0.0f, 1.0f },//ambient;
		{ 0.1f, 0.35f, 0.1f, 1.0f }, //diffuse
		{ 0.45f, 0.55f, 0.45f, 1.0f },//specular
		0.25f * 128//shininess
	},
	{ //20 Green
		{ 0.0f, 0.05f, 0.00f, 1.0f },//ambient;
		{ 0.4f, 0.5f, 0.4f, 1.0f }, //diffuse
		{ 0.04f, 0.7f, 0.04f, 1.0f },//specular
		0.078125f * 128//shininess
	},


	{ //3 Pearl
		{ 0.25f, 0.20725f, 0.20725f, 1.0f },//ambient;
		{ 1.0f, 0.829f, 0.829f, 1.0f }, //diffuse
		{ 0.296648f, 0.296648f, 0.296648f, 1.0f },//specular
		0.88f * 128//shininess
	},
	{ //9 Copper
		{ 0.19125f, 0.0735f, 0.0225f, 1.0f },//ambient;
		{ 0.7038f, 0.27048f, 0.0828f, 1.0f }, //diffuse
		{ 0.25677f, 0.137622f, 0.086014f, 1.0f },//specular
		0.1f * 128//shininess
	},
	{ //15 Red
		{ 0.0f, 0.0f, 0.0f, 1.0f },//ambient;
		{ 0.5f, 0.0f, 0.0f, 1.0f }, //diffuse
		{ 0.7f, 0.6f, 0.6f, 1.0f },//specular
		0.25f * 128//shininess
	},
	{ //21 Red
		{ 0.05f, 0.0f, 0.0f, 1.0f },//ambient;
		{ 0.5f, 0.4f, 0.4f, 1.0f }, //diffuse
		{ 0.7f, 0.04f, 0.04f, 1.0f },//specular
		0.078125f * 128//shininess
	},
	{ //4 Ruby
		{ 0.1745f, 0.01175f, 0.01175f, 1.0f },//ambient;
		{ 0.61424f, 0.04136f, 0.04136f, 1.0f }, //diffuse
		{ 0.727811f, 0.626959f, 0.626959f, 1.0f },//specular
		0.6f * 128//shininess
	},
			{ //10 Gold
		{ 0.24725f, 0.1995f, 0.0745f, 1.0f },//ambient;
		{ 0.75164f, 0.60648f, 0.22648f, 1.0f }, //diffuse
		{ 0.628281f, 0.555802f, 0.366065f, 1.0f },//specular
		0.4 * 128//shininess
			},
		{ //16 White
		{ 0.0f, 0.0f, 0.0f, 1.0f },//ambient;
		{ 0.55f, 0.55f, 0.55f, 1.0f }, //diffuse
		{ 0.70f, 0.70f, 0.70f, 1.0f },//specular
		0.25f * 128//shininess
		},

	{ //22 White
		{ 0.05f, 0.05f, 0.05f, 1.0f },//ambient;
		{ 0.5f, 0.5f, 0.5f, 1.0f }, //diffuse
		{ 0.7f, 0.7f, 0.7f, 1.0f },//specular
		0.078125f * 128//shininess
	},
	{ //5 Turquoise
		{ 0.1f, 0.18725f, 0.1745f, 1.0f },//ambient;
		{ 0.396f, 0.74151f, 0.69102f, 1.0f }, //diffuse
		{ 0.297254f, 0.30829f, 0.306678f, 1.0f },//specular
		0.1f * 128//shininess
	},

	{ //11 Silver
		{ 0.19225f, 0.19225f, 0.19225f, 1.0f },//ambient;
		{ 0.50745f, 0.50745f, 0.50745f, 1.0f }, //diffuse
		{ 0.508273f, 0.508273f, 0.508273f, 1.0f },//specular
		0.4f * 128//shininess
	},


	{ //17 Yellow Plastic
		{ 0.0f, 0.0f, 0.0f, 1.0f },//ambient;
		{ 0.5f, 0.5f, 0.0f, 1.0f }, //diffuse
		{ 0.60f, 0.60f, 0.50f, 1.0f },//specular
		0.25f * 128//shininess
	},

	{ //23 Yellow Rubber
		{ 0.05f, 0.05f, 0.0f, 1.0f },//ambient;
		{ 0.5f, 0.5f, 0.4f, 1.0f }, //diffuse
		{ 0.7f, 0.7f, 0.04f, 1.0f },//specular
		0.078125f * 128//shininess
	}
};


GLenum gGlEnumResult;
GLuint gGlShaderProgramObject;

GLuint vaoQuad;
GLuint vboQuadPosition;
GLuint vboQuadNormal;

GLfloat angleQuad;

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
						case XK_x:
						case XK_X:
							gfAngleXAxisRot = 90.0f;
							giRotationAxis = enRA_X;
							break;

						case XK_Y:
						case XK_y:
							gfAngleYAxisRot = 90.0f;
							giRotationAxis = enRA_Y;
							break;

						case XK_z:
						case XK_Z:
							gfAngleZAxisRot = 90.0f;
							giRotationAxis = enRA_Z;
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

	laUniform = glGetUniformLocation(gGlShaderProgramObject, "u_la");
	kaUniform = glGetUniformLocation(gGlShaderProgramObject, "u_ka");

	ldUniform = glGetUniformLocation(gGlShaderProgramObject, "u_ld");
	kdUniform = glGetUniformLocation(gGlShaderProgramObject, "u_kd");

	lsUniform = glGetUniformLocation(gGlShaderProgramObject, "u_ls");
	ksUniform = glGetUniformLocation(gGlShaderProgramObject, "u_ks");

	lightPositionUniform = glGetUniformLocation(gGlShaderProgramObject, "u_light_position");

	materialShininessUniform = glGetUniformLocation(gGlShaderProgramObject, "u_materialShininess");

	int iItr = 0;
	{
//		memset(str, 0, 255);
//		sprintf_s(str, "u_v_matrix[%d]", iItr);
		gLightMaterial[iItr].viewUniform = glGetUniformLocation(gGlShaderProgramObject, "u_v_matrix");

//		memset(str, 0, 255);
//		sprintf_s(str, "u_m_matrix[%d]", iItr);
		gLightMaterial[iItr].modelUniform = glGetUniformLocation(gGlShaderProgramObject, "u_m_matrix");
		
//		memset(str, 0, 255);
//		sprintf_s(str, "u_p_matrix[%d]", iItr);
		gLightMaterial[iItr].projectionUniform = glGetUniformLocation(gGlShaderProgramObject, "u_p_matrix");

//		memset(str, 0, 255);
//		sprintf_s(str, "u_l_key_pressed[%d]", iItr);
		gLightMaterial[iItr].lKeyPressedUniform = glGetUniformLocation(gGlShaderProgramObject, "u_l_key_pressed");

//		memset(str, 0, 255);
//		sprintf_s(str, "u_la[%d]", iItr);
		gLightMaterial[iItr].laUniform = glGetUniformLocation(gGlShaderProgramObject, "u_la");

//		memset(str, 0, 255);
//		sprintf_s(str, "u_ka[%d]", iItr);
		gLightMaterial[iItr].kaUniform = glGetUniformLocation(gGlShaderProgramObject, "u_ka");

//		memset(str, 0, 255);
//		sprintf_s(str, "u_ld[%d]", iItr);
		gLightMaterial[iItr].ldUniform = glGetUniformLocation(gGlShaderProgramObject, "u_ld");

//		memset(str, 0, 255);
//		sprintf_s(str, "u_kd[%d]", iItr);
		gLightMaterial[iItr].kdUniform = glGetUniformLocation(gGlShaderProgramObject, "u_kd");

//		memset(str, 0, 255);
//		sprintf_s(str, "u_ls[%d]", iItr);
		gLightMaterial[iItr].lsUniform = glGetUniformLocation(gGlShaderProgramObject, "u_ls");

//		memset(str, 0, 255);
//		sprintf_s(str, "u_ks[%d]", iItr);
		gLightMaterial[iItr].ksUniform = glGetUniformLocation(gGlShaderProgramObject, "u_ks");

//		memset(str, 0, 255);
//		sprintf_s(str, "u_light_position[%d]", iItr);
		gLightMaterial[iItr].lightPositionUniform = glGetUniformLocation(gGlShaderProgramObject, "u_light_position");

//		memset(str, 0, 255);
//		sprintf_s(str, "u_materialShininess");
		gLightMaterial[iItr].materialShininessUniform = glGetUniformLocation(gGlShaderProgramObject, "u_materialShininess");
	}


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

	gW = (GLfloat)iWidth / (GLfloat)gTotalNumberOfRows;
	gH = (GLfloat)iHeight / (GLfloat)gTotalNumberOfRows;
	gX = (iWidth - (gW * gTotalNumberOfColumns)) / 2;
	gY = (iHeight - (gH * gTotalNumberOfRows)) / 2;


}

void display(void)
{
	void Draw24Spheres();

	// usual display code
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
        glUseProgram(gGlShaderProgramObject);

	Draw24Spheres();

        // un-use the program
        glUseProgram(0);

	glXSwapBuffers(gpDisplay, gWindow); // double buffer
}

void update()
{
	if(gbCubeAnimation == True)
	{
		if (enRA_X == giRotationAxis)
		{
			gfAngleXAxisRot += 0.005f;
			gfAngleYAxisRot = 0.0f;
			gfAngleZAxisRot = 0.0f;

		}
		else if (enRA_Y == giRotationAxis)
		{
			gfAngleYAxisRot += 0.005f;
			gfAngleXAxisRot = 0.0f;
			gfAngleZAxisRot = 0.0f;
		}
		else if (enRA_Z == giRotationAxis)
		{
			gfAngleZAxisRot += 0.005f;
			gfAngleXAxisRot = 0.0f;
			gfAngleYAxisRot = 0.0f;
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

void Draw24Spheres()
{
	void drawSphere(float trnasX, float trnasY, float trnasZ, int iIndex, float x, float y, float widht, float height);

	int iIndex = 0;

	float trnasX;
	float trnasY;
	float trnasZ;

	trnasX = -6.0f;
	trnasY = 3.7f;
	trnasZ = -10.0f;
	for (int iItr1 = 0; iItr1 < 4; iItr1++)
	{
		for (int iItr = 0; iItr < 6; iItr++)
		{
			drawSphere(trnasX, trnasY, trnasZ, iIndex, gX+(iItr1*gW), gY+(iItr*gH), gW , gH);
			iIndex++;
			trnasY -= 1.5f;
		}
		trnasY = 3.7f;
		trnasX += 4.0f;
	}
}

void drawSphere(float trnasX, float trnasY, float trnasZ, int iIndex, float x, float y, float widht, float height)
{
	// declaration of metrices
	mat4 translationMatrix;
	mat4 rotationMatrix;
	mat4 scaleMatrix;

	mat4 modelMatrix;
	mat4 viewMatrix;
	mat4 projectionMatrix;
	perspectiveProjectionMatrix = mat4::identity();
	glViewport((GLint)x, (GLint)y, (GLsizei)widht, (GLsizei)height);
	perspectiveProjectionMatrix = perspective(60.0f,
		(GLfloat)widht / (GLfloat)height,
		0.1f,
		100.0f);


	// initializing above metrices to identity
	translationMatrix = mat4::identity();
	rotationMatrix = mat4::identity();
	scaleMatrix = mat4::identity();

	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	projectionMatrix = mat4::identity();

	// do neccessry transformation, rotation, scaling
	//translationMatrix = translate(trnasX, trnasY, trnasZ);
	translationMatrix = translate(0.0f, 0.0f, -3.0f);
	//scaleMatrix = scale(0.5f, 0.5f, 0.5f);
	float fLightPosition[4] = { 0.0f, 0.0f, 2.0f, 1.0f };
	if (enRA_X == giRotationAxis)
	{
		rotationMatrix = rotate(gfAngleXAxisRot, 1.0f, 0.0f, 0.0f);
		gLightMaterial[iIndex].fLightPosition[0] = 0.0f;
		gLightMaterial[iIndex].fLightPosition[1] = (GLfloat)sin(gfAngleXAxisRot) * 100;
		gLightMaterial[iIndex].fLightPosition[2] = (GLfloat)cos(gfAngleXAxisRot) * 100;
		gLightMaterial[iIndex].fLightPosition[3] = 1.0f;

		fLightPosition[0] = 0.0f;
		fLightPosition[1] = (GLfloat)sin(gfAngleXAxisRot) * 100;
		fLightPosition[2] = (GLfloat)cos(gfAngleXAxisRot) * 100;;
		fLightPosition[3] = 1.0f;
	}
	else if (enRA_Y == giRotationAxis)
	{
		rotationMatrix = rotate(gfAngleYAxisRot, 0.0f, 1.0f, 0.0f);
		gLightMaterial[iIndex].fLightPosition[0] = (GLfloat)sin(gfAngleYAxisRot)*100;
		gLightMaterial[iIndex].fLightPosition[1] = 0.0f;
		gLightMaterial[iIndex].fLightPosition[2] = (GLfloat)cos(gfAngleYAxisRot) * 100;
		gLightMaterial[iIndex].fLightPosition[3] = 1.0f;

		fLightPosition[1] = 0.0f;
		fLightPosition[0] = (GLfloat)sin(gfAngleYAxisRot) * 100;
		fLightPosition[2] = (GLfloat)cos(gfAngleYAxisRot) * 100;
		fLightPosition[3] = 1.0f;
	}
	else if (enRA_Z == giRotationAxis)
	{
		rotationMatrix = rotate(gfAngleZAxisRot, 0.0f, 0.0f, 1.0f);
		gLightMaterial[iIndex].fLightPosition[0] = (GLfloat)sin(gfAngleZAxisRot)*100;
		gLightMaterial[iIndex].fLightPosition[1] = (GLfloat)cos(gfAngleZAxisRot) * 100;
		gLightMaterial[iIndex].fLightPosition[2] = 0.0f;
		gLightMaterial[iIndex].fLightPosition[3] = 1.0f;

		fLightPosition[2] = 0.0f;
		fLightPosition[0] = (GLfloat)sin(gfAngleZAxisRot) * 100;
		fLightPosition[1] = (GLfloat)cos(gfAngleZAxisRot) * 100;;
		fLightPosition[3] = 1.0f;
	}

	if (gbLighting == True)
	{
		/*glUniform1i(lKeyPressedUniform, 1);

		glUniform3fv(laUniform, 1, gLightAmbient);
		glUniform3fv(kaUniform, 1, gMaterialAmbient);

		glUniform3fv(ldUniform, 1, gLightDiffuse); // white color / light color
		glUniform3fv(kdUniform, 1, gMaterialDiffuse); // gray color / material color

		glUniform3fv(lsUniform, 1, gLightSpecular);
		glUniform3fv(ksUniform, 1, gMaterialSpecular);

		glUniform4fv(lightPositionUniform, 1, gLightPosition); // light position

		glUniform1f(materialShininessUniform, gMaterialShininess);*/

		float fLightAmbient[4] = {0.0f, 0.0f, 0.0f, 1.0f};
		float fLightDiffuse[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		float fLightSpecular[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		


		glUniformMatrix4fv(gLightMaterial[/*iIndex*/0].viewUniform, 1, GL_FALSE, viewMatrix);
		glUniformMatrix4fv(gLightMaterial[/*iIndex*/0].modelUniform, 1, GL_FALSE, modelMatrix);
		glUniformMatrix4fv(gLightMaterial[/*iIndex*/0].projectionUniform, 1, GL_FALSE, projectionMatrix);

		glUniform1i(gLightMaterial[/*iIndex*/0].lKeyPressedUniform, 1);

		glUniform3fv(gLightMaterial[/*iIndex*/0].laUniform, 1, fLightAmbient);//gLightMaterial[/*iIndex*/0].fLightAmbient);
		glUniform3fv(gLightMaterial[/*iIndex*/0].kaUniform, 1, materials[iIndex].materialAmbient); //gLightMaterial[iIndex].fMaterialAmbient);

		glUniform3fv(gLightMaterial[/*iIndex*/0].ldUniform, 1, fLightDiffuse);//gLightMaterial[/*iIndex*/0].fLightDiffuse); // white color / light color
		glUniform3fv(gLightMaterial[/*iIndex*/0].kdUniform, 1, materials[iIndex].materialDiffuse);//gLightMaterial[iIndex].fMaterialDiffuse); // gray color / material color

		glUniform3fv(gLightMaterial[/*iIndex*/0].lsUniform, 1, fLightSpecular); //gLightMaterial[/*iIndex*/0].fLightSpecular);
		glUniform3fv(gLightMaterial[/*iIndex*/0].ksUniform, 1, materials[iIndex].materialSpecular);//gLightMaterial[iIndex].fMaterialSpecular);

		glUniform4fv(gLightMaterial[/*iIndex*/0].lightPositionUniform, 1, fLightPosition);//gLightMaterial[iIndex].fLightPosition); // light position

		glUniform1f(gLightMaterial[/*iIndex*/0].materialShininessUniform, materials[iIndex].materialShininess);// [iIndex].fMaterialSpecular[0]);
	}
	else
	{
		glUniform1i(lKeyPressedUniform, 0);
	}

	// do necessary matrix multiplication
	//modelMatrix = (translationMatrix * scaleMatrix) * rotationMatrix;
	modelMatrix = translationMatrix;
	projectionMatrix = perspectiveProjectionMatrix;

	glUniformMatrix4fv(viewUniform, 1, GL_FALSE, viewMatrix);
	glUniformMatrix4fv(modelUniform, 1, GL_FALSE, modelMatrix);
	glUniformMatrix4fv(projectionUniform, 1, GL_FALSE, projectionMatrix);


	// *** bind vao ***
	glBindVertexArray(gVao_sphere);

	// *** draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_sphere_element);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);

	// *** unbind vao ***
	glBindVertexArray(0);

}


