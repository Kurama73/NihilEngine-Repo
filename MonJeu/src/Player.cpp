// src/Player.cpp
#include <MonJeu/Player.h>
#include <MonJeu/VoxelWorld.h>
#include <MonJeu/Constants.h>
#include <NihilEngine/Input.h>
#include <GLFW/glfw3.h>
#include <algorithm>

namespace MonJeu {

    Player::Player()
        : m_Position(0.0f, 10.0f, 0.0f),
          m_Velocity(0.0f),
          m_Yaw(-90.0f),
          m_Pitch(0.0f)
    {
        // Initialisation de l'audio (simplifié, comme avant)
        m_AudioSource = new NihilEngine::AudioSource();
        m_FootstepBuffer = new NihilEngine::AudioBuffer();
        m_JumpBuffer = new NihilEngine::AudioBuffer();
        m_LandBuffer = new NihilEngine::AudioBuffer();
        // TODO: Charger les fichiers audio
        // m_FootstepBuffer->loadFromFile("assets/sounds/footstep.wav");
        // m_JumpBuffer->loadFromFile("assets/sounds/jump.wav");
        // m_LandBuffer->loadFromFile("assets/sounds/land.wav");
    }

    Player::~Player() {
        delete m_AudioSource;
        delete m_FootstepBuffer;
        delete m_JumpBuffer;
        delete m_LandBuffer;
    }

    void Player::Update(float deltaTime, NihilEngine::Camera& camera, VoxelWorld& world, bool isCurrent) {

        // 1. Gérer la rotation (souris)
        if (isCurrent) {
            double mouseDeltaX, mouseDeltaY;
            NihilEngine::Input::GetMouseDelta(mouseDeltaX, mouseDeltaY);

            m_Yaw += static_cast<float>(mouseDeltaX) * Constants::MOUSE_SENSITIVITY;
            m_Pitch += static_cast<float>(mouseDeltaY) * Constants::MOUSE_SENSITIVITY;
            m_Pitch = glm::clamp(m_Pitch, -Constants::MAX_PITCH, Constants::MAX_PITCH);

            camera.SetRotation(m_Yaw, m_Pitch);
            m_Facing = camera.GetForward();
        } else {
            // Logique pour les autres joueurs (non contrôlés)
        }

        // 2. Gérer le mouvement (clavier)
        glm::vec3 forward = camera.GetForward();
        glm::vec3 right = camera.GetRight();
        forward.y = 0.0f;
        right.y = 0.0f;
        if (glm::length(forward) > 0.0f) forward = glm::normalize(forward);
        if (glm::length(right) > 0.0f) right = glm::normalize(right);

        glm::vec3 moveInput(0.0f);
        if (NihilEngine::Input::IsKeyDown(GLFW_KEY_W)) moveInput += forward;
        if (NihilEngine::Input::IsKeyDown(GLFW_KEY_S)) moveInput -= forward;
        if (NihilEngine::Input::IsKeyDown(GLFW_KEY_A)) moveInput -= right;
        if (NihilEngine::Input::IsKeyDown(GLFW_KEY_D)) moveInput += right;

        if (glm::length(moveInput) > 0.0f) {
            moveInput = glm::normalize(moveInput) * Constants::PLAYER_SPEED;
        }

        // 3. Gérer le vol et le saut
        if (NihilEngine::Input::IsKeyTriggered(GLFW_KEY_F)) {
            m_IsFlying = !m_IsFlying;
            m_Velocity.y = 0.0f; // Annule la gravité en changeant de mode
        }

        if (m_IsFlying) {
            if (NihilEngine::Input::IsKeyDown(GLFW_KEY_SPACE)) moveInput.y += Constants::FLY_SPEED;
            if (NihilEngine::Input::IsKeyDown(GLFW_KEY_LEFT_SHIFT)) moveInput.y -= Constants::FLY_SPEED;
            m_Velocity = glm::vec3(0.0f); // Pas d'inertie en vol
        } else {
            // Appliquer la gravité
            m_Velocity.y -= Constants::GRAVITY * deltaTime;
            if (NihilEngine::Input::IsKeyTriggered(GLFW_KEY_SPACE) && m_IsOnGround) {
                m_Velocity.y = Constants::JUMP_FORCE;
                m_IsOnGround = false;
                PlayJumpSound();
            }
        }

        // 4. Appliquer la physique et les collisions
        HandlePhysicsAndCollision(moveInput, deltaTime, world);

        // 5. Mettre à jour la caméra et l'audio
        if (isCurrent) {
            camera.SetPosition(m_Position + glm::vec3(0.0f, Constants::EYE_HEIGHT, 0.0f));
        }
        UpdateAudioPosition();

        // ... (Logique des sons de pas/atterrissage) ...
    }

    NihilEngine::AABB Player::GetAABB() const {
        // Une boîte de collision centrée sur m_Position
        float width = Constants::COLLISION_RADIUS * 2.0f;
        float height = Constants::PLAYER_HEIGHT;
        return NihilEngine::AABB{
            m_Position - glm::vec3(width / 2.0f, height / 2.0f, width / 2.0f),
            m_Position + glm::vec3(width / 2.0f, height / 2.0f, width / 2.0f)
        };
    }

