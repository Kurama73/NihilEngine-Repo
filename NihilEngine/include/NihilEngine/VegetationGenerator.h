#pragma once

#include <NihilEngine/BiomeGenerator.h>
#include <vector>
#include <glm/glm.hpp>

namespace NihilEngine {

enum class VegetationType {
    None,
    Grass,
    Flowers,
    Trees,
    Bushes,
    Cactus,
    Reeds
};

struct VegetationInstance {
    VegetationType type;
    glm::vec3 position;
    float scale;
    float rotation;
};

class VegetationGenerator {
public:
    VegetationGenerator(unsigned int seed = 0);
    ~VegetationGenerator() = default;

    // Génère de la végétation pour une région
    std::vector<VegetationInstance> generateVegetation(int width, int height, float scale,
                                                      const std::vector<std::vector<BiomeType>>& biomeMap,
                                                      const std::vector<std::vector<float>>& heightMap);

    // Paramètres de densité par biome
    void setDensity(BiomeType biome, float density);
    void setMaxInstances(int max) { maxInstances = max; }

private:
    Noise noise;
    std::vector<float> biomeDensities;
    int maxInstances;

    // Génère de la végétation pour un biome spécifique
    std::vector<VegetationInstance> generateForBiome(BiomeType biome, int startX, int startZ,
                                                    int endX, int endZ, float scale,
                                                    const std::vector<std::vector<float>>& heightMap);

    // Détermine le type de végétation pour un biome
    VegetationType getVegetationType(BiomeType biome, float randomValue) const;

    // Vérifie si une position est valide pour placer de la végétation
    bool isValidVegetationPosition(const glm::vec3& pos, VegetationType type,
                                  const std::vector<std::vector<float>>& heightMap,
                                  int mapWidth, int mapHeight, float scale) const;
};

}