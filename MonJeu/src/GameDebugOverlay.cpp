// MonJeu/src/GameDebugOverlay.cpp
#include <MonJeu/GameDebugOverlay.h>
#include <glad/glad.h>
#include <sstream>
#include <iomanip>

namespace MonJeu {

    GameDebugOverlay::GameDebugOverlay(int w, int h)
        : windowWidth(w), windowHeight(h) {
        // textRenderer est déjà initialisé par défaut
    }

    bool GameDebugOverlay::LoadFont(const std::string& fontPath, unsigned int fontSize) {
        return textRenderer.LoadFont(fontPath, fontSize);
    }

    void GameDebugOverlay::RenderDebugInfo(float fps, int chunkCount, const glm::vec3& cameraPos, const glm::vec3& playerPos) {
        if (!showDebugInfo) return;

        glDisable(GL_DEPTH_TEST);

        // On n'utilise PLUS glMatrixMode (legacy) → on utilise le shader de TextRenderer
        // → On supprime tout glOrtho / glPushMatrix

        float y = 30.0f;  // Commence en haut à gauche (y = 20)

        if (showFPS) {
            std::stringstream ss; ss << std::fixed << std::setprecision(1) << "FPS: " << fps;
            RenderText(ss.str(), 10.0f, y, glm::vec3(1.0f, 1.0f, 0.0f));  // Jaune
            y += 30.0f;
        }

        if (showChunkInfo) {
            std::stringstream ss; ss << "Chunks: " << chunkCount;
            RenderText(ss.str(), 10.0f, y, glm::vec3(0.0f, 1.0f, 1.0f));  // Cyan
            y += 30.0f;
        }

        if (showPositions) {
            std::stringstream ssCam; ssCam << std::fixed << std::setprecision(2)
                << "Cam: (" << cameraPos.x << ", " << cameraPos.y << ", " << cameraPos.z << ")";
            RenderText(ssCam.str(), 10.0f, y, glm::vec3(1.0f, 0.7f, 0.0f));  // Orange
            y += 30.0f;

            std::stringstream ssPlayer; ssPlayer << std::fixed << std::setprecision(2)
                << "Player: (" << playerPos.x << ", " << playerPos.y << ", " << playerPos.z << ")";
            RenderText(ssPlayer.str(), 10.0f, y, glm::vec3(0.0f, 1.0f, 0.0f));  // Vert
        }

        glEnable(GL_DEPTH_TEST);
    }

    void GameDebugOverlay::RenderText(const std::string& text, float x, float y, const glm::vec3& color) {
        // y vient du haut → FreeType commence en bas → inverse
        float screenY = windowHeight - y - 24.0f;  // ajuste selon taille police
        textRenderer.RenderText(text, x, screenY, 1.0f, color);
    }

} // namespace MonJeu