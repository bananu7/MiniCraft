#pragma once
#include <array>
#include <map>
#include <boost/range/adaptor/map.hpp>
class Minefield
{
    static const int size = 12;

public:
    struct WorldCoordTag;
    struct OuterChunkCoordTag;
    struct InnerChunkCoordTag;

    template<typename Tag>
    class Coord;

    typedef Coord<WorldCoordTag> WorldCoord;
    typedef Coord<OuterChunkCoordTag> OuterChunkCoord;
    typedef Coord<InnerChunkCoordTag> InnerChunkCoord;

    template<typename Tag>
    class Coord {
    public:
        int x, y, z;
        
        Coord() 
            : x(0), y(0), z(0) { }
        Coord(int _x, int _y, int _z)
            : x(_x), y(_y), z(_z) {
        }
    };

    OuterChunkCoord convertToOuter (WorldCoord const& wc) {
            auto convertOne = [] (const int p) -> int {
                if (p < 0)
                    return (p+1) / size - 1;
                else
                    return p / size;
            };

            return OuterChunkCoord(convertOne(wc.x), convertOne(wc.y), convertOne(wc.z));
    }

    InnerChunkCoord convertToInner (WorldCoord const& wc) {
        auto convertOne = [] (const int p) -> int {
            if (p<0)
                return (p % size + size) % size;
            else
                return p % size;
        };

        return InnerChunkCoord(convertOne(wc.x), convertOne(wc.y), convertOne(wc.z));
    }

    WorldCoord convertToWorld (InnerChunkCoord const& ic, OuterChunkCoord const& oc) {
        auto convertOne = [] (const int i, const int o) -> int {
            return o + i * size;
        };

        return WorldCoord(convertOne(ic.x,oc.x), convertOne(ic.y, oc.y), convertOne(ic.z, oc.z));
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

        friend class Minefield;
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

    void loadFromFile(std::string const& path);
    void saveToFile(std::string const& path) const;

    Minefield();
};

template<typename Tag>
bool operator<(Minefield::Coord<Tag> const& a, Minefield::Coord<Tag> const& b) {
        if (a.x != b.x)
            return a.x < b.x;
        else if (a.y != b.y)
            return a.y < b.y;
        else
            return a.z < b.z;
}

template<typename Tag>
bool operator==(Minefield::Coord<Tag> const& a, Minefield::Coord<Tag> const& b) {
    return (a.x==b.x && a.y==b.y && a.z==b.z);
}
