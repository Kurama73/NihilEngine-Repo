#include <NihilEngine/WaterGenerator.h>
#include <algorithm>
#include <cmath>

namespace NihilEngine {

WaterGenerator::WaterGenerator(unsigned int seed) : noise(seed + 5) {}

std::vector<WaterBody> WaterGenerator::generateWaterBodies(int width, int height, float scale,
                                                          const std::vector<std::vector<float>>& heightMap) {
    std::vector<WaterBody> waterBodies;

    // Génère les océans
    auto oceans = generateOceans(width, height, scale);
    waterBodies.insert(waterBodies.end(), oceans.begin(), oceans.end());

    // Génère les lacs
    auto lakes = generateLakes(width, height, scale, heightMap);
    waterBodies.insert(waterBodies.end(), lakes.begin(), lakes.end());

    return waterBodies;
}

void WaterGenerator::applyWaterLevels(std::vector<std::vector<float>>& heightMap,
                                     const std::vector<WaterBody>& waterBodies, float scale) {
    // Pour les océans, toute zone en dessous du niveau 0 devient de l'eau
    for (int z = 0; z < heightMap.size(); ++z) {
        for (int x = 0; x < heightMap[z].size(); ++x) {
            if (heightMap[z][x] < 0.0f) {
                heightMap[z][x] = 0.0f; // Niveau de l'océan
            }
        }
    }

    // Pour les lacs, on pourrait ajuster localement, mais pour simplifier,
    // on laisse les rivières gérer les niveaux d'eau locaux
}

bool WaterGenerator::isUnderwater(const glm::vec2& pos, const std::vector<WaterBody>& waterBodies) const {
    for (const auto& water : waterBodies) {
        if (water.type == WaterType::Ocean) {
            // Pour l'océan, tout ce qui est en dessous de 0 est sous l'eau
            // Dans une vraie implémentation, on vérifierait les contours
            return true; // Simplifié
        }
    }
    return false;
}

std::vector<WaterBody> WaterGenerator::generateOceans(int width, int height, float scale) {
    std::vector<WaterBody> oceans;

    // Crée un océan couvrant les bords de la carte
    WaterBody ocean;
    ocean.type = WaterType::Ocean;
    ocean.waterLevel = 0.0f;
    ocean.color = glm::vec3(0.0f, 0.3f, 0.8f); // Bleu océan

    // Définit un contour simple pour l'océan (toute la carte en dessous de 0)
    // Dans une vraie implémentation, on définirait des contours plus complexes
    oceans.push_back(ocean);

    return oceans;
}

std::vector<WaterBody> WaterGenerator::generateLakes(int width, int height, float scale,
                                                    const std::vector<std::vector<float>>& heightMap) {
    std::vector<WaterBody> lakes;

    // Trouve les dépressions potentielles pour les lacs
    auto depressions = findDepressions(width, height, scale, heightMap);

    for (const auto& depression : depressions) {
        // Calcule le niveau du lac
        float lakeLevel = calculateLakeLevel(depression, 20.0f, heightMap, scale);

        if (lakeLevel > 0.0f) { // Seulement si au-dessus du niveau de l'océan
            WaterBody lake;
            lake.type = WaterType::Lake;
            lake.waterLevel = lakeLevel;
            lake.color = glm::vec3(0.0f, 0.4f, 0.9f); // Bleu lac

            // Crée un contour circulaire simple
            const int numPoints = 16;
            for (int i = 0; i < numPoints; ++i) {
                float angle = (2.0f * 3.14159f * i) / numPoints;
                glm::vec2 point = depression + glm::vec2(std::cos(angle), std::sin(angle)) * 15.0f;
                lake.outline.push_back(point);
            }

            lakes.push_back(lake);
        }
    }

    return lakes;
}

std::vector<glm::vec2> WaterGenerator::findDepressions(int width, int height, float scale,
                                                      const std::vector<std::vector<float>>& heightMap) {
    std::vector<glm::vec2> depressions;

    // Échantillonne des points pour trouver les dépressions
    for (int z = 10; z < height - 10; z += 20) {
        for (int x = 10; x < width - 10; x += 20) {
            float centerHeight = heightMap[z][x];
            bool isDepression = true;

            // Vérifie si les voisins sont plus hauts
            for (int dz = -5; dz <= 5 && isDepression; ++dz) {
                for (int dx = -5; dx <= 5; ++dx) {
                    if (dx == 0 && dz == 0) continue;

                    int nx = x + dx;
                    int nz = z + dz;

                    if (nx >= 0 && nx < width && nz >= 0 && nz < height) {
                        if (heightMap[nz][nx] < centerHeight) {
                            isDepression = false;
                            break;
                        }
                    }
                }
            }

            if (isDepression && centerHeight > 1.0f) { // Pas trop près du niveau de l'océan
                depressions.emplace_back(x * scale, z * scale);
            }
        }
    }

    return depressions;
}

float WaterGenerator::calculateLakeLevel(const glm::vec2& center, float radius,
                                        const std::vector<std::vector<float>>& heightMap, float scale) {
    int cx = static_cast<int>(center.x / scale);
    int cz = static_cast<int>(center.y / scale);
    int r = static_cast<int>(radius / scale);

    float minHeight = std::numeric_limits<float>::max();
    float maxHeight = std::numeric_limits<float>::min();

    // Trouve les hauteurs dans le rayon
    for (int dz = -r; dz <= r; ++dz) {
        for (int dx = -r; dx <= r; ++dx) {
            int nx = cx + dx;
            int nz = cz + dz;

            if (nx >= 0 && nx < static_cast<int>(heightMap[0].size()) &&
                nz >= 0 && nz < static_cast<int>(heightMap.size())) {

                float dist = std::sqrt(dx * dx + dz * dz);
                if (dist <= r) {
                    float h = heightMap[nz][nx];
                    minHeight = std::min(minHeight, h);
                    maxHeight = std::max(maxHeight, h);
                }
            }
        }
    }

    // Le niveau du lac est le minimum local, mais au moins 0.5 au-dessus du minimum
    return std::max(minHeight + 0.5f, 0.5f);
}

}