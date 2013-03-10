#include "World.h"
#include "Shader.h"
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <Config.h>

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
		0.f, 0.f, 0.f,
		0.f, 0.f, 1.f,
		0.f, 1.f, 0.f,
		0.f, 1.f, 1.f,
		1.f, 0.f, 0.f,
		1.f, 0.f, 1.f,
		1.f, 1.f, 0.f,
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

	instanceLightingVbo.Bind();
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribDivisor(4, 1);
}

void World::recalcInstances()
{
	visibleCubesCount = 0;
	std::vector<glm::vec3> Translations;
	std::vector<glm::vec2> Texcoords;
	std::vector<float> Lighting;

	const unsigned size = field.getSize();

	for (unsigned x = 0; x < size; ++x)
		for (unsigned y = 0; y < size; ++y)
			for (unsigned z = 0; z < size; ++z)
			{
				auto block = field.get(x,y,z);
				if (block)
				{
					bool CanSkip = false;
					int numNeighs = 0;

					if (x > 0 && x < size-1 &&
					    y > 0 && y < size-1 &&
					    z > 0 && z < size-1)
					{
						numNeighs += field.get(x-1, y, z) ? 1 : 0;
						numNeighs += field.get(x, y-1, z) ? 1 : 0;
						numNeighs += field.get(x, y, z-1) ? 1 : 0;
						numNeighs += field.get(x+1, y, z) ? 1 : 0;
						numNeighs += field.get(x, y+1, z) ? 1 : 0;
						numNeighs += field.get(x, y, z+1) ? 1 : 0;

						if (numNeighs == 6)
							CanSkip = true;
					}

					// Add frustum culling here?

					if (!CanSkip)
					{
						Translations.push_back(glm::vec3(x, y, z));
						Texcoords.push_back(calculateTilePosition(block - 1));
						Lighting.push_back(1.f / numNeighs);

						++visibleCubesCount;
					}
				}
			}

	instanceTranslationsVbo.LoadData(Translations.data(), sizeof(glm::vec3) * Translations.size());
	instanceTexcoordsVbo.LoadData(Texcoords.data(), sizeof(glm::vec2) * Texcoords.size());
	instanceLightingVbo.LoadData(Lighting.data(), sizeof(float) * Lighting.size());
}

void World::draw()
{
	GLenum e = glGetError();
	if (e != GL_NO_ERROR)
		BREAKPOINT();

	vao.Bind();
	shader->Bind();

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4);

	//	glDrawArrays(GL_TRIANGLES, 0, visibleCubesCount * 36);
	//glDrawArraysInstancedBaseInstance(GL_TRIANGLES, 0, 36, visibleCubesCount, 0);
	glDrawArraysInstanced(GL_TRIANGLES, 0, 36, visibleCubesCount);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(3);
	glDisableVertexAttribArray(4);
}

#include <fstream>
#include <boost/test/utils/nullstream.hpp>

