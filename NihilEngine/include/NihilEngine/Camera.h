// NihilEngine/include/NihilEngine/Camera.h
#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace NihilEngine {
    enum class ProjectionType {
        Perspective,
        Orthographic
    };

    class Camera {
    public:
        Camera(float fov, float aspect, float near, float far, ProjectionType type = ProjectionType::Perspective);
        ~Camera() = default;

        void SetPosition(const glm::vec3& position);
        void SetRotation(float yaw, float pitch);
        void SetForward(const glm::vec3& forward);
        void SetProjectionType(ProjectionType type);
        void SetOrthoBounds(float left, float right, float bottom, float top);
        void SetTarget(const glm::vec3& target);

        glm::mat4 GetViewMatrix() const;
        glm::mat4 GetProjectionMatrix() const;
        glm::mat4 GetViewProjectionMatrix() const;

        glm::vec3 GetPosition() const { return m_Position; }
        glm::vec3 GetUp() const { return m_Up; }
        glm::vec3 GetRight() const;
        float GetYaw() const { return m_Yaw; }
        float GetPitch() const { return m_Pitch; }
        glm::vec3 GetForward() const;

        void SetAspect(float aspect);
        void UpdateProjectionMatrix();

        bool IsPointInFrustum(const glm::vec3& point) const;
        bool IsSphereInFrustum(const glm::vec3& center, float radius) const;

    private:
        void UpdateViewMatrix();

        glm::vec3 m_Position = glm::vec3(0.0f);
        glm::vec3 m_Up = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::vec3 m_Forward = glm::vec3(0.0f, 0.0f, -1.0f);
        float m_Yaw = 0.0f;
        float m_Pitch = 0.0f;

        float m_FOV = 45.0f;
        float m_Aspect = 1.0f;
        float m_Near = 0.1f;
        float m_Far = 100.0f;

        float m_OrthoLeft = -1.0f, m_OrthoRight = 1.0f;
        float m_OrthoBottom = -1.0f, m_OrthoTop = 1.0f;

        ProjectionType m_ProjectionType = ProjectionType::Perspective;
        glm::mat4 m_ProjectionMatrix;
        glm::mat4 m_ViewMatrix;
    };
}