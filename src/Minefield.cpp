#include "Minefield.h"
#include "simplex.h"
#include <random>
#include <cmath>
#include <fstream>
#include <cstdint>

/*#include "NBTFile.hpp"
#include "RegionLoader.hpp"*/

using std::make_pair;

Minefield::BlockType const& Minefield::get(const int x, const int y, const int z) {
    return get(WorldCoord(x,y,z));
}
Minefield::BlockType const& Minefield::get(Minefield::WorldCoord const& wc) {
    static BlockType empty;

    auto it = data.find(convertToOuter(wc));

    if (it != data.end()) {
        auto ic = convertToInner(wc);
        return it->second.get(ic);
    }
    else
        return empty;
}

void Minefield::set(const int x, const int y, const int z, unsigned value) {
    WorldCoord wc (x, y, z);
    OuterChunkCoord oc = convertToOuter(wc);

    auto it = data.find(oc);
    InnerChunkCoord ic = convertToInner(wc);

    if (it != data.end()) {
        it->second.set(ic, value);
    }
    else {		
        auto result = data.insert(make_pair(oc, Chunk(oc)));

        // result is pair<iterator, bool>
        // the bool says whether the insertion took place
        // the result of dereferencing the iterator (result.first)
        // is another pair in this case.

        if (result.second)
            result.first->second.set(ic, value);
    }
}

Minefield::Minefield() {
    /*for (auto & i : data) {
        i.value = 0;
        i.orientation = 0;
    }*/
    data.emplace(make_pair(OuterChunkCoord(0,0,0), Chunk(OuterChunkCoord(0,0,0))));	
    /*data.emplace(make_pair(OuterChunkCoord(1,0,0), Chunk(OuterChunkCoord(1,0,0))));
    data.emplace(make_pair(OuterChunkCoord(0,0,1), Chunk(OuterChunkCoord(0,0,1))));
    data.emplace(make_pair(OuterChunkCoord(-1,0,0), Chunk(OuterChunkCoord(-1,0,0))));
    data.emplace(make_pair(OuterChunkCoord(0,0,-1), Chunk(OuterChunkCoord(0,0,-1))));*/
}

void Minefield::Chunk::_generate(int cx, int cy, int cz) {
    // 2D version

    if (cx < 0) cx *= -1;
    if (cz < 0) cz *= -1;

    if (cy == 0) {
        for (unsigned x = 0; x < size; ++x) {
            for (unsigned z = 0; z < size; ++z) {
                float xf = float(cx * size + x) / (size * 6.0);
                float zf = float(cz * size + z) / (size * 6.0);

                double value = simplex_noise(1, xf*3, zf*3);

                //compress the range a bit:
                value *= 0.7;
                value += 0.2;
                if (value > 1.) value = 1.;

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

                    _access(x, i, z).value = block;
                }
                _access(x, 0, z).value = 19;
            }
        }
    }
    else if (cy < 0) {
        for (int x = 0; x < size; ++x) {
            for (int y = 0; y < size; ++y) {
                for (int z = 0; z < size; ++z) {
                        // 3D version
                    float xf = abs(float(cx * size + x) / (size));
                    float yf = abs(float(cy * size + y) / (size));
                    float zf = abs(float(cz * size + z) / (size));

                    double value = simplex_noise(1, xf*3, yf*3, zf*3);
                    unsigned block = (value > 1.1) ? 2 : 0;

                    _access(x, y, z).value = block; // stone
                }
            }
        }
    }
    /*else {
        // Leave the array default-initialized to 0
    }*/

    _generateCache();
}

// Serialization

struct Header {
    uint32_t version;

    // chunk size in blocks
    uint32_t numberOfChunks;
    int32_t chunkSizeX;
    int32_t chunkSizeY;
    int32_t chunkSizeZ;
        
    uint32_t blockDataSize;
};

