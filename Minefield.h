#pragma once
#include <array>
#include <map>
#include <boost/range/adaptor/map.hpp>
class Minefield
{
	static const int size = 16;

public:
	template<typename Tag>
	class Coord {
	public:
		int x, y, z;

		bool operator<(Coord<Tag> const& other) const {
			if (x != other.x)
				return x < other.x;
			else if (y != other.y)
				return y < other.y;
			else
				return z < other.z;
		}
		bool operator==(Coord<Tag> const& other) const {
			return (x==other.x && y==other.y && z==other.z);
		}
		Coord() 
			: x(0), y(0), z(0) { }
		Coord(int _x, int _y, int _z)
			: x(_x), y(_y), z(_z) {
		}
	};

	typedef Coord<struct WorldCoordTag> WorldCoord;
	typedef Coord<struct OuterChunkCoordTag> OuterChunkCoord;
	typedef Coord<struct InnerChunkCoordTag> InnerChunkCoord;

	InnerChunkCoord _convertToInnerChunkCoord(WorldCoord const& wc) {
		auto convert = [] (const int p) -> int {
			if (p<0)
				return (p % size + size) % size;
			else
				return p % size;
		};

		return InnerChunkCoord(convert(wc.x), convert(wc.y), convert(wc.z));
	}

	OuterChunkCoord _convertToOuterChunkCoord(WorldCoord const& wc) {
		auto convert = [] (const int p) -> int {
			if (p < 0)
				return p / size - 1;
			else
				return p / size;
		};

		return OuterChunkCoord(convert(wc.x), convert(wc.y), convert(wc.z));
	}

	struct BlockType {
		unsigned value;
		// TEMP - only the facing direction
		unsigned char orientation;

		BlockType(unsigned _value = 0, unsigned char _orientation = 0) 
			: value(_value), orientation(_orientation) { }
	};

	class Chunk {
		std::array<BlockType, size*size*size> data;

		void _generate(int cx, int cy, int cz);
	public:
		BlockType& access (InnerChunkCoord const& c) {
			return data[c.x + c.y * size + c.z * size * size];
		}
		BlockType const& access (InnerChunkCoord const& c) const {
			return data[c.x + c.y * size + c.z * size * size];
		}
		Chunk () { }
		Chunk (OuterChunkCoord const& ccoords);
	};

private:
	static OuterChunkCoord _getChunkCoordinates (int x, int y, int z);
	static int _innerChunkCoordFromOuterChunkCoord (int p);

	std::map<OuterChunkCoord, Chunk> data;

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