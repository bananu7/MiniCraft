#pragma once
#include <cmath>
#include <utility>
#include "Minefield.h"
#include <glm/glm.hpp>

class Player {
    const float speed;

    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec2 direction;
    bool crouch;

    glm::vec3 _calculateDirectionVector () {
        glm::vec2 dirInRad = direction / 180.f * 3.1416f;
        dirInRad += (3.1416f * 0.5f);
        return glm::vec3(-cosf(dirInRad.y), 0.f, -sinf(dirInRad.y));
    }

    Minefield::WorldCoord _calculateTarget() {
        // there are 4 possible options.
        // it's a situation similar to raycasting.
        Minefield::WorldCoord result(floorf(position.x), floorf(position.y), floorf(position.z));

        auto const dir = _calculateDirectionVector();

        // first, check the quadrant
        const bool xAdv = dir.x > 0;
        const bool zAdv = dir.z > 0;

        // calculate planes
        float xPlane = (xAdv) ? std::floorf(position.x) + 1.f : std::ceilf(position.x) - 1.f;
        float zPlane = (zAdv) ? std::floorf(position.z) + 1.f : std::ceilf(position.y) - 1.f;

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
    void move() {
        //using wc = Minefield::WorldCoord;
        typedef Minefield::WorldCoord wc;

        wc target = _calculateTarget();
        bool canProceed = false;

        if (!crouch) {
            wc up = target + wc(0, 1, 0);
            canProceed = (isPassableQuery(up) && isPassableQuery(target));
        } else {
            canProceed = isPassableQuery(target);
        }

        if (canProceed) {
            position += _calculateDirectionVector() * speed;
        }
    }

    void jump() { 
        velocity.y += 10.f;
    }

    void gravity() {
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
    Player(TQuery&& q) : isPassableQuery(std::forward<TQuery>(q)),
        velocity(0.f, 0.f, 0.f),
        speed(5.f / 60.f)
    {
    }
};


