#pragma once
#include <array>
class Minefield
{
	static const unsigned size = 70;
	using TBlockType = unsigned;
	std::array<TBlockType, size*size*size> data;

public:
	unsigned getSize () const { return size; }

	TBlockType get(unsigned x, unsigned y, unsigned z);
	void set(unsigned x, unsigned y, unsigned z, TBlockType value);

	Minefield();
};