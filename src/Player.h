#pragma once

#include <glm/glm.hpp>
#include "Minefield.h"

class Player {
    glm::vec3 position;
    bool crouch;

    Minefield::WorldCoord _calculateTarget() {

    }
public:
    void move() {
        using wc = Minefield::WorldCoord;

        wc target = _calculateTarget();

        if (!crouch) {
            if (target + wc(0, 1, 0));
        }
    }
};