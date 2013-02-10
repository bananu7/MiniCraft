#include "Minefield.h"
#include <random>

bool Minecraft::get(unsigned x, unsigned y, unsigned z) {
	return bits[x * size * size + y * size + z];
}
void Minecraft::set(unsigned x, unsigned y, unsigned z, bool value) {
	bits[x * size * size + y * size + z] = value;
}

Minecraft::Minecraft() {
	bits.reset();

	std::random_device rd;
	for (unsigned x = 0; x < size; ++x)
		for (unsigned z = 0; z < size; ++z)
		{
			unsigned int val = rd();
			val -= rd.min();
			double val_norm = static_cast<double>(val) / rd.max();

			val = static_cast<unsigned int>(val_norm * size);

			for (unsigned y = 0; y < val; ++y)
				set(x, y, z, true);
		}
}