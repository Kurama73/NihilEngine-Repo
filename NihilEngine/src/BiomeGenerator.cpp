#include <NihilEngine/BiomeGenerator.h>
#include <algorithm>

namespace NihilEngine {

BiomeGenerator::BiomeGenerator(unsigned int seed)
    : temperatureNoise(seed + 1), humidityNoise(seed + 2) {
    initializeBiomes();
}

void BiomeGenerator::initializeBiomes() {
    biomes = {
        {BiomeType::Ocean, glm::vec3(0.0f, 0.0f, 0.0f), 0.3f, 0.8f, -10.0f, 0.0f},
        {BiomeType::Beach, glm::vec3(0.9f, 0.8f, 0.6f), 0.6f, 0.4f, 0.0f, 1.0f},
        {BiomeType::Plains, glm::vec3(0.4f, 0.8f, 0.2f), 0.5f, 0.5f, 1.0f, 5.0f},
        {BiomeType::Forest, glm::vec3(0.3f, 0.7f, 0.2f), 0.4f, 0.7f, 1.0f, 8.0f},
        {BiomeType::Desert, glm::vec3(0.8f, 0.8f, 0.2f), 0.8f, 0.2f, 1.0f, 4.0f},
        {BiomeType::Tundra, glm::vec3(0.6f, 0.8f, 0.6f), 0.1f, 0.3f, 2.0f, 6.0f},
        {BiomeType::Mountains, glm::vec3(0.5f, 0.5f, 0.5f), 0.2f, 0.4f, 6.0f, 20.0f},
        {BiomeType::Swamp, glm::vec3(0.2f, 0.4f, 0.1f), 0.3f, 0.9f, 0.0f, 3.0f}
    };
}

BiomeType BiomeGenerator::getBiome(float x, float z, float height) const {
    // Génère température et humidité
    float temperature = temperatureNoise.fractal(x * 0.001f, z * 0.001f, 3, 0.5f, 1.0f) * 0.5f + 0.5f;
    float humidity = humidityNoise.fractal(x * 0.001f, z * 0.001f, 3, 0.5f, 1.0f) * 0.5f + 0.5f;

    // Ajuste selon la hauteur (plus haut = plus froid et sec)
    temperature -= height * 0.01f;
    humidity -= height * 0.005f;

    temperature = std::clamp(temperature, 0.0f, 1.0f);
    humidity = std::clamp(humidity, 0.0f, 1.0f);

    // Logique de détermination du biome basée sur température, humidité et hauteur
    if (height < 0.0f) {
        return BiomeType::Ocean;
    } else if (height < 1.0f) {
        return BiomeType::Beach;
    } else if (height > 10.0f) {
        return BiomeType::Mountains;
    } else if (temperature < 0.2f) {
        return BiomeType::Tundra;
    } else if (temperature > 0.7f && humidity < 0.3f) {
        return BiomeType::Desert;
    } else if (humidity > 0.7f && temperature < 0.5f) {
        return BiomeType::Swamp;
    } else if (humidity > 0.5f) {
        return BiomeType::Forest;
    } else {
        return BiomeType::Plains;
    }
}

std::vector<std::vector<BiomeType>> BiomeGenerator::generateBiomeMap(int width, int height, float scale,
                                                                   const std::vector<std::vector<float>>& heightMap) const {
    std::vector<std::vector<BiomeType>> biomeMap(height, std::vector<BiomeType>(width));

    for (int z = 0; z < height; ++z) {
        for (int x = 0; x < width; ++x) {
            float worldX = static_cast<float>(x) * scale;
            float worldZ = static_cast<float>(z) * scale;
            float height = heightMap[z][x];
            biomeMap[z][x] = getBiome(worldX, worldZ, height);
        }
    }

    return biomeMap;
}

const Biome& BiomeGenerator::getBiomeProperties(BiomeType type) const {
    for (const auto& biome : biomes) {
        if (biome.type == type) {
            return biome;
        }
    }
    // Retourne le premier biome par défaut (ne devrait pas arriver)
    return biomes[0];
}

}