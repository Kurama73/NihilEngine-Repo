// MonJeu/src/Player.cpp
#include <MonJeu/Player.h>
#include <MonJeu/VoxelWorld.h>
#include <MonJeu/Constants.h>
#include <NihilEngine/Input.h>
#include <NihilEngine/Renderer.h>
#include <NihilEngine/Physics.h>
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
          m_IsFlying(false),
          m_ShowFOV(true),
          m_Yaw(-90.0f),
          m_Pitch(0.0f)
    {}

    Player::~Player() = default;

    void Player::Update(float deltaTime, NihilEngine::Camera& camera, VoxelWorld& world, bool isCurrent) {
        if (isCurrent) {
            m_Yaw = camera.GetYaw();
            m_Pitch = camera.GetPitch();
        }

        double mouseDeltaX, mouseDeltaY;
        NihilEngine::Input::GetMouseDelta(mouseDeltaX, mouseDeltaY);

        m_Yaw += static_cast<float>(mouseDeltaX) * Constants::MOUSE_SENSITIVITY;
        m_Pitch += static_cast<float>(mouseDeltaY) * Constants::MOUSE_SENSITIVITY;
        m_Pitch = glm::clamp(m_Pitch, -Constants::MAX_PITCH, Constants::MAX_PITCH);

        camera.SetRotation(m_Yaw, m_Pitch);

        if (isCurrent) {
            m_Facing = camera.GetForward();
        }

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
            moveInput = glm::normalize(moveInput) * Constants::PLAYER_SPEED;
        }

        if (NihilEngine::Input::IsKeyTriggered(GLFW_KEY_F)) {
            m_IsFlying = !m_IsFlying;
        }

        if (!m_IsFlying) {
            m_Velocity.y -= Constants::GRAVITY * deltaTime;
            if (NihilEngine::Input::IsKeyDown(GLFW_KEY_SPACE)) {
                glm::vec3 belowFeet = m_Position - glm::vec3(0, Constants::PLAYER_HEIGHT * 0.5f + Constants::COLLISION_OFFSET, 0);

                if (!world.IsPositionValid(belowFeet, Constants::COLLISION_RADIUS)) {
                    m_Velocity.y = Constants::JUMP_FORCE;
                }
            }
        } else {
            m_Velocity.y = 0.0f;
        }

        if (m_IsFlying) {
            if (NihilEngine::Input::IsKeyDown(GLFW_KEY_E)) moveInput.y += Constants::FLY_SPEED;
            if (NihilEngine::Input::IsKeyDown(GLFW_KEY_Q)) moveInput.y -= Constants::FLY_SPEED;
        }

        glm::vec3 motion = moveInput * deltaTime + m_Velocity * deltaTime;

        glm::vec3 newPos = m_Position;

        newPos.y += motion.y;
        if (!world.IsPositionValid(newPos, Constants::PLAYER_HEIGHT)) {
            newPos.y = m_Position.y;
            m_Velocity.y = 0.0f;
        }

        newPos.x += motion.x;
        if (!world.IsPositionValid(newPos, Constants::PLAYER_HEIGHT)) {
            newPos.x = m_Position.x;
        }

        newPos.z += motion.z;
        if (!world.IsPositionValid(newPos, Constants::PLAYER_HEIGHT)) {
            newPos.z = m_Position.z;
        }

        m_Position = newPos;

        glm::vec3 eye = m_Position + glm::vec3(0.0f, Constants::EYE_HEIGHT, 0.0f);
        if (isCurrent) {
            camera.SetPosition(eye);
        }
    }

    void Player::Render(NihilEngine::Renderer& renderer, const NihilEngine::Camera& camera) {
        glm::vec3 min = m_Position + glm::vec3(-0.5f, 0.0f, -0.5f);
        glm::vec3 max = m_Position + glm::vec3( 0.5f, Constants::PLAYER_HEIGHT,  0.5f);
        renderer.DrawWireCube(min, max, camera, glm::vec3(0.0f, 0.0f, 1.0f));

        if (m_ShowFOV) {
            glm::vec3 eye = m_Position + glm::vec3(0.0f, Constants::EYE_HEIGHT, 0.0f);
            glm::vec3 forward = m_Facing;
            float fovRad = glm::radians(Constants::FOV_DEGREES);
            glm::mat4 rotLeft = glm::rotate(glm::mat4(1.0f), fovRad / 2.0f, glm::vec3(0.0f, 1.0f, 0.0f));
            glm::mat4 rotRight = glm::rotate(glm::mat4(1.0f), -fovRad / 2.0f, glm::vec3(0.0f, 1.0f, 0.0f));
            glm::vec3 leftDir = glm::vec3(rotLeft * glm::vec4(forward, 0.0f));
            glm::vec3 rightDir = glm::vec3(rotRight * glm::vec4(forward, 0.0f));
            renderer.DrawLine3D(eye, eye + forward * Constants::FOV_DISTANCE, camera, glm::vec3(1.0f, 1.0f, 0.0f), Constants::LINE_WIDTH);
            renderer.DrawLine3D(eye, eye + leftDir * Constants::FOV_DISTANCE, camera, glm::vec3(1.0f, 1.0f, 0.0f), Constants::LINE_WIDTH);
            renderer.DrawLine3D(eye, eye + rightDir * Constants::FOV_DISTANCE, camera, glm::vec3(1.0f, 1.0f, 0.0f), Constants::LINE_WIDTH);
        }
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

} // namespace MonJeu