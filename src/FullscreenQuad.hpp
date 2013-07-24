#pragma once

#include "Engine/Config.h"

#include <VertexBuffer.h>
#include <VertexAttributeArray.h>
#include <Shader.h>

#include <string>
#include <array>

#define NL "\n"

class FullscreenQuad
{
    gldr::VertexBuffer<> vbo;
    gldr::VertexAttributeArray vao;
    ProgramGLM program;

public:
    FullscreenQuad() : vbo(gldr::VertexBuffer<>::Usage::STATIC_DRAW)
    {
        vao.bind();

        // Vertex shader is trivial; it just covers the whole screen
        std::string vert = 
        "#version 400 core"
        NL"// Fullscreen Quad Vert"
        NL "precision highp float;"
        NL "layout(location = 0) in vec2 in_position;"
        NL "layout(location = 1) in vec2 in_texcoord;"
        NL "out vec2 var_texCoord;"
        NL "void main() {"
        NL "    var_texCoord = in_texcoord;"
        NL "    gl_Position = vec4(in_position, 0.0, 1.0);"
        NL "}" NL;

        std::string frag = 
        "#version 400 core"
        NL"// Fullscreen Quad Frag"
        NL "precision highp float;"
        NL "out vec4 out_Color;"
        NL "in vec2 var_texCoord;"
        NL "uniform sampler2D screen;"
        NL "void main () {"
        NL "   vec2 uv = var_texCoord;"
        NL"    const float sampleDist = 1.0;"
        NL"    const float sampleStrength = 2.2;"
        NL "   float samples[10];"
        NL "samples[0] = -0.08;"
        NL "samples[1] = -0.05;"
        NL "samples[2] = -0.03;"
        NL "samples[3] = -0.02;"
        NL "samples[4] = -0.01;"
        NL "samples[5] =  0.01;"
        NL "samples[6] =  0.02;"
        NL "samples[7] =  0.03;"
        NL "samples[8] =  0.05;"
        NL "samples[9] =  0.08;"
        NL "vec2 dir = 0.5 - uv;"
        NL "float dist = sqrt(dir.x*dir.x + dir.y*dir.y);"
        NL "dir = dir/dist;"

        NL "vec4 color = texture2D(screen,uv);"
        NL "vec4 sum = color;"

        NL "for (int i = 0; i < 10; i++)"
        NL "    sum += texture2D( screen, uv + dir * samples[i] * sampleDist );"

        NL "sum *= 1.0/11.0;"
        NL "float t = dist * sampleStrength;"
        NL "t = clamp( t ,0.0,1.0);"
        //NL "out_Color = mix(color, sum, t);"
        NL "out_Color = texture2D(screen,uv);"
        //NL "    out_Color = vec4(texture(screen, var_texCoord).rgb, 1.0);"
        // "    out_Color = vec4(var_texCoord.rg, 0.0, 1.0);"
        NL "}" NL;

        typedef unsigned char uc;

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

        program.setTex("screen", 0);
        
        // vertex specification
        vao.bind();
        //std::array<glm::vec2, 8> a;
        std::vector<glm::vec2> a(8);
        a[0] = glm::vec2(-1., -1.); a[1] = glm::vec2(0., 0.); 
        a[2] = glm::vec2(1., -1.);  a[3] = glm::vec2(1., 0.); 
        a[4] = glm::vec2(-1., 1.);  a[5] = glm::vec2(0., 1.); 
        a[6] = glm::vec2(1., 1.);   a[7] = glm::vec2(1., 1.); 
        vbo.data(a);
        vbo.bind();
        gl::VertexAttribPointer(0, 2, gl::FLOAT, gl::FALSE_, sizeof(glm::vec2)*2, 0);
        gl::VertexAttribPointer(1, 2, gl::FLOAT, gl::FALSE_, sizeof(glm::vec2)*2, (void*)(sizeof(glm::vec2)));
    }

    void draw ()
    {
        vao.bind();
        vbo.bind();
        program.bind();
        gl::EnableVertexAttribArray(0);
        gl::EnableVertexAttribArray(1);
        gl::Disable(gl::DEPTH_TEST);
        gl::DrawArrays(gl::TRIANGLE_STRIP, 0, 4);
        gl::Enable(gl::DEPTH_TEST);
        gl::DisableVertexAttribArray(0);
        gl::DisableVertexAttribArray(1);
    }
};