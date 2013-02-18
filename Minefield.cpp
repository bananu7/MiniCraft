#include "Minefield.h"
#include <random>
#include <cmath>

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

	std::array<float, size*size> temp;

	for (unsigned x = 0; x < size; ++x)
		for (unsigned z = 0; z < size; ++z)
		{
			unsigned int val = rd();
			val -= rd.min();
			double val_norm = static_cast<double>(val) / rd.max();

			temp[x * size + z] = val_norm * size * 2.f;
		}

	// convolution filter
	for (unsigned pass = 0; pass < 30; ++pass)
	{
		for (unsigned x = 1; x < size-1; ++x)
			for (unsigned z = 1; z < size-1; ++z)
			{
				temp[x*size+z] = 0.5f * temp[x*size+z] +
								 0.125f * (temp[(x+1)*size+z+1] +
										   temp[(x-1)*size+z+1] +
										   temp[(x+1)*size+z-1] +
										   temp[(x-1)*size+z-1]);
			}
	}
	for (unsigned x = 1; x < size-1; ++x)
			for (unsigned z = 1; z < size-1; ++z)
				temp[x*size+z] = std::max(0.f, temp[x*size+z] - 20.f);

	for (unsigned x = 0; x < size; ++x)
		for (unsigned z = 0; z < size; ++z)
		{
			for (unsigned y = 0; y < static_cast<unsigned>(temp[x*size+z]); ++y)
			{
				int val;
				if (y < 25)
					val = 2;
				else if (y < 28)
					val = 19;
				else if (y < 38)
					val = 4;
				else 
					val = 17;

				set(x, y, z, val);
			}
		}
}