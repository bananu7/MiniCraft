#pragma once
#include <string>
#include <map>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GL/glew.h>
#include <fstream>
#include <Image.h>
#include "helpers.hpp"
#include <Config.h>
#include <pugixml.hpp>
#include <algorithm>
#include <exception>
#include <memory>
#include <Shader.h>
#include <VertexAttributeArray.h>
#include <VertexBuffer.h>

using std::make_pair;

using pugi::xml_document;
using pugi::xml_node;
using pugi::xml_attribute;

#define NL "\n"

class Font {
    engine::VertexBuffer vbo;
    engine::VertexAttributeArray vao;
    engine::Program program;

    void _renderGlyph (int glyph, glm::vec2 const& position) {
        program.SetUniform("Position", position);
        glDrawArrays(GL_TRIANGLE_STRIP, glyph, 4);
    }

    void _initShaders() {
        std::string vert = 
        "#version 330 core"
        NL "layout(location = 0) in vec2 in_offset;"
        NL "layout(location = 1) in vec2 in_texcoord;"
        NL "uniform mat4 Projection;"
        NL "uniform vec2 Position;"
        NL "out vec2 var_tc;"
        NL "out vec2 var_accumTc;"
        NL "void main() {"
        NL "    var_tc = in_texcoord;"
        NL "    var_accumTc = vec2((Position.x + in_offset.x)/1280.0, (1-(Position.y + in_offset.y))/800.0);"
        NL "    vec2 pos = Position + in_offset;"
        NL "    gl_Position = Projection * vec4(pos, 0.0, 1.0);"
        NL "}" NL;

        std::string frag = 
        "#version 330 core"
        NL "out vec4 out_Color;"
        NL "in vec2 var_accumTc;"
        NL "in vec2 var_tc;"
        NL "uniform sampler2D tex;"
        NL "uniform sampler2D accum;"
        NL "void main () {"
        NL "    out_Color = (1 - vec4(texture(tex, var_tc).rgb, 1.0)) * vec4(texture(accum, var_accumTc).rgb, 1.0);"
        //NL "    out_Color = vec4(0.0, 1.0, 0.0, 1.0);"
        //NL "    out_Color = vec4(var_tc, 0.0, 1.0);"
        NL "}" NL;

        typedef unsigned char uc;

        {
            auto vs = std::make_shared<engine::VertexShader>(vert);
            vs->Compile();
            auto s = vs->Status();
            if (!s.empty())
                BREAKPOINT();
            program.AttachShader(vs);
        }
        {
            auto fs = std::make_shared<engine::FragmentShader>(frag);
            fs->Compile();
            auto s = fs->Status();
            if (!s.empty())
                BREAKPOINT();
            program.AttachShader(fs);
        }
        
        auto status = program.Link();
        program.Bind();
        program.SetTex("tex", 0);
        program.SetTex("accum", 1);

        try {
            if (!status.empty())
                throw std::runtime_error(status);
            if (!program)
                throw std::runtime_error("Program not valid");
        }
        catch (std::exception const& e)
        {
            //MessageBox(0, e.what(), "Program link error", MB_OK | MB_ICONERROR);
            BREAKPOINT();
        }
        
    }

    class FontDescriptor {
        struct CharDescriptor {
            int id;
            int x, y;
            int width, height;

            int xOffset, yOffset;

            int xAdvance;

            int num;

            bool operator< (CharDescriptor const& other) {
                return id < other.id;
            }
        };

        std::map<int, CharDescriptor> data;
        int xSize, ySize;

    public:
        void load () {
            xml_document Doc;
            if (Doc.load_file("../data/font.xml").status != pugi::status_ok)
                throw std::runtime_error("File opening failed");

            xml_node Root = Doc.first_child();
            xml_node chars = Root.child("chars");

            xSize = Root.child("common").attribute("scaleW").as_int();
            ySize = Root.child("common").attribute("scaleH").as_int();

            int num = 0;

            for (xml_node character = chars.first_child(); character; character = character.next_sibling())
            {
                CharDescriptor cd;

                cd.id = character.attribute("id").as_int();
                cd.x = character.attribute("x").as_int();
                cd.y = character.attribute("y").as_int();
                cd.width = character.attribute("width").as_int();
                cd.height = character.attribute("height").as_int();

                cd.xOffset = character.attribute("xoffset").as_int();
                cd.yOffset = character.attribute("yoffset").as_int();

                cd.xAdvance = character.attribute("xadvance").as_int();

                cd.num = num++;

                data.insert(make_pair(cd.id, cd));
            }
        }

        std::map<int, CharDescriptor> const& getData() const { return data; }
        int getXSize() const { return xSize; }
        int getYSize() const { return ySize; }

        FontDescriptor() {
        }

        /*template<typename Range>
        FontDescriptor(Range r) {
            
        }*/
    };

    FontDescriptor fd;
    engine::Image texture;

    void _load () {
        std::basic_ifstream<unsigned char> file ("../data/font.png", std::ios::binary);
        if (!file)
            throw std::runtime_error("File not open");

        // image creates a texture for us automagically
        texture = engine::Image::Load(istreambuf_range<unsigned char>(file));

        /*std::fstream descriptorFile ("../data/font.xml");
        if (!descriptorFile)
            throw std::runtime_error("File not open");*/
            
        fd.load();

        std::vector<glm::vec2> vboData;

        // vbo "position" is character offset [+ character size]
        // texture coords are normalized texture coordinates

        // Texture size from descriptor
        glm::vec2 ts ((float)fd.getXSize(), (float)fd.getYSize());

        for (auto p : fd.getData()) {
            vboData.push_back(glm::vec2(p.second.xOffset, p.second.yOffset));
            vboData.push_back(glm::vec2(p.second.x, p.second.y) / ts);

            vboData.push_back(glm::vec2(p.second.xOffset + p.second.width, p.second.yOffset));
            vboData.push_back(glm::vec2(p.second.x + p.second.width, p.second.y) / ts);

            vboData.push_back(glm::vec2(p.second.xOffset, p.second.yOffset + p.second.height));
            vboData.push_back(glm::vec2(p.second.x, p.second.y + p.second.height) / ts);

            vboData.push_back(glm::vec2(p.second.xOffset + p.second.width, p.second.yOffset + p.second.height));
            vboData.push_back(glm::vec2(p.second.x + p.second.width, p.second.y + p.second.height) / ts);
        }

        vao.Bind();
        vbo.Bind();

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2) * 2, nullptr);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2) * 2, (void*)(sizeof(glm::vec2)));

        vbo.LoadData(vboData.data(), vboData.size() * sizeof(glm::vec2));

        _initShaders();
    }

public:
    void draw (std::string const& text, glm::vec2 position) {
        // fixed width:
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);

        vao.Bind();
        vbo.Bind();

        glm::mat4 Proj = glm::ortho(0.f, 1280.f, 800.f, 0.f);
        program.SetUniform("Projection", Proj);
        program.Bind();
        program.SetTex("tex", 0);
        texture.bind(0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        for (char c : text) {
            auto it = fd.getData().find(c);
            if (it != fd.getData().end()) {
                auto const& glyph = it->second;

                _renderGlyph(glyph.num * 4, position);
                position += glm::vec2(glyph.xAdvance, 0);
            }
        }

        glEnable(GL_CULL_FACE);
    }

    Font () {
        _load();
        // Load the image and create textures from it.

    }

};