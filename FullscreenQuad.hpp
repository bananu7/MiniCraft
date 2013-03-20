#pragma once

#include <Config.h>

#include <VertexBuffer.h>
#include <VertexAttributeArray.h>
#include <Shader.h>

#include <string>
#include <array>

#define NL "\n"

class FullscreenQuad : boost::noncopyable 
{
	engine::VertexBuffer vbo;
	engine::VertexAttributeArray vao;
	engine::Program program;

public:
	FullscreenQuad() : vbo(engine::VertexBuffer::DATA_BUFFER, engine::VertexBuffer::STATIC_DRAW)
	{
		vao.Bind();

		// Vertex shader is trivial; it just covers the whole screen
		std::string vert = 
		"#version 400 core"
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
		NL "precision highp float;"
		NL "out vec4 out_Color;"
		NL "in vec2 var_texCoord;"
		NL "uniform sampler2D screen;"
		NL "void main () {"
		NL "    out_Color = vec4(1.0, 0.0, 0.0, 0.5) * texture(screen, var_texCoord);"
		//NL "    out_Color = vec4(texture(screen, var_texCoord).rgb, 1.0);"
		// "    out_Color = vec4(var_texCoord.rg, 0.0, 1.0);"
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
			std::string s = fs->Status();
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

		program.SetTex("screen", 0);
		
		// vertex specification
		vao.Bind();
		std::array<glm::vec2, 8> a;
		a[0] = glm::vec2(-1., -1.); a[1] = glm::vec2(0., 0.); 
		a[2] = glm::vec2(1., -1.);  a[3] = glm::vec2(1., 0.); 
		a[4] = glm::vec2(-1., 1.);  a[5] = glm::vec2(0., 1.); 
		a[6] = glm::vec2(1., 1.);   a[7] = glm::vec2(1., 1.); 
		vbo.LoadData(a.data(), a.size() * sizeof(glm::vec2));
		vbo.Bind();
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2)*2, 0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2)*2, (void*)(sizeof(glm::vec2)));
	}

	void draw ()
	{
		vao.Bind();
		vbo.Bind();
		program.Bind();
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glDisable(GL_DEPTH_TEST);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glEnable(GL_DEPTH_TEST);
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
	}
};