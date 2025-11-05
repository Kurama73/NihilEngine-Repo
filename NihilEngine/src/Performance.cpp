#include <NihilEngine/Performance.h>
#include <GLFW/glfw3.h>
#include <algorithm>
#include <chrono>

namespace NihilEngine {

LODManager::LODManager() {
    // Default LOD levels
    addLODLevel(10.0f, 1.0f);  // Close: full detail
    addLODLevel(50.0f, 0.7f);  // Medium: reduced detail
    addLODLevel(100.0f, 0.3f); // Far: low detail
}

void LODManager::addLODLevel(float distance, float detailLevel) {
    m_LODLevels.push_back({distance, detailLevel});
    // Sort by distance
    std::sort(m_LODLevels.begin(), m_LODLevels.end(),
              [](const LODLevel& a, const LODLevel& b) { return a.distance < b.distance; });
}

float LODManager::getDetailLevel(const glm::vec3& position, const glm::vec3& cameraPosition) const {
    float distance = glm::distance(position, cameraPosition);

    for (size_t i = 0; i < m_LODLevels.size(); ++i) {
        if (distance <= m_LODLevels[i].distance) {
            if (i == 0) return m_LODLevels[i].detailLevel;
            // Interpolate between levels
            float t = (distance - m_LODLevels[i-1].distance) /
                     (m_LODLevels[i].distance - m_LODLevels[i-1].distance);
            return glm::mix(m_LODLevels[i-1].detailLevel, m_LODLevels[i].detailLevel, t);
        }
    }

    // Beyond last level
    return m_LODLevels.back().detailLevel * 0.5f;
}

void LODManager::enableVSync(bool enable) {
    glfwSwapInterval(enable ? 1 : 0);
}

void LODManager::setTargetFPS(float fps) {
    // This is a simple implementation - in a real engine you'd use a more sophisticated frame pacing
    if (fps > 0.0f) {
        float targetFrameTime = 1.0f / fps;
        // Note: This doesn't actually limit FPS, just sets a target
        // You'd need to implement frame timing in the main loop
    }
}

PerformanceMonitor::PerformanceMonitor() : m_LastFrameTime(0.0f), m_FrameTime(0.0f), m_FPS(0.0f) {}

PerformanceMonitor& PerformanceMonitor::getInstance() {
    static PerformanceMonitor instance;
    return instance;
}

void PerformanceMonitor::startFrame() {
    m_LastFrameTime = glfwGetTime();
}

void PerformanceMonitor::endFrame() {
    double currentTime = glfwGetTime();
    m_FrameTime = currentTime - m_LastFrameTime;
    m_FPS = 1.0f / m_FrameTime;
}

float PerformanceMonitor::getFPS() const {
    return m_FPS;
}

float PerformanceMonitor::getFrameTime() const {
    return m_FrameTime;
}

}