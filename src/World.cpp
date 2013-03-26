#include "World.h"
#include "Shader.h"
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <Config.h>


glm::vec2 offset (unsigned value) {
    return glm::vec2 (((value-1) % 16) / 16.0, ((value-1) / 16) / 16.0);
}

glm::vec2 calculateTexCoords (Minefield::BlockType block, unsigned wall)
{
    // if block is oriented, we have to set different value to the top.
    if (block.value == 3)
        if (wall == 4) // top
            return offset(1);
        else if (wall == 5)
            return offset(3);
        else 
            return offset(4);
    else
        return offset(block.value);
}

void World::init()
{
    // create
    vao.Bind();
    //vertex-data position
    positionVbo.Bind();
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);

    //vertex-data texcoords
    texcoordVbo.Bind();
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);

    normalVbo.Bind();
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);

    GLenum e = glGetError();
    if (e != GL_NO_ERROR)
        BREAKPOINT();
}

void World::recalcInstances()
{
    // specify data
    
    static const std::array<glm::vec3, 8> Verts = {
        glm::vec3(0.f, 0.f, 0.f),
        glm::vec3(0.f, 0.f, 1.f),
        glm::vec3(0.f, 1.f, 0.f),
        glm::vec3(0.f, 1.f, 1.f),
        glm::vec3(1.f, 0.f, 0.f),
        glm::vec3(1.f, 0.f, 1.f),
        glm::vec3(1.f, 1.f, 0.f),
        glm::vec3(1.f, 1.f, 1.f),
    };

    static const std::array<glm::vec2, 6> TexCoords = {
        glm::vec2(1.f, 0.f),
        glm::vec2(0.f, 0.f),
        glm::vec2(0.f, 1.f),
        glm::vec2(1.f, 0.f),
        glm::vec2(0.f, 1.f),
        glm::vec2(1.f, 1.f),
    };

    static const std::array<glm::vec3, 6> NormalTable = {
        glm::vec3(0.f,0.f,1.f),
        glm::vec3(1.f,0.f,0.f),
        glm::vec3(0.f,0.f,-1.f),
        glm::vec3(-1.f,0.f,0.f),
        glm::vec3(0.f,1.f,0.f),
        glm::vec3(0.f,-1.f,0.f),
    };
    /*

       2------6
      /      /|
     /      / |
    3--0---7  4
    |      | /
    |      |/
    1------5

    */

    static int Walls[] = {
        7,3,1,    7,1,5, // front
        6,7,5,    6,5,4, // right
        2,6,4,    2,4,0, // back
        3,2,0,    3,0,1, // left
        6,2,3,    6,3,7, // top
        5,1,0,    5,0,4  // bottom
    };

    visibleCubesCount = 0;
    std::vector<glm::vec3> Positions;
    std::vector<glm::vec2> Texcoords;
    std::vector<glm::vec3> Normals;

    const int chunkSize = field.getSize();

    for (auto const& chunk : field.getChunks()) {
        glm::vec3 chunkOffset (chunk.first.x * chunkSize, chunk.first.y * chunkSize, chunk.first.z * chunkSize);

        for (int x = 0; x < chunkSize; ++x)
            for (int y = 0; y < chunkSize; ++y)
                for (int z = 0; z < chunkSize; ++z)
                {
                    auto block = chunk.second.access(Minefield::InnerChunkCoord(x,y,z));
                    if (block.value)
                    {
                        bool CanSkip = false;
                        unsigned numNeighs = 0;

                        // Add frustum culling here?

                        // Generate output vertices into VBOs
                        if (!CanSkip)
                        {
                            static const int xOff [] = { 0, 1, 0, -1, 0, 0 };
                            static const int yOff [] = { 0, 0, 0, 0, 1, -1 };
                            static const int zOff [] = { 1, 0, -1, 0, 0, 0 };

                            // 6 walls
                            for (unsigned wall = 0; wall < 6; ++wall) {
                                //check if the wall is obscured

                                int neighVal = field.get(chunk.first.x * chunkSize + x + xOff[wall],
                                                         chunk.first.y * chunkSize + y + yOff[wall],
                                                         chunk.first.z * chunkSize + z + zOff[wall]).value;
                                if (neighVal == 0) {
                                    // Each wall has two triangles
                                    for (int t = 0; t < 6; ++t) {

                                        unsigned vert_num = Walls[t + wall * 6];
                                        Positions.push_back(Verts[vert_num] + glm::vec3(x,y,z) + chunkOffset);

                                        Texcoords.push_back(TexCoords[t]/16.f + calculateTexCoords(block, wall));
                                        Normals.push_back(NormalTable[wall]);
                                    }
                                }
                            }
                    
                            ++visibleCubesCount;
                        }
                    }
                }
    }
    positionVbo.LoadData(Positions.data(), Positions.size() * sizeof(glm::vec3));
    texcoordVbo.LoadData(Texcoords.data(), Texcoords.size() * sizeof(glm::vec2));
    normalVbo.LoadData(Normals.data(), Normals.size() * sizeof(glm::vec3));
}

void World::draw()
{
    GLenum e = glGetError();
    if (e != GL_NO_ERROR)
        BREAKPOINT();

    vao.Bind();
    shader->Bind();

    //glDisable(GL_CULL_FACE);
    glDrawArrays(GL_TRIANGLES, 0, visibleCubesCount * 36);
    //glDrawArraysInstancedBaseInstance(GL_TRIANGLES, 0, 36, visibleCubesCount, 0);
    //glDrawArraysInstanced(GL_TRIANGLES, 0, 36, visibleCubesCount);
}

