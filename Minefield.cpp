#include "Minefield.h"
#include "simplex.h"
#include <random>
#include <cmath>

using std::make_pair;

Minefield::BlockType Minefield::get(const int x, const int y, const int z) {
	WorldCoord wc (x, y, z);
	auto it = data.find(_convertToOuterChunkCoord(wc));

	if (it != data.end()) {
		auto ic = _convertToInnerChunkCoord(wc);
		return it->second.access(ic);
	}
	else
		return BlockType();
}
void Minefield::set(const int x, const int y, const int z, unsigned value) {
	WorldCoord wc (x, y, z);
	OuterChunkCoord oc = _convertToOuterChunkCoord(wc);

	auto it = data.find(oc);
	auto ic = _convertToInnerChunkCoord(wc);

	if (it != data.end()) {
		it->second.access(ic).value = value;
	}
	else {		
		auto result = data.insert(make_pair(oc, Chunk(oc)));

		// result is pair<iterator, bool>
		// the bool says whether the insertion took place
		// the result of dereferencing the iterator (result.first)
		// is another pair in this case.

		if (result.second)
			result.first->second.access(ic).value = value;
	}
}

Minefield::Minefield() {
	/*for (auto & i : data) {
		i.value = 0;
		i.orientation = 0;
	}*/
	data.emplace(make_pair(OuterChunkCoord(0,0,0), Chunk(OuterChunkCoord(0,0,0))));	
	data.emplace(make_pair(OuterChunkCoord(1,0,0), Chunk(OuterChunkCoord(1,0,0))));
	data.emplace(make_pair(OuterChunkCoord(0,0,1), Chunk(OuterChunkCoord(0,0,1))));
	data.emplace(make_pair(OuterChunkCoord(-1,0,0), Chunk(OuterChunkCoord(-1,0,0))));
	data.emplace(make_pair(OuterChunkCoord(0,0,-1), Chunk(OuterChunkCoord(0,0,-1))));
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

			/*for (int i = 0; i < h; ++i) {
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
			}*/
			access(InnerChunkCoord(x, 0, z)).value = 1;
		}
	}
}

Minefield::Chunk::Chunk(OuterChunkCoord const& ccoords) {
	_generate(ccoords.x, ccoords.y, ccoords.z);
}
