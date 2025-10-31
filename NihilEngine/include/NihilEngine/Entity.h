#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <NihilEngine/Mesh.h>  // Mesh est non copiable

namespace NihilEngine {

    class Entity {
    public:
        // Constructeur par déplacement (Mesh&&)
        Entity(Mesh&& mesh,
               const glm::vec3& position = glm::vec3(0.0f),
               const glm::vec3& rotation = glm::vec3(0.0f),
               const glm::vec3& scale = glm::vec3(1.0f),
               const glm::vec4& color = glm::vec4(1.0f, 0.5f, 0.2f, 1.0f));

        // Interdire la copie
        Entity(const Entity&) = delete;
        Entity& operator=(const Entity&) = delete;

        // Autoriser le déplacement (optionnel, mais propre)
        Entity(Entity&&) noexcept = default;
        Entity& operator=(Entity&&) noexcept = default;

        // Méthodes
        void SetPosition(const glm::vec3& position);
        void SetRotation(const glm::vec3& rotation); // En degrés
        void SetScale(const glm::vec3& scale);
        void SetColor(const glm::vec4& color);

        glm::mat4 GetModelMatrix() const;
        const Mesh& GetMesh() const;
        glm::vec4 GetColor() const;

    private:
        Mesh m_Mesh;           // Mesh possédé (non copiable)
        glm::vec3 m_Position;
        glm::vec3 m_Rotation;
        glm::vec3 m_Scale;
        glm::vec4 m_Color;
    };

} 