#include <fstream>
#include <boost/test/utils/nullstream.hpp>

std::vector<World::CubePos> World::raycast(glm::vec3 const& pos, glm::vec3 const& normal, float len, int raycastParams)
{
    double x = pos.x, y = pos.y, z = pos.z;
    const double nx = normal.x, ny = normal.y, nz = normal.z;

    std::ofstream Log("raycast.txt");
    //boost::onullstream Log;

    std::vector<World::CubePos> Results;

    Log << "------BEGIN RAYCAST--------\n";
    Log << "Initial data:\n"
    << "pos(xyz) : [" << x << ", " << y << ", " << z << "]\n"
    << "normal(xyz) : [" << nx << ", " << ny << ", " << nz << "]\n"
    << "maximum length : " << len << "\n";

    const char X_BIT = 1;
    const char Y_BIT = 2;
    const char Z_BIT = 4;

    char Octant = 0;
    Octant |= (nx > 0.f) ? X_BIT : 0;
    Octant |= (ny > 0.f) ? Y_BIT : 0;
    Octant |= (nz > 0.f) ? Z_BIT : 0;

    int x_advance = (Octant & X_BIT) ? 1 : -1;
    int y_advance = (Octant & Y_BIT) ? 1 : -1;
    int z_advance = (Octant & Z_BIT) ? 1 : -1;

    auto hit_x_plane = [&](double plane){ return (plane - x) / nx; };
    auto hit_y_plane = [&](double plane){ return (plane - y) / ny; };
    auto hit_z_plane = [&](double plane){ return (plane - z) / nz; };

    double t_c = 0.f;
    int current_x, current_y, current_z;
    double t_x, t_y, t_z; // lengths of the ray intersecting next planes
    // determine next possible planes
    glm::vec3 NextPlanes;

    /*

    x/y/z:
     -1     0     1     2
    --|-----|-----|-----|---
        -1     0     1     2
    current_x/y/z:

    */

    current_x = (x > 0.f) ? static_cast<int>(x) : static_cast<int>(x - 1.f);
    current_y = (y > 0.f) ? static_cast<int>(y) : static_cast<int>(y - 1.f);
    current_z = (z > 0.f) ? static_cast<int>(z) : static_cast<int>(z - 1.f);

    if (raycastParams & INCLUDE_FIRST)
        Results.push_back(CubePos(current_x, current_y, current_z));

    Log << "Starting cube : " << current_x << " " << current_y << " " << current_z << '\n';

    int TempBlockVal = 1;
    
    while (true)
    {
        Log << "-----Pass " << TempBlockVal << " ---------\n";

        NextPlanes.x = (Octant & X_BIT) ? floor(x) + 1.f : ceil(x) - 1.f;
        NextPlanes.y = (Octant & Y_BIT) ? floor(y) + 1.f : ceil(y) - 1.f;
        NextPlanes.z = (Octant & Z_BIT) ? floor(z) + 1.f : ceil(z) - 1.f;

        Log << "Next planes to hit : " << NextPlanes.x << ", " << NextPlanes.y << ", " << NextPlanes.z << '\n';

        t_x = hit_x_plane(NextPlanes.x);
        t_y = hit_y_plane(NextPlanes.y);
        t_z = hit_z_plane(NextPlanes.z);

        Log << "T values : " << t_x << ", " << t_y << ", " << t_z << '\n';

        double t;

        // calculate entry position
        // find the lowest value of all 3 components and determine the next voxel
        if (t_x < t_y) {
            if (t_x < t_z) {
                current_x += x_advance;
                t = t_x;

                // to ensure that no roundings around 0 happen
                x = static_cast<double>(current_x);
                y += t * ny;
                z += t * nz;

                Log << "advancing X by " << x_advance << '\n';
            }
            else {
                current_z += z_advance;
                t = t_z;

                x += t * nx;
                y += t * ny;
                z = static_cast<double>(current_z);

                Log << "advancing Z by " << z_advance << '\n';
            }
        }
        else {
            if (t_y < t_z) {
                current_y += y_advance;
                t = t_y;

                x += t * nx;
                y = static_cast<double>(current_y);
                z += t * nz;

                Log << "advancing Y by " << y_advance << '\n';
            }
            else {
                current_z += z_advance;
                t = t_z;

                x += t * nx;
                y += t * ny;
                z = static_cast<double>(current_z);

                Log << "advancing Z by " << z_advance << '\n';
            }
        }

        t_c += t;
        Log << "Total ray length : " << t_c << '\n';

        // ray is too long
        if (t_c > len)
            break;

        Log << "Current position : " << x << ", " << y << ", " << z << '\n';

        auto currentVal = field.get(current_x, current_y, current_z);
        if ((currentVal.value != 0) || (raycastParams & INCLUDE_EMPTY))
        {
            Results.push_back(World::CubePos(current_x, current_y, current_z));
            if ((raycastParams & STOP_ON_FIRST) && (currentVal.value != 0))
                break;
        }
        Log << "------- end of pass ---------\n";
    }
    Log << "---------------END OF RAYCAST------------\n";

    return Results;
}

World::World(std::shared_ptr<engine::Program> _shader) :
    shader(std::move(_shader)),
    positionVbo(engine::VertexBuffer::DATA_BUFFER, engine::VertexBuffer::STATIC_DRAW),
    texcoordVbo(engine::VertexBuffer::DATA_BUFFER, engine::VertexBuffer::STATIC_DRAW),
    normalVbo(engine::VertexBuffer::DATA_BUFFER, engine::VertexBuffer::STATIC_DRAW)
{

}
