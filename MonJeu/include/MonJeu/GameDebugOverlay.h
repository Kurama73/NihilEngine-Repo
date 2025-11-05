// MonJeu/include/MonJeu/GameDebugOverlay.h
#pragma once
#include <NihilEngine/DebugOverlay.h>
#include <NihilEngine/TextRenderer.h> //
#include <NihilEngine/Performance.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>

namespace MonJeu {

/**
 * @brief Implémentation spécifique au jeu de l'overlay de débogage.
 * Gère l'affichage des FPS, de la position et d'autres infos de jeu.
 */
class GameDebugOverlay : public NihilEngine::DebugOverlay { //
public:
    GameDebugOverlay(int w, int h);
    ~GameDebugOverlay() override = default;

    /**
     * @brief Fonction principale de rendu appelée par la boucle de jeu.
     */
    void RenderDebugInfo(float fps, int chunkCount, const glm::vec3& cameraPos, const glm::vec3& playerPos, const std::vector<NihilEngine::PerformanceSection>& sections = {}); //

    // Toggles spécifiques au jeu
    void ToggleFPS() { showFPS = !showFPS; }
    void TogglePositions() { showPositions = !showPositions; }
    void ToggleChunkInfo() { showChunkInfo = !showChunkInfo; }
    void TogglePerformance() { showPerformance = !showPerformance; }

    /**
     * @brief Ajoute du texte personnalisé à afficher pour la frame suivante.
     */
    void AddText(const std::string& text, float x, float y);

private:
    // Le TextRenderer du moteur fait le vrai travail
    NihilEngine::TextRenderer textRenderer; //
    int windowWidth, windowHeight;

    // Drapeaux pour savoir quoi afficher
    bool showFPS = true;
    bool showPositions = true;
    bool showChunkInfo = true;
    bool showPerformance = true;

    // Stockage temporaire pour le texte personnalisé
    struct CustomText {
        std::string text;
        float x, y;
    };
    std::vector<CustomText> customTexts;

    /**
     * @brief Fonction utilitaire pour le rendu de texte, gère la conversion des coordonnées.
     */
    void RenderText(const std::string& text, float x, float y, const glm::vec3& color = glm::vec3(1.0f));
};

} // namespace MonJeu