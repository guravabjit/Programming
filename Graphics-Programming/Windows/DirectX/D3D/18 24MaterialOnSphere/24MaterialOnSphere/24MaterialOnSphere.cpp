#include <Windows.h>
#include <stdio.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include "Sphere.h"

#pragma warning( disable : 4838 )
#include "..\..\XNAMath\xnamath.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment (lib, "d3dcompiler.lib")
#pragma comment (lib, "Sphere.lib")

#define WIN_WIDTH	800
#define WIN_HEIGHT	600

#define MATERIAL_ROWS		6
#define MATERIAL_COLUMNS	4

enum RotationAxis
{
	enRA_X,
	enRA_Y,
	enRA_Z
};

RotationAxis giRotationAxis = enRA_X;
float gfAngleXAxisRot = 0.0f;
float gfAngleYAxisRot = 0.0f;
float gfAngleZAxisRot = 0.0f;
float gW = 1.0f;
float gH = 1.0f;
float gX = 0.0f;
float gY = 0.0f;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void WriteLog(HRESULT, const char*);

HWND ghWnd = NULL;

bool gbIsActiveWindow = false;
bool gbIsFullScreen = false;
bool gbEscapeKeyIsPressed = false;

FILE *gpLogFile = NULL;
char gszLogFileName[] = "Log.txt";

DWORD gdwStyle = 0;
WINDOWPLACEMENT gwpPrev = { sizeof(WINDOWPLACEMENT) };

bool gbLighting = false;
bool gbPerPixelLighting = false;

