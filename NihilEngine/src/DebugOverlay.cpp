#include <NihilEngine/DebugOverlay.h>
#include <glad/glad.h>

namespace NihilEngine {

DebugOverlay::DebugOverlay() {
    // Initialize text renderer or other debug tools
}

void DebugOverlay::ToggleWireframe() {
    showWireframe = !showWireframe;
    if (showWireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
}

void DebugOverlay::RenderText(const std::string& text, float x, float y, const glm::vec3& color) {
    m_TextRenderer.RenderText(text, x, y, 1.0f, color);
}

} // namespace NihilEngine