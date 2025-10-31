#include <NihilEngine/Camera.h>

namespace NihilEngine {

    Camera::Camera(float fov, float aspect, float near, float far)
        : m_Position(0.0f, 0.0f, 0.0f), m_Yaw(0.0f), m_Pitch(0.0f),
          m_FOV(fov), m_Aspect(aspect), m_Near(near), m_Far(far) {
        UpdateProjectionMatrix();
    }

    void Camera::SetPosition(const glm::vec3& position) {
        m_Position = position;
    }

    void Camera::SetRotation(float yaw, float pitch) {
        m_Yaw = yaw;
        m_Pitch = glm::clamp(pitch, -89.0f, 89.0f); // Limiter pour éviter le gimbal lock
    }

    glm::mat4 Camera::GetViewMatrix() const {
        glm::vec3 front;
        front.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
        front.y = sin(glm::radians(m_Pitch));
        front.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
        front = glm::normalize(front);

        glm::vec3 right = glm::normalize(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f)));
        glm::vec3 up = glm::normalize(glm::cross(right, front));

        return glm::lookAt(m_Position, m_Position + front, up);
    }

    glm::mat4 Camera::GetProjectionMatrix() const {
        return m_ProjectionMatrix;
    }

    glm::mat4 Camera::GetViewProjectionMatrix() const {
        return m_ProjectionMatrix * GetViewMatrix();
    }

    glm::vec3 Camera::GetForward() const {
        glm::vec3 front;
        front.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
        front.y = sin(glm::radians(m_Pitch));
        front.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
        return glm::normalize(front);
    }

    void Camera::UpdateProjectionMatrix() {
        m_ProjectionMatrix = glm::perspective(glm::radians(m_FOV), m_Aspect, m_Near, m_Far);
    }
}