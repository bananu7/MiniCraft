#include "Camera.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define M_PI       3.14159265358979323846

namespace engine {

using glm::vec3;
using glm::mat4;

mat4 Camera::CreateRotation (float x, float y, float z)
{
    mat4 Temp(1.0);
    Temp = glm::rotate(Temp, x, glm::vec3(1.f, 0.f, 0.f));
    Temp = glm::rotate(Temp, y, glm::vec3(0.f, 1.f, 0.f));
    Temp = glm::rotate(Temp, z, glm::vec3(0.f, 0.f, 1.f));
    return Temp;
}

mat4 Camera::CreateRotation(vec3 const& rot)
{
    mat4 Temp(1.0);
    Temp = glm::rotate(Temp, rot.x, glm::vec3(1.f, 0.f, 0.f));
    Temp = glm::rotate(Temp, rot.y, glm::vec3(0.f, 1.f, 0.f));
    Temp = glm::rotate(Temp, rot.z, glm::vec3(0.f, 0.f, 1.f));
    return Temp;
}

mat4 Camera::CreateTranslation(float x, float y, float z)
{
    return glm::translate(mat4(), vec3(x, y, z));
}
mat4 Camera::CreateTranslation(vec3 const& vec)
{
    return glm::translate(mat4(), vec);
}
mat4 Camera::CreateProjection(float fov, float ratio, float nearClip, float farClip)
{
    return glm::perspective(fov, ratio, nearClip, farClip);
}

mat4 Camera::CreateFrustum(float left, float right, float bottom, float top, float znear, float zfar)
{
    return glm::frustum(left, right, bottom, top, znear, zfar);
}

mat4 Camera::CreateOrtho(float left, float top, float right, float bottom, float znear, float zfar)
{
    return glm::ortho(left, right, bottom, top);
}

mat4 Camera::CreateScale (float x, float y, float z)
{
    return glm::scale(mat4(), vec3(x,y,z));
}

mat4 Camera::CreateModelview(vec3 const& eye, vec3 const& center)
{
    return glm::lookAt(eye, center, vec3(0.f, 1.f, 0.f));
}

void Camera::CalculateProjection()
{
    m_ProjectionMatrix = CreateProjection(70, 16.f/10.f, 0.01f, 2000.f);
}
void Camera::CalculateView()
{
    m_ViewMatrix = mat4();
}

Camera::Camera(void)
{
}


Camera::~Camera(void)
{
}

} // namespace engine
