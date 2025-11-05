#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <optional>
#include <vector>
#include <glad/glad.h>
#include <NihilEngine/Mesh.h>

namespace NihilEngine {

    struct Material {
        glm::vec4 color = glm::vec4(1.0f);
        std::optional<GLuint> textureID;
    };

    class Entity {
    public:
        Entity(Mesh&& mesh,
               const glm::vec3& position = glm::vec3(0.0f),
               const glm::vec3& rotation = glm::vec3(0.0f),
               const glm::vec3& scale = glm::vec3(1.0f),
               const Material& material = Material());

        Entity(Entity&&) noexcept = default;
        Entity& operator=(Entity&&) noexcept = default;

        // === Getters ajoutés ===
        const glm::vec3& GetPosition() const { return m_Position; }
        const glm::vec3& GetRotation() const { return m_Rotation; }
        const glm::vec3& GetScale() const { return m_Scale; }
        // =======================

        void SetPosition(const glm::vec3& position);
        void SetRotation(const glm::vec3& rotation);
        void SetScale(const glm::vec3& scale);
        void SetMaterial(const Material& material);
        void SetMesh(Mesh&& newMesh);

        glm::mat4 GetModelMatrix() const;
        const Mesh& GetMesh() const;
        const Material& GetMaterial() const;

        void AddChild(Entity* child);
        std::vector<Entity*>& GetChildren();

    private:
        Mesh m_Mesh;
        glm::vec3 m_Position;
        glm::vec3 m_Rotation;
        glm::vec3 m_Scale;
        Material m_Material;
        std::vector<Entity*> m_Children;
        Entity* m_Parent = nullptr;
    };

}