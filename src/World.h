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
    struct DisplayChunk {
        unsigned visibleWallsCount;
        bool needsRecalc;
        engine::VertexAttributeArray vao;
        engine::VertexBuffer positionVbo, texcoordVbo, normalVbo;
        Minefield::OuterChunkCoord coord;
        static const int size = Minefield::size;

        // it has to be cached because physics doesn't make a copy
        std::vector<glm::vec3> Positions;   

        void draw();

        // I hate Visual Studio compiler.
        DisplayChunk (DisplayChunk&& other) :
            visibleWallsCount(other.visibleWallsCount),
            vao(std::move(other.vao)),
            positionVbo(std::move(other.positionVbo)),
            texcoordVbo(std::move(other.texcoordVbo)),
            normalVbo(std::move(other.normalVbo)),
            coord(std::move(other.coord)),
            needsRecalc(true)
        { }
        DisplayChunk (Minefield::OuterChunkCoord c);
    };

private:
    Minefield field;
    // rendering
    std::shared_ptr<engine::Program> shader;
    std::map<Minefield::OuterChunkCoord, DisplayChunk> displayChunks;

    void _recalcChunk(DisplayChunk & dc);

public:
    void init();
    void recalcInstances(bool forceFullRecalc = false);
    void draw();

    void set(Minefield::WorldCoord const& pos, int val);
    Minefield::BlockType const& get(Minefield::WorldCoord const& pos) { return field.get(pos); }

    enum RayCastParams {
        STOP_ON_FIRST = 1, // stop on first nonzero cube hit
        INCLUDE_EMPTY = 2, // include empty(zero) cubes traversed in the results
        INCLUDE_FIRST = 4  // include the starting cube at the beggining of the result
    };
    std::vector<Minefield::WorldCoord> raycast(glm::vec3 const& start, glm::vec3 const& normal, float l, int stopOnFirstHit);

    void saveToFile (std::string const& path) const { field.saveToFile(path); }
    void loadFromFile (std::string const& path) { field.loadFromFile(path); recalcInstances(true); }

    World(std::shared_ptr<engine::Program> shader);
};
