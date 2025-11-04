#include <NihilEngine/Camera.h>
#include <NihilEngine/Constants.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>

namespace NihilEngine {

    Camera::Camera(float fov, float aspect, float near, float far, ProjectionType type)
        : m_FOV(fov), m_Aspect(aspect), m_Near(near), m_Far(far),
          m_ProjectionType(type),
          m_Forward(0.0f, 0.0f, Constants::FORWARD_Z_DEFAULT)
    {
        UpdateProjectionMatrix();
        UpdateViewMatrix();
    }

    void Camera::SetPosition(const glm::vec3& position) {
        m_Position = position;
        UpdateViewMatrix();
    }

    void Camera::SetRotation(float yaw, float pitch) {
        m_Yaw = yaw;
        m_Pitch = glm::clamp(pitch, -Constants::MAX_PITCH, Constants::MAX_PITCH);
        UpdateViewMatrix();
    }

    void Camera::SetForward(const glm::vec3& forward) {
        m_Forward = glm::normalize(forward);
        m_Yaw = glm::degrees(std::atan2(m_Forward.z, m_Forward.x));
        m_Pitch = glm::degrees(std::asin(m_Forward.y));
        UpdateViewMatrix();
    }

    void Camera::SetProjectionType(ProjectionType type) {
        m_ProjectionType = type;
        UpdateProjectionMatrix();
    }

    void Camera::SetOrthoBounds(float left, float right, float bottom, float top) {
        m_OrthoLeft = left;
        m_OrthoRight = right;
        m_OrthoBottom = bottom;
        m_OrthoTop = top;
        if (m_ProjectionType == ProjectionType::Orthographic) {
            UpdateProjectionMatrix();
        }
    }

    void Camera::SetTarget(const glm::vec3& target) {
        glm::vec3 direction = glm::normalize(target - m_Position);
        m_Forward = direction;
        m_Yaw = glm::degrees(std::atan2(direction.z, direction.x));
        m_Pitch = glm::degrees(std::asin(direction.y));
        UpdateViewMatrix();
    }

    void Camera::UpdateViewMatrix() {
        glm::vec3 front = GetForward();
        glm::vec3 right = glm::normalize(glm::cross(front, m_Up));
        glm::vec3 up = glm::normalize(glm::cross(right, front));
        m_ViewMatrix = glm::lookAt(m_Position, m_Position + front, up);
    }

    glm::mat4 Camera::GetViewMatrix() const {
        return m_ViewMatrix;
    }

    glm::mat4 Camera::GetProjectionMatrix() const {
        return m_ProjectionMatrix;
    }

    glm::mat4 Camera::GetViewProjectionMatrix() const {
        return m_ProjectionMatrix * m_ViewMatrix;
    }

    glm::vec3 Camera::GetForward() const {
        glm::vec3 front;
        front.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
        front.y = sin(glm::radians(m_Pitch));
        front.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
        return glm::normalize(front);
    }

    glm::vec3 Camera::GetRight() const {
        return glm::normalize(glm::cross(GetForward(), m_Up));
    }

    void Camera::SetAspect(float aspect) {
        m_Aspect = aspect;
        UpdateProjectionMatrix();
    }

    void Camera::UpdateProjectionMatrix() {
        if (m_ProjectionType == ProjectionType::Perspective) {
            m_ProjectionMatrix = glm::perspective(glm::radians(m_FOV), m_Aspect, m_Near, m_Far);
        } else {
            m_ProjectionMatrix = glm::ortho(m_OrthoLeft, m_OrthoRight, m_OrthoBottom, m_OrthoTop, m_Near, m_Far);
        }
    }

    bool Camera::IsPointInFrustum(const glm::vec3& point) const {
        return true;
    }

    bool Camera::IsSphereInFrustum(const glm::vec3& center, float radius) const {
        return true;
    }

}