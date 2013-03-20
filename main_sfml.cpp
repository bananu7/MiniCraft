

#include <SFML/Window.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GL/glew.h>

#include <string>
#include <iostream>
#include <array>
#include <vector>
#include <list>
#include <istream>
#include <fstream>

#include <Engine.h>
#include <CameraAdds.h>
#include <Image.h>
#include <Config.h>
#include <FrameBuffer.h>

#include "World.h"
#include "Line.hpp"
#include "FullscreenQuad.hpp"

#ifdef MINICRAFT_WINDOWS
	#include <Windows.h>
#endif


bool	keys[256];			// Array Used For The Keyboard Routine
bool	active = true;		// Window Active Flag Set To TRUE By Default
bool	fullscreen= true;	// Fullscreen Flag Set To Fullscreen Mode By Default

const int ScreenXSize = 1280;
const int ScreenYSize = 800;

#define NL "\n"
 
using std::vector;
using std::string;
using std::unique_ptr;

glm::mat4 Projection, View;

engine::CameraFly Camera;

float Time = 0.f;

engine::Image image;
auto shader = std::make_shared<engine::Program>();
World w { shader };

void mouse(double dmx, double dmy);

void CheckForGLError()
{
	int Err = glGetError();
	if (Err != 0)
		BREAKPOINT();
}

#include <boost/range/istream_range.hpp>
// istreambuf version of boost::istream_range; required for binary data
//FIXME traits support and possibly char<->uchar
template<class Elem> inline
        boost::iterator_range<std::istreambuf_iterator<Elem> >
        istreambuf_range(std::basic_istream<Elem>& in)
        {
            return boost::iterator_range<std::istreambuf_iterator<Elem> >(
                std::istreambuf_iterator<Elem>(in),
                std::istreambuf_iterator<Elem>());
        }

Line* g_L;
bool g_Run = true;

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
	CheckForGLError();

	std::string vert = 
            "#version 400 core"
          NL"precision mediump float;"
          NL"layout(location = 0) in vec3 position;"
		  NL"layout(location = 1) in vec2 texCoord;"
		  NL
		  NL"uniform mat4 Projection, View;"
		  NL
		  NL"out vec2 var_texCoord;"
		  NL
          NL"void main () {"
		  NL"    var_texCoord = texCoord;"
          NL"    gl_Position = Projection * View * vec4(position, 1.0);"
          NL"}";
	
	std::string frag = 
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

	typedef unsigned char uc;
	try 
	{
		{
			auto vs = std::make_shared<engine::VertexShader>(vert);
			vs->Compile();
			if (!vs)
				throw (vs->Status());
			shader->AttachShader(vs);
		}
		CheckForGLError();

		{
			auto fs = std::make_shared<engine::FragmentShader>(frag);
			fs->Compile();
			if (!fs)
				throw (fs->Status());
			shader->AttachShader(fs);
		}
		CheckForGLError();
	
		shader->Link();
		CheckForGLError();
		if (!shader->Validate())
			throw std::runtime_error("Program not valid");
		shader->Bind();
		CheckForGLError();
		shader->SetUniform("Projection", Projection);
		CheckForGLError();
	}
	catch (std::exception const& e) {
		//MessageBox(0, e.what(), "Exception", MB_OK | MB_ICONERROR);
		BREAKPOINT();
	}

	// FIXME : rekompilacja psuje uniformy!
//	shader.Compile();
}

void initResources()
{
	try {
		std::basic_ifstream<unsigned char> file ("../data/terrain.png", std::ios::binary);
		if (!file)
			throw std::runtime_error("File not open");

		// I prefer the shorter version
		image = engine::Image::Load(istreambuf_range<unsigned char>(file));
		//image = engine::Image::Load(boost::make_iterator_range(std::istreambuf_iterator<unsigned char>(file),
		//													   std::istreambuf_iterator<unsigned char>()));
	}
	catch (std::exception const& e) {
		//MessageBox(0, e.what(), "Texture loading failed", MB_OK | MB_ICONERROR);
		BREAKPOINT();
	}

	const unsigned texUnitNum = 0;
	image.Bind(texUnitNum);
	// pixels!
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	shader->SetTex("Texture", texUnitNum);
}

