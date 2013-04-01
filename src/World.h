#pragma once
#include "Minefield.h"
#include <VertexAttributeArray.h>
#include <VertexBuffer.h>
#include <glm/glm.hpp>
#include <vector>
#include <memory>

namespace engine { class Program; }

class World
{
    Minefield field;
    // rendering
    std::shared_ptr<engine::Program> shader;

    struct DisplayChunk {
        unsigned visibleWallsCount;
        engine::VertexAttributeArray vao;
        engine::VertexBuffer positionVbo, texcoordVbo, normalVbo;
        Minefield::OuterChunkCoord coord;
        static const int size = Minefield::size;

        void draw();

        // I hate Visual Studio compiler.
        DisplayChunk (DisplayChunk&& other) :
            visibleWallsCount(other.visibleWallsCount),
            vao(std::move(other.vao)),
            positionVbo(std::move(other.positionVbo)),
            texcoordVbo(std::move(other.texcoordVbo)),
            normalVbo(std::move(other.normalVbo)),
            coord(std::move(other.coord))
        { }
        DisplayChunk (Minefield::OuterChunkCoord c);
    };

    std::map<Minefield::OuterChunkCoord, DisplayChunk> displayChunks;

    void _recalcChunk(DisplayChunk & dc);

public:
    void init();
    void recalcInstances();
    void draw();

    struct CubePos { int x, y, z; CubePos(int _x, int _y, int _z) : x(_x), y(_y), z(_z) {} };

    void set(CubePos const& pos, int val) { field.set(pos.x, pos.y, pos.z, val); }

    enum RayCastParams {
        STOP_ON_FIRST = 1, // stop on first nonzero cube hit
        INCLUDE_EMPTY = 2, // include empty(zero) cubes traversed in the results
        INCLUDE_FIRST = 4  // include the starting cube at the beggining of the result
    };
    std::vector<CubePos> raycast(glm::vec3 const& start, glm::vec3 const& normal, float l, int stopOnFirstHit);

    void saveToFile (std::string const& path) const { field.saveToFile(path); }
    void loadFromFile (std::string const& path) { field.loadFromFile(path); recalcInstances(); }

    World(std::shared_ptr<engine::Program> shader);
};
