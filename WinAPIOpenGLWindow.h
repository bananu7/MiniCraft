#pragma once

#include "windows.h"
#include <GL/glew.h>
#include <GL/wglew.h>

class CWinAPIOpenGLWindow
{
	HDC			m_hDC;			// Private GDI Device Context
	HGLRC		m_hRC;			// Permanent Rendering Context
	HWND		m_hWnd;			// Holds Our Window Handle
	HINSTANCE	m_hInstance;		// Holds The Instance Of The Application
	bool		m_Fullscreen;

public:
	bool Create(char* title, int width, int height, int bits, bool fullscreenflag, WNDPROC wndProc);
	void Kill();
	void Resize(GLsizei width, GLsizei height);
	void Display ()
	{
		SwapBuffers(m_hDC);
	}

	HWND GetHwnd() { return m_hWnd; }

	CWinAPIOpenGLWindow(void);
	~CWinAPIOpenGLWindow(void);
};

