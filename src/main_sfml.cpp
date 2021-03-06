

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

#include <lundi.hpp>

#include "World.h"
#include "Line.hpp"
#include "FullscreenQuad.hpp"
#include "Font.hpp"
#include "Texture.hpp"
#include "Console.hpp"
#include "helpers.hpp"
#include "Player.h"
#include "Model.h"

#ifdef MINICRAFT_WINDOWS
    #include <Windows.h>
#endif


bool keys[256]; // Array Used For The Keyboard Routine
bool active = true; // Window Active Flag Set To TRUE By Default
bool fullscreen= true; // Fullscreen Flag Set To Fullscreen Mode By Default

const int ScreenXSize = 1280;
const int ScreenYSize = 800;

using std::vector;
using std::string;
using std::unique_ptr;

glm::mat4 Projection, View;

engine::CameraFly Camera;

float Time = 0.f;

engine::Image image;
engine::Image test_texture;
auto world_shader = std::make_shared<engine::Program>();
auto trivial_shader = std::make_shared<engine::Program>();
auto texturing_shader = std::make_shared<engine::Program>();

void CheckForGLError()
{
    int Err = glGetError();
    if (Err != 0) {
        ERROR_MESSAGE("", "OpenGL error");
        BREAKPOINT();
    }
}

Line* g_L;
bool g_Run = true;
bool consoleOn = false;

void init()
{
    Projection = glm::perspective(70.0, 16.0/10.0, 0.1, 1000.0);

    Camera.Position = glm::vec3(0, 0, 20);
    glEnable(GL_DEPTH_TEST);
    //glEnable(GL_CULL_FACE);

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_FRAMEBUFFER_SRGB);
}

