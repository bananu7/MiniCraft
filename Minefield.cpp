#include "Minefield.h"
#include <random>

Minefield::TBlockType Minefield::get(unsigned x, unsigned y, unsigned z) {
	return data[x * size * size + y * size + z];
}
void Minefield::set(unsigned x, unsigned y, unsigned z, Minefield::TBlockType value) {
	data[x * size * size + y * size + z] = value;
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

			const unsigned max_height = 16;
			val = static_cast<unsigned int>(val_norm * max_height);

			for (unsigned y = 0; y < val; ++y)
				set(x, y, z, y);
		}
}