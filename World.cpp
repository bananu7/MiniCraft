#include "World.h"
#include "Shader.h"
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

glm::vec2 calculateTilePosition (unsigned number)
{
	return glm::vec2 ((number % 16) / 16.0, (number / 16) / 16.0);
}

void World::init()
{
 	// create
	vao.Bind();
	vertexVbo.Bind();
	instanceTranslationsVbo.Bind();
	instanceTexcoordsVbo.Bind();
 
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
	
	/*

	   2------6
	  /      /|
	 /      / |
	3--0---7  4
	|      | /
	|      |/
	1------5

	*/

	#define vert(n) Verts[(n)*3],Verts[(n)*3+1],Verts[(n)*3+2]
	const float RotationlessCube[] = {
		// front
		vert(7), 1.f, 0.f, vert(3), 0.f, 0.f, vert(1), 0.f, 1.f,
		vert(7), 1.f, 0.f, vert(1), 0.f, 1.f, vert(5), 1.f, 1.f,

		// right
		vert(6), 1.f, 0.f, vert(7), 0.f, 0.f, vert(5), 0.f, 1.f,
		vert(6), 1.f, 0.f, vert(5), 0.f, 1.f, vert(4), 1.f, 1.f,

		// back
		vert(2), 1.f, 0.f, vert(6), 0.f, 0.f, vert(4), 0.f, 1.f,
		vert(2), 1.f, 0.f, vert(4), 0.f, 1.f, vert(0), 1.f, 1.f,

		// left
		vert(3), 1.f, 0.f, vert(2), 0.f, 0.f, vert(0), 0.f, 1.f,
		vert(3), 1.f, 0.f, vert(0), 0.f, 1.f, vert(1), 1.f, 1.f,

		// top
		vert(6), 1.f, 0.f, vert(2), 0.f, 0.f, vert(3), 0.f, 1.f,
		vert(6), 1.f, 0.f, vert(3), 0.f, 1.f, vert(7), 1.f, 1.f,

		// bottom
		vert(5), 1.f, 0.f, vert(1), 0.f, 0.f, vert(0), 0.f, 1.f,
		vert(5), 1.f, 0.f, vert(0), 0.f, 1.f, vert(4), 1.f, 1.f,
	};
	#undef vert

	// load data
	vertexVbo.LoadData(RotationlessCube, sizeof(RotationlessCube));

	// give meaning to data
	vertexVbo.Bind();
	glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (GLvoid*)(3 * sizeof(float)));

	// Texture offset and model matrix attributes used for instancing
	// we don't have to send the whole matrix; position will do
	instanceTranslationsVbo.Bind();	
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribDivisor(2, 1);

	instanceTexcoordsVbo.Bind();
	// texture coordinates
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribDivisor(3, 1);
}

void World::recalcInstances()
{
	visibleCubesCount = 0;
	std::vector<glm::vec3> Translations;
	std::vector<glm::vec2> Texcoords;

	const unsigned size = field.getSize();

	for (unsigned x = 0; x < size; ++x)
		for (unsigned y = 0; y < size; ++y)
			for (unsigned z = 0; z < size; ++z)
			{
				auto block = field.get(x,y,z);
				if (block)
				{
					auto Model = glm::mat4();
					bool CanSkip = false;

					if (x > 0 && x < size-1 &&
					    y > 0 && y < size-1 &&
					    z > 0 && z < size-1)
					{
						if (
							field.get(x-1, y, z) &&
							field.get(x, y-1, z) &&
							field.get(x, y, z-1) &&
							field.get(x+1, y, z) &&
							field.get(x, y+1, z) &&
							field.get(x, y, z+1)
						)
						CanSkip = true;
					}

					// Add frustum culling here?

					if (!CanSkip)
					{
						Translations.push_back(glm::vec3(x*2.f, y*2.f, z*2.f));
						Texcoords.push_back(calculateTilePosition(block - 1));

						++visibleCubesCount;
					}
				}
			}

	instanceTranslationsVbo.LoadData(Translations.data(), sizeof(glm::vec3) * Translations.size());
	instanceTexcoordsVbo.LoadData(Texcoords.data(), sizeof(glm::vec2) * Texcoords.size());
}

void World::draw()
{
	GLenum e = glGetError();
	if (e != GL_NO_ERROR)
		_CrtDbgBreak();

	vao.Bind();

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);

	//	glDrawArrays(GL_TRIANGLES, 0, visibleCubesCount * 36);
	//glDrawArraysInstancedBaseInstance(GL_TRIANGLES, 0, 36, visibleCubesCount, 0);
	glDrawArraysInstanced(GL_TRIANGLES, 0, 36, visibleCubesCount);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(3);
}

World::World(CShader* _shader) :
	shader(_shader),
	vertexVbo(CVertexBuffer::DATA_BUFFER, CVertexBuffer::STATIC_DRAW),
	instanceTexcoordsVbo(CVertexBuffer::DATA_BUFFER, CVertexBuffer::STATIC_DRAW),
	instanceTranslationsVbo(CVertexBuffer::DATA_BUFFER, CVertexBuffer::STATIC_DRAW)
{

}
