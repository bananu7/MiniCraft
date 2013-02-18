#pragma once
#include "Minefield.h"
#include <VertexAttributeArray.h>
#include <VertexBuffer.h>

class CShader;

class World
{
	Minefield field;

	// rendering
	CShader* shader;
	CVertexAttributeArray vao;
	CVertexBuffer vertexVbo;
	CVertexBuffer instanceTranslationsVbo, instanceTexcoordsVbo;

	unsigned visibleCubesCount;

public:
	void init();
	void recalcInstances();
	void draw();

	void raycast(float x, float y, float z, float nx, float ny, float nz, float l);

	World(CShader* shader);
};

