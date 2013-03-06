#pragma once
#include "Minefield.h"
#include <VertexAttributeArray.h>
#include <VertexBuffer.h>
#include <glm/glm.hpp>
#include <vector>
#include <memory>

namespace engine { class Program; }

class World
{
	Minefield field;
	// rendering
	std::shared_ptr<engine::Program> shader;
	engine::CVertexAttributeArray vao;
	engine::CVertexBuffer vertexVbo;
	engine::CVertexBuffer instanceTranslationsVbo, instanceTexcoordsVbo;
	engine::CVertexBuffer instanceLightingVbo;

	unsigned visibleCubesCount;

public:
	void init();
	void recalcInstances();
	void draw();

	struct CubePos { int x, y, z; CubePos(int _x, int _y, int _z) : x(_x), y(_y), z(_z) {} };

	void set(CubePos const& pos, int val) { field.set(pos.x, pos.y, pos.z, val); }

	enum RayCastParams {
		STOP_ON_FIRST = 1, // stop on first nonzero cube hit
		INCLUDE_EMPTY = 2, // include empty(zero) cubes traversed in the results
		INCLUDE_FIRST = 4  // include the starting cube at the beggining of the result
	};
	std::vector<CubePos> raycast(glm::vec3 const& start, glm::vec3 const& normal, float l, int stopOnFirstHit);

	World(std::shared_ptr<engine::Program> shader);
};

