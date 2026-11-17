//Include and link appropriate libraries and headers//
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dx11.lib")
#pragma comment(lib, "d3dx10.lib")
#pragma comment (lib, "dinput8.lib")

#include <windows.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dx10.h>
#include <dinput.h>
#include <fstream>
using namespace std;
#include "DXRender.h"

LPCTSTR WndClassName = L"firstwindow";
HWND hwnd = NULL;

const int Width = 800;
const int Height = 600;

DXRender *dxrender;

//Function Prototypes//
bool InitializeWindow(HINSTANCE hInstance,
	int ShowWnd,
	int width, int height,
	bool windowed);
int messageloop();

LRESULT CALLBACK WndProc(HWND hWnd,
	UINT msg,
	WPARAM wParam,
	LPARAM lParam);


int WINAPI WinMain(HINSTANCE hInstance,	//Main windows function
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nShowCmd)
{
	dxrender = new DXRender();
	if (!InitializeWindow(hInstance, nShowCmd, Width, Height, true))
	{
		MessageBox(0, L"Window Initialization - Failed",
			L"Error", MB_OK);
		return 0;
	}

	if (!dxrender->InitDevice(hwnd, Width, Height))	//Initialize Direct3D
	{
		MessageBox(0, L"Direct3D Initialization - Failed",
			L"Error", MB_OK);
		return 0;
	}

	if (!dxrender->InitScene(Width, Height))	//Initialize our scene
	{
		MessageBox(0, L"Scene Initialization - Failed",
			L"Error", MB_OK);
		return 0;
	}

	if(!dxrender->InitDirectInput(hInstance, hwnd))
	{
		MessageBox(0, L"Direct Input Initialization - Failed",
			L"Error", MB_OK);
		return 0;
	}

	messageloop();

	dxrender->ReleaseObjects();

	return 0;
}

bool InitializeWindow(HINSTANCE hInstance,
	int ShowWnd,
	int width, int height,
	bool windowed)
{
	typedef struct _WNDCLASS {
		UINT cbSize;
		UINT style;
		WNDPROC lpfnWndProc;
		int cbClsExtra;
		int cbWndExtra;
		HANDLE hInstance;
		HICON hIcon;
		HCURSOR hCursor;
		HBRUSH hbrBackground;
		LPCTSTR lpszMenuName;
		LPCTSTR lpszClassName;
	} WNDCLASS;

	WNDCLASSEX wc;

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = NULL;
	wc.cbWndExtra = NULL;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 2);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = WndClassName;
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	if (!RegisterClassEx(&wc))
	{
		MessageBox(NULL, L"Error registering class",
			L"Error", MB_OK | MB_ICONERROR);
		return 1;
	}

	hwnd = CreateWindowEx(
		NULL,
		WndClassName,
		L"Pablito Pinwi",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		width, height,
		NULL,
		NULL,
		hInstance,
		NULL
		);

	if (!hwnd)
	{
		MessageBox(NULL, L"Error creating window",
			L"Error", MB_OK | MB_ICONERROR);
		return 1;
	}

	ShowWindow(hwnd, ShowWnd);
	UpdateWindow(hwnd);

	return true;
}

int messageloop(){
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));
	while (true)
	{
		BOOL PeekMessageL(
			LPMSG lpMsg,
			HWND hWnd,
			UINT wMsgFilterMin,
			UINT wMsgFilterMax,
			UINT wRemoveMsg
			);

		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				break;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else{
			// run game code

			dxrender->UpdateScene();
			dxrender->DrawScene();

		}
	}
	return msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd,
	UINT msg,
	WPARAM wParam,
	LPARAM lParam)
{
	switch (msg)
	{
	case WM_KEYDOWN:
		if(dxrender && dxrender->IsMenuVisible())
		{
			if(wParam==VK_ESCAPE)DestroyWindow(hwnd);
			else if((lParam & (1L<<30))==0)dxrender->HandleMenuKey(wParam);
			return 0;
		}
		// Bit 30 marks auto-repeat: one camera change per physical C press.
		if (wParam == 'C' && (lParam & (1L << 30)) == 0)
			dxrender->ToggleCameraPerspective();
		if (wParam == 'E' && (lParam & (1L << 30)) == 0)
			dxrender->RequestInteraction();
		if (wParam == 'Q' && (lParam & (1L << 30)) == 0)
			dxrender->RequestDrop();
		if (wParam == 'R' && (lParam & (1L << 30)) == 0)
			dxrender->RequestRecipe();
		if (wParam == VK_RETURN && (lParam & (1L << 30)) == 0)
			dxrender->RequestRestart();
		if (wParam == VK_F6 && (lParam & (1L << 30)) == 0)
			dxrender->ToggleDayNightPause();
		if (wParam == VK_ESCAPE){
			DestroyWindow(hwnd);
		}
		return 0;

	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
		if(dxrender && dxrender->IsMenuVisible())
		{
			if(msg==WM_LBUTTONDOWN)SetCapture(hwnd);
			dxrender->HandleMenuPointer(msg,static_cast<short>(LOWORD(lParam)),static_cast<short>(HIWORD(lParam)));
			if(msg==WM_LBUTTONUP && GetCapture()==hwnd)ReleaseCapture();
			return 0;
		}
		break;
	case WM_CAPTURECHANGED:
	case WM_KILLFOCUS:
		if(dxrender)dxrender->CancelMenuClick();
		break;
	case WM_SETCURSOR:
		if(dxrender && dxrender->IsMenuVisible() && LOWORD(lParam)==HTCLIENT)
		{SetCursor(LoadCursor(nullptr,IDC_ARROW));return TRUE;}
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd,
		msg,
		wParam,
		lParam);
}
