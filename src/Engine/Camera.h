#pragma once
#include <glm/glm.hpp>

namespace engine {

class Camera
{
protected:
    glm::mat4 m_ProjectionMatrix, m_ViewMatrix;

public:
    // ModelView
    static glm::mat4 CreateScale (float x, float y, float z);
    static glm::mat4 CreateRotation (float x, float y, float z);
    static inline glm::mat4 CreateRotation (glm::vec3 const& rot);
    static glm::mat4 CreateTranslation(float x, float y, float z);
    static inline glm::mat4 CreateTranslation (glm::vec3 const& vec);

    // Projection
    static glm::mat4 CreateProjection(float fov, float ratio, float nearClip, float farClip);
    static glm::mat4 CreateModelview(glm::vec3 const& eye, glm::vec3 const& target);
    static glm::mat4 CreateOrtho(float left, float top, float right, float bottom, float znear, float zfar);
    static glm::mat4 CreateFrustum(float left, float right, float bottom, float top, float znear, float zfar);

    void SetProjectionMat(glm::mat4 const& mat4) { m_ProjectionMatrix = mat4; }
    inline glm::mat4 const& GetProjectionMat () const { return m_ProjectionMatrix; }
    inline glm::mat4 const& GetViewMat () const { return m_ViewMatrix ;}

    virtual void CalculateView();
    virtual void CalculateProjection();

    Camera();
    ~Camera(void);
};

} // namespace engine