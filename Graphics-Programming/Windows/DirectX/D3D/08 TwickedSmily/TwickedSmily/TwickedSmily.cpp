#include <Windows.h>
#include <stdio.h>
#include <d3d11.h>
#include <d3dcompiler.h>

#include "..\..\Texture_Files\WICTextureLoader.h"

#pragma warning( disable : 4838 )
#include "..\..\XNAMath\xnamath.h"


#pragma comment(lib, "d3d11.lib")
#pragma comment (lib, "d3dcompiler.lib")
#pragma comment (lib, "..\\..\\Texture_Files\\DirectXTK.lib ")


#define WIN_WIDTH	800
#define WIN_HEIGHT	600

enum enSelectedPart
{
	enDefault = 0,
	enLeftBottom = 1,
	enFull,
	enOnlyCenterPixel,
	enFourTiling,
	enRightBottom,
	enLeftTop,
	enRightTop
};

int giSelectedVal = 0;

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

float gfClearColor[4];
IDXGISwapChain *gpIDXGISwapChain = NULL;
ID3D11Device *gpID3D11Device = NULL;
ID3D11DeviceContext *gpID3D11DeviceContext = NULL;
ID3D11RenderTargetView *gpID3D11RenderTargetView = NULL;

ID3D11VertexShader *gpID3D11VertexShader = NULL;
ID3D11PixelShader *gpID3D11PixelShader = NULL;
ID3D11Buffer *gpID3D11Buffer_VertexBuffer_Quad = NULL;
ID3D11Buffer *gpID3D11Buffer_VertexBuffer_Quad_Texture = NULL;
ID3D11InputLayout *gpID3D11InputLayout = NULL;
ID3D11Buffer *gpID3D11Buffer_ConstantBuffer = NULL;

ID3D11RasterizerState* gpID3D11RasterizerState = NULL; // backface culling

ID3D11DepthStencilView* gpID3D11DepthStencilView = NULL; // depth

ID3D11ShaderResourceView* gpID3D11ShaderResourceView_Quad_Texture = NULL;
ID3D11SamplerState* gpID3D11SamplerState_Quad_Texture = NULL;