void keyboard()
{
	if (keys[sf::Keyboard::D])
		Camera.Strafe(-0.2f);

	if (keys[sf::Keyboard::A])
		Camera.Strafe(0.2f);

	if (keys[sf::Keyboard::E])
		Camera.LookDir.y += 5;
	if (keys[sf::Keyboard::Q])
		Camera.LookDir.y -= 5;

	if (keys[sf::Keyboard::S])
		Camera.Fly(-0.2f);
	if (keys[sf::Keyboard::W])
		Camera.Fly(0.2f);
	if (keys[sf::Keyboard::Space])
		Camera.Position.y += 0.2f;
	if (keys[sf::Keyboard::C])
		Camera.Position.y -= 0.2f;

	if (keys[sf::Keyboard::R]) // remove
	{
		glm::mat4 RMatrix = engine::Camera::CreateRotation(Camera.LookDir.x, Camera.LookDir.y, 0.f);
		glm::vec3 NormV (RMatrix[0].z, RMatrix[1].z, RMatrix[2].z);
		NormV *= -1.f;
		
		auto result = w.raycast(Camera.Position, NormV, 50.f, World::STOP_ON_FIRST);
		if (!result.empty())
		{
			w.set(result.back(), 0);
			w.recalcInstances();
		}

		g_L->set(Camera.Position, Camera.Position + NormV * 50.f);
		keys[sf::Keyboard::R] = false;
	}
	if (keys[sf::Keyboard::T]) // add
	{
		glm::mat4 RMatrix = engine::Camera::CreateRotation(Camera.LookDir.x, Camera.LookDir.y, 0.f);
		glm::vec3 NormV (RMatrix[0].z, RMatrix[1].z, RMatrix[2].z);
		NormV *= -1.f;
		
		auto result = w.raycast(Camera.Position, NormV, 50.f,
			World::INCLUDE_EMPTY | World::STOP_ON_FIRST | World::INCLUDE_FIRST);
		if (result.size() > 1)
		{
			w.set(result[result.size()-2], 9);
			w.recalcInstances();
		}

		g_L->set(Camera.Position, Camera.Position + NormV * 50.f);
		keys[sf::Keyboard::T] = false;
	}

	if (keys[sf::Keyboard::Escape])
		g_Run = false;
}

void mouse(double dmx, double dmy)
{
	Camera.LookDir.y += dmx * 50.0;
	Camera.LookDir.x += dmy * 50.0;
}


#ifdef MINICRAFT_WINDOWS
int WINAPI WinMain (HINSTANCE hInstance,
					HINSTANCE hPrevInstance,
					LPSTR lpCmdLine,
					int iCmdShow)
#else
int main()
#endif
{
	sf::Window window(sf::VideoMode(ScreenXSize, ScreenYSize), "Minicraft v0.2", 7, sf::ContextSettings(24, 0, 0, 4, 0));

	window.setMouseCursorVisible(false);

	glewInit();

	init();
	initShadersEngine();
	initResources();
	w.init();
	w.recalcInstances();

	Line L[3];
	g_L = &L[2];
	L[0].set(glm::vec3(0, -0.05f, 0.f), glm::vec3(0, 0.05f, 0.f));
	L[1].set(glm::vec3(-0.05f, 0.f, 0.f), glm::vec3(0.05f, 0.f, 0.f));
	L[2].set(glm::vec3(-0.05f, 0.f, 0.f), glm::vec3(0.05f, 0.f, 0.f));

	FullscreenQuad fq;
	GLuint mainTexture, depthTexture;
	glGenTextures(1, &mainTexture);
	glGenTextures(1, &depthTexture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mainTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ScreenXSize, ScreenYSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

	glBindTexture(GL_TEXTURE_2D, depthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, ScreenXSize, ScreenYSize, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	engine::Framebuffer mainFbo;
	mainFbo.AttachTexture(GL_TEXTURE_2D, mainTexture);
	mainFbo.AttachTexture(GL_TEXTURE_2D, depthTexture, GL_DEPTH_ATTACHMENT);
	if (!mainFbo.IsValid())
		BREAKPOINT();

	while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

			if (event.type == sf::Event::KeyPressed)
				keys[event.key.code] = true;

			if (event.type == sf::Event::KeyReleased)
				keys[event.key.code] = false;

			if (event.type == sf::Event::GainedFocus)
				active = true;
			if (event.type == sf::Event::LostFocus)
				active = false;
        }

		if (!g_Run)
		{
			window.close();
			break;
		}

        if (active)
		{
			keyboard();
			sf::Vector2i mouseP = sf::Mouse::getPosition();
			sf::Vector2f mouseNormalized;

			mouseNormalized.x = (mouseP.x - ScreenXSize/2) / (float)(ScreenXSize/2);
			mouseNormalized.y = (mouseP.y - ScreenYSize/2) / (float)(ScreenYSize/2);

			mouse(mouseNormalized.x, mouseNormalized.y);
			sf::Mouse::setPosition(sf::Vector2i(ScreenXSize/2, ScreenYSize/2));

			// rendering
			
			mainFbo.Bind();

			glClearColor(63.f/255, 215.f/255, 252.f/255, 1.f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			Camera.CalculateView();	
			glm::mat4 View = Camera.GetViewMat();
			shader->SetUniform("View", View);

			image.Bind(0);
			glEnable(GL_DEPTH_TEST);
			w.draw();

			L[0].draw(glm::mat4(1.0), glm::mat4(1.0));
			L[1].draw(glm::mat4(1.0), glm::mat4(1.0));
			L[2].draw(Projection, View);

			engine::Framebuffer::Disable();

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, mainTexture);
			fq.draw();
		}
		

        window.display();
    }

	return 0;
}

//=========================================================================================


