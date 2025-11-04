// MonJeu/src/Player.cpp
#include <MonJeu/Player.h>
#include <MonJeu/VoxelWorld.h>
#include <NihilEngine/Input.h>
#include <NihilEngine/Renderer.h>
#include <NihilEngine/Physics.h>  // Pour RaycastHit
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <GLFW/glfw3.h>

namespace MonJeu {

    Player::Player()
        : m_Position(0.0f, 10.0f, 0.0f),
          m_Velocity(0.0f),
          m_Height(1.8f),
          m_ID(0),
          m_ShowRaycast(true),
          m_IsFlying(false)
    {}

    Player::~Player() = default;

    void Player::Update(float deltaTime, NihilEngine::Camera& camera, VoxelWorld& world) {
        const float speed = 5.0f;
        const float gravity = 40.0f;
        const float jumpForce = 10.0f;
        const float playerHeight = 1.8f;
        const float mouseSensitivity = 0.1f;

        // === 1. ROTATION CAMÉRA (souris) ===
        static float yaw = -90.0f;   // regarde +Z au début
        static float pitch = 0.0f;

        double mouseDeltaX, mouseDeltaY;
        NihilEngine::Input::GetMouseDelta(mouseDeltaX, mouseDeltaY);

        yaw += static_cast<float>(mouseDeltaX) * mouseSensitivity;
        pitch += static_cast<float>(mouseDeltaY) * mouseSensitivity;
        pitch = glm::clamp(pitch, -89.0f, 89.0f);

        camera.SetRotation(yaw, pitch);  // Utilise ta méthode existante !

        // === 2. POSITION JOUEUR + CAMÉRA ===
        glm::vec3 forward = camera.GetForward();
        glm::vec3 right = camera.GetRight();

        forward.y = 0.0f;
        if (glm::length(forward) > 0.0f) forward = glm::normalize(forward);
        right.y = 0.0f;
        if (glm::length(right) > 0.0f) right = glm::normalize(right);

        glm::vec3 moveInput(0.0f);
        if (NihilEngine::Input::IsKeyDown(GLFW_KEY_W)) moveInput += forward;
        if (NihilEngine::Input::IsKeyDown(GLFW_KEY_S)) moveInput -= forward;
        if (NihilEngine::Input::IsKeyDown(GLFW_KEY_A)) moveInput -= right;
        if (NihilEngine::Input::IsKeyDown(GLFW_KEY_D)) moveInput += right;

        if (glm::length(moveInput) > 0.0f) {
            moveInput = glm::normalize(moveInput) * speed;
        }

        // Toggle flying mode
        if (NihilEngine::Input::IsKeyTriggered(GLFW_KEY_F)) {
            m_IsFlying = !m_IsFlying;
        }

        // === 3. GRAVITÉ & SAUT ===
        if (!m_IsFlying) {
            m_Velocity.y -= gravity * deltaTime;
            if (NihilEngine::Input::IsKeyDown(GLFW_KEY_SPACE)) {
                // CORRECTION :
                // 1. On teste LÉGÈREMENT SOUS les pieds (0.9f + 0.05f)
                // 2. On vérifie si cet endroit est SOLIDE (donc !IsPositionValid)

                // playerHeight * 0.5f = 0.9f. On teste à 0.95f (juste en dessous)
                glm::vec3 belowFeet = m_Position - glm::vec3(0, playerHeight * 0.5f + 0.05f, 0);

                // Si c'est solide (!world.IsPositionValid), ALORS on peut sauter.
                if (!world.IsPositionValid(belowFeet, 0.1f)) {
                    m_Velocity.y = jumpForce;
                }
            }
        } else {
            m_Velocity.y = 0.0f;
        }

        if (m_IsFlying) {
            if (NihilEngine::Input::IsKeyDown(GLFW_KEY_E)) moveInput.y += speed;
            if (NihilEngine::Input::IsKeyDown(GLFW_KEY_Q)) moveInput.y -= speed;
        }

        glm::vec3 motion = moveInput * deltaTime + m_Velocity * deltaTime;

        // === 4. COLLISION (Corrigée avec séparation des axes) ===
        // On teste chaque axe séparément pour "glisser"
        glm::vec3 newPos = m_Position;

        // D'abord l'axe Y (vertical)
        newPos.y += motion.y;
        if (!world.IsPositionValid(newPos, playerHeight)) {
            newPos.y = m_Position.y; // Annule le mouvement Y
            m_Velocity.y = 0.0f;       // Stoppe la vitesse verticale (chute/saut)
        }

        // Puis l'axe X (horizontal)
        newPos.x += motion.x;
        if (!world.IsPositionValid(newPos, playerHeight)) {
            newPos.x = m_Position.x; // Annule le mouvement X
        }

        // Enfin l'axe Z (horizontal)
        newPos.z += motion.z;
        if (!world.IsPositionValid(newPos, playerHeight)) {
            newPos.z = m_Position.z; // Annule le mouvement Z
        }

        // Applique la position finale validée
        m_Position = newPos;

        // === 5. MISE À JOUR CAMÉRA ===
        glm::vec3 eye = m_Position + glm::vec3(0.0f, 0.9f, 0.0f);
        camera.SetPosition(eye);
        // SetRotation déjà fait → pas besoin de SetTarget
    }

    // === Rendu du joueur (cube plein) ===
    void Player::Render(NihilEngine::Renderer& renderer, const NihilEngine::Camera& camera) {
        // On va simuler un cube plein avec DrawWireCube + remplissage ?
        // Non : ton Renderer n'a pas de cube plein → on peut ajouter une fonction ou utiliser Entity

        // Solution temporaire : **cube wireframe vert**
        glm::vec3 min = m_Position + glm::vec3(-0.4f, 0.0f, -0.4f);
        glm::vec3 max = m_Position + glm::vec3( 0.4f, 1.8f,  0.4f);
        renderer.DrawWireCube(min, max, camera, glm::vec3(0.0f, 1.0f, 0.0f));
    }

    // === Raycast visuel ===
    void Player::RenderRaycast(NihilEngine::Renderer& renderer, const NihilEngine::Camera& camera, VoxelWorld& voxelWorld) {
        if (!m_ShowRaycast) return;

        const float maxDist = 10.0f;
        glm::vec3 start = camera.GetPosition();
        glm::vec3 dir = camera.GetForward();

        NihilEngine::RaycastHit hit;
        bool hitSomething = voxelWorld.Raycast(start, dir, maxDist, hit);

        glm::vec3 end = hitSomething ? hit.hitPoint : start + dir * maxDist;

        // Ligne du raycast
        renderer.DrawLine3D(start, end, camera, glm::vec3(1.0f, 1.0f, 0.0f), 2.0f);

        // Highlight du bloc
        if (hitSomething) {
            glm::vec3 blockMin = glm::vec3(hit.blockPosition);
            glm::vec3 blockMax = blockMin + glm::vec3(1.0f);
            renderer.DrawWireCube(blockMin, blockMax, camera, glm::vec3(1.0f, 0.5f, 0.0f));
        }
    }

    // === Helpers supprimés (plus nécessaires, tout est dans Renderer) ===
    // DrawRaycastLine → remplacé par DrawLine3D
    // DrawBlockHighlight → remplacé par DrawWireCube

} // namespace MonJeu