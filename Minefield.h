#pragma once
#include <array>
class Minefield
{

public:
	struct BlockType {
		unsigned value;
		// TEMP - only the facing direction
		unsigned char orientation;

		BlockType(unsigned _value = 0, unsigned char _orientation = 0) 
			: value(_value), orientation(_orientation) { }
	};

private:
	static const unsigned size = 30;
	std::array<BlockType, size*size*size> data;

public:
	unsigned getSize () const { return size; }

	BlockType get(int x, int y, int z);
	void set(int x, int y, int z, unsigned value);

	Minefield();
};