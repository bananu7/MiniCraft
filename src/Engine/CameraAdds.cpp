#include "CameraAdds.h"
#include "Misc.h"
#include <glm/gtc/matrix_transform.hpp>

namespace engine {

using glm::vec3;
using glm::mat4;

void CameraSimple::CalculateView()
{
    m_ViewMatrix = CreateModelview(Position, Target);
}

void CameraFly::CalculateView()
{
    mat4 RMatrix = Camera::CreateRotation(LookDir.y, LookDir.x, 0.f);
    mat4 TMatrix = Camera::CreateTranslation(-Position.x, -Position.y, -Position.z);
    m_ViewMatrix = mat4(1.0);
    m_ViewMatrix = glm::rotate(m_ViewMatrix, LookDir.x, vec3(1., 0., 0.));
    m_ViewMatrix = glm::rotate(m_ViewMatrix, LookDir.y, vec3(0., 1., 0.));
    m_ViewMatrix = glm::translate(m_ViewMatrix, -Position);
    //m_ViewMatrix *= RMatrix;
}
void CameraFly::Fly (float len)
{
//    float Pi = 3.14159f;
//    Position.x += cosf(LookDir.x - Pi/2.f) * len;
//    Position.z += sinf(LookDir.x - Pi/2.f) * len;


    mat4 RMatrix = Camera::CreateRotation(LookDir.x, LookDir.y, 0.f);
    vec3 Norm (0.f, 0.f, -1.f);
    vec3 Delta;

    // MAGIC - do not touch
    // it involves taking individual components from rotation matrix
    Delta.x = RMatrix.operator[](0).z;
    Delta.y = RMatrix.operator[](1).z;
    Delta.z = RMatrix.operator[](2).z;

    Delta *= -len;
    Position += Delta;
}

void CameraFly::Strafe(float left)
{
    mat4 RMatrix = Camera::CreateRotation(LookDir.x, LookDir.y + PI * 0.5f, 0.f);
    vec3 Delta(0.f);

    // MAGIC - do not touch
    // it involves taking individual components from rotation matrix
    Delta.x = cosf(LookDir.y / 180.f * PI);
    Delta.z = sinf(LookDir.y / 180.f * PI);

    Delta *= -left;
    Position += Delta;
}

void CCameraTrack::CalculateView()
{
    vec3 Delta;
    Delta.y = 5.0f;
    Delta.x = Radius * cosf(Direction/* + M_PI * .5f*/);
    Delta.z = Radius * sinf(Direction/* + M_PI * .5f*/);
    m_ViewMatrix = Camera::CreateModelview(Delta + Target, Target);
}

} // namespace engine
