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
            void Update(float deltaTime, NihilEngine::Camera& camera, VoxelWorld& world, bool isCurrent = true);
            void Render(NihilEngine::Renderer& renderer, const NihilEngine::Camera& camera);


            // Debug info rendering moved to overlay, but keep raycast vis here
            void RenderRaycast(NihilEngine::Renderer& renderer, const NihilEngine::Camera& camera, VoxelWorld& voxelWorld);

            // Getters
            glm::vec3 GetPosition() const { return m_Position; }
            int GetID() const { return m_ID; }
            bool IsRaycastVisible() const { return m_ShowRaycast; }
            glm::vec3 GetFacing() const { return m_Facing; }
            float GetYaw() const { return m_Yaw; }
            float GetPitch() const { return m_Pitch; }

            // Setters
            void SetPosition(const glm::vec3& pos) { m_Position = pos; }
            void SetShowFOV(bool show) { m_ShowFOV = show; }
            void SetFacing(const glm::vec3& facing) { m_Facing = facing; }
            void SetYaw(float yaw) { m_Yaw = yaw; }
            void SetPitch(float pitch) { m_Pitch = pitch; }

            // New: Toggle raycast visualization
            void ToggleRaycastVis() { m_ShowRaycast = !m_ShowRaycast; }

        private:
            glm::vec3 m_Position;
            glm::vec3 m_Velocity;
            float m_Height = 1.8f;
            int m_ID;

            bool m_ShowRaycast = true;
            bool m_IsFlying = false;
            bool m_ShowFOV = true;
            glm::vec3 m_Facing = glm::vec3(0.0f, 0.0f, 1.0f);
            float m_Yaw = -90.0f;   // regarde +Z au début
            float m_Pitch = 0.0f;

    };
}