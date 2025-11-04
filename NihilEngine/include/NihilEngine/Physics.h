#pragma once

#include <glm/glm.hpp>
#include <functional>

namespace NihilEngine {

    struct RaycastHit {
        bool hit = false;
        glm::ivec3 blockPosition;
        glm::vec3 hitNormal;
        glm::vec3 hitPoint;
    };

    struct AABB {
        glm::vec3 min;
        glm::vec3 max;
    };

    bool RaycastVoxel(
        const glm::vec3& origin,
        const glm::vec3& direction,
        float maxDistance,
        std::function<bool(const glm::ivec3&)> voxelCheck,
        RaycastHit& outHit
    );

    bool CheckAABBCollision(const AABB& a, const AABB& b);
    bool RayAABBIntersection(const glm::vec3& origin, const glm::vec3& direction, const AABB& box, float& t);

}