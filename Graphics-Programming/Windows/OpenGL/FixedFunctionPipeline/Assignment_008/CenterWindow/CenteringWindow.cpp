#include <Windows.h>

#define WIN_WIDTH 800
#define WIN_HEIGHT 600

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
	WNDCLASSEX wndclass;
	TCHAR szAppName[] = TEXT("CenteringWindow");
	HWND hWnd = NULL;
	MSG msg;
	POINT pt;
	RECT rc;

	wndclass.cbClsExtra = 0;
	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.cbWndExtra = 0;
	wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndclass.hCursor = LoadCursor(hInstance, IDC_ARROW);
	wndclass.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
	wndclass.hIconSm = LoadIcon(hInstance, IDI_APPLICATION);
	wndclass.hInstance = hInstance;
	wndclass.lpfnWndProc = WndProc;
	wndclass.lpszClassName = szAppName;
	wndclass.lpszMenuName = NULL;
	wndclass.style = CS_HREDRAW | CS_VREDRAW;

	if (NULL == RegisterClassEx(&wndclass))
	{
		MessageBox(NULL, TEXT("Failed to register wndclass"), TEXT("Error"), MB_OK);
		return 0;
	}


	SystemParametersInfo(SPI_GETWORKAREA, 0, (PVOID)&rc, 0);
	pt.x = ((rc.right / 2) - (WIN_WIDTH / 2));
	pt.y = ((rc.bottom / 2) - (WIN_HEIGHT / 2));
	
	hWnd = CreateWindow(szAppName,
		TEXT("Centering the Window"),
		WS_OVERLAPPEDWINDOW,
		pt.x,
		pt.y,
		WIN_WIDTH,
		WIN_HEIGHT,
		HWND_DESKTOP,
		NULL,
		hInstance,
		NULL);

	if (NULL == hWnd)
	{
		MessageBox(NULL, TEXT("Failed to create window"), TEXT("Error"), MB_OK);
		return 0;
	}

	ShowWindow(hWnd, iCmdShow);
	UpdateWindow(hWnd);

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return ((int)msg.wParam);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	switch (iMsg)
	{

	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hWnd, iMsg, wParam, lParam);
}

