#include <Windows.h>

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
	WNDCLASSEX wndclass;
	TCHAR szAppName[] = TEXT("SpecializedDC");
	HWND hWnd = NULL;
	MSG msg;

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

	hWnd = CreateWindow(szAppName,
		TEXT("Specialized DC Painting"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
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
	RECT rc;
	HDC hDC;
	PAINTSTRUCT ps;
	TCHAR str[] = TEXT("Hello World !!");

	switch (iMsg)
	{

	case WM_PAINT:
		GetClientRect(hWnd, &rc);
		hDC = BeginPaint(hWnd, &ps);
		SetBkColor(hDC, RGB(0, 0, 0));
		SetTextColor(hDC, RGB(0, 255, 0));
		DrawText(hDC, str, -1, &rc, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
		EndPaint(hWnd, &ps);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	}
	return DefWindowProc(hWnd, iMsg, wParam, lParam);
}

