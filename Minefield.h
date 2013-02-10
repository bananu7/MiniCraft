#pragma once
#include <bitset>
class Minecraft 
{
	static const unsigned size = 5;
	std::bitset<size*size*size> bits;

public:
	bool get(unsigned x, unsigned y, unsigned z);
	void set(unsigned x, unsigned y, unsigned z, bool value);

	Minecraft();
};