struct ChunkHeader {
    int32_t chunkPositionX;
    int32_t chunkPositionY;
    int32_t chunkPositionZ;
};

/*void loadFromNBT () {
    try {
        mNBT::RegionLoader* loader;
        mNBT::Tag* temp;
        for (int x = 0; x > -2; x--)
            for (int z = 0; z > -2; z--) {
                loader = new mNBT::RegionLoader("world",x,z);
                for (int x1 = 0; x1 < 32; x1++)
                    for (int z1 = 0; z1 < 32; z1++) {
                        temp = loader->getChunk(x1,z1);
                        if(x1 == 0 || x1 == 31) {
                            delete temp;
                            temp = loader->getEmptyChunkNBT(x1,z1);
                        }
                        loader->putChunk(temp,x1,z1);
                        delete temp;
                    }
                loader->save();
                delete loader;
            }
    } catch (mNBT::NBTErr error) {
        //std::cerr << "Error: " << error.getReason();
        throw;
    }
}*/

void Minefield::loadFromFile(std::string const& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file)
        throw std::runtime_error("problem opening file");

    data.clear();

    uint32_t headerSize;
    file.read((char*)(&headerSize), sizeof(headerSize));

    Header h;
    file.read((char*)(&h), sizeof(h));

    if (h.version != 1)
        throw std::runtime_error("wrong file version");

    for (unsigned i = 0; i < h.numberOfChunks; ++i) {
        ChunkHeader ch;
        file.read((char*)(&ch), sizeof(ch));

        Chunk c;

        file.read((char *)(c.data.data()), c.data.size() * sizeof(BlockType));

        data.emplace(make_pair(OuterChunkCoord(ch.chunkPositionX, ch.chunkPositionY, ch.chunkPositionZ), c));
    }
}
void Minefield::saveToFile(std::string const& path) const {
    std::ofstream file(path, std::ios::binary);
    if (!file)
        throw std::runtime_error("problem opening file");
    
    const uint32_t headerSize = sizeof(Header);
    file.write((const char*)(&headerSize), sizeof(headerSize));

    Header h;
    h.version = 1;
    h.blockDataSize = sizeof(BlockType);
    h.numberOfChunks = data.size();
    h.chunkSizeX = h.chunkSizeY = h.chunkSizeZ = size;

    file.write((const char*)(&h), sizeof(h));

    // chunks

    auto serializeChunk = [&file] (Chunk const& chunk) {
        file.write((const char *)(chunk.data.data()), chunk.data.size() * sizeof(BlockType));
    };

    for (auto const& v : data) {

        ChunkHeader ch;
        ch.chunkPositionX = v.first.x;
        ch.chunkPositionY = v.first.y;
        ch.chunkPositionZ = v.first.z;

        file.write((const char*)(&ch), sizeof(ch));

        serializeChunk(v.second);
    }
}

 void Minefield::Chunk::_generateCache() { 
    for (unsigned x = 1; x < size-1; ++x) {
        for (unsigned y = 1; y < size-1; ++y) {
            for (unsigned z = 1; z < size-1; ++z) {
                // check all 6 walls
                static const int xOff [] = { 0, 1, 0, -1, 0, 0 };
                static const int yOff [] = { 0, 0, 0, 0, 1, -1 };
                static const int zOff [] = { 1, 0, -1, 0, 0, 0 };

                // 6 walls
                for (unsigned wall = 0; wall < 6; ++wall) {
                    //check if the wall is obscured

                    int neighVal = get(InnerChunkCoord(x + xOff[wall],
                                                       y + yOff[wall],
                                                       z + zOff[wall])).value;
                    // true if it has a neighbor on said wall
                    _access(x,y,z).neighbors[wall] = (neighVal != 0);
                }
            }
        }
    }
}

Minefield::Chunk::Chunk(OuterChunkCoord const& ccoords) {
    _generate(ccoords.x, ccoords.y, ccoords.z);
}

