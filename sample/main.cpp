
//
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//


#include <windows.h>
#include <string>

#include <dwmapi.h>

#include <GL/glew.h>
#include <GL/wglew.h>


HWND		g_hWnd;
HDC			g_hDC;
HGLRC		g_hCtx;
TCHAR       g_szTitle[64]       = { "Genlock" };			
TCHAR       g_szWindowClass[64] = { "OGL" };		

int		    g_FontBase;
bool	    g_framelock				= false;


int			g_width  = 600; 
int			g_height = 600;
LONGLONG	g_freq;


void Resize(int nWidth, int nHeight);
void BuildFont(GLvoid);

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);




bool OpenWindow(bool bFullScreen = false)
{
	DWORD			dwExStyle;
	DWORD			dwStyle;
	int				mPixelFormat;


	if (bFullScreen)
	{
		dwExStyle=WS_EX_APPWINDOW;								
		dwStyle=WS_POPUP;										
		//ShowCursor(FALSE);	
	}
	else
	{
		dwExStyle=WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;	
		dwStyle=WS_OVERLAPPEDWINDOW;
	}

	g_hWnd = CreateWindowEx(
		dwExStyle,
		g_szWindowClass, 
		g_szTitle,
		dwStyle,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		g_width,
		g_height,
		NULL,
		NULL,
		(HINSTANCE)GetModuleHandle(NULL),
		NULL);

	if (!g_hWnd)
		return FALSE;


	static PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW |
		PFD_SUPPORT_OPENGL |
		PFD_DOUBLEBUFFER   |
		PFD_TYPE_RGBA,
		24,
		8, 24, 8, 16, 8, 8,
		8,
		0,
		0,
		0, 0, 0, 0,
		32,
		0,
		0,
		PFD_MAIN_PLANE,
		0,
		0, 0, 0
	};

	g_hDC = GetDC(g_hWnd);

	if (!g_hDC)
		return FALSE;

	mPixelFormat = ChoosePixelFormat(g_hDC, &pfd);

    if (!mPixelFormat)
		return FALSE;

	SetPixelFormat(g_hDC, mPixelFormat, &pfd);

	g_hCtx = wglCreateContext(g_hDC);

	wglMakeCurrent(g_hDC, g_hCtx);

	ShowWindow(g_hWnd, SW_SHOW);

	UpdateWindow(g_hWnd);

	return TRUE;
}



void CloseWindow()
{
	if (g_hCtx)
	{
		wglMakeCurrent(g_hDC, NULL);

		wglDeleteContext(g_hCtx);

		ReleaseDC(g_hWnd, g_hDC);

		DestroyWindow(g_hWnd);

		UnregisterClass(g_szWindowClass, GetModuleHandle(NULL));
	}
}


void Resize(int nWidth, int nHeight)
{

	g_width  = nWidth;
	g_height = nHeight;

	glViewport(0, 0, nWidth, nHeight);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glOrtho(-nWidth/2.0f, nWidth/2.0f, -nHeight/2.0f, nHeight/2.0f, -1.0, 1.0);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}


bool enableFramelock()
{
    const GLuint	nGroup   = 1;
	const GLuint	nBarrier = 1;
	GLuint			nMaxBarriers, nMaxGroups;

	// check for the availability of SwapGroups amd SwapBarriers
	if (!wglQueryMaxSwapGroupsNV(g_hDC, &nMaxGroups, &nMaxBarriers))
		return false;

	if (nMaxGroups <= 0 || nMaxBarriers <= 0)
		return false;
    
    // Currently only one SwapGroup and one Barrier are supported
    // Join to Group 1
	if (!wglJoinSwapGroupNV(g_hDC, nGroup))
		return false;

    // Bind to Barrier 1
	if (!wglBindSwapBarrierNV(nGroup, nBarrier))
		return false;

		
	return true;
}



void disableFramelock()
{
	if (WGLEW_NV_swap_group)
	{
		wglJoinSwapGroupNV(g_hDC, 0);
	}
}



void InitGL()
{
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

	glEnable(GL_DEPTH_TEST);

	BuildFont();
	glListBase(g_FontBase - 32);

	wglSwapIntervalEXT(0);

	QueryPerformanceFrequency((LARGE_INTEGER*)&g_freq);
}



