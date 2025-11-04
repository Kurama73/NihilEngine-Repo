// MonJeu/include/MonJeu/GameDebugOverlay.h
#pragma once
#include <NihilEngine/DebugOverlay.h>
#include <NihilEngine/TextRenderer.h>
#include <glm/glm.hpp>
#include <string>

namespace MonJeu {

class GameDebugOverlay : public NihilEngine::DebugOverlay {
public:
    GameDebugOverlay(int w, int h);
    ~GameDebugOverlay() override = default;

    void RenderDebugInfo(float fps, int chunkCount, const glm::vec3& cameraPos, const glm::vec3& playerPos) override;

    bool LoadFont(const std::string& fontPath, unsigned int fontSize);
    void ToggleFPS() { showFPS = !showFPS; }
    void TogglePositions() { showPositions = !showPositions; }
    void ToggleChunkInfo() { showChunkInfo = !showChunkInfo; }

private:
    NihilEngine::TextRenderer textRenderer;
    int windowWidth, windowHeight;

    bool showFPS = true;
    bool showPositions = true;
    bool showChunkInfo = true;

    void RenderText(const std::string& text, float x, float y, const glm::vec3& color = glm::vec3(1.0f));
};

} // namespace MonJeu