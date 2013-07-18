#pragma once
#include <cmath>
#include <utility>
#include "Minefield.h"
#include <glm/glm.hpp>

class Player {
public:
    enum class Direction { 
        Forward, Back, StrafeLeft, StrafeRight
    };

private:
    const float speed;

    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec2 direction;
    bool crouch;

    glm::vec3 _calculateDirectionVector (Direction move_direction) {
        float direction_offset = 0.f;
        switch (move_direction) {
            case Direction::Forward: direction_offset = 0.f; break;
            case Direction::Back: direction_offset = 180.f; break;
            case Direction::StrafeLeft: direction_offset = 270.f; break;
            case Direction::StrafeRight: direction_offset = 90.f; break;
        }

        glm::vec2 dirInRad = (direction + glm::vec2(0.f, direction_offset)) / 180.f * 3.1416f;
        dirInRad += (3.1416f * 0.5f);
        return glm::vec3(-std::cos(dirInRad.y), 0.f, -std::sin(dirInRad.y));
    }

    Minefield::WorldCoord _calculateTarget(Direction move_direction) {
        // there are 4 possible options.
        // it's a situation similar to raycasting.
        Minefield::WorldCoord result(std::floor(position.x), std::floor(position.y), std::floor(position.z));

        auto const dir = _calculateDirectionVector(move_direction);

        // first, check the quadrant
        const bool xAdv = dir.x > 0;
        const bool zAdv = dir.z > 0;

        // calculate planes
        float xPlane = (xAdv) ? std::floor(position.x) + 1.f : std::ceil(position.x) - 1.f;
        float zPlane = (zAdv) ? std::floor(position.z) + 1.f : std::ceil(position.y) - 1.f;

        float xLen = (xPlane - position.x) / dir.x;
        float zLen = (zPlane - position.z) / dir.z;

        if (xLen < zLen)
            result += (xAdv) ? Minefield::WorldCoord(1,0,0) : Minefield::WorldCoord(-1,0,0);
        else
            result += (zAdv) ? Minefield::WorldCoord(0,0,1) : Minefield::WorldCoord(0,0,-1);

        return result;
    }

    std::function<bool(Minefield::WorldCoord const&)> isPassableQuery;

public:
    bool flying;

    void move(Direction direction) {
        //using wc = Minefield::WorldCoord;
        typedef Minefield::WorldCoord wc;

        wc target = _calculateTarget(direction);
        bool canProceed = false;

        if (!crouch) {
            wc up = target + wc(0, 1, 0);
            canProceed = (isPassableQuery(up) && isPassableQuery(target));
        } else {
            canProceed = isPassableQuery(target);
        }

        if (canProceed) {
            position += _calculateDirectionVector(direction) * speed;
        }
    }

    void jump() { 
        velocity.y += 10.f;
    }

    void gravity() {
        if (flying)
            return;

        Minefield::WorldCoord under (position.x, position.y-1.f, position.z);

        velocity.y -= 9.81f / 60.f;
        position.y += (velocity.y) / 60.f;

        if (!isPassableQuery(under)) {
            if (position.y < float(under.y + 1)) {
                position.y = float(under.y + 1);
                velocity.y = 0.f;
            }
        }
    }

    void setPosition(glm::vec3 p) { position = p; }
    void setDirection(glm::vec2 d) { direction = d; }
    glm::vec3 const& getPosition() const { return position; }
    glm::vec2 const& getDirection() const { return direction; }
    
    template<typename TQuery>
    Player(TQuery&& q) 
        : velocity(0.f, 0.f, 0.f)
        , isPassableQuery(std::forward<TQuery>(q))
        , speed(5.f / 60.f)
        , position(0.f, 20.f, 0.f)
        , flying(false)
    {
    }
};


