#pragma once
#include <vector>
#include <glm/glm.hpp>

namespace NihilEngine {

struct LODLevel {
    float distance;
    float detailLevel; // 0.0 = lowest detail, 1.0 = highest detail
};

class LODManager {
public:
    LODManager();

    void addLODLevel(float distance, float detailLevel);
    float getDetailLevel(const glm::vec3& position, const glm::vec3& cameraPosition) const;

    // Performance utilities
    static void enableVSync(bool enable);
    static void setTargetFPS(float fps);

private:
    std::vector<LODLevel> m_LODLevels;
};

class PerformanceMonitor {
public:
    static PerformanceMonitor& getInstance();

    void startFrame();
    void endFrame();
    float getFPS() const;
    float getFrameTime() const;

private:
    PerformanceMonitor();
    float m_LastFrameTime;
    float m_FrameTime;
    float m_FPS;
};

}