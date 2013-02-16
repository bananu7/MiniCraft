#include "WinAPIOpenGLWindow.h"

#include <GL/glew.h>
#include <string>
#include <iostream>
#include <array>
#include <vector>
#include <list>

#include "Engine.h"
#include "CameraAdds.h"
#include "Image.h"
#include <glm/gtc/matrix_transform.hpp>

#include "World.h"


bool	keys[256];			// Array Used For The Keyboard Routine
bool	active = true;		// Window Active Flag Set To TRUE By Default
bool	fullscreen= true;	// Fullscreen Flag Set To Fullscreen Mode By Default

const unsigned int ScreenXSize = 1280;
const unsigned int ScreenYSize = 800;

CWinAPIOpenGLWindow Win;

GLuint vboId;
GLuint vaoId;
GLuint programId;
GLuint shaderIds[4];
 
//GL_, GL_TESS_CONTROL_SHADER, GL_TESS_EVALUATION_SHADER, GL_GEOMETRY_SHADER, or GL_FRAGMENT_SHADER.
#define NL "\n"
 
using std::vector;
using std::string;
using std::unique_ptr;

template<class T> T& as_lvalue(T&& v){ return v; }
 
void CheckForError()
{
    GLint E = glGetError();
    if (E != 0)
        _CrtDbgBreak();
}
 

glm::mat4 Projection, View;

CCameraFly Camera;

float Time = 0.f;

CImage Image;
CShader Shader;
World w (&Shader);

void registerMouse()
{
	#ifndef HID_USAGE_PAGE_GENERIC
    #define HID_USAGE_PAGE_GENERIC         ((USHORT) 0x01)
    #endif
    #ifndef HID_USAGE_GENERIC_MOUSE
    #define HID_USAGE_GENERIC_MOUSE        ((USHORT) 0x02)
    #endif

    RAWINPUTDEVICE Rid[1];
    Rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC; 
    Rid[0].usUsage = HID_USAGE_GENERIC_MOUSE; 
    Rid[0].dwFlags = RIDEV_INPUTSINK;   
	Rid[0].hwndTarget = Win.GetHwnd();
    RegisterRawInputDevices(Rid, 1, sizeof(Rid[0]));
}

void mouse(double dmx, double dmy);


LRESULT CALLBACK WndProc(	HWND	hWnd,
							UINT	uMsg,
							WPARAM	wParam,
							LPARAM	lParam)
{
	switch (uMsg)
	{
		case WM_ACTIVATE:
		{
			if (LOWORD(wParam) != 0)
			{
				active = true;
				ShowCursor(false);
			}
			else
			{
				active = false;
				ShowCursor(true);
			}

			return 0;								// Return To The Message Loop
		}

		case WM_SYSCOMMAND:							// Intercept System Commands
		{
			switch (wParam)							// Check System Calls
			{
				case SC_SCREENSAVE:					// Screensaver Trying To Start?
				case SC_MONITORPOWER:				// Monitor Trying To Enter Powersave?
				return 0;							// Prevent From Happening
			}
			break;									// Exit
		}

		case WM_CLOSE:								// Did We Receive A Close Message?
		{
			PostQuitMessage(0);						// Send A Quit Message
			return 0;								// Jump Back
		}

		case WM_KEYDOWN:							// Is A Key Being Held Down?
		{
			keys[wParam] = TRUE;					// If So, Mark It As TRUE
			return 0;								// Jump Back
		}

		case WM_KEYUP:								// Has A Key Been Released?
		{
			keys[wParam] = FALSE;					// If So, Mark It As FALSE
			return 0;								// Jump Back
		}

		case WM_INPUT: 
		{
			UINT dwSize = 40;
			static BYTE lpb[40];
    
			GetRawInputData((HRAWINPUT)lParam, RID_INPUT, 
							lpb, &dwSize, sizeof(RAWINPUTHEADER));
    
			RAWINPUT* raw = (RAWINPUT*)lpb;
    
			if (raw->header.dwType == RIM_TYPEMOUSE) 
			{
				if (raw->data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE)
				{
					_CrtDbgBreak();
				}
				else
				{
					int xPosRelative = raw->data.mouse.lLastX;
					int yPosRelative = raw->data.mouse.lLastY;
					mouse(xPosRelative, yPosRelative);
				}
			} 
			return 0;
		}
		
		/*case WM_SIZE:								// Resize The OpenGL Window
		{
			Re(LOWORD(lParam),HIWORD(lParam));  // LoWord=Width, HiWord=Height
			return 0;								// Jump Back
		}*/
	}

	// Pass All Unhandled Messages To DefWindowProc
	return DefWindowProc(hWnd,uMsg,wParam,lParam);
}

void CheckForGLError()
{
	int Err = glGetError();
	if (Err != 0)
		_CrtDbgBreak();
}


