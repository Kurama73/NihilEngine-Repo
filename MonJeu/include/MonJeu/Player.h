// include/MonJeu/Player.h
#pragma once
#include <NihilEngine/Renderer.h>
#include <NihilEngine/Camera.h>
#include <NihilEngine/Physics.h>
#include <NihilEngine/IControllableEntity.h>
#include <NihilEngine/Audio.h>
#include <glm/glm.hpp>

namespace MonJeu {
    class VoxelWorld;

    class Player : public NihilEngine::IControllableEntity {
    public:
        Player();
        ~Player();

        // --- Implémentation de IControllableEntity ---
        void Update(float deltaTime, NihilEngine::Camera& camera, VoxelWorld& world, bool isCurrent) override;
        void Render(NihilEngine::Renderer& renderer, const NihilEngine::Camera& camera) override;

        glm::vec3 GetPosition() const override { return m_Position; }
        glm::vec3 GetFacing() const override { return m_Facing; }
        float GetYaw() const override { return m_Yaw; }
        float GetPitch() const override { return m_Pitch; }

        void SetYaw(float yaw) override { m_Yaw = yaw; }
        void SetPitch(float pitch) override { m_Pitch = pitch; }
        // --- Fin de l'implémentation ---


        void RenderRaycast(NihilEngine::Renderer& renderer, const NihilEngine::Camera& camera, VoxelWorld& voxelWorld);
        void SetPosition(const glm::vec3& pos) { m_Position = pos; }
        void ToggleRaycastVis() { m_ShowRaycast = !m_ShowRaycast; }
        bool IsRaycastVisible() const { return m_ShowRaycast; }

    private:
        /**
         * @brief Gère la physique du personnage (gravité, collisions)
         * Mouvement par axe pour éviter les blocages.
         */
        void HandlePhysicsAndCollision(const glm::vec3& moveInput, float deltaTime, VoxelWorld& world);

        /**
         * @brief Calcule la boîte de collision (AABB) du joueur à sa position actuelle.
         */
        NihilEngine::AABB GetAABB() const;

        // Audio
        void PlayFootstepSound();
        void PlayJumpSound();
        void PlayLandSound();
        void UpdateAudioPosition();

        // État du joueur
        glm::vec3 m_Position;
        glm::vec3 m_Velocity;
        glm::vec3 m_Facing = glm::vec3(0.0f, 0.0f, 1.0f);
        float m_Yaw = -90.0f;
        float m_Pitch = 0.0f;

        // Drapeaux de débogage/état
        bool m_ShowRaycast = true;
        bool m_IsFlying = false;
        bool m_IsOnGround = false;

        // Audio
        NihilEngine::AudioSource* m_AudioSource = nullptr;
        NihilEngine::AudioBuffer* m_FootstepBuffer = nullptr;
        NihilEngine::AudioBuffer* m_JumpBuffer = nullptr;
        NihilEngine::AudioBuffer* m_LandBuffer = nullptr;
    };
}