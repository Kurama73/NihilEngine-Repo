#include <NihilEngine/TerrainGenerator.h>
#include <algorithm>

namespace NihilEngine {

TerrainGenerator::TerrainGenerator(unsigned int seed)
    : noise(seed), baseHeight(0.0f), amplitude(10.0f), frequency(0.01f), octaves(4), persistence(0.5f) {}

float TerrainGenerator::getHeight(float x, float z) const {
    // Génère du bruit fractal pour le terrain de base
    float baseNoise = noise.fractal(x, z, octaves, persistence, frequency);

    // Ajoute du bruit ridged pour les montagnes
    float mountainNoise = noise.ridged(x, z, octaves / 2, persistence * 0.8f, frequency * 2.0f);

    // Combine les bruits
    float combinedNoise = baseNoise * 0.7f + mountainNoise * 0.3f;

    // Applique l'amplitude et la hauteur de base
    return baseHeight + combinedNoise * amplitude;
}

std::vector<std::vector<float>> TerrainGenerator::generateHeightMap(int width, int height, float scale) const {
    std::vector<std::vector<float>> heightMap(height, std::vector<float>(width));

    for (int z = 0; z < height; ++z) {
        for (int x = 0; x < width; ++x) {
            float worldX = static_cast<float>(x) * scale;
            float worldZ = static_cast<float>(z) * scale;
            heightMap[z][x] = getHeight(worldX, worldZ);
        }
    }

    return heightMap;
}

}