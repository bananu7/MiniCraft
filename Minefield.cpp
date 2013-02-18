#include "Minefield.h"
#include <random>

Minefield::TBlockType Minefield::get(int x, int y, int z) {
	if (x >= 0 && y >= 0 && z >= 0)
	{
		if (x < size && y < size && z < size)
			return data[x * size * size + y * size + z];
		else
			return 0;
	}
	else
	{
//		_CrtDbgBreak();
		return 0;
	}
}
void Minefield::set(int x, int y, int z, Minefield::TBlockType value) {
	if (x >= 0 && y >= 0 && z >= 0)
	{
		if (x < size && y < size && z < size)
		{
			data[x * size * size + y * size + z] = value;
		}
	}
	//else
	//	_CrtDbgBreak();
}

Minefield::Minefield() {
	for (auto & i : data)
		i = 0;

	std::random_device rd;
	for (unsigned x = 0; x < size; ++x)
		for (unsigned z = 0; z < size; ++z)
		{
			unsigned int val = rd();
			val -= rd.min();
			double val_norm = static_cast<double>(val) / rd.max();

			const unsigned max_height = size;
			val = static_cast<int>(val_norm * max_height);

			for (unsigned y = 0; y < 1; ++y)
				set(x, y, z, 22);
		}
}