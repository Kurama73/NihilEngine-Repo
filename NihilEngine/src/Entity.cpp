#include <NihilEngine/Entity.h>

namespace NihilEngine {

    Entity::Entity(Mesh&& mesh,
                   const glm::vec3& position,
                   const glm::vec3& rotation,
                   const glm::vec3& scale,
                   const glm::vec4& color)
        : m_Mesh(std::move(mesh)),
          m_Position(position),
          m_Rotation(rotation),
          m_Scale(scale),
          m_Color(color) {}

    void Entity::SetPosition(const glm::vec3& position) {
        m_Position = position;
    }

    void Entity::SetRotation(const glm::vec3& rotation) {
        m_Rotation = rotation;
    }

    void Entity::SetScale(const glm::vec3& scale) {
        m_Scale = scale;
    }

    void Entity::SetColor(const glm::vec4& color) {
        m_Color = color;
    }

    glm::mat4 Entity::GetModelMatrix() const {
        glm::mat4 model = glm::translate(glm::mat4(1.0f), m_Position);
        model = glm::rotate(model, glm::radians(m_Rotation.x), glm::vec3(1,0,0));
        model = glm::rotate(model, glm::radians(m_Rotation.y), glm::vec3(0,1,0));
        model = glm::rotate(model, glm::radians(m_Rotation.z), glm::vec3(0,0,1));
        model = glm::scale(model, m_Scale);
        return model;
    }

    const Mesh& Entity::GetMesh() const {
        return m_Mesh;
    }

    glm::vec4 Entity::GetColor() const {
        return m_Color;
    }

    void Entity::SetMesh(Mesh&& newMesh) {
        m_Mesh = std::move(newMesh);
    }

} // namespace NihilEngine