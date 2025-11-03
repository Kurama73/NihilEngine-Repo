#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace NihilEngine {
    class Camera {
    public:
        Camera(float fov, float aspect, float near, float far);
        ~Camera() = default;

        void SetPosition(const glm::vec3& position);
        void SetRotation(float yaw, float pitch);

        glm::mat4 GetViewMatrix() const;
        glm::mat4 GetProjectionMatrix() const;
        glm::mat4 GetViewProjectionMatrix() const;

        glm::vec3 GetPosition() const { return m_Position; }
        float GetYaw() const { return m_Yaw; }
        float GetPitch() const { return m_Pitch; }
        glm::vec3 GetForward() const;

        void SetAspect(float aspect);

    private:
        glm::vec3 m_Position;
        float m_Yaw;   // Rotation horizontale
        float m_Pitch; // Rotation verticale

        float m_FOV;
        float m_Aspect;
        float m_Near;
        float m_Far;

        glm::mat4 m_ProjectionMatrix;
        void UpdateProjectionMatrix();
    };
}