void DrawScene()
{
	char			buf[256];
	static int		nFrameCnt = 0;
	static int		xpos = -g_width/2;
	static int		idx = 0;
	static double	AvrgTime = 0.0;
	static LONGLONG	elapsed = 0;
	static LONGLONG	startCnt = 0;
	LONGLONG		stopCnt;


    if (g_framelock)
    {
        GLuint swapCount;

        wglQueryFrameCountNV(g_hDC, &swapCount);

        xpos = ((swapCount*5) % g_width) - g_width/2;
    }
    else
    {
        xpos += 5;

	    if (xpos > g_width/2)
		    xpos = -g_width/2;
    }

	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBegin(GL_LINES);

	glVertex2i(xpos,  g_height/2);
	glVertex2i(xpos, -g_height/2);

	glEnd();

    sprintf_s(buf,"FRAME : %d   %.2f (%.2f FPS), Framelock %d ", nFrameCnt, AvrgTime * 1000.0f, 1.0f/AvrgTime, g_framelock);
	
	glWindowPos2i(10, 10);
	glCallLists((GLsizei)strlen(buf), GL_UNSIGNED_BYTE, buf);

	SwapBuffers(g_hDC);

	QueryPerformanceCounter((LARGE_INTEGER*)&stopCnt);

	elapsed += (stopCnt - startCnt);

	if ((nFrameCnt % 1) == 0)
	{
		AvrgTime = elapsed / 1.0;
		AvrgTime = AvrgTime /(double)g_freq;
		elapsed  = 0;
	}

	++nFrameCnt;

	startCnt = stopCnt;
}



int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nShowCmd)
{
    WNDCLASSEX		wndclass;

    // Register WindowClass
	wndclass.cbSize         = sizeof(WNDCLASSEX);
	wndclass.style          = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wndclass.lpfnWndProc    = WndProc;
	wndclass.cbClsExtra     = 0;
	wndclass.cbWndExtra     = 0;
	wndclass.hInstance      = (HINSTANCE)GetModuleHandle(NULL);
	wndclass.hIcon		    = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor        = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground  = NULL;
	wndclass.lpszMenuName   = NULL;
	wndclass.lpszClassName  = g_szWindowClass;
	wndclass.hIconSm		= LoadIcon(NULL, IDI_APPLICATION);


	if (!RegisterClassEx(&wndclass))
		return FALSE;
	const bool FS = true;

	// Determine current RefreshRate
	DEVMODE				DevMode;

	EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &DevMode);

	if (FS)
	{
		g_width  = DevMode.dmPelsWidth;
		g_height = DevMode.dmPelsHeight;
	}

	OpenWindow(FS);

	if (glewInit() != GLEW_OK)
		return WM_QUIT;

	if (!WGLEW_NV_swap_group)
	{
		MessageBox(NULL, "Framelock is not supported by this card. It requires a FirePro Nothern Island or newer based GPU.", "Note", MB_OK);
	    return WM_QUIT;
	}

	InitGL();

	Resize(g_width, g_height);

	BuildFont();

	bool done = false;

    MSG msg;

	while (!done)
	{

		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				done = true;
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		DrawScene();
	}

	CloseWindow();

	return WM_QUIT;
}



LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		char c;

	case WM_CHAR:
		c = (char)wParam;

		switch (c)
        {
		case 'f':
			if (g_framelock)
			{
				disableFramelock();
				g_framelock = false;
			}
			else
			{
				g_framelock = enableFramelock();
			}

			break;

		case 'q':
		case VK_ESCAPE:
			PostQuitMessage(0);
			break;
		}

		return 0;

	case WM_CREATE:
		return 0;

	case WM_SIZE:
		Resize(LOWORD(lParam), HIWORD(lParam));
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


void BuildFont(GLvoid)								
{
	HFONT	font;										
	HFONT	oldfont;

	g_FontBase = glGenLists(96);	

	font = CreateFont(-12,								// Height Of Font
						0,								// Width Of Font
						0,								// Angle Of Escapement
						0,								// Orientation Angle
						FW_BOLD,						// Font Weight
						FALSE,							// Italic
						FALSE,							// Underline
						FALSE,							// Strikeout
						ANSI_CHARSET,					// Character Set Identifier
						OUT_TT_PRECIS,					// Output Precision
						CLIP_DEFAULT_PRECIS,			// Clipping Precision
						ANTIALIASED_QUALITY,			// Output Quality
						FF_DONTCARE|DEFAULT_PITCH,		// Family And Pitch
						"Times New Roman");				// Font Name

	oldfont = (HFONT)SelectObject(g_hDC, font);          
	wglUseFontBitmaps(g_hDC, 32, 96, g_FontBase);		// Builds 96 Characters Starting At Character 32
	SelectObject(g_hDC, oldfont);						
	DeleteObject(font);			
}
