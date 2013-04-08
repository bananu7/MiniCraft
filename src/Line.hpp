#pragma once

#include <Config.h>

#include <VertexBuffer.h>
#include <VertexAttributeArray.h>
#include <Shader.h>

#include <string>
#include <array>

#define NL "\n"

class Line {
    engine::VertexBuffer vbo;
    engine::VertexAttributeArray vao;
    engine::Program program;

public:
    Line() : vbo(engine::VertexBuffer::DATA_BUFFER, engine::VertexBuffer::STATIC_DRAW)
    {
        vao.Bind();

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

    void set (glm::vec3 const& start, glm::vec3 const& end)
    {
        vao.Bind();
        std::array<glm::vec3, 2> a;
        a[0] = start;
        a[1] = end;
        vbo.LoadData(a.data(), sizeof(glm::vec3) * 2);
        vbo.Bind();
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    }
    void draw (glm::mat4 const& projection, glm::mat4 const& view)
    {
        vao.Bind();
        program.SetUniform("Projection", projection);
        program.SetUniform("View", view);
        program.Bind();
        
        glDrawArrays(GL_LINES, 0, 2);
    }
};