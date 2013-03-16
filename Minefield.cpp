#include "Minefield.h"
#include "simplex.h"
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

	// 3D version
	/*for (unsigned x = 0; x < size; ++x)
		for (unsigned y = 0; y < size; ++y)
			for (unsigned z = 0; z < size; ++z)
			{
				float xf = float(x)/size;
				float yf = float(y)/size;	
				float zf = float(z)/size;
				double value = simplex_noise(1, xf*3, yf*3, zf*3);
				set(x, y, z, (value > 1.1) ? 6 : 0);
			}
	*/

	// 2D version
	for (unsigned x = 0; x < size; ++x)
		for (unsigned z = 0; z < size; ++z)
		{
			float xf = float(x)/size;
			float zf = float(z)/size;
			double value = simplex_noise(1, xf*3, zf*3);

			//compress the range a bit:
			value *= 0.2;
			value += 0.3;

			int h = value * size;

			for (int i = 0; i < h; ++i) {
				int block = 1;

				if (i < 10)
					block = 19;
				else if (i < 15)
					block = 1;
				else if (i < 20)
					block = 4;
				else
					block = 2;

				set(x, i, z, block);
			}
		}
}