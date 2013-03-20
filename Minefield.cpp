#include "Minefield.h"
#include "simplex.h"
#include <random>
#include <cmath>

using std::make_pair;

Minefield::KeyType Minefield::_getChunkCoordinates (int x, int y, int z) {
	// integer division to find the chunk
	// negative values would result in -0, so we have to substract one

	auto fromWorld = [](int x){ return (x >= 0) ? (x / size) : (-x / size - 1); };

	return Minefield::KeyType(
		fromWorld(x),
		fromWorld(y),
		fromWorld(z)
	);
}

Minefield::BlockType Minefield::get(int x, int y, int z) {
	// modulo to find position in chunk coordinates
	auto inChunk = [](int x){ return (x >= 0) ? (x % size) : size-( (-x) % size); };

	x = inChunk(x);
	y = inChunk(y);
	z = inChunk(z);

	auto it = data.find(_getChunkCoordinates(x, y, z));

	if (it != data.end()) {
		return it->second.access(x,y,z);
	}
	else
		return BlockType();
}
void Minefield::set(int x, int y, int z, unsigned value) {
	// modulo to find position in chunk coordinates
	x = (x >= 0) ? (x % size) : size-(-x % size);
	y = (y >= 0) ? (y % size) : size-(-y % size);
	z = (z >= 0) ? (z % size) : size-(-z % size);

	KeyType ccoords = _getChunkCoordinates(x, y, z);

	auto it = data.find(ccoords);

	if (it != data.end()) {
		it->second.access(x,y,z).value = value;
	}
	else {
		
		auto result = data.insert(make_pair(ccoords, Chunk(ccoords)));

		// result is pair<iterator, bool>
		// the bool says whether the insertion took place
		// the result of dereferencing the iterator (result.first)
		// is another pair in this case.

		if (result.second)
			result.first->second.access(x, y, z).value = value;
	}
}

Minefield::Minefield() {
	/*for (auto & i : data) {
		i.value = 0;
		i.orientation = 0;
	}*/
	data.emplace(make_pair(KeyType(0,0,0), Chunk(0,0,0)));	
	data.emplace(make_pair(KeyType(1,0,0), Chunk(1,0,0)));
	data.emplace(make_pair(KeyType(0,0,1), Chunk(0,0,1)));
	data.emplace(make_pair(KeyType(-1,0,0), Chunk(-1,0,0)));
	data.emplace(make_pair(KeyType(0,0,-1), Chunk(0,0,-1)));
}

void Minefield::Chunk::_generate(int cx, int cy, int cz) {
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
	for (unsigned x = 1; x < size-1; ++x) {
		for (unsigned z = 1; z < size-1; ++z) {
			float xf = float(x)/size;
			float zf = float(z)/size;
			double value = simplex_noise(1, xf*3, zf*3);

			//compress the range a bit:
			value *= 0.3;
			value += 0.3;

			int h = value * size;

			for (int i = 0; i < h; ++i) {
				int block = 1;

				if (i < 0.2*size)
					block = 19;
				else if (i < 0.3*size)
					block = 1;
				else if (i < 0.4*size)
					block = 3;
				else if (i < 0.5*size)
					block = 2;
				else
					block = 67;

				access(x, i, z).value = block;
			}
		}
	}
}

Minefield::Chunk::Chunk(KeyType const& ccoords) {
	_generate(ccoords.x, ccoords.y, ccoords.z);
}

Minefield::Chunk::Chunk (int cx, int cy, int cz) {
	_generate(cx, cy, cz);
}