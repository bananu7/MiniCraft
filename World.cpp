#include "World.h"
#include "Shader.h"
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

glm::vec2 calculateTilePosition (unsigned number)
{
	return glm::vec2 ((number % 16) / 15.0, (number / 16) / 15.0);
}

void World::draw()
{
	for (unsigned x = 0; x < 5; ++x)
		for (unsigned y = 0; y < 5; ++y)
			for (unsigned z = 0; z < 5; ++z)
			{
				auto block = field.get(x,y,z);
				if (block)
				{
					auto Model = glm::mat4();

					/*Model = glm::translate (Model, glm::vec3(0, 2, -20));
			
					Model = glm::rotate(Model, Time, glm::vec3(0.0, 1.0, 0.0));
					Model = glm::rotate (Model, -Time * 0.5f, glm::vec3(0.0, 0.0, 1.0));
					Model = glm::rotate (Model, -Time * 0.3f, glm::vec3(1.0, 0.0, 0.0));*/

					Model = glm::translate (Model, glm::vec3(x*2, y*2, z*2));
					Model = glm::translate (Model, glm::vec3(-2.5, -2.5, -2.5));

					shader->SetUniform("Model", Model);

					//shader->SetUniform("TexCoord", calculateTilePosition();
			
					glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
				}
			}
}

World::World(CShader* _shader)
	: shader(_shader)
{

}