float gfClearColor[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
IDXGISwapChain *gpIDXGISwapChain = NULL;
ID3D11Device *gpID3D11Device = NULL;
ID3D11DeviceContext *gpID3D11DeviceContext = NULL;
ID3D11RenderTargetView *gpID3D11RenderTargetView = NULL;

ID3D11VertexShader *gpID3D11VertexShader = NULL;
ID3D11PixelShader *gpID3D11PixelShader = NULL;
ID3D11Buffer *gpID3D11Buffer_VertexBuffer_Sphere = NULL;
ID3D11Buffer *gpID3D11Buffer_VertexBuffer_Sphere_Normal = NULL;
ID3D11InputLayout *gpID3D11InputLayout = NULL;
ID3D11Buffer *gpID3D11Buffer_ConstantBuffer = NULL;

ID3D11RasterizerState* gpID3D11RasterizerState = NULL; // backface culling

ID3D11DepthStencilView* gpID3D11DepthStencilView = NULL; // depth

ID3D11Buffer *gpID3D11Buffer_IndexBuffer = NULL;

struct CBUFFER
{
	XMMATRIX WorldMatrix;
	XMMATRIX ViewMatrix;
	XMMATRIX ProjectionMatrix;
	XMVECTOR laR;
	XMVECTOR laG;
	XMVECTOR laB;
	XMVECTOR ka;
	XMVECTOR ldR;
	XMVECTOR ldG;
	XMVECTOR ldB;
	XMVECTOR kd;
	XMVECTOR lsR;
	XMVECTOR lsG;
	XMVECTOR lsB;
	XMVECTOR ks;
	XMVECTOR lightPositionR;
	XMVECTOR lightPositionG;
	XMVECTOR lightPositionB;
	float materialShininess;
	unsigned int lKeyPressed;
	unsigned int lPerPixelKeyPressed;
};

float lightAmbientRed[] = { 0.0f, 0.0f, 0.0f, 1.0f };
float lightDiffuseRed[] = { 1.0f, 1.0f, 1.0f, 1.0f };
float lightSpecularRed[] = { 1.0f, 1.0f, 1.0f, 1.0f };
float lightPositionRed[] = { 2.0f, 0.0f, 0.0f, 1.0f };

float lightAmbientGreen[] = { 0.0f, 0.0f, 0.0f, 1.0f };
float lightDiffuseGreen[] = { 1.0f, 1.0f, 1.0f, 1.0f };
float lightSpecularGreen[] = { 1.0f, 1.0f, 1.0f, 1.0f };
float lightPositionGreen[] = { 0.0f, 0.0f, 2.0f, 1.0f };

float lightAmbientBlue[] = { 0.0f, 0.0f, 0.0f, 1.0f };
float lightDiffuseBlue[] = { 1.0f, 1.0f, 1.0f, 1.0f };
float lightSpecularBlue[] = { 1.0f, 1.0f, 1.0f, 1.0f };
float lightPositionBlue[] = { -2.0f, 0.0f, 0.0f, 1.0f };

float materialAmbient[MATERIAL_COLUMNS][MATERIAL_ROWS][4] = {
	{
		{0.0215f, 0.1745f, 0.0215f, 1.0f },
		{0.135f, 0.2225f, 0.1575f, 1.0f },
		{0.05375f, 0.05f, 0.06625f, 1.0f },
		{0.25f, 0.20725f, 0.20725f, 1.0f },
		{0.1745f, 0.01175f, 0.01175f, 1.0f },
		{0.1f, 0.18725f, 0.1745f, 1.0f }
	},
	{
		{0.329412f, 0.223529f, 0.027451f, 1.0f },
		{0.2125f, 0.1275f, 0.054f, 1.0f },
		{0.25f, 0.25f, 0.25f, 1.0f },
		{0.19125f, 0.0735f, 0.0225f, 1.0f },
		{0.24725f, 0.1995f, 0.0745f, 1.0f },
		{0.19225f, 0.19225f, 0.19225f, 1.0f }
	},
	{
		{0.0f, 0.0f, 0.0f, 1.0f },
		{0.0f, 0.1f, 0.06f, 1.0f },
		{0.0f, 0.0f, 0.0f, 1.0f },
		{0.0f, 0.0f, 0.0f, 1.0f },
		{0.0f, 0.0f, 0.0f, 1.0f },
		{0.0f, 0.0f, 0.0f, 1.0f }
	},
	{
		{0.02f, 0.02f, 0.02f, 1.0f },
		{0.0f, 0.05f, 0.05f, 1.0f },
		{0.0f, 0.05f, 0.0f, 1.0f },
		{0.05f, 0.0f, 0.0f, 1.0f },
		{0.05f, 0.05f, 0.05f, 1.0f },
		{0.05f, 0.05f, 0.0f, 1.0f }
	}
};

float materialDiffuse[MATERIAL_COLUMNS][MATERIAL_ROWS][4] = {
	{
		{0.07568f, 0.61424f, 0.07568f, 1.0f},
		{0.54f, 0.89f, 0.63f, 1.0f},
		{0.18275f, 0.17f, 0.22525f, 1.0f},
		{1.0f, 0.829f, 0.829f, 1.0f},
		{0.61424f, 0.04136f, 0.04136f, 1.0f},
		{0.396f, 0.74151f, 0.69102f, 1.0f},
	},
	{
		{0.780392f, 0.568627f, 0.113725f, 1.0f},
		{0.714f, 0.4284f, 0.18144f, 1.0f},
		{0.4f, 0.4f, 0.4f, 1.0f},
		{0.7038f, 0.27048f, 0.0828f, 1.0f},
		{0.75164f, 0.60648f, 0.22648f, 1.0f},
		{0.50754f, 0.50754f, 0.50754f, 1.0f},
	},
	{
		{0.01f, 0.01f, 0.01f, 1.0f},
		{0.0f, 0.50980392f, 0.50980392f, 1.0f},
		{0.1f, 0.35f, 0.1f, 1.0f},
		{0.5f, 0.0f, 0.0f, 1.0f},
		{0.55f, 0.55f, 0.55f, 1.0f},
		{0.5f, 0.5f, 0.0f, 1.0f},
	},
	{
		{0.01f, 0.01f, 0.01f, 1.0f},
		{0.4f, 0.5f, 0.5f, 1.0f},
		{0.4f, 0.5f, 0.4f, 1.0f},
		{0.5f, 0.4f, 0.4f, 1.0f},
		{0.5f, 0.5f, 0.5f, 1.0f},
		{0.5f, 0.5f, 0.4f, 1.0f},
	},
};

float materialSpecular[MATERIAL_COLUMNS][MATERIAL_ROWS][4] = {
	{
		{0.633f, 0.727811f, 0.633f, 1.0f},
		{0.316228f, 0.316228f, 0.316228f, 1.0f},
		{0.332741f, 0.328634f, 0.346435f, 1.0f},
		{0.296648f, 0.296648f, 0.296648f, 1.0f},
		{0.727811f, 0.626959f, 0.626959f, 1.0f},
		{0.297254f, 0.30829f, 0.306678f, 1.0f},
	},
	{
		{0.992157f, 0.941176f, 0.807843f, 1.0f},
		{0.393548f, 0.271906f, 0.166721f, 1.0f},
		{0.774597f, 0.774597f, 0.774597f, 1.0f},
		{0.256777f, 0.137622f, 0.086014f, 1.0f},
		{0.628281f, 0.555802f, 0.366065f, 1.0f},
		{0.508273f, 0.508273f, 0.508273f, 1.0f},
	},
	{
		{0.50f, 0.50f, 0.50f, 1.0f},
		{0.50196078f, 0.50196078f, 0.50196078f, 1.0f},
		{0.45f, 0.55f, 0.45f, 1.0f},
		{0.7f, 0.6f, 0.6f, 1.0f},
		{0.70f, 0.70f, 0.70f, 1.0f},
		{0.60f, 0.60f, 0.50f, 1.0f},
	},
	{
		{0.4f, 0.4f, 0.4f, 1.0f},
		{0.04f, 0.7f, 0.7f, 1.0f},
		{0.04f, 0.7f, 0.04f, 1.0f},
		{0.7f, 0.04f, 0.04f, 1.0f},
		{0.7f, 0.7f, 0.7f, 1.0f},
		{0.7f, 0.7f, 0.04f, 1.0f},
	}
};

float materialShininess[MATERIAL_COLUMNS][MATERIAL_ROWS] = {
	{
		0.6f * 128.0f,
		0.1f * 128.0f,
		0.3f * 128.0f,
		0.088f * 128.0f,
		0.6f * 128.0f,
		0.1f * 128.0f
	},
	{
		0.21794872f * 128.0f,
		0.2f * 128.0f,
		0.6f * 128.0f,
		0.1f * 128.0f,
		0.4f * 128.0f,
		0.4f * 128.0f
	},
	{
		0.25f * 128.0f,
		0.25f * 128.0f,
		0.25f * 128.0f,
		0.25f * 128.0f,
		0.25f * 128.0f,
		0.25f * 128.0f
	},
	{
		0.078125f * 128.0f,
		0.078125f * 128.0f,
		0.078125f * 128.0f,
		0.078125f * 128.0f,
		0.078125f * 128.0f,
		0.078125f * 128.0f
	}
};


XMMATRIX gPerspectiveProjectionMatrix;

float sphere_vertices[1146];
float sphere_normals[1146];
float sphere_textures[764];
unsigned short sphere_elements[2280];
unsigned int gNumElements;
unsigned int gNumVertices;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR szCmdLine, int iCmdShow)
{
	HRESULT initialize();
	void uninitialize();
	void display();

	bool bIsDone = false;
	int iRet = 0;
	TCHAR szAppName[] = TEXT("Direct3D11");
	TCHAR szErrMsg[MAX_PATH];
	HRESULT hr;
	WNDCLASSEX wndclass;
	HWND hWnd;
	MSG msg;

	iRet = fopen_s(&gpLogFile, gszLogFileName, "w");
	if (0 != iRet)
	{
		ZeroMemory(szErrMsg, MAX_PATH);
		//StringCchPrintfW(szErrMsg, MAX_PATH, TEXT("Log file creation failed.\nError code : %d"), GetLastError());
		wsprintf(szErrMsg, TEXT("Log file creation failed.\nError code : %d"), GetLastError());
		MessageBox(NULL, szErrMsg, TEXT("Error"), MB_OK | MB_ICONERROR);
		return 0;
	}
	else
	{
		fprintf_s(gpLogFile, "Log file created.\n");
		fclose(gpLogFile);
	}

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

	hWnd = CreateWindow(szAppName,
		TEXT("Direct3D11 3DShapesColouredAnimation"),
		WS_OVERLAPPEDWINDOW,
		100,
		100,
		WIN_WIDTH,
		WIN_HEIGHT,
		NULL,
		NULL,
		hInstance,
		0);

	if (hWnd == NULL)
	{
		fopen_s(&gpLogFile, "Log.txt", "a+");
		fprintf_s(gpLogFile, "CreateWindow() failed.\n");
		fclose(gpLogFile);
		exit(0);
	}

	ghWnd = hWnd;
	hr = initialize();
	WriteLog(hr, "initialize()");

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
				if (gbEscapeKeyIsPressed == true)
				{
					bIsDone = true;
				}
			}
			else
			{

			}
		}
	}

	uninitialize();

	return ((int)msg.wParam);
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	HRESULT resize(int, int);
	//void display();
	void ToggleFullScreen();
	void uninitialize();

	HRESULT hr;

	switch (iMsg)
	{
	case WM_ACTIVATE:
		if (HIWORD(wParam) == 0)
		{
			gbIsActiveWindow = true;
		}
		else
		{
			gbIsActiveWindow = false;
		}
		break;

	case WM_ERASEBKGND:
		return 0;

	case WM_SIZE:
		if (gpID3D11DeviceContext)
		{
			hr = resize(LOWORD(lParam), HIWORD(lParam));
			WriteLog(hr, "resize() ");
		}
		break;

	case WM_CLOSE:
		uninitialize();
		break;

	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_ESCAPE:
			DestroyWindow(hWnd);
			break;
		}
		break;
	case WM_CHAR:
		switch (wParam)
		{
		case 'F':
		case 'f':
			ToggleFullScreen();
			break;
		case 'L':
		case 'l':
			if (gbLighting)
			{
				gbLighting = false;
			}
			else
			{
				gbLighting = true;
			}
			break;
		case 'p':
		case 'P':
			if (gbPerPixelLighting)
			{
				gbPerPixelLighting = false;
			}
			else
			{
				gbPerPixelLighting = true;
			}
			break;
		case 'x':
		case 'X':
			gfAngleXAxisRot = 90.0f;
			giRotationAxis = enRA_X;
			break;

		case 'y':
		case 'Y':
			gfAngleYAxisRot = 90.0f;
			giRotationAxis = enRA_Y;
			break;

		case 'z':
		case 'Z':
			gfAngleZAxisRot = 90.0f;
			giRotationAxis = enRA_Z;
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

HRESULT initialize()
{
	void uninitialize();
	HRESULT resize(int, int);

	HRESULT hr;
	D3D_DRIVER_TYPE d3dDriverType;
	D3D_DRIVER_TYPE d3dDriverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE, D3D_DRIVER_TYPE_WARP, D3D_DRIVER_TYPE_REFERENCE
	};

	D3D_FEATURE_LEVEL d3dFeatureLevelRequired = D3D_FEATURE_LEVEL_11_0;
	D3D_FEATURE_LEVEL d3dFeatureLevelAcquired = D3D_FEATURE_LEVEL_10_0;

	UINT createDeviceFlags = 0;
	UINT numDriverTypes = 0;
	UINT numFeatureLevels = 1;

	numDriverTypes = sizeof(d3dDriverTypes) / sizeof(d3dDriverTypes[0]);

	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	ZeroMemory((void*)&dxgiSwapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));

	dxgiSwapChainDesc.BufferCount = 1;
	dxgiSwapChainDesc.BufferDesc.Width = WIN_WIDTH;
	dxgiSwapChainDesc.BufferDesc.Height = WIN_HEIGHT;
	dxgiSwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	dxgiSwapChainDesc.OutputWindow = ghWnd;
	dxgiSwapChainDesc.SampleDesc.Count = 1;
	dxgiSwapChainDesc.SampleDesc.Quality = 0;
	dxgiSwapChainDesc.Windowed = TRUE;

	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		d3dDriverType = d3dDriverTypes[driverTypeIndex];
		hr = D3D11CreateDeviceAndSwapChain(
			NULL,
			d3dDriverType,
			NULL,
			createDeviceFlags,
			&d3dFeatureLevelRequired,
			numFeatureLevels,
			D3D11_SDK_VERSION,
			&dxgiSwapChainDesc,
			&gpIDXGISwapChain,
			&gpID3D11Device,
			&d3dFeatureLevelAcquired,
			&gpID3D11DeviceContext
		);

		if (SUCCEEDED(hr))
		{
			break;
		}
	}

	WriteLog(hr, "D3D11CreateDeviceAndSwapChain() ");

	fopen_s(&gpLogFile, gszLogFileName, "a+");
	fprintf_s(gpLogFile, "The chosen friver is of ");
	if (d3dDriverType == D3D_DRIVER_TYPE_HARDWARE)
	{
		fprintf_s(gpLogFile, "Hardware type.\n");
	}
	else if (d3dDriverType == D3D_DRIVER_TYPE_WARP)
	{
		fprintf_s(gpLogFile, "Warp type.\n");
	}
	else if (d3dDriverType == D3D_DRIVER_TYPE_REFERENCE)
	{
		fprintf_s(gpLogFile, "Reference type.\n");
	}
	else
	{
		fprintf_s(gpLogFile, "Unknown type.\n");
	}

	fprintf_s(gpLogFile, "The supported highest feature level is ");
	if (d3dFeatureLevelAcquired == D3D_FEATURE_LEVEL_11_0)
	{
		fprintf_s(gpLogFile, "11.0\n");
	}
	else if (d3dFeatureLevelAcquired == D3D_FEATURE_LEVEL_10_1)
	{
		fprintf_s(gpLogFile, "10.1\n");
	}
	else if (d3dFeatureLevelAcquired == D3D_FEATURE_LEVEL_10_0)
	{
		fprintf_s(gpLogFile, "10.0\n");
	}
	else
	{
		fprintf_s(gpLogFile, "Unknown.\n");
	}

	fclose(gpLogFile);

	const char* vertexShaderSourceCode =
		"cbuffer ConstantBuffer" \
		"{" \
		"float4x4 worldMatrix;" \
		"float4x4 viewMatrix;" \
		"float4x4 projectionMatrix;" \
		"float4 laR;" \
		"float4 laG;" \
		"float4 laB;" \
		"float4 ka;" \
		"float4 ldR;" \
		"float4 ldG;" \
		"float4 ldB;" \
		"float4 kd;" \
		"float4 lsR;" \
		"float4 lsG;" \
		"float4 lsB;" \
		"float4 ks;" \
		"float4 lightPositionR;" \
		"float4 lightPositionG;" \
		"float4 lightPositionB;" \
		"float materialShininess;" \
		"uint lKeyPressed;" \
		"uint lPixelKeyPressed;" \
		"}\n" \
		"struct vertex_output" \
		"{" \
		"float4 position	:	SV_POSITION;" \
		"float3 transformedNormal :	NORMAL0;" \
		"float3 lightDirectionR	 :	NORMAL1;" \
		"float3 lightDirectionG	 :	NORMAL2;" \
		"float3 lightDirectionB	 :	NORMAL3;" \
		"float3 viewVector		 :	NORMAL4;" \
		"float4 phong_ads_light	:	COLOR;" \
		"};" \
		"\n" \
		"float4 CalcPerVerColor(float4 laa, float4 ldd, float4 lss, float3 poss, float4 nrml, float4 eyeCoord);" \
		"\n" \
		"vertex_output main(float4 pos: POSITION, float4 normal: NORMAL)" \
		"{" \
		"vertex_output output;" \
		"if(lKeyPressed == 1)" \
		"{" \
		"float4 eyeCoordinates = mul(worldMatrix, pos);" \
		"eyeCoordinates = mul(viewMatrix, eyeCoordinates);" \
		"if(lPixelKeyPressed == 1)" \
		"{" \
		"output.transformedNormal = mul((float3x3)(mul(viewMatrix, worldMatrix)), (float3)normal);" \
		"output.lightDirectionR = (float3)(lightPositionR - eyeCoordinates);" \
		"output.lightDirectionG = (float3)(lightPositionG - eyeCoordinates);" \
		"output.lightDirectionB = (float3)(lightPositionB - eyeCoordinates);" \
		"output.viewVector = normalize(-eyeCoordinates.xyz);" \
		"}" \
		"else" \
		"{" \
		"float4 phong_ads_colorR = CalcPerVerColor(laR, ldR, lsR, lightPositionR, normal, eyeCoordinates);" \
		"float4 phong_ads_colorG = CalcPerVerColor(laG, ldG, lsG, lightPositionG, normal, eyeCoordinates);" \
		"float4 phong_ads_colorB = CalcPerVerColor(laB, ldB, lsB, lightPositionB, normal, eyeCoordinates);" \
		"output.phong_ads_light = phong_ads_colorR /*+ phong_ads_colorG + phong_ads_colorB*/;" \
		"}" \
		"}" \
		"float4 out_position = mul(worldMatrix, pos);" \
		"out_position = mul(viewMatrix, out_position);" \
		"out_position = mul(projectionMatrix, out_position);" \
		"output.position = out_position;" \
		"return output;" \
		"}\n" \
		"float4 CalcPerVerColor(float4 la, float4 ld, float4 ls, float3 lightPosition, float4 normal, float4 eyeCoordinates)" \
		"{" \
		"float3 transposeNormal = normalize(mul((float3x3)mul(viewMatrix, worldMatrix), (float3)normal));" \
		"float3 source_light_direction = (float3)normalize(lightPosition.xyz - eyeCoordinates.xyz);" \
		"float transposeNormalDotLightDir = max(dot(transposeNormal, source_light_direction), 0.0);" \
		"float4 ambient = la * ka;" \
		"float4 diffuse = ld * kd * transposeNormalDotLightDir;" \
		"float3 reflectionVector = reflect(-source_light_direction, transposeNormal);" \
		"float3 viewVector = normalize(-eyeCoordinates.xyz);" \
		"float4 specular = ls * ks * pow(max(dot(reflectionVector, viewVector), 0.0), materialShininess);" \
		"float4 phong_ads_color = ambient + diffuse + specular;" \
		"return phong_ads_color;" \
		"}";


	ID3DBlob *pID3DBlob_VertexShaderCode = NULL;
	ID3DBlob *pID3DBlob_Error = NULL;

	hr = D3DCompile(vertexShaderSourceCode,
		lstrlenA(vertexShaderSourceCode) + 1,
		"VS",
		NULL,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",
		"vs_5_0",
		0,
		0,
		&pID3DBlob_VertexShaderCode,
		&pID3DBlob_Error);

	if (FAILED(hr))
	{
		if (pID3DBlob_Error != NULL)
		{
			fopen_s(&gpLogFile, gszLogFileName, "a+");
			fprintf_s(gpLogFile, "D3DCompile() Failed For Vertex Shader : %s.\n", (char*)pID3DBlob_Error->GetBufferPointer());
			fclose(gpLogFile);
			pID3DBlob_Error->Release();
			pID3DBlob_Error = NULL;
			return(hr);
		}
	}
	else
	{
		fopen_s(&gpLogFile, gszLogFileName, "a+");
		fprintf_s(gpLogFile, "D3DCompile() Succeeded For Vertex Shader.\n ");
		fclose(gpLogFile);
	}

	hr = gpID3D11Device->CreateVertexShader(pID3DBlob_VertexShaderCode->GetBufferPointer(),
		pID3DBlob_VertexShaderCode->GetBufferSize(),
		NULL,
		&gpID3D11VertexShader);

	if (FAILED(hr))
	{
		fopen_s(&gpLogFile, gszLogFileName, "a+");
		fprintf_s(gpLogFile, "ID3D11Device::CreateVertexShader() Failed.\n");
		fclose(gpLogFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpLogFile, gszLogFileName, "a+");
		fprintf_s(gpLogFile, "ID3D11Device::CreateVertexShader() Succeeded. \n ");
		fclose(gpLogFile);
	}

	const char* pixelShaderSourceCode =
		"cbuffer ConstantBuffer" \
		"{" \
		"float4x4 worldMatrix;" \
		"float4x4 viewMatrix;" \
		"float4x4 projectionMatrix;" \
		"float4 laR;" \
		"float4 laG;" \
		"float4 laB;" \
		"float4 ka;" \
		"float4 ldR;" \
		"float4 ldG;" \
		"float4 ldB;" \
		"float4 kd;" \
		"float4 lsR;" \
		"float4 lsG;" \
		"float4 lsB;" \
		"float4 ks;" \
		"float4 lightPositionR;" \
		"float4 lightPositionG;" \
		"float4 lightPositionB;" \
		"float materialShininess;" \
		"uint lKeyPressed;" \
		"uint lPixelKeyPressed;" \
		"}" \
		"\n" \
		"struct vertex_output" \
		"{" \
		"float4 position			 :	SV_POSITION;" \
		"float3 transformedNormal :	NORMAL0;" \
		"float3 lightDirectionR	 :	NORMAL1;" \
		"float3 lightDirectionG	 :	NORMAL2;" \
		"float3 lightDirectionB	 :	NORMAL3;" \
		"float3 viewVector		 :	NORMAL4;" \
		"float4 phong_ads_light	:	COLOR;" \
		"};" \
		"\n" \
		"float4 CalcPerPixelColor(float4 laaa, float4 lddd, float4 lsss, float3 posss, float3 vv, float3 tn);\n" \
		"float4 main(vertex_output output): SV_TARGET" \
		"{" \
		"float4 phong_ads_light_per_pixelR;" \
		"float4 phong_ads_light_per_pixelG;" \
		"float4 phong_ads_light_per_pixelB;" \
		"float4 phong_ads_light_per_pixel;"
		"if(lKeyPressed == 1)" \
		"{" \
		"if(lPixelKeyPressed == 1)" \
		"{" \
		"phong_ads_light_per_pixelR = CalcPerPixelColor(laR, ldR, ldR, output.lightDirectionR, output.viewVector, output.transformedNormal);\n" \
		"phong_ads_light_per_pixelG = CalcPerPixelColor(laG, ldG, ldG, output.lightDirectionG, output.viewVector, output.transformedNormal);\n" \
		"phong_ads_light_per_pixelB = CalcPerPixelColor(laB, ldB, ldB, output.lightDirectionB, output.viewVector, output.transformedNormal);\n" \
		"phong_ads_light_per_pixel = phong_ads_light_per_pixelR /*+ phong_ads_light_per_pixelG + phong_ads_light_per_pixelB*/;" \
		"}" \
		"else" \
		"{" \
		"return(output.phong_ads_light);" \
		"}" \
		"}" \
		"else" \
		"{" \
		"phong_ads_light_per_pixel = float4(1.0, 1.0, 1.0, 1.0);" \
		"}" \
		"return phong_ads_light_per_pixel;" \
		"}\n"\
		"float4 CalcPerPixelColor(float4 la, float4 ld, float4 ls, float3 lightDirection, float3 viewVector, float3 transformedNormal)" \
		"{" \
		"float3 normTransposeNormal = normalize(transformedNormal);" \
		"float3 normSource_light_direction = normalize(lightDirection);" \
		"float3 normviewerVector = normalize(viewVector);" \
		"float transposeNormalDotLightDir = max(dot(normSource_light_direction, normTransposeNormal), 0.0);" \
		"float4 ambient = la * ka;" \
		"float4 diffuse = ld * kd * transposeNormalDotLightDir;" \
		"float3 reflectionVector = reflect(-normSource_light_direction, normTransposeNormal);" \
		"float4 specular = ls * ks * pow(max(dot(reflectionVector, normviewerVector), 0.0), materialShininess);" \
		"float4 phong_ads_light_color = ambient + diffuse + specular;" \
		"return phong_ads_light_color;" \
		"}";

	ID3DBlob * pID3DBlob_PixelShaderCode = NULL;
	pID3DBlob_Error = NULL;
	hr = D3DCompile(pixelShaderSourceCode,
		lstrlenA(pixelShaderSourceCode) + 1,
		"PS",
		NULL,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",
		"ps_5_0",
		0,
		0,
		&pID3DBlob_PixelShaderCode,
		&pID3DBlob_Error);

	if (FAILED(hr))
	{
		if (pID3DBlob_Error != NULL)
		{
			fopen_s(&gpLogFile, gszLogFileName, "a+");
			fprintf_s(gpLogFile, "D3DCompile() Failed For Pixel Shader : %s.\n", (char*)pID3DBlob_Error->GetBufferPointer());
			fclose(gpLogFile);
			pID3DBlob_Error->Release();
			pID3DBlob_Error = NULL;
			return(hr);
		}
	}
	else
	{
		fopen_s(&gpLogFile, gszLogFileName, "a+");
		fprintf_s(gpLogFile, "D3DCompile() Succeeded For Pixel Shader.\n ");
		fclose(gpLogFile);
	}

	hr = gpID3D11Device->CreatePixelShader(pID3DBlob_PixelShaderCode->GetBufferPointer(),
		pID3DBlob_PixelShaderCode->GetBufferSize(),
		NULL,
		&gpID3D11PixelShader);

	if (FAILED(hr))
	{
		fopen_s(&gpLogFile, gszLogFileName, "a+");
		fprintf_s(gpLogFile, "ID3D11Device::CreatePixelShader() Failed.\n");
		fclose(gpLogFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpLogFile, gszLogFileName, "a+");
		fprintf_s(gpLogFile, "ID3D11Device::CreatePixelShader() Succeeded.\n ");
		fclose(gpLogFile);
	}

	D3D11_INPUT_ELEMENT_DESC inputElementDesc[2];
	inputElementDesc[0].SemanticName = "POSITION"; // need same in shader
	inputElementDesc[0].SemanticIndex = 0;
	inputElementDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDesc[0].InputSlot = 0; //  enum amc_pos
	inputElementDesc[0].AlignedByteOffset = 0;
	inputElementDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	inputElementDesc[0].InstanceDataStepRate = 0;

	inputElementDesc[1].SemanticName = "NORMAL"; // need same in shader
	inputElementDesc[1].SemanticIndex = 0;
	inputElementDesc[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDesc[1].InputSlot = 1;	// amc_clr
	inputElementDesc[1].AlignedByteOffset = 0;
	inputElementDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	inputElementDesc[1].InstanceDataStepRate = 0;

	hr = gpID3D11Device->CreateInputLayout(inputElementDesc,
		_ARRAYSIZE(inputElementDesc),
		pID3DBlob_VertexShaderCode->GetBufferPointer(),
		pID3DBlob_VertexShaderCode->GetBufferSize(),
		&gpID3D11InputLayout);

	if (FAILED(hr))
	{
		fopen_s(&gpLogFile, gszLogFileName, "a+");
		fprintf_s(gpLogFile, "ID3D11Device::CreateInputLayout() Failed.\n");
		fclose(gpLogFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpLogFile, gszLogFileName, "a+");
		fprintf_s(gpLogFile, "ID3D11Device::CreateInputLayout() Succeeded.\n");
		fclose(gpLogFile);
	}

	gpID3D11DeviceContext->VSSetShader(gpID3D11VertexShader, 0, 0);
	pID3DBlob_VertexShaderCode->Release();
	pID3DBlob_VertexShaderCode = NULL;

	gpID3D11DeviceContext->PSSetShader(gpID3D11PixelShader, 0, 0);
	pID3DBlob_PixelShaderCode->Release();
	pID3DBlob_PixelShaderCode = NULL;

	gpID3D11DeviceContext->IASetInputLayout(gpID3D11InputLayout);


	D3D11_BUFFER_DESC bufferDesc;
	D3D11_MAPPED_SUBRESOURCE mappedSubresource;

	getSphereVertexData(sphere_vertices, sphere_normals, sphere_textures, sphere_elements);
	gNumVertices = getNumberOfSphereVertices();
	gNumElements = getNumberOfSphereElements();

	ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = sizeof(float) * ARRAYSIZE(sphere_vertices);
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	hr = gpID3D11Device->CreateBuffer(&bufferDesc, NULL, &gpID3D11Buffer_VertexBuffer_Sphere);

	if (FAILED(hr))
	{
		fopen_s(&gpLogFile, gszLogFileName, "a+");
		fprintf_s(gpLogFile, "ID3D11Device::CreateBuffer() Failed For Vertex Buffer for quad.\n");
		fclose(gpLogFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpLogFile, gszLogFileName, "a+");
		fprintf_s(gpLogFile, "ID3D11Device::CreateBuffer() Succeeded For Vertex Buffer for quad.\n");
		fclose(gpLogFile);
	}

	ZeroMemory(&mappedSubresource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	gpID3D11DeviceContext->Map(gpID3D11Buffer_VertexBuffer_Sphere,
		NULL,
		D3D11_MAP_WRITE_DISCARD,
		NULL,
		&mappedSubresource);
	memcpy(mappedSubresource.pData, sphere_vertices, sizeof(sphere_vertices));
	gpID3D11DeviceContext->Unmap(gpID3D11Buffer_VertexBuffer_Sphere, NULL);

	//create vertex buffer
	ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = sizeof(float) * ARRAYSIZE(sphere_normals);
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	hr = gpID3D11Device->CreateBuffer(&bufferDesc, NULL, &gpID3D11Buffer_VertexBuffer_Sphere_Normal);

	if (FAILED(hr))
	{
		fopen_s(&gpLogFile, gszLogFileName, "a+");
		fprintf_s(gpLogFile, "ID3D11Device::CreateBuffer() Failed For Vertex Buffer for quad normal.\n");
		fclose(gpLogFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpLogFile, gszLogFileName, "a+");
		fprintf_s(gpLogFile, "ID3D11Device::CreateBuffer() Succeeded For Vertex Buffer for quad normal.\n");
		fclose(gpLogFile);
	}

	ZeroMemory(&mappedSubresource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	gpID3D11DeviceContext->Map(gpID3D11Buffer_VertexBuffer_Sphere_Normal,
		NULL,
		D3D11_MAP_WRITE_DISCARD,
		NULL,
		&mappedSubresource);
	memcpy(mappedSubresource.pData, sphere_normals, sizeof(sphere_normals));
	gpID3D11DeviceContext->Unmap(gpID3D11Buffer_VertexBuffer_Sphere_Normal, NULL);

	ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = gNumElements * sizeof(short);
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	hr = gpID3D11Device->CreateBuffer(&bufferDesc, NULL, &gpID3D11Buffer_IndexBuffer);
	if (FAILED(hr))
	{
		fopen_s(&gpLogFile, gszLogFileName, "a+");
		fprintf_s(gpLogFile, "ID3D11Device::CreateBuffer() Failed For Index Buffer.\n");
		fclose(gpLogFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpLogFile, gszLogFileName, "a+");
		fprintf_s(gpLogFile, "ID3D11Device::CreateBuffer() Succeeded For Index Buffer.\n");
		fclose(gpLogFile);
	}

	ZeroMemory(&mappedSubresource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	gpID3D11DeviceContext->Map(gpID3D11Buffer_IndexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mappedSubresource);
	memcpy(mappedSubresource.pData, sphere_elements, gNumElements * sizeof(short));
	gpID3D11DeviceContext->Unmap(gpID3D11Buffer_IndexBuffer, NULL); // unmap buffer


	D3D11_BUFFER_DESC bufferDesc_ConstantBuffer;
	ZeroMemory(&bufferDesc_ConstantBuffer, sizeof(D3D11_BUFFER_DESC));
	bufferDesc_ConstantBuffer.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc_ConstantBuffer.ByteWidth = sizeof(CBUFFER);
	bufferDesc_ConstantBuffer.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	hr = gpID3D11Device->CreateBuffer(&bufferDesc_ConstantBuffer,
		nullptr,
		&gpID3D11Buffer_ConstantBuffer);

	if (FAILED(hr))
	{
		fopen_s(&gpLogFile, gszLogFileName, "a+");
		fprintf_s(gpLogFile, "ID3D11Device::CreateInputLayout() Failed For Constant Buffer.\n");
		fclose(gpLogFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpLogFile, gszLogFileName, "a+");
		fprintf_s(gpLogFile, "ID3D11Device::CreateInputLayout() Succeeded For Constant Buffer.\n");
		fclose(gpLogFile);
	}
	gpID3D11DeviceContext->VSSetConstantBuffers(0, 1, &gpID3D11Buffer_ConstantBuffer);
	gpID3D11DeviceContext->PSSetConstantBuffers(0, 1, &gpID3D11Buffer_ConstantBuffer);


	D3D11_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory((void*)&rasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));
	rasterizerDesc.AntialiasedLineEnable = FALSE;
	rasterizerDesc.CullMode = D3D11_CULL_NONE; // backface culling
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.DepthBiasClamp = 0.0f;
	rasterizerDesc.DepthClipEnable = TRUE;
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.FrontCounterClockwise = FALSE;
	rasterizerDesc.MultisampleEnable = FALSE;
	rasterizerDesc.ScissorEnable = FALSE;
	rasterizerDesc.SlopeScaledDepthBias = 0.0f;
	hr = gpID3D11Device->CreateRasterizerState(&rasterizerDesc, &gpID3D11RasterizerState);
	if (FAILED(hr))
	{
		fopen_s(&gpLogFile, gszLogFileName, "a+");
		fprintf_s(gpLogFile, "ID3D11Device::CreateRasterizerState() Failed For Culling.\n");
		fclose(gpLogFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpLogFile, gszLogFileName, "a+");
		fprintf_s(gpLogFile, "ID3D11Device::CreateRasterizerState() Succeeded For Culling.\n");
		fclose(gpLogFile);
	}
	gpID3D11DeviceContext->RSSetState(gpID3D11RasterizerState);

	gfClearColor[0] = 0.5f;
	gfClearColor[1] = 0.5f;
	gfClearColor[2] = 0.5f;
	gfClearColor[3] = 1.0f;

	gPerspectiveProjectionMatrix = XMMatrixIdentity();

	hr = resize(WIN_WIDTH, WIN_HEIGHT);
	WriteLog(hr, "resize() ");

	return 0;
}

HRESULT resize(int iWidth, int iHeight)
{
	HRESULT hr = S_OK;

	gW = (float)iWidth / (float)MATERIAL_ROWS;
	gH = (float)iHeight / (float)MATERIAL_ROWS;
	gX = (iWidth - (gW * MATERIAL_COLUMNS)) / 2;
	gY = (iHeight - (gH * MATERIAL_ROWS)) / 2;

	if (gpID3D11DepthStencilView)
	{
		gpID3D11DepthStencilView->Release();
		gpID3D11DepthStencilView = NULL;
	}

	if (gpID3D11RenderTargetView)
	{
		gpID3D11RenderTargetView->Release();
		gpID3D11RenderTargetView = NULL;
	}

	gpIDXGISwapChain->ResizeBuffers(1, iWidth, iHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0);

	ID3D11Texture2D *pID3D11Texture2DBackBuffer = NULL;
	gpIDXGISwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pID3D11Texture2DBackBuffer);

	hr = gpID3D11Device->CreateRenderTargetView(pID3D11Texture2DBackBuffer, NULL, &gpID3D11RenderTargetView);
	WriteLog(hr, "gpID3D11Device->CreateRenderTargetView() ");
	pID3D11Texture2DBackBuffer->Release();
	pID3D11Texture2DBackBuffer = NULL;
	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(D3D11_TEXTURE2D_DESC));
	textureDesc.Width = (UINT)iWidth;
	textureDesc.Height = (UINT)iHeight;
	textureDesc.ArraySize = 1;
	textureDesc.MipLevels = 1;
	textureDesc.SampleDesc.Count = 1; //in real world, this can be up to 4
	textureDesc.SampleDesc.Quality = 0; //if above is 4, then it is 1
	textureDesc.Format = DXGI_FORMAT_D32_FLOAT;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	ID3D11Texture2D* pID3D11Texture2D_DepthBuffer;
	gpID3D11Device->CreateTexture2D(&textureDesc, NULL, &pID3D11Texture2D_DepthBuffer);

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	ZeroMemory(&depthStencilViewDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
	depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
	hr = gpID3D11Device->CreateDepthStencilView(pID3D11Texture2D_DepthBuffer,
		&depthStencilViewDesc,
		&gpID3D11DepthStencilView);
	if (FAILED(hr))
	{
		fopen_s(&gpLogFile, gszLogFileName, "a+");
		fprintf_s(gpLogFile, "ID3D11Device::CreateDepthStencilView() Failed.\n");
		fclose(gpLogFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpLogFile, gszLogFileName, "a+");
		fprintf_s(gpLogFile, "ID3DllDevice::CreateDepthStencilView() Succeeded.\n");
		fclose(gpLogFile);
	}
	pID3D11Texture2D_DepthBuffer->Release();
	pID3D11Texture2D_DepthBuffer = NULL;

	//set render target view as render target
	gpID3D11DeviceContext->OMSetRenderTargets(1, &gpID3D11RenderTargetView, gpID3D11DepthStencilView);

	D3D11_VIEWPORT d3dViewPort;
	d3dViewPort.TopLeftX = 0;
	d3dViewPort.TopLeftY = 0;
	d3dViewPort.Width = (float)iWidth;
	d3dViewPort.Height = (float)iHeight;
	d3dViewPort.MinDepth = 0.0f;
	d3dViewPort.MaxDepth = 1.0f;

	gpID3D11DeviceContext->RSSetViewports(1, &d3dViewPort);

	if (iHeight == 0)
	{
		iHeight = 1;
	}

	gPerspectiveProjectionMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(45.0f),
		float(iWidth) / float(iHeight),
		0.1f,
		100.0f);

	return hr;
}

