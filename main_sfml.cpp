

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
#include <sstream>

#include <Engine.h>
#include <CameraAdds.h>
#include <Image.h>
#include <Config.h>
#include <FrameBuffer.h>

#include "World.h"
#include "Line.hpp"
#include "FullscreenQuad.hpp"
#include "Font.hpp"
#include "Texture.hpp"
#include "Console.hpp"
#include "helpers.hpp"

#ifdef MINICRAFT_WINDOWS
    #include <Windows.h>
#endif


bool keys[256]; // Array Used For The Keyboard Routine
bool active = true; // Window Active Flag Set To TRUE By Default
bool fullscreen= true; // Fullscreen Flag Set To Fullscreen Mode By Default

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

Line* g_L;
bool g_Run = true;
bool consoleOn = false;

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
          NL"layout(location = 0) in vec3 position;"
          NL"layout(location = 1) in vec2 texCoord;"
          NL"layout(location = 2) in vec3 normal;"
          NL
          NL"uniform mat4 Projection, View;"
          NL
          NL"out vec2 var_texCoord;"
          NL"out vec3 var_lightIntensity;"
          NL
          NL"void main () {"
          NL"    var_texCoord = texCoord;"
          NL
          NL"    vec4 LightPosition = vec4(1000, 1000, 1000, 1);"
          NL"    vec3 LightIntensity = vec3(0.8, 0.8, 0.8);"
          NL"    vec3 MaterialDiffuseReflectivity = vec3(1.0, 1.0, 1.0);"
          NL
          NL"    mat3 NormalMatrix = inverse(transpose(mat3(View)));"
          NL"    vec3 tnorm = normalize(NormalMatrix * normal);"
          NL"    vec4 eye = View * vec4(position, 1.0);"
          NL"    vec3 s = normalize(vec3(LightPosition - eye));"
          NL"    // The diffuse shading equation"
          NL"    var_lightIntensity = LightIntensity * MaterialDiffuseReflectivity * max( dot( s, tnorm ), 0.0 );"
          NL
          NL"    gl_Position = Projection * View * vec4(position, 1.0);"
          NL"}";
    
    std::string frag = 
            "#version 400 core"
          NL
          NL"out vec4 out_Color;"
          NL"in vec2 var_texCoord;"
          NL"in vec3 var_lightIntensity;"
          NL
          NL"uniform sampler2D Texture;"
          NL
          NL"void main () {"
        //NL"    out_Color = vec4(0.0, 1.0, 0.0, 1.0);"
         // NL"    out_Color = vec4((out_position.xyz + 1)/2, 1.0);"
        //  NL"    out_Color = vec4(out_texCoord, 0.0, 1.0);"
          NL"    vec2 tc = var_texCoord;"
          NL"    out_Color = /*vec4(var_lightIntensity, 1.0) */ vec4(texture(Texture, tc).rgb, 1.0);"
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
        
        /*auto result = w.raycast(Camera.Position, NormV, 50.f,
            World::INCLUDE_EMPTY | World::STOP_ON_FIRST | World::INCLUDE_FIRST);
        if (result.size() > 1)
        {
            w.set(result[result.size()-2], 9);
            w.recalcInstances();
        }*/

        auto result = w.raycast(Camera.Position, NormV, 50.f,
            World::INCLUDE_EMPTY | World::INCLUDE_FIRST);

        for(auto const & c :result){
            w.set(c,9);
        }

        w.recalcInstances();

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

    Font font;

    FullscreenQuad fq;

    Console console(glm::ivec2(80, 4));
    console.setCallback([&console](std::string const& s){
        if (s == "exit")
            console.write("OMG EXIT");
        else if (s == "recalc")
            w.recalcInstances();
    });

    Texture<TextureType::Texture_2D> mainTexture;
    mainTexture.bind(0);
    mainTexture.setFiltering(FilteringDirection::Minification, FilteringMode::Nearest);
    mainTexture.setFiltering(FilteringDirection::Magnification, FilteringMode::Nearest);
    mainTexture.imageData(ScreenXSize, ScreenYSize, TextureFormat::RGBA, TextureFormat::RGBA, TextureDataType::UnsignedByte, nullptr);

    Texture<TextureType::Texture_2D> depthTexture;
    depthTexture.bind();
    depthTexture.setFiltering(FilteringDirection::Minification, FilteringMode::Nearest);
    depthTexture.setFiltering(FilteringDirection::Magnification, FilteringMode::Nearest);
    depthTexture.imageData(ScreenXSize, ScreenYSize, TextureFormat::Depth, TextureFormat::Depth, TextureDataType::Float, nullptr);

    engine::Framebuffer mainFbo;
    mainFbo.AttachTexture(GL_TEXTURE_2D, mainTexture.getId());
    mainFbo.AttachTexture(GL_TEXTURE_2D, depthTexture.getId(), GL_DEPTH_ATTACHMENT);
    if (!mainFbo.IsValid())
        BREAKPOINT();


    // temporary test lines
    // x-line
    /*for (int i = -20; i < 20; i++) {
        w.set(World::CubePos(1, i, 1), 2);
    }*/

    /*// y-line
    for (int i = -2; i < 5; i++) {
        w.set(World::CubePos(i, 2, 1), 2);
    }
    // z-line
    for (int i = -2; i < 5; i++) {
        w.set(World::CubePos(i, 5, 1), 2);
    }*/
    w.recalcInstances();

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

            if (consoleOn) {
                if (event.type == sf::Event::TextEntered) {
                    if (event.text.unicode == '\b') // ignore backspaces
                        continue;
                    if (event.text.unicode == '\n') // ignore newlines
                        continue;
                    if (event.text.unicode == '\r') // ignore newlines
                        continue;

                    console.inputCharacter(event.text.unicode);
                }
            }
            else {
                if (event.type == sf::Event::KeyPressed)
                    keys[event.key.code] = true;
            }

            if (event.type == sf::Event::KeyReleased) {
                if (event.key.code == sf::Keyboard::Tilde) {
                    consoleOn = !consoleOn;
                    continue;
                }

                if (consoleOn) {
                    if (event.key.code == sf::Keyboard::Return)
                        console.enter();
                    else if (event.key.code == sf::Keyboard::BackSpace)
                        console.backspace();
                } else {
                    keys[event.key.code] = false;
                }
            }

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

            // rendering (pre-pass)
            glBindTexture(GL_TEXTURE_2D, 0);
            mainFbo.Bind();

            glClearColor(63.f/255, 215.f/255, 252.f/255, 1.f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            Camera.CalculateView();	
            glm::mat4 View = Camera.GetViewMat();
            shader->SetUniform("View", View);

            // world (cubes)
            image.Bind(0);
            glEnable(GL_DEPTH_TEST);
            w.draw();

            // crosshair
            L[0].draw(glm::mat4(1.0), glm::mat4(1.0));
            L[1].draw(glm::mat4(1.0), glm::mat4(1.0));

            // raycast line
            L[2].draw(Projection, View);

            // end preprocessing
            engine::Framebuffer::Disable();
            mainTexture.bind(0);

            // final pass with postprocessing
            fq.draw();

            glDisable(GL_DEPTH_TEST);

            // currently targeted block
            glm::mat4 RMatrix = engine::Camera::CreateRotation(Camera.LookDir.x, Camera.LookDir.y, 0.f);
            glm::vec3 NormV (RMatrix[0].z, RMatrix[1].z, RMatrix[2].z);
            NormV *= -1.f;
        
            auto result = w.raycast(Camera.Position, NormV, 50.f, World::STOP_ON_FIRST);
            if (!result.empty()) {
                std::stringstream s;
                s << "[" << result.back().x << ", " << result.back().y << ", " << result.back().z << "]";
                font.draw(s.str(), glm::vec2(10.f, 10.f));
            } else
                font.draw("no target", glm::vec2(10.f, 10.f));

            /*font.draw("ABCDEFGHIJKLMNOPQRSTUVWXYZ", glm::vec2(20.f, 20.f));
            font.draw("abcdefghijklmnopqrstuvwxyz", glm::vec2(20.f, 50.f));
            font.draw("1234567890!@#$%^&*()-=_+[]{};':\",./<>?", glm::vec2(20.f, 80.f));*/

            // CONSOLE
            if (consoleOn) {
                auto const& cbuf = console.getBuffer();
                glm::vec2 position (10.f, 760.f);

                font.draw("> " + console.getInputBuffer(), position);

                for (auto const& line : cbuf) {
                    position += glm::vec2(0.f, -30.f);
                    font.draw(line, position);
                }
            }
        }
        

        window.display();
    }

    return 0;
}

//=========================================================================================


