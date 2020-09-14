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
ID3D11Buffer *gpID3D11Buffer_VertexBuffer = NULL;
ID3D11InputLayout *gpID3D11InputLayout = NULL;
ID3D11Buffer *gpID3D11Buffer_ConstantBuffer = NULL;

struct CBUFFER
{
	XMMATRIX WorldViewProjectionMatrix;
};

XMMATRIX gOrthographicProjectionMatrix;

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
						TEXT("Direct3D11 Ortho"),
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
											"float4x4 worldViewProjectionMatrix;" \
										"}" \
										"float4 main(float4 pos : POSITION) : SV_POSITION" \
										"{" \
											"float4 position = mul(worldViewProjectionMatrix, pos);" \
											"return(position);" \
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
										"float4 main(void) : SV_TARGET" \
										"{" \
											"return(float4(1.0f, 1.0f, 1.0f, 1.0f));" \
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
	inputElementDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDesc.InputSlot = 0; //  enum amc_
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

	float vertices[] =
	{
		0.0f, 50.0f, 0.0f,
		50.0f, -50.0f, 0.0f,
		-50.0f, -50.0f, 0.0f,
	};

	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = sizeof(float) * ARRAYSIZE(vertices);
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	hr = gpID3D11Device->CreateBuffer(&bufferDesc, NULL, &gpID3D11Buffer_VertexBuffer);

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
	gpID3D11DeviceContext->Map(gpID3D11Buffer_VertexBuffer,
								NULL,
								D3D11_MAP_WRITE_DISCARD,
								NULL,
								&mappedSubresource);
	memcpy(mappedSubresource.pData, vertices, sizeof(vertices));
	gpID3D11DeviceContext->Unmap(gpID3D11Buffer_VertexBuffer, NULL);

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

	gfClearColor[0] = 0.0f;
	gfClearColor[1] = 0.0f;
	gfClearColor[2] = 0.0f;
	gfClearColor[3] = 1.0f;

	gOrthographicProjectionMatrix = XMMatrixIdentity();

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

	if (iWidth <= iHeight)
	{
		gOrthographicProjectionMatrix = XMMatrixOrthographicOffCenterLH(-100.0f,
																		100.0f,
																		-100.0f * ((float)iHeight / (float)iWidth),
																		100.0f * ((float)iHeight / (float)iWidth),
																		-100.0f,
																		100.0f);
	}
	else
	{
		gOrthographicProjectionMatrix = XMMatrixOrthographicOffCenterLH(-100.0f * ((float)iWidth / (float)iHeight),
																		100.0f * ((float)iWidth / (float)iHeight),
																		-100.0f,
																		100.0f,
																		-100.0f,
																		100.0f);
	}

	return hr;
}

void display()
{
	gpID3D11DeviceContext->ClearRenderTargetView(gpID3D11RenderTargetView, gfClearColor);
	
	UINT stride = sizeof(float) * 3;
	UINT offset = 0;
	gpID3D11DeviceContext->IASetVertexBuffers(0, 1, &gpID3D11Buffer_VertexBuffer, &stride, &offset);

	gpID3D11DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	XMMATRIX worldMatrix = XMMatrixIdentity();
	XMMATRIX viewMatrix = XMMatrixIdentity();

	XMMATRIX wvpMatrix = worldMatrix * viewMatrix * gOrthographicProjectionMatrix;

	CBUFFER constantBuffer;
	constantBuffer.WorldViewProjectionMatrix = wvpMatrix;
	gpID3D11DeviceContext->UpdateSubresource(gpID3D11Buffer_ConstantBuffer, 0, NULL, &constantBuffer, 0, 0);

	gpID3D11DeviceContext->Draw(3, 0);

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
	if (gpID3D11Buffer_VertexBuffer)
	{
		gpID3D11Buffer_VertexBuffer->Release();
		gpID3D11Buffer_VertexBuffer = NULL;
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


