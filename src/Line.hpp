#pragma once

#include "Engine/Config.h"

#include <VertexBuffer.h>
#include <VertexAttributeArray.h>
#include <Program.hpp>

#include <string>
#include <array>

#define NL "\n"

class Line {
    gldr::VertexBuffer<> vbo;
    gldr::VertexAttributeArray vao;
    ProgramGLM program;

public:
    Line() : vbo(gldr::VertexBuffer<>::Usage::STATIC_DRAW)
    {
        vao.bind();

        std::string vert = 
        "#version 330 core"
        NL"// Line Vert"
        NL "precision mediump float;"
        NL "layout(location = 0) in vec3 in_position;"
        NL "uniform mat4 Projection, View;"
        NL "void main() {"
        NL "    gl_Position = Projection * View * vec4(in_position, 1.0);"
        NL "}" NL;

        std::string frag = 
        "#version 330 core"
        NL"// Line Frag"
        NL "precision mediump float;"
        NL "out vec4 out_Color;"
        NL "void main () {"
        NL "    out_Color = vec4(1.0, 0.0, 0.0, 1.0);"
        NL "}" NL;

        {
            auto vs = gldr::VertexShader(vert);
            program.attachShader(vs);
        }
        {
            auto fs = gldr::FragmentShader(frag);
            program.attachShader(fs);
        }
        
        program.link();
        program.bind();
    }

    void set (glm::vec3 const& start, glm::vec3 const& end)
    {
        vao.bind();
        std::vector<glm::vec3> a(2);
        a[0] = start;
        a[1] = end;
        vbo.data(a);
        vbo.bind();
        gl::EnableVertexAttribArray(0);
        gl::VertexAttribPointer(0, 3, gl::FLOAT, gl::FALSE_, 0, 0);
    }
    void draw (glm::mat4 const& projection, glm::mat4 const& view)
    {
        vao.bind();
        program.setUniform("Projection", projection);
        program.setUniform("View", view);
        program.bind();
        
        gl::DrawArrays(gl::LINES, 0, 2);
    }
};