void display()
{
	void Draw24Spheres();
	static float fAngleRot = 0;
	gpID3D11DeviceContext->ClearRenderTargetView(gpID3D11RenderTargetView, gfClearColor);
	gpID3D11DeviceContext->ClearDepthStencilView(gpID3D11DepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	Draw24Spheres();

	gpIDXGISwapChain->Present(0, 0);

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

void uninitialize()
{
	void ToggleFullScreen();

	if (gbIsFullScreen == true)
	{
		ToggleFullScreen();
	}

	if (gpID3D11Buffer_ConstantBuffer)
	{
		gpID3D11Buffer_ConstantBuffer->Release();
		gpID3D11Buffer_ConstantBuffer = NULL;
	}
	if (gpID3D11InputLayout)
	{
		gpID3D11InputLayout->Release();
		gpID3D11InputLayout = NULL;
	}

	if (gpID3D11PixelShader)
	{
		gpID3D11PixelShader->Release();
		gpID3D11PixelShader = NULL;
	}
	if (gpID3D11VertexShader)
	{
		gpID3D11VertexShader->Release();
		gpID3D11VertexShader = NULL;
	}

	if (gpID3D11RenderTargetView)
	{
		gpID3D11RenderTargetView->Release();
		gpID3D11RenderTargetView = NULL;
	}

	if (gpIDXGISwapChain)
	{
		gpIDXGISwapChain->Release();
		gpIDXGISwapChain = NULL;
	}

	if (gpID3D11DeviceContext)
	{
		gpID3D11DeviceContext->Release();
		gpID3D11DeviceContext = NULL;
	}

	if (gpID3D11Device)
	{
		gpID3D11Device->Release();
		gpID3D11Device = NULL;
	}

	if (gpLogFile)
	{
		fopen_s(&gpLogFile, gszLogFileName, "a+");
		fprintf_s(gpLogFile, "uninitialize() succeeded.\n Log file is closed.");
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

void WriteLog(HRESULT hr, const char* strLog)
{
	fopen_s(&gpLogFile, gszLogFileName, "a+");

	if (FAILED(hr))
	{
		fprintf_s(gpLogFile, "%s failed.\n", strLog);
		fclose(gpLogFile);
		DestroyWindow(ghWnd);
	}
	else
	{
		fprintf_s(gpLogFile, "%s success.\n", strLog);
		fclose(gpLogFile);
	}
}

void Draw24Spheres()
{
	void drawSphere(float topX, float topY, int row, int col);

	for (int itr1 = 0; itr1 < MATERIAL_COLUMNS; itr1++)
	{
		for (int itr = 0; itr < MATERIAL_ROWS; itr++)
		{
			float topX = gX + (itr1 * gW);
			float topY = gY + (itr * gH);
			drawSphere(topX, topY, itr, itr1);
		}
	}

}

void drawSphere(float topX, float topY, int row, int col)
{

	D3D11_VIEWPORT viewPort;

	viewPort.Width = (float)gW;
	viewPort.Height = (float)gH;
	viewPort.MinDepth = 0.0f;
	viewPort.MaxDepth = 1.0f;

	viewPort.TopLeftX = topX;
	viewPort.TopLeftY = topY;
	gpID3D11DeviceContext->RSSetViewports(1, &viewPort);

	UINT stride = sizeof(float) * 3;
	UINT offset = 0;
	gpID3D11DeviceContext->IASetVertexBuffers(0, 1, &gpID3D11Buffer_VertexBuffer_Sphere, &stride, &offset);
	gpID3D11DeviceContext->IASetVertexBuffers(1, 1, &gpID3D11Buffer_VertexBuffer_Sphere_Normal, &stride, &offset);
	gpID3D11DeviceContext->IASetIndexBuffer(gpID3D11Buffer_IndexBuffer, DXGI_FORMAT_R16_UINT, 0);
	gpID3D11DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	XMMATRIX worldMatrix = XMMatrixIdentity();
	XMMATRIX viewMatrix = XMMatrixIdentity();
	XMMATRIX translationMatrix = XMMatrixIdentity();
	XMMATRIX rotationMatrix = XMMatrixIdentity();
	XMMATRIX scaleMatrix = XMMatrixIdentity();
	XMMATRIX projectionMatrix = XMMatrixIdentity();

	translationMatrix = XMMatrixTranslation(0.0f, 0.0f, 3.0f);
	//rotationMatrix = XMMatrixRotationY(fAngleRot);
	worldMatrix = scaleMatrix * rotationMatrix * translationMatrix;

	XMMATRIX wMatrix = worldMatrix;
	XMMATRIX vMatrix = viewMatrix;
	XMMATRIX pMatrix = projectionMatrix * gPerspectiveProjectionMatrix;

	CBUFFER constantBuffer;
	//ZeroMemory(&constantBuffer, sizeof(CBUFFER));
	constantBuffer.WorldMatrix = wMatrix;
	constantBuffer.ViewMatrix = vMatrix;
	constantBuffer.ProjectionMatrix = pMatrix;
	if (gbLighting == true)
	{
		if (gbPerPixelLighting == true)
		{
			constantBuffer.lPerPixelKeyPressed = 1;
		}
		else
		{
			constantBuffer.lPerPixelKeyPressed = 0;
		}
		constantBuffer.lKeyPressed = 1;


		if (enRA_X == giRotationAxis)
		{
			lightPositionRed[0] = 0.0f;
			lightPositionRed[1] = (float)sin(gfAngleXAxisRot) * 100;
			lightPositionRed[2] = (float)cos(gfAngleXAxisRot) * 100;
		}
		else if (enRA_Y == giRotationAxis)
		{
			lightPositionRed[0] = sinf(gfAngleYAxisRot) * 100.0f;
			lightPositionRed[1] = 0.0f;
			lightPositionRed[2] = cosf(gfAngleYAxisRot) * 100.0f + 3.0f;
		}
		else if (enRA_Z == giRotationAxis)
		{
			lightPositionRed[0] = cosf(gfAngleZAxisRot) * 100.0f;
			lightPositionRed[1] = sinf(gfAngleZAxisRot) * 100.0f;
			lightPositionRed[2] = 0.0f;
		}

		constantBuffer.laR = XMVectorSet(lightAmbientRed[0], lightAmbientRed[1], lightAmbientRed[2], lightAmbientRed[3]);
		constantBuffer.ldR = XMVectorSet(lightDiffuseRed[0], lightDiffuseRed[1], lightDiffuseRed[2], lightDiffuseRed[3]);
		constantBuffer.lsR = XMVectorSet(lightSpecularRed[0], lightSpecularRed[1], lightSpecularRed[2], lightSpecularRed[3]);
		constantBuffer.lightPositionR = XMVectorSet(lightPositionRed[0], lightPositionRed[1], lightPositionRed[2], lightPositionRed[3]);

		constantBuffer.laG = XMVectorSet(lightAmbientGreen[0], lightAmbientGreen[1], lightAmbientGreen[2], lightAmbientGreen[3]);
		constantBuffer.ldG = XMVectorSet(lightDiffuseGreen[0], lightDiffuseGreen[1], lightDiffuseGreen[2], lightDiffuseGreen[3]);
		constantBuffer.lsG = XMVectorSet(lightSpecularGreen[0], lightSpecularGreen[1], lightSpecularGreen[2], lightSpecularGreen[3]);
		constantBuffer.lightPositionG = XMVectorSet(lightPositionGreen[0], lightPositionGreen[1], lightPositionGreen[2], lightPositionGreen[3]);

		constantBuffer.laB = XMVectorSet(lightAmbientBlue[0], lightAmbientBlue[1], lightAmbientBlue[2], lightAmbientBlue[3]);
		constantBuffer.ldB = XMVectorSet(lightDiffuseBlue[0], lightDiffuseBlue[1], lightDiffuseBlue[2], lightDiffuseBlue[3]);
		constantBuffer.lsB = XMVectorSet(lightSpecularBlue[0], lightSpecularBlue[1], lightSpecularBlue[2], lightSpecularBlue[3]);
		constantBuffer.lightPositionB = XMVectorSet(lightPositionBlue[0], lightPositionBlue[1], lightPositionBlue[2], lightPositionBlue[3]);

		constantBuffer.ka = XMVectorSet(materialAmbient[col][row][0], materialAmbient[col][row][1], materialAmbient[col][row][2], materialAmbient[col][row][3]);
		constantBuffer.kd = XMVectorSet(materialDiffuse[col][row][0], materialDiffuse[col][row][1], materialDiffuse[col][row][2], materialDiffuse[col][row][3]);
		constantBuffer.ks = XMVectorSet(materialSpecular[col][row][0], materialSpecular[col][row][1], materialSpecular[col][row][2], materialSpecular[col][row][3]);
		constantBuffer.materialShininess = materialShininess[col][row];
	}
	else
	{
		constantBuffer.lKeyPressed = 0;
	}
	gpID3D11DeviceContext->UpdateSubresource(gpID3D11Buffer_ConstantBuffer, 0, NULL, &constantBuffer, 0, 0);

	gpID3D11DeviceContext->DrawIndexed(gNumElements, 0, 0);

}