template<typename VSdata, typename FSdata>
void safeLoadShader(std::shared_ptr<engine::Program>& shader, VSdata vsd, FSdata fsd) {
    try 
    {
        {
            auto vs = std::make_shared<engine::VertexShader>(vsd);
            auto compResult = vs->Compile();
            if (!compResult.empty())
                throw std::runtime_error(compResult);
            if (!vs)
                throw std::runtime_error(vs->Status());
            shader->AttachShader(vs);
        }
        CheckForGLError();

        {
            auto fs = std::make_shared<engine::FragmentShader>(fsd);
            auto compResult = fs->Compile();
            if (!compResult.empty())
                throw std::runtime_error(compResult);
            if (!fs)
                throw std::runtime_error(fs->Status());
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
        
        BREAKPOINT();
    }
}

void initShadersEngine()
{
    CheckForGLError();
    {
        std::ifstream vert("../Data/shaders/main_cubes.vert");
        std::ifstream frag("../Data/shaders/main_cubes.frag");

        safeLoadShader(world_shader, istreambuf_range<char>(vert), istreambuf_range<char>(frag));
    }
    
    {
        std::string vert = 
        "#version 330 core"
        NL "precision mediump float;"
        NL "layout(location = 0) in vec3 in_position;"
        NL "uniform mat4 Projection, View;"
        NL "void main() {"
        NL "    gl_Position = Projection * View * vec4(in_position, 1.0);"
        NL "}" NL;

        std::string frag = 
        "#version 330 core"
        NL "precision mediump float;"
        NL "out vec4 out_Color;"
        NL "void main () {"
        NL "    out_Color = vec4(1.0, 0.0, 0.0, 1.0);"
        NL "}" NL;

        safeLoadShader(trivial_shader, vert, frag);
    }

    {
        std::ifstream vert("../Data/shaders/texturing_and_light.vert");
        std::ifstream frag("../Data/shaders/texturing_and_light.frag");
        safeLoadShader(texturing_shader, istreambuf_range<char>(vert), istreambuf_range<char>(frag));
    }

    // FIXME : rekompilacja psuje uniformy!
//	shader.Compile();
}

void initResources()
{
    auto safeLoadImage = [](std::string const& path, bool srgb) -> engine::Image {
        try {
            std::ifstream file (path, std::ios::binary);
            if (!file)
            throw std::runtime_error(path + " file not open");

            // I prefer the shorter version
            return engine::Image::Load(istreambuf_range<char>(file), srgb);   
            //image = engine::Image::Load(boost::make_iterator_range(std::istreambuf_iterator<char>(file),
            //													   std::istreambuf_iterator<char>()));
        }
        catch (std::exception const& e) {
            ERROR_MESSAGE(e.what(), "initResources");
            BREAKPOINT();
        }

        return engine::Image();
    };

    image = safeLoadImage("../Data/terrain.png", true);
    test_texture = safeLoadImage("../Data/models/cube/Untitled.png", true);

    const unsigned texUnitNum = 0;
    image.bind(texUnitNum);
    // pixels!
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    world_shader->SetTex("Texture", texUnitNum);

    test_texture.bind();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

class App {
public:
    Console console;
    lua::state luaVm;
    World world;
    Model fence;
    Player player;

    App()
        : console(glm::ivec2(80, 4))
        , luaVm([this](std::string const& error) {
            this->console.write(error);
        })
        , world(world_shader)
        , fence("../Data/models/fence2.obj", texturing_shader)
        , player([this](Minefield::WorldCoord const& wc) {
            return (world.get(wc).value) ? false : true;
        })
    {
        console.setCallback([this](std::string const& s){
            this->luaVm.eval(s);
        });

        luaVm.register_function("exit", [this]{ this->console.write("OMG EXIT"); });
        luaVm.register_function("print", [this](std::string s){ this->console.write(std::move(s)); });
        luaVm.register_function("recalc", [this]{ this->world.recalcInstances(); });
        luaVm.register_function("recalc_full", [this]{ this->world.recalcInstances(true); });
        luaVm.register_function("save", [this]{ this->world.saveToFile("world.mcw"); });
        luaVm.register_function("load", [this]{ this->world.loadFromFile("world.mcw"); });
        luaVm.register_function("set", [this](int x, int y, int z, int v) { world.set(Minefield::WorldCoord(x,y,z), v); });
        luaVm.register_function("get", [this](int x, int y, int z) { return static_cast<int>(world.get(Minefield::WorldCoord(x,y,z)).value); });
        luaVm.register_function("fly", [this]{ player.flying = !player.flying; });

        init();
        initShadersEngine();
        initResources();

        world.init();
        world.recalcInstances();
    }

    void keyboard()
    {
        if (keys[sf::Keyboard::D]) {
            player.move(Player::Direction::StrafeRight);
            //Camera.Strafe(-0.2f);
        }

        if (keys[sf::Keyboard::A]) {
            player.move(Player::Direction::StrafeLeft);
            //Camera.Strafe(0.2f);
        }

        if (keys[sf::Keyboard::S]) {
            //Camera.Fly(-0.2f);
            player.move(Player::Direction::Back);
        }
        if (keys[sf::Keyboard::W]) {
            //Camera.Fly(0.2f);
            player.move(Player::Direction::Forward);
        }
        if (keys[sf::Keyboard::Space]) {
            //Camera.Position.y += 0.2f;
            player.jump();
            keys[sf::Keyboard::Space] = false;
        }
        if (keys[sf::Keyboard::C])
            Camera.Position.y -= 0.2f;

        if (keys[sf::Keyboard::R]) // remove
        {
            glm::mat4 RMatrix = engine::Camera::CreateRotation(Camera.LookDir.x, Camera.LookDir.y, 0.f);
            glm::vec3 NormV (RMatrix[0].z, RMatrix[1].z, RMatrix[2].z);
            NormV *= -1.f;
        
            auto result = world.raycast(Camera.Position, NormV, 50.f, World::STOP_ON_FIRST);
            if (!result.empty())
            {
                world.set(result.back(), 0);
                world.recalcInstances();
            }

            g_L->set(Camera.Position, Camera.Position + NormV * 30.f);
            keys[sf::Keyboard::R] = false;
        }
        if (keys[sf::Keyboard::T]) // add
        {
            glm::mat4 RMatrix = engine::Camera::CreateRotation(Camera.LookDir.x, Camera.LookDir.y, 0.f);
            glm::vec3 NormV (RMatrix[0].z, RMatrix[1].z, RMatrix[2].z);
            NormV *= -1.f;
        
            auto result = world.raycast(Camera.Position, NormV, 50.f,
                World::INCLUDE_EMPTY | World::STOP_ON_FIRST | World::INCLUDE_FIRST);
            if (result.size() > 1)
            {
                world.set(result[result.size()-2], 9);
                world.recalcInstances();
            }

            /*auto result = world.raycast(Camera.Position, NormV, 30.f,
                World::INCLUDE_EMPTY | World::INCLUDE_FIRST | World::STOP_ON_FIRST);

            for(auto const & c :result){
                world.set(c,9);
            }
            world.recalcInstances();
            */

            g_L->set(Camera.Position, Camera.Position + NormV * 20.f);
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

};


#ifdef MINICRAFT_WINDOWS
int WINAPI WinMain (HINSTANCE hInstance,
                    HINSTANCE hPrevInstance,
                    LPSTR lpCmdLine,
                    int iCmdShow)
#else
int main()
#endif
{
    //ERROR_MESSAGE("hello", "hello");
    sf::Window window(sf::VideoMode(ScreenXSize, ScreenYSize), "Minicraft v0.3", 7, sf::ContextSettings(24, 0, 0, 3, 3));

    window.setMouseCursorVisible(false);

    glewInit();

    Line L[3];
    g_L = &L[2];
    L[0].set(glm::vec3(0, -0.05f, 0.f), glm::vec3(0, 0.05f, 0.f));
    L[1].set(glm::vec3(-0.05f, 0.f, 0.f), glm::vec3(0.05f, 0.f, 0.f));
    L[2].set(glm::vec3(-0.05f, 0.f, 0.f), glm::vec3(0.05f, 0.f, 0.f));

    App app;

    Font font;
    app.player.setDirection(Camera.LookDir);

    FullscreenQuad fq;


    namespace tex_desc = engine::texture_desc;

    engine::Texture<tex_desc::Type::Texture_2D> mainTexture;
    mainTexture.bind(0);
    mainTexture.setFiltering(tex_desc::FilteringDirection::Minification, tex_desc::FilteringMode::Nearest);
    mainTexture.setFiltering(tex_desc::FilteringDirection::Magnification, tex_desc::FilteringMode::Nearest);
    //mainTexture.imageData(ScreenXSize, ScreenYSize, TextureFormat::RGBA, TextureInternalFormat::SRGBA, TextureDataType::UnsignedByte, nullptr);
    mainTexture.imageData(ScreenXSize, ScreenYSize, tex_desc::Format::RGBA, tex_desc::InternalFormat::RGBA, tex_desc::DataType::Float, nullptr);

    engine::Texture<tex_desc::Type::Texture_2D> depthTexture;
    depthTexture.bind();
    depthTexture.setFiltering(tex_desc::FilteringDirection::Minification, tex_desc::FilteringMode::Nearest);
    depthTexture.setFiltering(tex_desc::FilteringDirection::Magnification, tex_desc::FilteringMode::Nearest);
    depthTexture.imageData(ScreenXSize, ScreenYSize, tex_desc::Format::Depth, tex_desc::InternalFormat::Depth, tex_desc::DataType::Float, nullptr);

    engine::Framebuffer mainFbo;
    mainFbo.AttachTexture(mainTexture, engine::fbo_desc::Attachment::Color_0);
    mainFbo.AttachTexture(depthTexture, engine::fbo_desc::Attachment::Depth);
    if (!mainFbo.IsValid())
        BREAKPOINT();


    // temporary surrounding generation
    for (int j = -1; j < 1; ++j)
        for (int i = -1; i < 1; ++i) {
            app.world.set(Minefield::WorldCoord(i*24, 1, j*24), 2);
        }

    app.world.recalcInstances();

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

                    app.console.inputCharacter(event.text.unicode);
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
                        app.console.enter();
                    else if (event.key.code == sf::Keyboard::BackSpace)
                        app.console.backspace();
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

        // updates:
        app.player.gravity();
        Camera.Position = app.player.getPosition();
        // compensate for eye height
        Camera.Position.y += 1.620f;
        app.player.setDirection(Camera.LookDir);

        if (active)
        {
            app.keyboard();
            sf::Vector2i mouseP = sf::Mouse::getPosition();
            sf::Vector2f mouseNormalized;

            mouseNormalized.x = (mouseP.x - ScreenXSize/2) / (float)(ScreenXSize/2);
            mouseNormalized.y = (mouseP.y - ScreenYSize/2) / (float)(ScreenYSize/2);

            app.mouse(mouseNormalized.x, mouseNormalized.y);
            sf::Mouse::setPosition(sf::Vector2i(ScreenXSize/2, ScreenYSize/2));

            // rendering (pre-pass)
            glBindTexture(GL_TEXTURE_2D, 0);
            mainFbo.Bind();

            glClearColor(63.f/255, 215.f/255, 252.f/255, 1.f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            Camera.CalculateView();	
            glm::mat4 View = Camera.GetViewMat();
            world_shader->SetUniform("View", View);
            world_shader->SetUniform("Eye", Camera.Position);

            // world (cubes)
            image.bind(0);
            glEnable(GL_DEPTH_TEST);
            app.world.draw();

            trivial_shader->SetUniform("View", View);

            texturing_shader->SetUniform("View", View);
            texturing_shader->SetUniform("Eye", Camera.Position);
            // test fence
            test_texture.bind(0);
            app.fence.draw();

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

            mainTexture.bind(1);
        
            auto result = app.world.raycast(Camera.Position, NormV, 20.f, World::STOP_ON_FIRST);
            if (!result.empty()) {
                std::stringstream s;
                s << "[" << result.back().x << ", " << result.back().y << ", " << result.back().z << "]";
                font.draw(s.str(), glm::vec2(10.f, 10.f));
            } else
                font.draw("no target", glm::vec2(10.f, 10.f));

            //player position 
            {
                std::stringstream s;
                glm::vec3 pp = app.player.getPosition();
                s << "[" << pp.x << ", " << pp.y << ", " << pp.z << "]";
                font.draw(s.str(), glm::vec2(10.f, 30.f));
            }

            /*font.draw("ABCDEFGHIJKLMNOPQRSTUVWXYZ", glm::vec2(20.f, 20.f));
            font.draw("abcdefghijklmnopqrstuvwxyz", glm::vec2(20.f, 50.f));
            font.draw("1234567890!@#$%^&*()-=_+[]{};':\",./<>?", glm::vec2(20.f, 80.f));*/

            // CONSOLE
            if (consoleOn) {
                auto const& cbuf = app.console.getBuffer();
                glm::vec2 position (10.f, 760.f);

                font.draw("> " + app.console.getInputBuffer(), position);

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


