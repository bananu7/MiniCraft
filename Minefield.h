#pragma once
#include <array>
class Minefield
{
	static const unsigned size = 5;

	using TBlockType = unsigned;
	std::array<TBlockType, size*size*size> data;

public:
	TBlockType get(unsigned x, unsigned y, unsigned z);
	void set(unsigned x, unsigned y, unsigned z, TBlockType value);

	Minefield();
};