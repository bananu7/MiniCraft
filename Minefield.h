#pragma once
#include <array>
#include <map>
#include <boost/range/adaptor/map.hpp>
class Minefield
{
private:
	// default chunk is located at 0,0,0
	struct KeyType {
		int x, y, z;
		bool operator<(KeyType const& other) const {
			if (x != other.x)
				return x < other.x;
			else if (y != other.y)
				return y < other.y;
			else
				return z < other.z;
		}
		bool operator==(KeyType const& other) const {
			return (x==other.x && y==other.y && z==other.z);
		}
		KeyType() 
			: x(0), y(0), z(0) {
		}
		KeyType(int _x, int _y, int _z)
			: x(_x), y(_y), z(_z) {
		}
	};

public:
	struct BlockType {
		unsigned value;
		// TEMP - only the facing direction
		unsigned char orientation;

		BlockType(unsigned _value = 0, unsigned char _orientation = 0) 
			: value(_value), orientation(_orientation) { }
	};

	static const int size = 16;

	class Chunk {
		std::array<BlockType, size*size*size> data;

		void _generate(int cx, int cy, int cz);
	public:
		BlockType& access (int x, int y, int z) {
			if (x < 0 || y < 0 || z < 0 || 
				x >= size || y >= size || z>= size)
				throw std::runtime_error("Out of bounds access");

			return data[x + y * size + z * size * size];
		}
		BlockType const& access (int x, int y, int z) const {
			if (x < 0 || y < 0 || z < 0 || 
				x >= size || y >= size || z>= size)
				throw std::runtime_error("Out of bounds access");

			return data[x + y * size + z * size * size];
		}
		Chunk () { }
		Chunk (int cx, int cy, int cz);		
		Chunk (KeyType const& ccoords);
	};

private:
	static KeyType _getChunkCoordinates (int x, int y, int z);

	std::map<KeyType, Chunk> data;

public:
	unsigned getSize () const { return size; }

	BlockType get(int x, int y, int z);
	void set(int x, int y, int z, unsigned value);

	/*auto getChunks () -> decltype(data | boost::adaptors::map_values) {
		return data | boost::adaptors::map_values; 
	}*/
	auto getChunks () -> decltype(data) {
		return data; 
	}

	Minefield();
};