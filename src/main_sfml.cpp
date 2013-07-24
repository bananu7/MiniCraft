

#include <SFML/Window.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glload/gl_4_3.hpp>
#include <glload/gl_load.hpp>

#include <string>
#include <iostream>
#include <array>
#include <vector>
#include <list>
#include <istream>
#include <fstream>
#include <sstream>

#include "Engine/CameraAdds.h"
#include "Engine/Image.h"
#include "Engine/Config.h"
#include <FrameBuffer.hpp>
#include <debug.hpp>

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


class App {
public:
    Console console;
    lua::state luaVm;
    std::shared_ptr<ProgramGLM> world_shader, trivial_shader, texturing_shader;
    World world;
    Model fence;
    Player player;

    glm::mat4 Projection;

    engine::CameraFly camera;

    float Time;

    engine::Image image;
    engine::Image test_texture;

    //Line* g_L;
    bool g_Run;
    bool consoleOn;

    App()
        : console(glm::ivec2(80, 4))
        , Time(0.f)
        , g_Run(true)
        , consoleOn(false)
        , luaVm([this](std::string const& error) {
            this->console.write(error);
        })
        , world_shader(std::make_shared<ProgramGLM>())
        , texturing_shader(std::make_shared<ProgramGLM>())
        , trivial_shader(std::make_shared<ProgramGLM>())
        , world(world_shader)
        , fence("../Data/models/fence2.obj", texturing_shader)
        , player([this](Minefield::WorldCoord const& wc) {
            return (world.get(wc).value) ? false : true;
        })
    {
        // init
        {
            Projection = glm::perspective(70.0, 16.0 / 10.0, 0.1, 1000.0);

            camera.Position = glm::vec3(0, 0, 20);
            gl::Enable(gl::DEPTH_TEST);
            //glEnable(gl::CULL_FACE);

            gl::Enable(gl::TEXTURE_2D);
            gl::Enable(gl::FRAMEBUFFER_SRGB);
        }

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

        //initShadersEngine()
        {
            auto initShader = [&](std::shared_ptr<ProgramGLM>& shader, std::string const& vertPath, std::string const& fragPath) {
                std::ifstream vert(vertPath);
                std::ifstream frag(fragPath);
                shader->attachShader(gldr::VertexShader(istreambuf_range(vert)));
                shader->attachShader(gldr::FragmentShader(istreambuf_range(frag)));
                shader->link();
                shader->bindFragDataLocation("out_Color", 0);
                shader->setUniform("Projection", Projection);
            };

            initShader(world_shader, "../Data/shaders/main_cubes.vert", "../Data/shaders/main_cubes.frag");
            initShader(texturing_shader, "../Data/shaders/texturing_and_light.vert", "../Data/shaders/texturing_and_light.frag");
            initShader(trivial_shader, "../Data/shaders/trivial.vert", "../Data/shaders/trivial.frag");

            /*texturing_shader->attachShader(gldr::VertexShader(istreambuf_range(std::ifstream("../Data/shaders/texturing_and_light.vert"))));
            texturing_shader->attachShader(gldr::FragmentShader(istreambuf_range(std::ifstream("../Data/shaders/texturing_and_light.frag"))));

            trivial_shader->attachShader(gldr::VertexShader(istreambuf_range(std::ifstream("../Data/shaders/trivial.vert"))));
            trivial_shader->attachShader(gldr::FragmentShader(istreambuf_range(std::ifstream("../Data/shaders/trivial.frag"))));
            */
        }
        //initResources()
        {
            auto safeLoadImage = [](std::string const& path, bool srgb) -> engine::Image {
                try {
                    std::ifstream file(path, std::ios::binary);
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
            image.bindToUnit(texUnitNum);
            // pixels!
            gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_MIN_FILTER, gl::NEAREST);
            gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_MAG_FILTER, gl::NEAREST);
            world_shader->setTex("Texture", texUnitNum);

            test_texture.bindToUnit(0);
            gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_MIN_FILTER, gl::NEAREST);
            gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_MAG_FILTER, gl::NEAREST);
        }


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
            camera.Position.y -= 0.2f;

        if (keys[sf::Keyboard::R]) // remove
        {
            glm::mat4 RMatrix = engine::Camera::CreateRotation(camera.LookDir.x, camera.LookDir.y, 0.f);
            glm::vec3 NormV (RMatrix[0].z, RMatrix[1].z, RMatrix[2].z);
            NormV *= -1.f;
        
            auto result = world.raycast(camera.Position, NormV, 50.f, World::STOP_ON_FIRST);
            if (!result.empty())
            {
                world.set(result.back(), 0);
                world.recalcInstances();
            }

            //g_L->set(camera.Position, camera.Position + NormV * 30.f);
            keys[sf::Keyboard::R] = false;
        }
        if (keys[sf::Keyboard::T]) // add
        {
            glm::mat4 RMatrix = engine::Camera::CreateRotation(camera.LookDir.x, camera.LookDir.y, 0.f);
            glm::vec3 NormV (RMatrix[0].z, RMatrix[1].z, RMatrix[2].z);
            NormV *= -1.f;
        
            auto result = world.raycast(camera.Position, NormV, 50.f,
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

            //g_L->set(camera.Position, camera.Position + NormV * 20.f);
            keys[sf::Keyboard::T] = false;
        }

        if (keys[sf::Keyboard::Escape])
            g_Run = false;
    }
    void mouse(double dmx, double dmy)
    {
        camera.LookDir.y += dmx * 50.0;
        camera.LookDir.x += dmy * 50.0;
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
    if (!glload::LoadFunctions())
        throw std::runtime_error("Error loading GL functions");

    gl::Enable(gl::DEBUG_OUTPUT);
    debugMessageControl(DebugMessageSource::DontCare, DebugMessageType::DontCare, DebugMessageSeverity::DontCare, true);

    App app;

    /*Line L[3];
    app.g_L = &L[2];
    L[0].set(glm::vec3(0, -0.05f, 0.f), glm::vec3(0, 0.05f, 0.f));
    L[1].set(glm::vec3(-0.05f, 0.f, 0.f), glm::vec3(0.05f, 0.f, 0.f));
    L[2].set(glm::vec3(-0.05f, 0.f, 0.f), glm::vec3(0.05f, 0.f, 0.f));*/

    Font font;
    app.player.setDirection(app.camera.LookDir);

    FullscreenQuad fq;

    namespace td = gldr::texture_desc;

    gldr::Texture2d mainTexture;
    mainTexture.bindToUnit(0);
    mainTexture.setMinFiltering(td::MinFilteringMode::Nearest);
    mainTexture.setMagFiltering(td::MagFilteringMode::Nearest);
    //mainTexture.imageData(ScreenXSize, ScreenYSize, TextureFormat::RGBA, TextureInternalFormat::SRGBA, TextureDataType::UnsignedByte, nullptr);
    mainTexture.imageData(ScreenXSize, ScreenYSize, td::Format::RGBA, td::InternalFormat::RGBA, td::DataType::Float, nullptr);

    gldr::Texture2d depthTexture;
    depthTexture.bindToUnit(0);
    depthTexture.setMinFiltering(td::MinFilteringMode::Nearest);
    depthTexture.setMagFiltering(td::MagFilteringMode::Nearest);
    depthTexture.imageData(ScreenXSize, ScreenYSize, td::Format::Depth, td::InternalFormat::Depth, td::DataType::Float, nullptr);

    namespace fd = gldr::fbo_desc;

    gldr::Framebuffer mainFbo;
    mainFbo.AttachTexture(mainTexture, fd::Attachment::Color_0);
    mainFbo.AttachTexture(depthTexture, fd::Attachment::Depth);
    if (!mainFbo.IsValid())
        throw std::runtime_error("Main FBO is invalid");


    // temporary surrounding generation
    for (int j = -1; j < 1; ++j)
        for (int i = -1; i < 1; ++i) {
            app.world.set(Minefield::WorldCoord(i*24, 1, j*24), 2);
        }

    app.world.recalcInstances();

    // log
    {
        std::ofstream log("gllog.txt");
        auto errors = getDebugLog();
        for (auto const& error : errors) {
            log << error;
        }

        auto dump = app.world_shader->debugDump();

        log << "\n\n\n" << dump;

        app.console.write(dump);
    }

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

            if (app.consoleOn) {
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
                    app.consoleOn = !app.consoleOn;
                    continue;
                }

                if (app.consoleOn) {
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

        if (!app.g_Run)
        {
            window.close();
            break;
        }

        // updates:
        app.player.gravity();
        app.camera.Position = app.player.getPosition();
        // compensate for eye height
        app.camera.Position.y += 1.620f;
        app.player.setDirection(app.camera.LookDir);

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
            gl::BindTexture(gl::TEXTURE_2D, 0);
            mainFbo.bind();

            gl::ClearColor(63.f/255, 215.f/255, 252.f/255, 1.f);
            gl::Clear(gl::COLOR_BUFFER_BIT | gl::DEPTH_BUFFER_BIT);

            app.camera.CalculateView();	
            glm::mat4 View = app.camera.GetViewMat();
            app.world_shader->setUniform("View", View);
            app.world_shader->setUniform("Eye", app.camera.Position);

            // world (cubes)
            app.image.bindToUnit(0);
            gl::Enable(gl::DEPTH_TEST);
            app.world.draw();

            app.trivial_shader->setUniform("View", View);

            app.texturing_shader->setUniform("View", View);
            app.texturing_shader->setUniform("Eye", app.camera.Position);
            // test fence
            app.test_texture.bindToUnit(0);
            app.fence.draw();

            // crosshair
            //L[0].draw(glm::mat4(1.0), glm::mat4(1.0));
            //L[1].draw(glm::mat4(1.0), glm::mat4(1.0));

            // raycast line
            //L[2].draw(app.Projection, app.View);

            // end preprocessing
            gldr::Framebuffer::unbind();
            mainTexture.bindToUnit(0);

            // final pass with postprocessing
            fq.draw();

            gl::Disable(gl::DEPTH_TEST);

            // currently targeted block
            glm::mat4 RMatrix = engine::Camera::CreateRotation(app.camera.LookDir.x, app.camera.LookDir.y, 0.f);
            glm::vec3 NormV (RMatrix[0].z, RMatrix[1].z, RMatrix[2].z);
            NormV *= -1.f;

            mainTexture.bindToUnit(1);
        
            auto result = app.world.raycast(app.camera.Position, NormV, 20.f, World::STOP_ON_FIRST);
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
            if (app.consoleOn) {
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


