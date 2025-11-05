#pragma once

#include <NihilEngine/Noise.h>
#include <vector>
#include <glm/glm.hpp>

namespace NihilEngine {

enum class BiomeType {
    Ocean,
    Beach,
    Plains,
    Forest,
    Desert,
    Tundra,
    Mountains,
    Swamp
};

struct Biome {
    BiomeType type;
    glm::vec3 grassColor;
    float temperature; // 0.0 = froid, 1.0 = chaud
    float humidity;    // 0.0 = sec, 1.0 = humide
    float minHeight;
    float maxHeight;
};

class BiomeGenerator {
public:
    BiomeGenerator(unsigned int seed = 0);
    ~BiomeGenerator() = default;

    // Détermine le biome pour une position donnée
    BiomeType getBiome(float x, float z, float height) const;

    // Génère une carte de biomes
    std::vector<std::vector<BiomeType>> generateBiomeMap(int width, int height, float scale,
                                                         const std::vector<std::vector<float>>& heightMap) const;

    // Obtient les propriétés d'un biome
    const Biome& getBiomeProperties(BiomeType type) const;

private:
    Noise temperatureNoise;
    Noise humidityNoise;
    std::vector<Biome> biomes;

    void initializeBiomes();
};

}