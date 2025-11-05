#pragma once

#include <glm/glm.hpp>
#include <NihilEngine/TextRenderer.h>
#include <vector>
#include <string>

namespace NihilEngine {
    struct PerformanceSection;
}

namespace NihilEngine {
    class DebugOverlay {
    public:
        DebugOverlay();
        virtual ~DebugOverlay() = default;

        // Toggles
        bool showPlayerRaycast = false;
        bool showDebugInfo = false;
        bool showWireframe = false;
        bool showBoundingBoxes = false;

        // Methods
        virtual void RenderDebugInfo(float fps, int chunkCount, const glm::vec3& cameraPos, const glm::vec3& playerPos, const std::vector<PerformanceSection>& sections = {}) = 0;
        void ToggleWireframe();
        void ToggleDebugInfo() { showDebugInfo = !showDebugInfo; }
        void RenderText(const std::string& text, float x, float y, const glm::vec3& color = glm::vec3(1.0f));

    protected:
        TextRenderer m_TextRenderer;
    };
}