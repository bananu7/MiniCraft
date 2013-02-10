#pragma once
#include "Minefield.h"

class CShader;

class World
{
	Minefield field;
	CShader* shader;

public:
	void draw();

	World(CShader* shader);
};

