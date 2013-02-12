#include <GL/glew.h>
#include <GL/freeglut.h>
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
 
void keyboard(unsigned char key, int x, int y);
void display(void);
void mouse(int x, int y);
 
GLuint vboId;
GLuint vaoId;
GLuint programId;
GLuint shaderIds[4];
enum EShaderNames {
    VERTEX_SHADER_ID = 0,
    TESSELATION_SHADER_ID,
    GEOMETRY_SHADER_ID,
    FRAGMENT_SHADER_ID,
    SHADER_COUNT
};
 
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
 
CVertexAttributeArray Vao;
CVertexBuffer Vbo(CVertexBuffer::DATA_BUFFER, CVertexBuffer::STATIC_DRAW);
CVertexBuffer TexVbo(CVertexBuffer::DATA_BUFFER, CVertexBuffer::STATIC_DRAW);
CVertexBuffer VboIndex(CVertexBuffer::INDEX_BUFFER, CVertexBuffer::STATIC_DRAW);

CCameraFly Camera;
glm::mat4 Model, Projection, View;
float Time = 0.f;

CImage Image;
CShader Shader;
World w (&Shader);

void init ()
{
	Model = glm::mat4(1.0);
	//Projection = glm::ortho(-8.0, 8.0, 8.0, -8.0, -16.0, 16.0);
	Projection = glm::perspective(70.0, 16.0/10.0, 1.0, 100.0);

	Camera.Position = glm::vec3(0, -2, 20);

    CheckForError();
  
	// create
	Vao.Bind();
	Vbo.Bind();
	TexVbo.Bind();
	VboIndex.Bind();

    CheckForError();
 
	// specify data
    const float Verts[] = { 
        -1.f, -1.f, -1.f,
		-1.f, -1.f, 1.f,
       -1.f, 1.f, -1.f,
		-1.f, 1.f, 1.f,
		1.f, -1.f, -1.f,
		1.f, -1.f, 1.f,
		1.f, 1.f, -1.f,
		1.f, 1.f, 1.f,
    };

	const float TexCoords[] = {
		1.f, 1.f,
		0.f, 1.f,
		1.f, 0.f,
		0.f, 0.f,
		1.f, 1.f,
		0.f, 1.f,
		1.f, 0.f,
		0.f, 0.f,
	};
	
	const unsigned Indices[] = {
		7, 3, 1,
		7, 1, 5,
		6, 7, 5,
		6, 5, 4,
		2, 6, 4,
		2, 4, 0,
		3, 2, 0,
		3, 0, 1,
		6, 2, 3,
		6, 3, 7,
		5, 1, 0,
		5, 0, 4,

	};

	// load data
	Vbo.LoadData(Verts, sizeof(Verts));
	TexVbo.LoadData(TexCoords, sizeof(TexCoords));
	VboIndex.LoadData(Indices, sizeof(Indices));

	// give meaning to data
	Vbo.Bind();
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	TexVbo.Bind();
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
 
    CheckForError();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	glEnable(GL_TEXTURE_2D);

	CSimpleFileLoader Loader("data/terrain.png");
	auto Result = Image.Load(Loader);
	if (Result != "")
		_CrtDbgBreak();

	const unsigned texUnitNum = 0;
	Image.Bind(texUnitNum);
	Shader.SetTex("Texture", texUnitNum);
}



void initShadersEngine()
{
	CSimpleDirectLoader::TDataMap Data;
	std::string Vert = 
            "#version 400 core"
          NL"precision highp float;"
          NL"layout(location = 0) in vec3 position;"
		  NL"layout(location = 1) in vec2 texCoord;"
		  NL"out vec3 out_position;"
		  NL"out vec2 out_texCoord;"
		  NL
		  NL"uniform mat4 Model, Projection, View;"
		  NL"uniform vec2 TexOffset;"
		  NL
          NL"void main () {"
		  NL"    out_position = position;"
		  NL"    out_texCoord = TexOffset + texCoord;"
          NL"    gl_Position = Projection * View * Model * vec4(position, 1.0);"
          NL"}";
	
	std::string Frag = 
            "#version 400 core"
		  NL"in vec3 out_position;"
		  NL"in vec2 out_texCoord;"
		  NL"uniform float bias;"
		  NL"out vec4 out_Color;"
		  NL
		  NL"uniform sampler2D Texture;"
		  NL
          NL"void main () {"
        //NL"    out_Color = vec4(0.0, 1.0, 0.0, 1.0);"
		 // NL"    out_Color = vec4((out_position.xyz + 1)/2, 1.0);"
		  //NL"    out_Color = vec4(out_texCoord, 0.0, 1.0);"
		  NL"    out_Color = vec4(texture(Texture, out_texCoord).rgb, 1.0);"
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
	Shader.SetUniform("Model", Model);

	// FIXME : rekompilacja psuje uniformy!
//	Shader.Compile();
}

#ifdef _WINDOWS
int WINAPI WinMain (HINSTANCE hInstance,
                    HINSTANCE hPrevInstance,
                    LPSTR lpCmdLine,
                    int iCmdShow)
#else
int main(int argc, char** argv)
#endif
{
	#ifdef _WINDOWS
    int argc = 1;
    char* argv[] = {"program.exe"};
	#endif
    glutInit(&argc, argv);
    
    glutInitContextVersion (4,0);
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);
    glutInitContextProfile(GLUT_CORE_PROFILE);
    glutCreateWindow("GLUT Test");
 
    glewExperimental=TRUE;
    GLenum err=glewInit();
	if(err!=GLEW_OK)
    {
  		std::cout<<"glewInit failed, aborting.";
        _CrtDbgBreak();
    }
    // Explicit check for GLEW error
    err = glGetError();
    // If it's something different than INVALID_ENUM, we have a problem
    if (err != GL_INVALID_ENUM)
        _CrtDbgBreak();
 
    init();
	initShadersEngine();
 
    glutKeyboardFunc(&keyboard);
    glutDisplayFunc(&display);
	glutMotionFunc(&mouse);
 
    glutMainLoop();
 
    return EXIT_SUCCESS;
}
 
 
void keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
    case '\x1B':
        exit(EXIT_SUCCESS);
        break;
	case 'd':
		Camera.Strafe(-0.2f);
		break;
	case 'a':
		Camera.Strafe(0.2f);
		break;

	case 'e':
		Camera.LookDir.y += 5;
		break;
	case 'q':
		Camera.LookDir.y -= 5;
		break;

	Camera.Fly(0.2f);

	case 's':
		Camera.Fly(-0.2f);
		break;
	case 'w':
		Camera.Fly(0.2f);
		break;
	case ' ':
		Camera.Position.y += 0.2;
		break;
	case 'c':
		Camera.Position.y -= 0.2;
		break;
    }
}

void mouse(int x, int y)
{
	Camera.LookDir.x = (static_cast<float>(y) / 800 - 0.5f ) * 180;
	Camera.LookDir.y = (static_cast<float>(x) / 800 - 0.5f ) * 180;
}
 
void display()
{
	Time += 0.1f;

    glClearColor(0.5f, .5f, .5f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 
	//Shader.SetUniform("Projection", Projection);
	
    glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
    
	Camera.CalculateView();	
	glm::mat4 View = Camera.GetViewMat();
	Shader.SetUniform("View", View);

	w.draw();

    glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
 
    glFlush();
	glutPostRedisplay ();
	Sleep(1);
}