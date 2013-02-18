#pragma once
#include <array>
class Minefield
{
	static const unsigned size = 50;
	typedef unsigned TBlockType;
	std::array<TBlockType, size*size*size> data;

public:
	unsigned getSize () const { return size; }

	TBlockType get(int x, int y, int z);
	void set(int x, int y, int z, TBlockType value);

	Minefield();
};