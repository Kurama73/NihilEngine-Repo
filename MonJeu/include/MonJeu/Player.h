#pragma once
#include <NihilEngine/Renderer.h>
#include <NihilEngine/Camera.h>
#include <NihilEngine/Physics.h>
#include <glm/glm.hpp>

namespace MonJeu {
    class VoxelWorld;

    class Player {
        public:
            Player();
            ~Player();
            void Update(float deltaTime, NihilEngine::Camera& camera, VoxelWorld& world);
            void Render(NihilEngine::Renderer& renderer, const NihilEngine::Camera& camera);


            // Debug info rendering moved to overlay, but keep raycast vis here
            void RenderRaycast(NihilEngine::Renderer& renderer, const NihilEngine::Camera& camera, VoxelWorld& voxelWorld);

            // Getters
            glm::vec3 GetPosition() const { return m_Position; }
            int GetID() const { return m_ID; }
            bool IsRaycastVisible() const { return m_ShowRaycast; }

            // Setters
            void SetPosition(const glm::vec3& pos) { m_Position = pos; }

            // New: Toggle raycast visualization
            void ToggleRaycastVis() { m_ShowRaycast = !m_ShowRaycast; }

        private:
            glm::vec3 m_Position;
            glm::vec3 m_Velocity;
            float m_Height = 1.8f;
            int m_ID;

            bool m_ShowRaycast = true;
            bool m_IsFlying = false;

    };
}