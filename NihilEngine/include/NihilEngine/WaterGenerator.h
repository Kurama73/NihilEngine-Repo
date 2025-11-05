#pragma once

#include <NihilEngine/TerrainGenerator.h>
#include <vector>
#include <glm/glm.hpp>

namespace NihilEngine {

enum class WaterType {
    Ocean,
    Lake,
    River
};

struct WaterBody {
    WaterType type;
    std::vector<glm::vec2> outline;
    float waterLevel;
    glm::vec3 color;
};

class WaterGenerator {
public:
    WaterGenerator(unsigned int seed = 0);
    ~WaterGenerator() = default;

    // Génère les corps d'eau pour une région
    std::vector<WaterBody> generateWaterBodies(int width, int height, float scale,
                                              const std::vector<std::vector<float>>& heightMap);

    // Modifie la heightmap pour inclure les niveaux d'eau
    void applyWaterLevels(std::vector<std::vector<float>>& heightMap, const std::vector<WaterBody>& waterBodies, float scale);

    // Détermine si une position est sous l'eau
    bool isUnderwater(const glm::vec2& pos, const std::vector<WaterBody>& waterBodies) const;

private:
    Noise noise;

    // Génère les océans (eau à hauteur 0)
    std::vector<WaterBody> generateOceans(int width, int height, float scale);

    // Génère les lacs dans les dépressions
    std::vector<WaterBody> generateLakes(int width, int height, float scale,
                                        const std::vector<std::vector<float>>& heightMap);

    // Trouve les dépressions locales pour les lacs
    std::vector<glm::vec2> findDepressions(int width, int height, float scale,
                                          const std::vector<std::vector<float>>& heightMap);

    // Calcule le niveau d'eau pour un lac
    float calculateLakeLevel(const glm::vec2& center, float radius,
                           const std::vector<std::vector<float>>& heightMap, float scale);
};

}