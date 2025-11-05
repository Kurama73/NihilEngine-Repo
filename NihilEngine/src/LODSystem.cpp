#include <NihilEngine/LODSystem.h>
#include <algorithm>

namespace NihilEngine {

LODSystem::LODSystem() = default;

ChunkLODLevel LODSystem::getLODLevel(const glm::vec3& position, const glm::vec3& cameraPos) const {
    float dist = distance(position, cameraPos);

    if (dist <= m_settings.highDetailDistance) {
        return ChunkLODLevel::HIGH_DETAIL;
    } else if (dist <= m_settings.mediumDetailDistance) {
        return ChunkLODLevel::MEDIUM_DETAIL;
    } else if (dist <= m_settings.lowDetailDistance) {
        return ChunkLODLevel::LOW_DETAIL;
    } else {
        return ChunkLODLevel::VERY_LOW_DETAIL;
    }
}

bool LODSystem::shouldCalculate(const glm::vec3& position, const glm::vec3& cameraPos) const {
    return distance(position, cameraPos) <= m_settings.calculationDistance;
}

bool LODSystem::shouldGenerate(const glm::vec3& position, const glm::vec3& cameraPos) const {
    return distance(position, cameraPos) <= m_settings.generationDistance;
}

bool LODSystem::shouldDisplay(const glm::vec3& position, const glm::vec3& cameraPos) const {
    return distance(position, cameraPos) <= m_settings.displayDistance;
}

float LODSystem::distance(const glm::vec3& a, const glm::vec3& b) const {
    glm::vec3 diff = a - b;
    return std::sqrt(diff.x * diff.x + diff.z * diff.z); // Distance 2D horizontale uniquement
}

}