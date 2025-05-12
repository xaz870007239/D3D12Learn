#include <Windows.h>

using namespace std;

LPCTSTR gWindowClassName = L"D3D12 App";

LRESULT CALLBACK WndProc(HWND inHWND, UINT inMSG, WPARAM inWParam, LPARAM inLParam)
{
	switch (inMSG)
	{
	case WM_CLOSE:
		PostQuitMessage(0);
		break;
	default:
		break;
	}

	return DefWindowProc(inHWND, inMSG, inWParam, inLParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int inShowCmd)
{
	WNDCLASSEX WinClassEx{};
	WinClassEx.cbSize = sizeof(WNDCLASSEX);
	WinClassEx.style = CS_HREDRAW | CS_VREDRAW;
	WinClassEx.cbClsExtra = NULL;
	WinClassEx.cbWndExtra = NULL;
	WinClassEx.hInstance = hInstance;
	WinClassEx.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WinClassEx.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	//WinClassEx.hCursor = LoadIcon(NULL, IDC_ARROW);
	WinClassEx.hbrBackground = NULL;
	WinClassEx.lpszMenuName = NULL;
	WinClassEx.lpszClassName = gWindowClassName;
	WinClassEx.lpfnWndProc = WndProc;
	
	if (!RegisterClassEx(&WinClassEx))
	{
		MessageBox(NULL, L"Register Class Failed!", L"Error", MB_OK | MB_ICONERROR);
		return -1;
	}

	int ViewWidth = 1600;
	int ViewHeight = 900;
	RECT ViewRect;
	ViewRect.left = 0;
	ViewRect.top = 0;
	ViewRect.right = ViewWidth;
	ViewRect.bottom = ViewHeight;

	AdjustWindowRect(&ViewRect, WS_OVERLAPPEDWINDOW, FALSE);

	int ScreenWidth = ViewRect.right - ViewRect.left;
	int ScreenHeight = ViewRect.bottom - ViewRect.top;
	HWND hwnd = CreateWindowEx(NULL, gWindowClassName, L"D3D12 App", WS_OVERLAPPEDWINDOW, 
		CW_USEDEFAULT, CW_USEDEFAULT, ScreenWidth, ScreenHeight, NULL, NULL, hInstance, NULL);

	if (!hwnd)
	{
		MessageBox(NULL, L"Create WIndow Failed!", L"Error", MB_OK | MB_ICONERROR);
		return -1;
	}

	ShowWindow(hwnd, inShowCmd);
	UpdateWindow(hwnd);

	MSG msg;
	while (true) 
	{
#if defined(_WIN32) || defined(_WIN64)
		ZeroMemory(&msg, sizeof(MSG));
#elif
		memset(msg, 0, sizeof(MSG);
#endif

		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				break;
			}

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			//Render
		}
	}

	return 0;
}