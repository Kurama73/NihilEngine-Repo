#pragma once

#include <unordered_map>
#include <vector>
#include <memory>
#include <glm/glm.hpp>

namespace NihilEngine {

enum class ChunkLODLevel {
    HIGH_DETAIL,    // Détail élevé (chunks proches)
    MEDIUM_DETAIL,  // Détail moyen
    LOW_DETAIL,     // Détail faible
    VERY_LOW_DETAIL // Détail très faible (très lointain)
};

struct LODSettings {
    float highDetailDistance = 4.0f;      // Distance pour détail élevé (en chunks)
    float mediumDetailDistance = 8.0f;    // Distance pour détail moyen
    float lowDetailDistance = 16.0f;      // Distance pour détail faible
    float veryLowDetailDistance = 32.0f;  // Distance pour détail très faible

    float calculationDistance = 64.0f;    // Distance de calcul (entités, événements)
    float generationDistance = 48.0f;     // Distance de génération des chunks
    float displayDistance = 32.0f;        // Distance d'affichage maximale
};

class LODSystem {
public:
    LODSystem();
    ~LODSystem() = default;

    // Configuration
    void setSettings(const LODSettings& settings) { m_settings = settings; }
    const LODSettings& getSettings() const { return m_settings; }

    // Détermine le niveau de LOD pour une position donnée
    ChunkLODLevel getLODLevel(const glm::vec3& position, const glm::vec3& cameraPos) const;

    // Vérifie si une position doit être calculée (entités, événements)
    bool shouldCalculate(const glm::vec3& position, const glm::vec3& cameraPos) const;

    // Vérifie si une position doit être générée
    bool shouldGenerate(const glm::vec3& position, const glm::vec3& cameraPos) const;

    // Vérifie si une position doit être affichée
    bool shouldDisplay(const glm::vec3& position, const glm::vec3& cameraPos) const;

    // Calcule la distance entre deux positions
    float distance(const glm::vec3& a, const glm::vec3& b) const;

private:
    LODSettings m_settings;
};

}