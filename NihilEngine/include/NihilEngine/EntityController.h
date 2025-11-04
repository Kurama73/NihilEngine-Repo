#pragma once

#include <NihilEngine/IControllableEntity.h>
#include <vector>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

namespace MonJeu {
    class VoxelWorld;
}

namespace NihilEngine {

    class Camera;
    class Renderer;

    class EntityController {
    public:
        EntityController(Camera& camera, int switchKey = GLFW_KEY_TAB);

        void AddControllableEntity(IControllableEntity* entity);
        void RemoveControllableEntity(IControllableEntity* entity);
        void ClearEntities();

        void Update(float deltaTime, MonJeu::VoxelWorld& world);

        void Render(Renderer& renderer, const Camera& camera, bool firstPerson = true);

        IControllableEntity* GetCurrentEntity() const;
        size_t GetEntityCount() const;
        int GetCurrentIndex() const;

        void SetSwitchKey(int key) { m_SwitchKey = key; }
        void SetCameraOffset(const glm::vec3& offset) { m_CameraOffset = offset; }

    private:
        Camera& m_Camera;
        std::vector<IControllableEntity*> m_Entities;
        size_t m_CurrentIndex = 0;
        int m_SwitchKey;
        glm::vec3 m_CameraOffset = glm::vec3(0.0f, 0.9f, 3.0f);

        void SwitchToNextEntity();
    };

}