void init()
{
	Projection = glm::perspective(70.0, 16.0/10.0, 0.1, 1000.0);

	Camera.Position = glm::vec3(0, -2, 20);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	glEnable(GL_TEXTURE_2D);
}

void initShadersEngine()
{
	CSimpleDirectLoader::TDataMap Data;
	std::string Vert = 
            "#version 400 core"
          NL"precision highp float;"
          NL"layout(location = 0) in vec3 position;"
		  NL"layout(location = 1) in vec2 texCoord;"
		  NL"layout(location = 2) in vec3 instance_position;"
		  NL"layout(location = 3) in vec2 instance_texCoord;"
		  NL
		  NL"uniform mat4 Projection, View;"
		  NL
		  NL"out vec2 var_texCoord;"
		  NL
          NL"void main () {"
		  NL"    var_texCoord = (texCoord) / 16.0 + instance_texCoord;"
          NL"    gl_Position = Projection * View * vec4(position + instance_position, 1.0);"
          NL"}";
	
	std::string Frag = 
            "#version 400 core"
		  NL
		  NL"out vec4 out_Color;"
		  NL"in vec2 var_texCoord;"
		  NL
		  NL"uniform sampler2D Texture;"
		  NL
          NL"void main () {"
        //NL"    out_Color = vec4(0.0, 1.0, 0.0, 1.0);"
		 // NL"    out_Color = vec4((out_position.xyz + 1)/2, 1.0);"
		//  NL"    out_Color = vec4(out_texCoord, 0.0, 1.0);"
		  NL"    vec2 tc = var_texCoord;"
		  NL"    out_Color = vec4(texture(Texture, tc).rgb, 1.0);"
          NL"}";

	using uc = unsigned char;

	Data["frag"] = vector<uc> (Frag.begin(), Frag.end());
	Data["vert"] = vector<uc> (Vert.begin(), Vert.end());

	auto Loader = CSimpleDirectLoader(Data);
	string Result = Shader.Load(Loader);
	if (!Result.empty())
		_CrtDbgBreak();
	Shader.Bind();

	Shader.SetUniform("Projection", Projection);

	// FIXME : rekompilacja psuje uniformy!
//	Shader.Compile();
}

void initResources()
{
	CSimpleFileLoader Loader("../data/terrain.png");
	auto Result = Image.Load(Loader);
	//if (Result != "")
	//	_CrtDbgBreak();

	const unsigned texUnitNum = 0;
	Image.Bind(texUnitNum);
	// pixels!
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	Shader.SetTex("Texture", texUnitNum);
}

void keyboard()
{
	if (keys['D'])
		Camera.Strafe(-0.2f);

	if (keys['A'])
		Camera.Strafe(0.2f);

	if (keys['E'])
		Camera.LookDir.y += 5;
	if (keys['Q'])
		Camera.LookDir.y -= 5;

	if (keys['S'])
		Camera.Fly(-0.2f);
	if (keys['W'])
		Camera.Fly(0.2f);
	if (keys[VK_SPACE])
		Camera.Position.y += 0.2;
	if (keys['C'])
		Camera.Position.y -= 0.2;
	if (keys[VK_ESCAPE])
		PostQuitMessage(0);
}

void mouse(double dmx, double dmy)
{
	Camera.LookDir.y += dmx * 50.0;
	Camera.LookDir.x += dmy * 50.0;
}

int WINAPI WinMain (HINSTANCE hInstance,
					HINSTANCE hPrevInstance,
					LPSTR lpCmdLine,
					int iCmdShow)
{
	Win.Create("OpenGLUI test", ScreenXSize, ScreenYSize, 32, false, WndProc);
	Win.Resize(ScreenXSize, ScreenYSize);
	
//	registerMouse();

	init();
	initShadersEngine();
	initResources();
	w.init();
	w.recalcInstances();

	bool Run = true;
	do 
	{
		MSG	msg;
		if (PeekMessage(&msg,NULL,0,0,PM_REMOVE))	// Is There A Message Waiting?
		{
			if (msg.message==WM_QUIT)				// Have We Received A Quit Message?
			{
				Run = false;
			}
			else									// If Not, Deal With Window Messages
			{
				TranslateMessage(&msg);				// Translate The Message
				DispatchMessage(&msg);				// Dispatch The Message
			}
		}

		//input
		if (active)
		{
			keyboard();
			POINT p;
			GetCursorPos(&p);
			p.x -= 640;
			p.y -= 400;
			mouse(p.x/640.0, p.y/400.0);
			SetCursorPos(640, 400);

			// rendering
			glClearColor(0.5f, .5f, .5f, 1.f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			Camera.CalculateView();	
			glm::mat4 View = Camera.GetViewMat();
			Shader.SetUniform("View", View);
			w.draw();
		}

		Win.Display();
	} while (Run);

	return 0;
}

//=========================================================================================