std::vector<World::CubePos> World::raycast(glm::vec3 const& pos, glm::vec3 const& normal, float len, int raycastParams)
{
	double x = pos.x, y = pos.y, z = pos.z;
	const double nx = normal.x, ny = normal.y, nz = normal.z;

	//std::ofstream Log("raycast.txt");
	boost::onullstream Log;

	std::vector<World::CubePos> Results;

	Log << "------BEGIN RAYCAST--------\n";
	Log << "Initial data:\n"
	<< "pos(xyz) : [" << x << ", " << y << ", " << z << "]\n"
	<< "normal(xyz) : [" << nx << ", " << ny << ", " << nz << "]\n"
	<< "maximum length : " << len << "\n";

	const char X_BIT = 1;
	const char Y_BIT = 2;
	const char Z_BIT = 4;

	char Octant = 0;
	Octant |= (nx > 0.f) ? X_BIT : 0;
	Octant |= (ny > 0.f) ? Y_BIT : 0;
	Octant |= (nz > 0.f) ? Z_BIT : 0;

	int x_advance = (Octant & X_BIT) ? 1 : -1;
	int y_advance = (Octant & Y_BIT) ? 1 : -1;
	int z_advance = (Octant & Z_BIT) ? 1 : -1;

	auto hit_x_plane = [&](double plane){ return (plane - x) / nx; };
	auto hit_y_plane = [&](double plane){ return (plane - y) / ny; };
	auto hit_z_plane = [&](double plane){ return (plane - z) / nz; };

	double t_c = 0.f;
	int current_x, current_y, current_z;
	double t_x, t_y, t_z; // lengths of the ray intersecting next planes
	// determine next possible planes
	glm::vec3 NextPlanes;

	/*

	x/y/z:
	 -1     0     1     2
	--|-----|-----|-----|---
	    -1     0     1     2
	current_x/y/z:

	*/

	current_x = (x > 0.f) ? static_cast<int>(x) : static_cast<int>(x - 1.f);
	current_y = (y > 0.f) ? static_cast<int>(y) : static_cast<int>(y - 1.f);
	current_z = (z > 0.f) ? static_cast<int>(z) : static_cast<int>(z - 1.f);

	if (raycastParams & INCLUDE_FIRST)
		Results.push_back(CubePos(current_x, current_y, current_z));

	Log << "Starting cube : " << current_x << " " << current_y << " " << current_z << '\n';

	int TempBlockVal = 1;
	
	while (true)
	{
		Log << "-----Pass " << TempBlockVal << " ---------\n";

		NextPlanes.x = (Octant & X_BIT) ? floor(x) + 1.f : ceil(x) - 1.f;
		NextPlanes.y = (Octant & Y_BIT) ? floor(y) + 1.f : ceil(y) - 1.f;
		NextPlanes.z = (Octant & Z_BIT) ? floor(z) + 1.f : ceil(z) - 1.f;

		Log << "Next planes to hit : " << NextPlanes.x << ", " << NextPlanes.y << ", " << NextPlanes.z << '\n';

		t_x = hit_x_plane(NextPlanes.x);
		t_y = hit_y_plane(NextPlanes.y);
		t_z = hit_z_plane(NextPlanes.z);

		Log << "T values : " << t_x << ", " << t_y << ", " << t_z << '\n';

		double t;

		// calculate entry position
		// find the lowest value of all 3 components and determine the next voxel
		if (t_x < t_y) {
			if (t_x < t_z) {
				current_x += x_advance;
				t = t_x;

				// to ensure that no roundings around 0 happen
				x = static_cast<double>(current_x);
				y += t * ny;
				z += t * nz;

				Log << "advancing X by " << x_advance << '\n';
			}
			else {
				current_z += z_advance;
				t = t_z;

				x += t * nx;
				y += t * ny;
				z = static_cast<double>(current_z);

				Log << "advancing Z by " << z_advance << '\n';
			}
		}
		else {
			if (t_y < t_z) {
				current_y += y_advance;
				t = t_y;

				x += t * nx;
				y = static_cast<double>(current_y);
				z += t * nz;

				Log << "advancing Y by " << y_advance << '\n';
			}
			else {
				current_z += z_advance;
				t = t_z;

				x += t * nx;
				y += t * ny;
				z = static_cast<double>(current_z);

				Log << "advancing Z by " << z_advance << '\n';
			}
		}

		t_c += t;
		Log << "Total ray length : " << t_c << '\n';

		// ray is too long
		if (t_c > len)
			break;

		Log << "Current position : " << x << ", " << y << ", " << z << '\n';

		auto currentVal = field.get(current_x, current_y, current_z);
		if ((currentVal != 0) || (raycastParams & INCLUDE_EMPTY))
		{
			Results.push_back(World::CubePos(current_x, current_y, current_z));
			if ((raycastParams & STOP_ON_FIRST) && (currentVal != 0))
				break;
		}
		Log << "------- end of pass ---------\n";
	}
	Log << "---------------END OF RAYCAST------------\n";

	return Results;
}

World::World(std::shared_ptr<engine::Program> _shader) :
	shader(std::move(_shader)),
	vertexVbo(engine::VertexBuffer::DATA_BUFFER, engine::VertexBuffer::STATIC_DRAW),
	instanceTexcoordsVbo(engine::VertexBuffer::DATA_BUFFER, engine::VertexBuffer::STATIC_DRAW),
	instanceTranslationsVbo(engine::VertexBuffer::DATA_BUFFER, engine::VertexBuffer::STATIC_DRAW)
{
	field.set(1,2,2, 2);
}

