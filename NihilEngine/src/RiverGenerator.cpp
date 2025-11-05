#include <NihilEngine/RiverGenerator.h>
#include <algorithm>
#include <cmath>

namespace NihilEngine {

RiverGenerator::RiverGenerator(unsigned int seed) : noise(seed + 3) {}

std::vector<RiverPoint> RiverGenerator::generateRiver(const glm::vec2& startPos, const TerrainGenerator& terrainGen, float maxLength) {
    std::vector<RiverPoint> river;
    glm::vec2 currentPos = startPos;
    float totalLength = 0.0f;
    float currentWidth = 2.0f; // Largeur initiale
    float currentDepth = 0.5f; // Profondeur initiale

    river.push_back({currentPos, currentWidth, currentDepth});

    while (totalLength < maxLength) {
        // Trouve la direction vers le point le plus bas
        glm::vec2 nextPos = findLowestNeighbor(currentPos, terrainGen);

        // Calcule la distance
        float distance = glm::distance(currentPos, nextPos);
        if (distance < 0.1f) {
            // Plus de pente, arrêt de la rivière
            break;
        }

        totalLength += distance;
        currentPos = nextPos;

        // Augmente légèrement la largeur et la profondeur au fur et à mesure
        currentWidth += 0.1f;
        currentDepth += 0.05f;

        // Limite les valeurs
        currentWidth = std::min(currentWidth, 10.0f);
        currentDepth = std::min(currentDepth, 3.0f);

        river.push_back({currentPos, currentWidth, currentDepth});

        // Arrêt si on atteint un océan ou une hauteur très basse
        if (terrainGen.getHeight(currentPos.x, currentPos.y) < -1.0f) {
            break;
        }
    }

    return river;
}

std::vector<std::vector<RiverPoint>> RiverGenerator::generateRivers(int width, int height, float scale,
                                                                  const TerrainGenerator& terrainGen, int numRivers) {
    std::vector<std::vector<RiverPoint>> rivers;

    for (int i = 0; i < numRivers; ++i) {
        // Choisit un point de départ aléatoire dans les hauteurs
        float startX = noise.perlin2D(i * 10.0f, 0.0f) * width * scale;
        float startZ = noise.perlin2D(0.0f, i * 10.0f) * height * scale;

        // S'assure que le point de départ est dans les limites et assez haut
        startX = std::clamp(startX, scale * 10.0f, scale * (width - 10.0f));
        startZ = std::clamp(startZ, scale * 10.0f, scale * (height - 10.0f));

        glm::vec2 startPos(startX, startZ);

        // Vérifie que la hauteur est suffisante pour une source de rivière
        if (terrainGen.getHeight(startPos.x, startPos.y) > 5.0f) {
            auto river = generateRiver(startPos, terrainGen);
            if (!river.empty() && river.size() > 10) { // Garde seulement les rivières significatives
                rivers.push_back(river);
            }
        }
    }

    return rivers;
}

void RiverGenerator::carveRivers(std::vector<std::vector<float>>& heightMap, const std::vector<std::vector<RiverPoint>>& rivers, float scale) {
    int mapWidth = heightMap[0].size();
    int mapHeight = heightMap.size();

    for (const auto& river : rivers) {
        for (size_t i = 0; i < river.size(); ++i) {
            const auto& point = river[i];
            int x = static_cast<int>(point.position.x / scale);
            int z = static_cast<int>(point.position.y / scale);

            if (x >= 0 && x < mapWidth && z >= 0 && z < mapHeight) {
                // Creuse la rivière
                heightMap[z][x] -= point.depth;

                // Creuse aussi les voisins pour créer une largeur
                int halfWidth = static_cast<int>(point.width / 2.0f / scale);
                for (int dx = -halfWidth; dx <= halfWidth; ++dx) {
                    for (int dz = -halfWidth; dz <= halfWidth; ++dz) {
                        int nx = x + dx;
                        int nz = z + dz;

                        if (nx >= 0 && nx < mapWidth && nz >= 0 && nz < mapHeight) {
                            // Distance du centre
                            float dist = std::sqrt(dx * dx + dz * dz);
                            if (dist <= halfWidth) {
                                // Creuse moins aux bords
                                float depthMultiplier = 1.0f - (dist / halfWidth);
                                heightMap[nz][nx] -= point.depth * depthMultiplier * 0.5f;
                            }
                        }
                    }
                }
            }
        }
    }
}

glm::vec2 RiverGenerator::findLowestNeighbor(const glm::vec2& pos, const TerrainGenerator& terrainGen, float stepSize) const {
    float currentHeight = terrainGen.getHeight(pos.x, pos.y);
    glm::vec2 lowestPos = pos;
    float lowestHeight = currentHeight;

    // Vérifie les 8 directions
    for (int dx = -1; dx <= 1; ++dx) {
        for (int dz = -1; dz <= 1; ++dz) {
            if (dx == 0 && dz == 0) continue;

            glm::vec2 testPos = pos + glm::vec2(dx * stepSize, dz * stepSize);
            float testHeight = terrainGen.getHeight(testPos.x, testPos.y);

            if (testHeight < lowestHeight) {
                lowestHeight = testHeight;
                lowestPos = testPos;
            }
        }
    }

    return lowestPos;
}

bool RiverGenerator::isValidPosition(const glm::vec2& pos, int width, int height, float scale) const {
    int x = static_cast<int>(pos.x / scale);
    int z = static_cast<int>(pos.y / scale);
    return x >= 0 && x < width && z >= 0 && z < height;
}

}