struct CBUFFER
{
	XMMATRIX WorldViewProjectionMatrix;
};

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
		case 49:
		case VK_NUMPAD1:
			giSelectedVal = enLeftBottom;
			break;

		case 50:
		case VK_NUMPAD2:
			giSelectedVal = enFull;
			break;

		case 51:
		case VK_NUMPAD3:
			giSelectedVal = enFourTiling;
			break;

		case 52:
		case VK_NUMPAD4:
			giSelectedVal = enOnlyCenterPixel;
			break;

		default:
			giSelectedVal = enDefault;
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
	HRESULT LoadD3DTexture(const wchar_t *, ID3D11ShaderResourceView**);

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
		"float4x4 worldViewProjectionMatrix;" \
		"}\n" \
		"struct vertex_output" \
		"{" \
		"float4 position	:	SV_POSITION;" \
		"float2 texcoord	:	TEXCOORD;" \
		"};" \
		"\n" \
		"vertex_output main(float4 pos : POSITION, float2 texcoord : TEXCOORD)" \
		"{" \
		"vertex_output output;" \
		"output.position = mul(worldViewProjectionMatrix, pos);" \
		"output.texcoord = texcoord;" \
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

	const char* pixelShaderSourceCode =
		"Texture2D myTexture2D;" \
		"SamplerState mySamplerState;" \
		"float4 main(float4 position: SV_POSITION, float2 texcoord : TEXCOORD) : SV_TARGET" \
		"{" \
		"float4 color = myTexture2D.Sample(mySamplerState, texcoord);" \
		"return(color);" \
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

	inputElementDesc[1].SemanticName = "TEXCOORD"; // need same in shader
	inputElementDesc[1].SemanticIndex = 0;
	inputElementDesc[1].Format = DXGI_FORMAT_R32G32_FLOAT;
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

	float verticesQuad[] =
	{
		-1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f,

		-1.0f, -1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		1.0f, -1.0f, 0.0f

	};

	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = sizeof(float) * ARRAYSIZE(verticesQuad);
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	hr = gpID3D11Device->CreateBuffer(&bufferDesc, NULL, &gpID3D11Buffer_VertexBuffer_Quad);

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

	D3D11_MAPPED_SUBRESOURCE mappedSubresource;
	ZeroMemory(&mappedSubresource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	gpID3D11DeviceContext->Map(gpID3D11Buffer_VertexBuffer_Quad, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mappedSubresource);
	memcpy(mappedSubresource.pData, verticesQuad, sizeof(verticesQuad));
	gpID3D11DeviceContext->Unmap(gpID3D11Buffer_VertexBuffer_Quad, NULL);

	float quadTexture[] =
	{
		0.0f, 0.0f,
		1.0f, 0.0f,
		0.0f, 1.0f,

		0.0f, 1.0f,
		1.0f, 0.0f,
		1.0f, 1.0f
	};

	ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = sizeof(float) * ARRAYSIZE(quadTexture);
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	hr = gpID3D11Device->CreateBuffer(&bufferDesc, NULL, &gpID3D11Buffer_VertexBuffer_Quad_Texture);

	if (FAILED(hr))
	{
		fopen_s(&gpLogFile, gszLogFileName, "a+");
		fprintf_s(gpLogFile, "ID3D11Device::CreateBuffer() Failed For Vertex Buffer for quad texture.\n");
		fclose(gpLogFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpLogFile, gszLogFileName, "a+");
		fprintf_s(gpLogFile, "ID3D11Device::CreateBuffer() Succeeded For Vertex Buffer for quad texture.\n");
		fclose(gpLogFile);
	}

	ZeroMemory(&mappedSubresource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	gpID3D11DeviceContext->Map(gpID3D11Buffer_VertexBuffer_Quad_Texture, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mappedSubresource);
	memcpy(mappedSubresource.pData, quadTexture, sizeof(quadTexture));
	gpID3D11DeviceContext->Unmap(gpID3D11Buffer_VertexBuffer_Quad_Texture, NULL);


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
	CoInitialize(NULL);

	hr = LoadD3DTexture(L"smiley.bmp", &gpID3D11ShaderResourceView_Quad_Texture);
	if (FAILED(hr))
	{
		fopen_s(&gpLogFile, gszLogFileName, "a+");
		fprintf_s(gpLogFile, "LoadD3DTexture() Failed For quad Texture.\n:");
		fclose(gpLogFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpLogFile, gszLogFileName, "a+");
		fprintf_s(gpLogFile, "LoadD3DTexture() Succeeded For quad Texture.\n ");
		fclose(gpLogFile);
	}

	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	hr = gpID3D11Device->CreateSamplerState(&samplerDesc, &gpID3D11SamplerState_Quad_Texture);
	if (FAILED(hr))
	{
		fopen_s(&gpLogFile, gszLogFileName, "a+");
		fprintf_s(gpLogFile, "ID3D11Device::CreateSamplerState() Failed For quad Texture.\n");
		fclose(gpLogFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpLogFile, gszLogFileName, "a+");
		fprintf_s(gpLogFile, "ID3D11Device::CreateSamplerState() Succeeded For quad Texture.\n");
		fclose(gpLogFile);
	}


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
	static float fAngleRot = 0;
	gpID3D11DeviceContext->ClearRenderTargetView(gpID3D11RenderTargetView, gfClearColor);
	gpID3D11DeviceContext->ClearDepthStencilView(gpID3D11DepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	UINT stride = sizeof(float) * 3;
	UINT offset = 0;

	XMMATRIX worldMatrix = XMMatrixIdentity();
	XMMATRIX viewMatrix = XMMatrixIdentity();
	XMMATRIX translationMatrix = XMMatrixIdentity();
	XMMATRIX rotationMatrix = XMMatrixIdentity();
	XMMATRIX scaleMatrix = XMMatrixIdentity();

	float quadTexture[12];

	switch (giSelectedVal)
	{
	case enLeftBottom:
		quadTexture[0] = 0.0f;
		quadTexture[1] = 0.5f;
		quadTexture[2] = 0.5f;
		quadTexture[3] = 0.5f;
		quadTexture[4] = 0.0f;
		quadTexture[5] = 1.0f;
		quadTexture[6] = 0.0f;
		quadTexture[7] = 1.0f;
		quadTexture[8] = 0.5f;
		quadTexture[9] = 0.5f;
		quadTexture[10] = 0.5f;
		quadTexture[11] = 1.0f;
		break;
	case enFull:
		quadTexture[0] = 0.0f;
		quadTexture[1] = 0.0f;
		quadTexture[2] = 1.0f;
		quadTexture[3] = 0.0f;
		quadTexture[4] = 0.0f;
		quadTexture[5] = 1.0f;
		quadTexture[6] = 0.0f;
		quadTexture[7] = 1.0f;
		quadTexture[8] = 1.0f;
		quadTexture[9] = 0.0f;
		quadTexture[10] = 1.0f;
		quadTexture[11] = 1.0f;
		break;
	case enOnlyCenterPixel:
		quadTexture[0] = 0.0f;
		quadTexture[1] = 0.0f;
		quadTexture[2] = 2.0f;
		quadTexture[3] = 0.0f;
		quadTexture[4] = 0.0f;
		quadTexture[5] = 2.0f;
		quadTexture[6] = 0.0f;
		quadTexture[7] = 2.0f;
		quadTexture[8] = 2.0f;
		quadTexture[9] = 0.0f;
		quadTexture[10] = 2.0f;
		quadTexture[11] = 2.0f;
		break;
	case enFourTiling:
		quadTexture[0] = 0.5f;
		quadTexture[1] = 0.5f;
		quadTexture[2] = 0.5f;
		quadTexture[3] = 0.5f;
		quadTexture[4] = 0.5f;
		quadTexture[5] = 0.5f;
		quadTexture[6] = 0.5f;
		quadTexture[7] = 0.5f;
		quadTexture[8] = 0.5f;
		quadTexture[9] = 0.5f;
		quadTexture[10] = 0.5f;
		quadTexture[11] = 0.5f;
		break;
	}

	D3D11_MAPPED_SUBRESOURCE mappedSubresource;
	ZeroMemory(&mappedSubresource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	gpID3D11DeviceContext->Map(gpID3D11Buffer_VertexBuffer_Quad_Texture, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mappedSubresource);
	memcpy(mappedSubresource.pData, quadTexture, sizeof(quadTexture));
	gpID3D11DeviceContext->Unmap(gpID3D11Buffer_VertexBuffer_Quad_Texture, NULL);

	stride = sizeof(float) * 3;
	offset = 0;
	gpID3D11DeviceContext->IASetVertexBuffers(0, 1, &gpID3D11Buffer_VertexBuffer_Quad, &stride, &offset);
	stride = sizeof(float) * 2;
	offset = 0;
	gpID3D11DeviceContext->IASetVertexBuffers(1, 1, &gpID3D11Buffer_VertexBuffer_Quad_Texture, &stride, &offset);
	gpID3D11DeviceContext->PSSetShaderResources(0, 1, &gpID3D11ShaderResourceView_Quad_Texture);
	gpID3D11DeviceContext->PSSetSamplers(0, 1, &gpID3D11SamplerState_Quad_Texture);

	gpID3D11DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	worldMatrix = XMMatrixIdentity();
	viewMatrix = XMMatrixIdentity();
	translationMatrix = XMMatrixIdentity();

	translationMatrix = XMMatrixTranslation(0.0f, 0.0f, 3.0f);
	/*rotationMatrix = XMMatrixRotationX(fAngleRot);
	rotationMatrix *= XMMatrixRotationY(fAngleRot);
	rotationMatrix *= XMMatrixRotationZ(fAngleRot);*/
	scaleMatrix = XMMatrixScaling(0.8f, 0.8f, 0.8f);
	worldMatrix = scaleMatrix * rotationMatrix * translationMatrix;

	XMMATRIX wvpMatrix = worldMatrix * viewMatrix * gPerspectiveProjectionMatrix;

	CBUFFER constantBuffer;
	ZeroMemory(&constantBuffer, sizeof(CBUFFER));
	constantBuffer.WorldViewProjectionMatrix = wvpMatrix;
	gpID3D11DeviceContext->UpdateSubresource(gpID3D11Buffer_ConstantBuffer, 0, NULL, &constantBuffer, 0, 0);

	gpID3D11DeviceContext->Draw(6, 0);
	gpID3D11DeviceContext->Draw(6, 6);
	gpID3D11DeviceContext->Draw(6, 12);
	gpID3D11DeviceContext->Draw(6, 18);
	gpID3D11DeviceContext->Draw(6, 24);
	gpID3D11DeviceContext->Draw(6, 30);

	gpIDXGISwapChain->Present(0, 0);

	fAngleRot += 0.001f;
	if (fAngleRot >= 360.0f)
	{
		fAngleRot = 360.0f;
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

HRESULT LoadD3DTexture(const wchar_t* textureFileName, ID3D11ShaderResourceView ** ppID3D11ShaderResourceView)
{
	// code
	HRESULT hr;
	// create texture
	hr = DirectX::CreateWICTextureFromFile(gpID3D11Device,
		gpID3D11DeviceContext,
		textureFileName,
		NULL,
		ppID3D11ShaderResourceView);
	if (FAILED(hr))
	{
		fopen_s(&gpLogFile, gszLogFileName, "a+");
		fprintf_s(gpLogFile, "CreateWICTextureFromFile() Failed For Texture Resource.\n");
		fclose(gpLogFile);
		return(hr);
	}
	else
	{
		fopen_s(&gpLogFile, gszLogFileName, "a+");
		fprintf_s(gpLogFile, "CreateWICTextureFromFile() Succeeded For Texture Resource.\n");
		fclose(gpLogFile);
	}

	return(hr);
}


