#include <Windows.h>

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
	WNDCLASSEX wndclass;
	TCHAR szAppName[] = TEXT("MessageHandling");
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
		TEXT("Message Handling"),
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
	switch (iMsg)
	{
	case WM_CREATE:
		MessageBox(hWnd, TEXT("WM_CREATE"), TEXT("Info"), MB_OK);
		break;
	case WM_LBUTTONDOWN:
		MessageBox(hWnd, TEXT("WM_LBUTTONDOWN"), TEXT("Info"), MB_ABORTRETRYIGNORE | MB_ICONERROR);
		break;
	case WM_RBUTTONDOWN:
		MessageBox(hWnd, TEXT("WM_RBUTTONDOWN"), TEXT("Info"), MB_ABORTRETRYIGNORE | MB_ICONERROR);
		break;
	case WM_KEYDOWN: // ESCAP & F
		switch (wParam)
		{
		case VK_ESCAPE:
			MessageBox(hWnd, TEXT("WM_KEYDOWN VK_ESCAPE"), TEXT(""), MB_OK | MB_ICONINFORMATION);
			DestroyWindow(hWnd);
			break;

		case 0x46:
			MessageBox(hWnd, TEXT("WM_KEYDOWN VK_F"), TEXT(""), MB_OK | MB_ICONINFORMATION);
			break;
		}
		break;
	case WM_DESTROY:
		MessageBox(hWnd, TEXT("WM_DESTROY"), TEXT("Info"), MB_YESNOCANCEL | MB_ICONQUESTION);
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hWnd, iMsg, wParam, lParam);
}

