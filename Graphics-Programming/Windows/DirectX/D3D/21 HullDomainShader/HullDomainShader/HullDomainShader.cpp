#include <Windows.h>
#include <stdio.h>
#include <d3d11.h>
#include <d3dcompiler.h>

#pragma warning( disable : 4838 )
#include "..\..\XNAMath\xnamath.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment (lib, "d3dcompiler.lib")

#define WIN_WIDTH	800
#define WIN_HEIGHT	600

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

float gfClearColor[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
IDXGISwapChain *gpIDXGISwapChain = NULL;
ID3D11Device *gpID3D11Device = NULL;
ID3D11DeviceContext *gpID3D11DeviceContext = NULL;
ID3D11RenderTargetView *gpID3D11RenderTargetView = NULL;

ID3D11VertexShader *gpID3D11VertexShader = NULL;
ID3D11PixelShader *gpID3D11PixelShader = NULL;
ID3D11HullShader* gpID3D11HullShader = NULL;
ID3D11DomainShader* gpID3D11DomainShader = NULL;

ID3D11Buffer *gpID3D11Buffer_VertexBuffer_Triangle = NULL;
ID3D11Buffer *gpID3D11Buffer_VertexBuffer_Triangle_Color = NULL;
ID3D11Buffer *gpID3D11Buffer_VertexBuffer_Quad = NULL;
ID3D11Buffer *gpID3D11Buffer_VertexBuffer_Quad_Color = NULL;
ID3D11InputLayout *gpID3D11InputLayout = NULL;
ID3D11Buffer *gpID3D11Buffer_ConstantBuffer = NULL;
ID3D11Buffer* gpID3D11Buffer_ConstantBuffer_HullShader = NULL;
ID3D11Buffer* gpID3D11Buffer_ConstantBuffer_DomainShader = NULL;
ID3D11Buffer* gpID3D11Buffer_ConstantBuffer_PixelShader = NULL;

ID3D11RasterizerState* gpID3D11RasterizerState = NULL; // backface culling

struct CBUFFER_HULL_SHADER
{
	XMVECTOR Hull_Constant_Function_Params;
};
struct CBUFFER_DOMAIN_SHADER
{
	XMMATRIX WorldViewProjectionMatrix;
};
struct CBUFFER_PIXEL_SHADER
{
	XMVECTOR LineColor;
};
unsigned int guiNumberOfLineSegments = 1;

XMMATRIX gPerspectiveProjectionMatrix;

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
		TEXT("Direct3D11 Two2DShapesColouredAnimated"),
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
		case VK_UP:
			guiNumberOfLineSegments++;
			if (guiNumberOfLineSegments >= 30)
			{
				guiNumberOfLineSegments = 30;
			}
			break;
		case VK_DOWN:
			guiNumberOfLineSegments--;
			if (guiNumberOfLineSegments <= 0)
			{
				guiNumberOfLineSegments = 1;
			}
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
		"struct vertex_output" \
		"{" \
		"   float4 position	:	POSITION;" \
		"};" \
		"\n" \
		"vertex_output main(float2 pos : POSITION)" \
		"{" \
		"vertex_output output;" \
		"output.position = float4(pos, 0.0f, 1.0f);" \
		"return output;" \
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

	const char* hullShaderSourceCode =
		"cbuffer ConstantBuffer" \
		"{" \
			"float4 hull_constant_function_params;" \
		"}" \
		"struct vertex_output" \
		"{" \
			"float4 position : POSITION;" \
		"};" \
		"struct hull_constant_output" \
		"{" \
			"float edges[2]: SV_TESSFACTOR;" \
		"};" \
		"hull_constant_output hull_constant_function(void)" \
		"{" \
			"hull_constant_output output;" \
			"float numberOfStrips = hull_constant_function_params[0];" \
			"float numberOfSegments = hull_constant_function_params[1];" \
			"output.edges[0] = numberOfStrips;" \
			"output.edges[1] = numberOfSegments;" \
			"return output;" \
		"}" \
		"struct hull_output" \
		"{" \
			"float4 position : POSITION;" \
		"};" \
		"[domain(\"isoline\")]" \
		"[partitioning(\"integer\")]" \
		"[outputtopology(\"line\")]" \
		"[outputcontrolpoints(4)]" \
		"[patchconstantfunc(\"hull_constant_function\")]" \
		"\n" \
		"hull_output main(InputPatch<vertex_output, 4> input_patch, uint i : SV_OUTPUTCONTROLPOINTID)" \
		"{" \
			"hull_output output;" \
			"output.position = input_patch[i].position;" \
			"return output;" \
		"}";
	ID3DBlob* pID3DBlob_HullShaderCode = NULL;
	pID3DBlob_Error = NULL;

	hr = D3DCompile(hullShaderSourceCode,
		lstrlenA(hullShaderSourceCode) + 1,
		"HS",
		NULL,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",
		"hs_5_0",
		0,
		0,
		&pID3DBlob_HullShaderCode,
		&pID3DBlob_Error);

	if (FAILED(hr))
	{
		if (pID3DBlob_Error != NULL)
		{
			fopen_s(&gpLogFile, gszLogFileName, "a+");
			fprintf_s(gpLogFile, "D3DCompile() Failed For Hull Shader : %s.\n", (char*)pID3DBlob_Error->GetBufferPointer());
			fclose(gpLogFile);
			pID3DBlob_Error->Release();
			pID3DBlob_Error = NULL;
			return(hr);
		}
	}
	else
	{
		fopen_s(&gpLogFile, gszLogFileName, "a+");
		fprintf_s(gpLogFile, "D3DCompile() Succeeded For Hull Shader.\n");
		fclose(gpLogFile);
	}

	hr = gpID3D11Device->CreateHullShader(pID3DBlob_HullShaderCode->GetBufferPointer(),
		pID3DBlob_HullShaderCode->GetBufferSize(),
		NULL,
		&gpID3D11HullShader);

	if (FAILED(hr))
	{
		fopen_s(&gpLogFile, gszLogFileName, "a+");
		fprintf_s(gpLogFile, "ID3D11Device::CreateHullShader() Failed.\n");
		fclose(gpLogFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpLogFile, gszLogFileName, "a+");
		fprintf_s(gpLogFile, "ID3D11Device::CreateHullShader() Succeeded.\n");
		fclose(gpLogFile);
	}
	gpID3D11DeviceContext->HSSetShader(gpID3D11HullShader, 0, 0);
	pID3DBlob_HullShaderCode->Release();
	pID3DBlob_HullShaderCode = NULL;

	//DomainShader
	const char* domainShaderSourceCode =
		"cbuffer ConstantBuffer" \
		"{" \
			"float4x4 worldViewProjectionMatrix;" \
		"}" \
		"struct hull_constant_output" \
		"{" \
			"float edges[2]: SV_TESSFACTOR;" \
		"};" \
		"struct hull_output" \
		"{" \
			"float4 position : POSITION;" \
		"};" \
		"struct domain_output" \
		"{" \
			"float4 position : SV_POSITION;" \
		"};" \
		"[domain(\"isoline\")]" \
		"\n" \
		"domain_output main(hull_constant_output input, OutputPatch<hull_output, 4> output_patch, float2 tessCoord : SV_DOMAINLOCATION)" \
		"{" \
			"domain_output output;" \
			"float u = tessCoord.x;" \
			"float3 p0 = output_patch[0].position.xyz;" \
			"float3 p1 = output_patch[1].position.xyz;" \
			"float3 p2 = output_patch[2].position.xyz;" \
			"float3 p3 = output_patch[3].position.xyz;" \
			"float u1 = (1.0f - u);" \
			"float u2 = u * u;" \
			"float b3 = u2 * u;" \
			"float b2 = 3.0f * u2 * u1;" \
			"float b1 = 3.0f * u * u1 * u1;" \
			"float b0 = u1 * u1 * u1;" \
			"float3 p = p0 * b0 + p1 * b1 + p2 * b2 + p3 * b3;" \
			"output.position = mul(worldViewProjectionMatrix, float4(p, 1.0f));" \
			"return output;" \
		"}";
	ID3DBlob* pID3DBlob_DomainShaderCode = NULL;
	pID3DBlob_Error = NULL;

	hr = D3DCompile(domainShaderSourceCode,
		lstrlenA(domainShaderSourceCode) + 1,
		"DS",
		NULL,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",
		"ds_5_0",
		0,
		0,
		&pID3DBlob_DomainShaderCode,
		&pID3DBlob_Error);

	if (FAILED(hr))
	{
		if (pID3DBlob_Error != NULL)
		{
			fopen_s(&gpLogFile, gszLogFileName, "a+");
			fprintf_s(gpLogFile, "D3DCompile() Failed For Domain Shader : %s.\n", (char*)pID3DBlob_Error->GetBufferPointer());
			fclose(gpLogFile);
			pID3DBlob_Error->Release();
			pID3DBlob_Error = NULL;
			return(hr);
		}
	}
	else
	{
		fopen_s(&gpLogFile, gszLogFileName, "a+");
		fprintf_s(gpLogFile, "D3DCompile() Succeeded For Domain Shader.\n");
		fclose(gpLogFile);
	}

	hr = gpID3D11Device->CreateDomainShader(pID3DBlob_DomainShaderCode->GetBufferPointer(),
		pID3DBlob_DomainShaderCode->GetBufferSize(),
		NULL,
		&gpID3D11DomainShader);

	if (FAILED(hr))
	{
		fopen_s(&gpLogFile, gszLogFileName, "a+");
		fprintf_s(gpLogFile, "ID3D11Device::CreateDomainShader() Failed.\n");
		fclose(gpLogFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpLogFile, gszLogFileName, "a+");
		fprintf_s(gpLogFile, "ID3D11Device::CreateDomainShader() Succeeded.\n");
		fclose(gpLogFile);
	}
	gpID3D11DeviceContext->DSSetShader(gpID3D11DomainShader, 0, 0);
	pID3DBlob_DomainShaderCode->Release();
	pID3DBlob_DomainShaderCode = NULL;

	 const char* pixelShaderSourceCode =
		"cbuffer ConstantBuffer" \
		"{" \
			"float4 lineColor;" \
		"}" \
		"float4 main() : SV_TARGET" \
		"{" \
			"float4 color;" \
			"color = lineColor;" \
			"return (color);" \
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

	D3D11_INPUT_ELEMENT_DESC inputElementDesc;
	inputElementDesc.SemanticName = "POSITION"; // need same in shader
	inputElementDesc.SemanticIndex = 0;
	inputElementDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDesc.InputSlot = 0; //  enum amc_pos
	inputElementDesc.AlignedByteOffset = 0;
	inputElementDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	inputElementDesc.InstanceDataStepRate = 0;

	hr = gpID3D11Device->CreateInputLayout(&inputElementDesc,
		1,
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

	float verticesTri[] =
	{
		-1.0f, -1.0f,
		-0.5f, 1.0f,
		0.5f, -1.0f,
		1.0f, 1.0f
	};

	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = sizeof(float) * ARRAYSIZE(verticesTri);
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	hr = gpID3D11Device->CreateBuffer(&bufferDesc, NULL, &gpID3D11Buffer_VertexBuffer_Triangle);

	if (FAILED(hr))
	{
		fopen_s(&gpLogFile, gszLogFileName, "a+");
		fprintf_s(gpLogFile, "ID3D11Device::CreateBuffer() Failed For Vertex Buffer.\n");
		fclose(gpLogFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpLogFile, gszLogFileName, "a+");
		fprintf_s(gpLogFile, "ID3D11Device::CreateBuffer() Succeeded For Vertex Buffer.\n");
		fclose(gpLogFile);
	}

	D3D11_MAPPED_SUBRESOURCE mappedSubresource;
	ZeroMemory(&mappedSubresource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	gpID3D11DeviceContext->Map(gpID3D11Buffer_VertexBuffer_Triangle,
		NULL,
		D3D11_MAP_WRITE_DISCARD,
		NULL,
		&mappedSubresource);
	memcpy(mappedSubresource.pData, verticesTri, sizeof(verticesTri));
	gpID3D11DeviceContext->Unmap(gpID3D11Buffer_VertexBuffer_Triangle, NULL);


	D3D11_BUFFER_DESC bufferDesc_ConstantBuffer;
	ZeroMemory(&bufferDesc_ConstantBuffer, sizeof(D3D11_BUFFER_DESC));
	bufferDesc_ConstantBuffer.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc_ConstantBuffer.ByteWidth = sizeof(CBUFFER_HULL_SHADER);
	bufferDesc_ConstantBuffer.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	hr = gpID3D11Device->CreateBuffer(&bufferDesc_ConstantBuffer, nullptr, &gpID3D11Buffer_ConstantBuffer_HullShader);

	if (FAILED(hr))
	{
		fopen_s(&gpLogFile, gszLogFileName, "a+");
		fprintf_s(gpLogFile, "ID3D11Device::CreateInputLayout() Failed For Constant Buffer For Hull Shader.\n");
		fclose(gpLogFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpLogFile, gszLogFileName, "a+");
		fprintf_s(gpLogFile, "ID3D11Device::CreateInputLayout() Succeeded For Constant Buffer For Hull Shader.\n");
		fclose(gpLogFile);
	}
	gpID3D11DeviceContext->HSSetConstantBuffers(0, 1, &gpID3D11Buffer_ConstantBuffer_HullShader);

	ZeroMemory(&bufferDesc_ConstantBuffer, sizeof(D3D11_BUFFER_DESC));
	bufferDesc_ConstantBuffer.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc_ConstantBuffer.ByteWidth = sizeof(CBUFFER_DOMAIN_SHADER);
	bufferDesc_ConstantBuffer.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	hr = gpID3D11Device->CreateBuffer(&bufferDesc_ConstantBuffer, nullptr, &gpID3D11Buffer_ConstantBuffer_DomainShader);

	if (FAILED(hr))
	{
		fopen_s(&gpLogFile, gszLogFileName, "a+");
		fprintf_s(gpLogFile, "ID3D11Device::CreateInputLayout() Failed For Constant Buffer For Domain Shader.\n");
		fclose(gpLogFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpLogFile, gszLogFileName, "a+");
		fprintf_s(gpLogFile, "ID3D11Device::CreateInputLayout() Succeeded For Constant Buffer For Domain Shader.\n");
		fclose(gpLogFile);
	}
	gpID3D11DeviceContext->DSSetConstantBuffers(0, 1, &gpID3D11Buffer_ConstantBuffer_DomainShader);

	ZeroMemory(&bufferDesc_ConstantBuffer, sizeof(D3D11_BUFFER_DESC));
	bufferDesc_ConstantBuffer.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc_ConstantBuffer.ByteWidth = sizeof(CBUFFER_PIXEL_SHADER);
	bufferDesc_ConstantBuffer.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	hr = gpID3D11Device->CreateBuffer(&bufferDesc_ConstantBuffer,
		nullptr,
		&gpID3D11Buffer_ConstantBuffer_PixelShader);

	if (FAILED(hr))
	{
		fopen_s(&gpLogFile, gszLogFileName, "a+");
		fprintf_s(gpLogFile, "ID3D11Device::CreateInputLayout() Failed For Constant Buffer For Pixel Shader.\n");
		fclose(gpLogFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpLogFile, gszLogFileName, "a+");
		fprintf_s(gpLogFile, "ID3D11Device::CreateInputLayout() Succeeded For Constant Buffer For Pixel Shader.\n");
		fclose(gpLogFile);
	}
	gpID3D11DeviceContext->PSSetConstantBuffers(0, 1, &gpID3D11Buffer_ConstantBuffer_PixelShader);


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

	gfClearColor[0] = 0.0f;
	gfClearColor[1] = 0.0f;
	gfClearColor[2] = 0.0f;
	gfClearColor[3] = 1.0f;

	gPerspectiveProjectionMatrix = XMMatrixIdentity();

	hr = resize(WIN_WIDTH, WIN_HEIGHT);
	WriteLog(hr, "resize() ");

	return 0;
}

HRESULT resize(int iWidth, int iHeight)
{
	HRESULT hr = S_OK;

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

	gpID3D11DeviceContext->OMSetRenderTargets(1, &gpID3D11RenderTargetView, NULL);

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
	static float fAngleRot = 0;
	gpID3D11DeviceContext->ClearRenderTargetView(gpID3D11RenderTargetView, gfClearColor);

	UINT stride = sizeof(float) * 2;
	UINT offset = 0;
	gpID3D11DeviceContext->IASetVertexBuffers(0, 1, &gpID3D11Buffer_VertexBuffer_Triangle, &stride, &offset);
	

	gpID3D11DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);

	XMMATRIX worldMatrix = XMMatrixIdentity();
	XMMATRIX viewMatrix = XMMatrixIdentity();
	XMMATRIX translationMatrix = XMMatrixIdentity();
	XMMATRIX rotationMatrix = XMMatrixIdentity();

	translationMatrix = XMMatrixTranslation(0.0f, 0.0f, 3.0f);
	rotationMatrix = XMMatrixRotationY(fAngleRot);
	worldMatrix = rotationMatrix * translationMatrix;

	XMMATRIX wvpMatrix = worldMatrix * viewMatrix * gPerspectiveProjectionMatrix;

	CBUFFER_DOMAIN_SHADER constantBuffer_domainShader;
	constantBuffer_domainShader.WorldViewProjectionMatrix = wvpMatrix;
	gpID3D11DeviceContext->UpdateSubresource(gpID3D11Buffer_ConstantBuffer_DomainShader, 0, NULL, &constantBuffer_domainShader, 0, 0);

	CBUFFER_HULL_SHADER constantBuffer_hullShader;
	constantBuffer_hullShader.Hull_Constant_Function_Params = XMVectorSet(1.0f, (FLOAT)guiNumberOfLineSegments, 0.0f, 0.0f);
	gpID3D11DeviceContext->UpdateSubresource(gpID3D11Buffer_ConstantBuffer_HullShader, 0, NULL, &constantBuffer_hullShader, 0, 0);
	TCHAR str[255];
	wsprintf(str, TEXT("Direct3D11 Window [ Segments = %2d ]"), guiNumberOfLineSegments);
	SetWindowText(ghWnd, str);

	CBUFFER_PIXEL_SHADER constantBuffer_pixelShader;
	if (guiNumberOfLineSegments >= 30)
	{
		constantBuffer_pixelShader.LineColor = XMVectorSet(1.0f, 0.0f, 0.0f, 1.0f);
	}
	else
	{
		constantBuffer_pixelShader.LineColor = XMVectorSet(1.0f, 1.0f, 0.0f, 1.0f);
	}
	gpID3D11DeviceContext->UpdateSubresource(gpID3D11Buffer_ConstantBuffer_PixelShader, 0, NULL, &constantBuffer_pixelShader, 0, 0);

	gpID3D11DeviceContext->Draw(4, 0);

	gpIDXGISwapChain->Present(0, 0);

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
	if (gpID3D11Buffer_VertexBuffer_Triangle)
	{
		gpID3D11Buffer_VertexBuffer_Triangle->Release();
		gpID3D11Buffer_VertexBuffer_Triangle = NULL;
	}
	if (gpID3D11Buffer_VertexBuffer_Quad)
	{
		gpID3D11Buffer_VertexBuffer_Quad->Release();
		gpID3D11Buffer_VertexBuffer_Quad = NULL;
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


