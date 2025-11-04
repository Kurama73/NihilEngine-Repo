#pragma once

#include <glm/glm.hpp>
namespace MonJeu {
    class VoxelWorld;
}

namespace NihilEngine {

    class Camera;
    class Renderer;

    class IControllableEntity {
    public:
        virtual ~IControllableEntity() = default;


        virtual void Update(float deltaTime, Camera& camera, MonJeu::VoxelWorld& world, bool isCurrent) = 0;
        virtual void Render(Renderer& renderer, const Camera& camera) = 0;

        // Getters pour la position et orientation
        virtual glm::vec3 GetPosition() const = 0;
        virtual glm::vec3 GetFacing() const = 0;
        virtual float GetYaw() const = 0;
        virtual float GetPitch() const = 0;

        // Setters pour synchroniser avec la caméra
        virtual void SetYaw(float yaw) = 0;
        virtual void SetPitch(float pitch) = 0;
    };

}