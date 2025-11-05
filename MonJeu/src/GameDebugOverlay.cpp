// MonJeu/src/GameDebugOverlay.cpp
#include <MonJeu/GameDebugOverlay.h>
#include <glad/glad.h>
#include <sstream>
#include <iomanip> // Pour std::setprecision
#include <iostream>

namespace MonJeu {

    GameDebugOverlay::GameDebugOverlay(int w, int h)
        : windowWidth(w), windowHeight(h) {

        // Tente de charger la police. Si elle n'existe pas, le moteur
        // de rendu de texte devrait simplement échouer sans crasher.
        // (Vous devriez ajouter une police dans ce chemin)
        if (!textRenderer.LoadFont("MonJeu/assets/WhiteRabbit.ttf", 24)) {
            std::cerr << "WARN: Impossible de charger la police '../../../MonJeu/assets/WhiteRabbit.ttf'" << std::endl;
        }
    }

    void GameDebugOverlay::RenderDebugInfo(float fps, int chunkCount, const glm::vec3& cameraPos, const glm::vec3& playerPos, const std::vector<NihilEngine::PerformanceSection>& sections) {
        if (!showDebugInfo) return; // (showDebugInfo est hérité de DebugOverlay)

        // Désactive le test de profondeur pour que le texte s'affiche par-dessus la 3D
        glDisable(GL_DEPTH_TEST);

        float y = 30.0f; // Coordonnée Y de départ (depuis le HAUT de l'écran)
        const float y_step = 25.0f; // Espace entre les lignes

        // Formatte et affiche les informations

        if (showFPS) {
            std::stringstream ss;
            ss << std::fixed << std::setprecision(1) << "FPS: " << fps;
            RenderText(ss.str(), 10.0f, y, glm::vec3(1.0f, 1.0f, 0.0f)); // Jaune
            y += y_step;
        }

        if (showChunkInfo) {
            std::stringstream ss;
            ss << "Chunks: " << chunkCount;
            RenderText(ss.str(), 10.0f, y, glm::vec3(0.0f, 1.0f, 1.0f)); // Cyan
            y += y_step;
        }

        if (showPositions) {
            std::stringstream ssCam;
            ssCam << std::fixed << std::setprecision(2)
                  << "Cam: (" << cameraPos.x << ", " << cameraPos.y << ", " << cameraPos.z << ")";
            RenderText(ssCam.str(), 10.0f, y, glm::vec3(1.0f, 0.7f, 0.0f)); // Orange
            y += y_step;

            std::stringstream ssPlayer;
            ssPlayer << std::fixed << std::setprecision(2)
                     << "Player: (" << playerPos.x << ", " << playerPos.y << ", " << playerPos.z << ")";
            RenderText(ssPlayer.str(), 10.0f, y, glm::vec3(0.0f, 1.0f, 0.0f)); // Vert
            y += y_step;
        }

        if (showPerformance && !sections.empty()) {
            RenderText("Performance (ms):", 10.0f, y, glm::vec3(1.0f, 0.5f, 1.0f)); // Magenta
            y += y_step;

            for (const auto& section : sections) {
                std::stringstream ss;
                ss << std::fixed << std::setprecision(3) << section.name << ": " << (section.duration * 1000.0);
                RenderText(ss.str(), 20.0f, y, glm::vec3(0.8f, 0.8f, 0.8f)); // Gris clair
                y += y_step;
            }
        }

        // Affiche tout texte personnalisé ajouté via AddText()
        for (const auto& customText : customTexts) {
            RenderText(customText.text, customText.x, customText.y, glm::vec3(1.0f, 1.0f, 1.0f)); // Blanc
        }

        // Vide la liste pour la prochaine frame
        customTexts.clear();

        // Réactive le test de profondeur
        glEnable(GL_DEPTH_TEST);
    }

    void GameDebugOverlay::AddText(const std::string& text, float x, float y) {
        customTexts.push_back({text, x, y}); //
    }

    // Fonction utilitaire pour gérer la conversion des coordonnées
    void GameDebugOverlay::RenderText(const std::string& text, float x, float y, const glm::vec3& color) {
        // Le Y que nous passons est depuis le HAUT (plus facile à gérer).
        // FreeType (utilisé par TextRenderer) dessine depuis le BAS.
        // Nous devons donc inverser la coordonnée Y.
        float screenY = windowHeight - y; //

        // Appelle le moteur de rendu de texte
        textRenderer.RenderText(text, x, screenY, 1.0f, color); //
    }

} // namespace MonJeu