    void Player::HandlePhysicsAndCollision(const glm::vec3& moveInput, float deltaTime, VoxelWorld& world) {

        glm::vec3 motion = (m_IsFlying ? moveInput : moveInput + m_Velocity) * deltaTime;

        // Si le mouvement est nul, on n'a rien à faire
        if (glm::length(motion) == 0.0f) {
            // Vérifie juste si on est au sol (pour le prochain saut)
            NihilEngine::AABB groundCheck = GetAABB();
            groundCheck.min.y -= 0.01f;
            m_IsOnGround = world.CheckCollision(groundCheck);
            return;
        }

        bool onGroundBeforeMove = m_IsOnGround;
        m_IsOnGround = false; // Présume qu'on n'est pas au sol

        // Résolution de collision par axe (évite les blocages)

        // Axe Y
        NihilEngine::AABB playerBox = GetAABB();
        playerBox.min.y += motion.y;
        playerBox.max.y += motion.y;

        if (world.CheckCollision(playerBox)) {
            // Collision en Y
            if (motion.y < 0) { // Touche le sol
                m_IsOnGround = true;
                if (!onGroundBeforeMove && m_Velocity.y < -5.0f) PlayLandSound();
            }
            m_Velocity.y = 0.0f;
            motion.y = 0.0f;
        }

        // Axe X
        playerBox = GetAABB(); // Réinitialise à la position (Y non appliqué)
        playerBox.min.x += motion.x;
        playerBox.max.x += motion.x;

        if (world.CheckCollision(playerBox)) {
            motion.x = 0.0f; // Collision en X
        }

        // Axe Z
        playerBox = GetAABB(); // Réinitialise
        playerBox.min.z += motion.z;
        playerBox.max.z += motion.z;

        if (world.CheckCollision(playerBox)) {
            motion.z = 0.0f; // Collision en Z
        }

        // Applique le mouvement résolu
        m_Position += motion;

        // Correction pour le vol : réinitialise la vélocité
        if (m_IsFlying) {
             m_Velocity = glm::vec3(0.0f);
        } else {
            // Applique le mouvement horizontal à la vélocité (pour l'inertie)
            m_Velocity.x = moveInput.x;
            m_Velocity.z = moveInput.z;
        }
    }

    void Player::Render(NihilEngine::Renderer& renderer, const NihilEngine::Camera& camera) {
        // Dessine la boîte de collision
        NihilEngine::AABB box = GetAABB();
        renderer.DrawWireCube(box.min, box.max, camera, glm::vec3(0.0f, 0.0f, 1.0f));

        // ... (Logique FOV si vous voulez la garder) ...
    }

    void Player::RenderRaycast(NihilEngine::Renderer& renderer, const NihilEngine::Camera& camera, VoxelWorld& voxelWorld) {
        if (!m_ShowRaycast) return;

        glm::vec3 start = camera.GetPosition();
        glm::vec3 dir = camera.GetForward();

        NihilEngine::RaycastHit hit;
        bool hitSomething = voxelWorld.Raycast(start, dir, Constants::RAYCAST_MAX_DISTANCE, hit);

        glm::vec3 end = hitSomething ? hit.hitPoint : start + dir * Constants::RAYCAST_MAX_DISTANCE;

        renderer.DrawLine3D(start, end, camera, glm::vec3(1.0f, 1.0f, 0.0f), Constants::LINE_WIDTH);

        if (hitSomething) {
            glm::vec3 blockMin = glm::vec3(hit.blockPosition);
            glm::vec3 blockMax = blockMin + glm::vec3(1.0f);
            renderer.DrawWireCube(blockMin, blockMax, camera, glm::vec3(1.0f, 0.5f, 0.0f));
        }
    }

    // --- Méthodes audio (à implémenter) ---
    void Player::PlayFootstepSound() {
        if (m_AudioSource && m_FootstepBuffer && m_FootstepBuffer->isLoaded()) {
            m_AudioSource->setBuffer(m_FootstepBuffer);
            m_AudioSource->setPosition(m_Position);
            m_AudioSource->setVolume(0.3f);
            m_AudioSource->play();
        }
    }

    void Player::PlayJumpSound() {
        if (m_AudioSource && m_JumpBuffer && m_JumpBuffer->isLoaded()) {
            m_AudioSource->setBuffer(m_JumpBuffer);
            m_AudioSource->setPosition(m_Position);
            m_AudioSource->setVolume(0.5f);
            m_AudioSource->play();
        }
    }

    void Player::PlayLandSound() {
        if (m_AudioSource && m_LandBuffer && m_LandBuffer->isLoaded()) {
            m_AudioSource->setBuffer(m_LandBuffer);
            m_AudioSource->setPosition(m_Position);
            m_AudioSource->setVolume(0.4f);
            m_AudioSource->play();
        }
    }

    void Player::UpdateAudioPosition() {
        if (m_AudioSource) {
            m_AudioSource->setPosition(m_Position);
        }
    }

} // namespace MonJeu