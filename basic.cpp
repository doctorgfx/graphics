#include "stdafx.h"
#include "basic.h"
#include <time.h>
#include <sstream>
#pragma comment(lib, "gdiplus.lib")
using namespace Gdiplus;
ULONG_PTR gdiplusToken;
#define MAX_LOADSTRING 100
#define KEYSTATE(n) ((GetAsyncKeyState(n) & 0x8000) ? TRUE: FALSE)
#define pi  3.1415926535897932384626433832795f
const int width = 640;
const int height = 480;
HWND hWnd = NULL;
HDC hdc, hMemDC;;
HBITMAP pBitmap;
COLORREF bitsData[width*height];
double countsPerSecond = 0.0;
__int64 CounterStart = 0;
int frameCount = 0;
int fps = 0;
__int64 frameTimeOld = 0;
double frameTime;
void StartTimer();
double GetTime();
double GetFrameTime();
void render(double);
int messageloop();
struct image_info
{
	unsigned long width, height, *bitsData;
};
image_info image;
void setImage(WCHAR *, image_info &);
void deleteImage(image_info &);
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
void setPixel(int, int, unsigned long);
int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPTSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_BASIC, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_BASIC));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{

		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{

			TranslateMessage(&msg);
			DispatchMessage(&msg);

		}
	}

	return (int)msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_BASIC));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCE(IDC_BASIC);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}


BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;

	hInst = hInstance; // Store instance handle in our global variable

	hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

	if (!hWnd)
	{
		return FALSE;
	}
	GdiplusStartupInput gdiplusStartupInput;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
	frameTime = GetFrameTime();
	hdc = GetDC(hWnd);
	setImage(L"mushroom2020.png", image);
	
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	MSG        uMsg;

	memset(&uMsg, 0, sizeof(uMsg));
	while (uMsg.message != WM_QUIT)
	{

		if (PeekMessage(&uMsg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&uMsg);
			DispatchMessage(&uMsg);
			if ((uMsg.message != WM_MOUSEMOVE) && (uMsg.message != WM_KEYDOWN) && (uMsg.message != WM_NCMOUSEMOVE))
				frameTime = GetFrameTime();
		}
		else
			messageloop();
	}
	PostQuitMessage(0);
	return TRUE;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;

	switch (message)
	{

	case WM_COMMAND:

		wmId = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{

		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:

			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;

	case WM_CREATE:
		
		
		hMemDC = CreateCompatibleDC(hdc);
		

		break;
	case WM_DESTROY:
		GdiplusShutdown(gdiplusToken);
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
void StartTimer()
{
	LARGE_INTEGER frequencyCount;
	QueryPerformanceFrequency(&frequencyCount);
	countsPerSecond = double(frequencyCount.QuadPart);
	QueryPerformanceCounter(&frequencyCount);
	CounterStart = frequencyCount.QuadPart;
}
double GetTime()
{
	LARGE_INTEGER currentTime;
	QueryPerformanceCounter(&currentTime);
	return double(currentTime.QuadPart - CounterStart) / countsPerSecond;
}
double GetFrameTime()
{
	LARGE_INTEGER currentTime;
	__int64 tickCount;
	QueryPerformanceCounter(&currentTime);
	tickCount = currentTime.QuadPart - frameTimeOld;
	frameTimeOld = currentTime.QuadPart;
	if (tickCount < 0.0f)tickCount = long long(0.0f);
	return float(tickCount) / countsPerSecond;
}
int messageloop()
{
	frameCount++;
	if (GetTime() > 1.0f)
	{
		fps = frameCount;
		frameCount = 0;
		StartTimer();
	}
	frameTime = GetFrameTime();
	render(frameTime);


	std::stringstream stream;
	stream << "FPS: " << fps;
	SetWindowText(hWnd, stream.str().c_str());
	return 0;
}
void setImage(WCHAR *filename, image_info &image)
{
	Bitmap *bitmap;
	bitmap = Bitmap::FromFile(filename, FALSE);
	image.bitsData = new DWORD[bitmap->GetWidth()*bitmap->GetHeight()];
	image.width = bitmap->GetWidth(); image.height = bitmap->GetHeight();
	Color getColor;
	for (UINT i = 0; i < bitmap->GetHeight(); i++)
	for (UINT j = 0; j < bitmap->GetWidth(); j++)
	{
		bitmap->GetPixel(j, i, &getColor);
		image.bitsData[j + i * bitmap->GetWidth()] = RGB(getColor.GetBlue(), getColor.GetGreen(), getColor.GetRed());
	}
}
void deleteImage(image_info &image)
{
	delete[] image.bitsData;
}
void setPixel(int x, int y, unsigned long color)
{
	if (color != 0xff00ff)
	if ((x >= 0) && (x < width) && (y >= 0) && (y < height))bitsData[x + y * width] = color;
}
void setLine(double x1, double z1, double x2, double z2)
{
	double n, ns = .0f;
	
	if (abs(x1 - x2)>abs(z1 - z2))
	{
		if (x1 > x2){
			std::swap(x1, x2); std::swap(z1, z2);
		}
		n = (z2 - z1) / (x2 - x1);
		for (double x = x1; x < x2; x++)
		{
			
			setPixel(x, ns + z1, 0xffff00);
			ns += n;
		}
	}
	else
	{
		if (z1 > z2){
			std::swap(x1, x2); std::swap(z1, z2);
		}
		n = (x2 - x1) / (z2 - z1);
		for (double z = z1; z < z2; z++)
		{
			setPixel(ns+x1, z , 0xffff00);
			ns += n;
		}
	}
	
}
void imageScale(int x, int y, int w, int h,image_info image)
{
	double xf, yf, xfs = 0.0f, yfs = 0.0f;
	int offsetX = 0, offsetY = 0;
	if (offsetX > x)offsetX -= x;
	if (offsetY > y)offsetY -= y;
	xf = image.width / (double)w;
	yf = image.height / (double)h;
	if ((x<640) && (y<480) && (x + w>0) && (y + h>0))
	for (int i = offsetY; i < h; i++)
	{
		for (int j = offsetX; j < w; j++)
		{
			setPixel(x + j, y + i, image.bitsData[int(xfs + offsetX*xf) + int(yfs + offsetY*yf) * image.width]);
			xfs += xf;
			if (x + j>640)break;
		}
		xfs = 0.0f;
		yfs += yf;
		if (y + i>480)break;
	}
}
void render(double time)
{
	memset(bitsData, 128, sizeof(bitsData));
	
	for (int n = 0; n < 80000;n++)
	imageScale(rand()%10000,rand()%10000,30,30, image);

	pBitmap = CreateBitmap(width, height, 1, 32, bitsData);
	SelectObject(hMemDC, pBitmap);
	DeleteObject(pBitmap);
	std::stringstream stream;
	stream << "FPS: " << fps;

	TextOut(hMemDC, 0, 0, stream.str().c_str(), stream.str().length());
	BitBlt(hdc, 0, 0, width, height, hMemDC, 0, 0, SRCCOPY);
	return;
}