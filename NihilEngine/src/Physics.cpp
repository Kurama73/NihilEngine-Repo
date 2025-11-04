// Physics.cpp
#include <NihilEngine/Physics.h>
#include <NihilEngine/Constants.h>
#include <limits>
#include <algorithm>

namespace NihilEngine {

bool RaycastVoxel(
    const glm::vec3& origin,
    const glm::vec3& direction,
    float maxDistance,
    std::function<bool(const glm::ivec3&)> voxelCheck,
    RaycastHit& outHit
) {
    glm::ivec3 currentVoxel = glm::floor(origin);
    glm::vec3 tMax, tDelta;
    glm::ivec3 step;

    for (int i = 0; i < Constants::AXIS_COUNT; ++i) {
        if (direction[i] > 0.0f) {
            step[i] = 1;
            tMax[i] = (currentVoxel[i] + 1 - origin[i]) / direction[i];
            tDelta[i] = 1.0f / direction[i];
        } else if (direction[i] < 0.0f) {
            step[i] = -1;
            tMax[i] = (currentVoxel[i] - origin[i]) / direction[i];
            tDelta[i] = -1.0f / direction[i];
        } else {
            step[i] = 0;
            tMax[i] = std::numeric_limits<float>::infinity();
            tDelta[i] = std::numeric_limits<float>::infinity();
        }
    }

    float distance = 0.0f;
    glm::vec3 lastNormal(0.0f);

    while (distance < maxDistance) {
        if (voxelCheck(currentVoxel)) {
            outHit.hit = true;
            outHit.blockPosition = currentVoxel;
            outHit.hitNormal = lastNormal;
            outHit.hitPoint = origin + direction * distance;
            return true;
        }

        int side = 0;
        if (tMax.x < tMax.y) {
            if (tMax.x < tMax.z) side = 0;
            else side = 2;
        } else {
            if (tMax.y < tMax.z) side = 1;
            else side = 2;
        }

        distance = tMax[side];
        if (distance > maxDistance) break;

        lastNormal = glm::vec3(0.0f);
        lastNormal[side] = -step[side];

        currentVoxel[side] += step[side];
        tMax[side] += tDelta[side];
    }

    outHit.hit = false;
    return false;
}

bool CheckAABBCollision(const AABB& a, const AABB& b) {
    return (a.min.x <= b.max.x && a.max.x >= b.min.x) &&
           (a.min.y <= b.max.y && a.max.y >= b.min.y) &&
           (a.min.z <= b.max.z && a.max.z >= b.min.z);
}

bool RayAABBIntersection(const glm::vec3& origin, const glm::vec3& direction, const AABB& box, float& t) {
    float tmin = -std::numeric_limits<float>::max();
    float tmax =  std::numeric_limits<float>::max();

    for (int i = 0; i < 3; ++i) {
        if (std::abs(direction[i]) < 1e-6f) {
            if (origin[i] < box.min[i] || origin[i] > box.max[i]) return false;
            continue;
        }
        float invD = 1.0f / direction[i];
        float t0 = (box.min[i] - origin[i]) * invD;
        float t1 = (box.max[i] - origin[i]) * invD;
        if (invD < 0.0f) std::swap(t0, t1);
        tmin = std::max(tmin, t0);
        tmax = std::min(tmax, t1);
        if (tmax <= tmin) return false;
    }
    t = tmin;
    return true;
}

}