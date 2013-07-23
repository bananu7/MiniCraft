#pragma once
#include "Camera.h"
#include <glm/glm.hpp>

namespace engine {

// Camera focusing on specified object
class CameraSimple:
    public Camera
{
public:
    glm::vec3 Target, Position;
    
    void CalculateView();
};

// Flying camera with First-Person perspective
class CameraFly :
    public Camera
{
public:
    glm::vec2 LookDir;
    glm::vec3 Position;

    void CalculateView();
    void Fly (float len);
    void Strafe (float left);
};

class CCameraTrack :
    public Camera
{
public:
    glm::vec3 Target;
    float Direction, Radius;

    void CalculateView();
};

// Camera rotating around given target
class CameraSpherical :
    public Camera
{
public:
};

// Camera which sets height appropriately based on ground height sampler
class CameraWalk
{
// add height sampler
public:
};

} // namespace engine