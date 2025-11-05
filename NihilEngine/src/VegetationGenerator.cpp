#include <NihilEngine/VegetationGenerator.h>
#include <algorithm>
#include <cmath>

namespace NihilEngine {

VegetationGenerator::VegetationGenerator(unsigned int seed)
    : noise(seed + 4), maxInstances(10000) {
    // Initialise les densités par défaut
    biomeDensities.resize(static_cast<size_t>(BiomeType::Swamp) + 1);
    biomeDensities[static_cast<size_t>(BiomeType::Plains)] = 0.3f;
    biomeDensities[static_cast<size_t>(BiomeType::Forest)] = 0.8f;
    biomeDensities[static_cast<size_t>(BiomeType::Desert)] = 0.05f;
    biomeDensities[static_cast<size_t>(BiomeType::Tundra)] = 0.1f;
    biomeDensities[static_cast<size_t>(BiomeType::Mountains)] = 0.2f;
    biomeDensities[static_cast<size_t>(BiomeType::Swamp)] = 0.4f;
    biomeDensities[static_cast<size_t>(BiomeType::Beach)] = 0.1f;
    biomeDensities[static_cast<size_t>(BiomeType::Ocean)] = 0.0f;
}

std::vector<VegetationInstance> VegetationGenerator::generateVegetation(int width, int height, float scale,
                                                                       const std::vector<std::vector<BiomeType>>& biomeMap,
                                                                       const std::vector<std::vector<float>>& heightMap) {
    std::vector<VegetationInstance> vegetation;

    // Génère de la végétation par biome
    for (size_t biomeIdx = 0; biomeIdx < biomeDensities.size(); ++biomeIdx) {
        BiomeType biome = static_cast<BiomeType>(biomeIdx);
        float density = biomeDensities[biomeIdx];

        if (density <= 0.0f) continue;

        // Collecte les régions de ce biome
        std::vector<std::pair<int, int>> biomePositions;
        for (int z = 0; z < height; ++z) {
            for (int x = 0; x < width; ++x) {
                if (biomeMap[z][x] == biome) {
                    biomePositions.emplace_back(x, z);
                }
            }
        }

        // Échantillonne la végétation
        int numSamples = static_cast<int>(biomePositions.size() * density);
        numSamples = std::min(numSamples, maxInstances / static_cast<int>(biomeDensities.size()));

        for (int i = 0; i < numSamples && !biomePositions.empty(); ++i) {
            // Choisit une position aléatoire dans le biome
            int randomIdx = static_cast<int>(noise.perlin2D(i * 100.0f, biomeIdx * 50.0f) * biomePositions.size());
            randomIdx = std::clamp(randomIdx, 0, static_cast<int>(biomePositions.size()) - 1);

            auto [x, z] = biomePositions[randomIdx];
            float worldX = x * scale;
            float worldZ = z * scale;
            float height = heightMap[z][x];

            // Génère le type de végétation
            float randomValue = noise.perlin2D(worldX * 0.1f, worldZ * 0.1f);
            VegetationType type = getVegetationType(biome, randomValue);

            if (type != VegetationType::None) {
                glm::vec3 position(worldX, height, worldZ);

                if (isValidVegetationPosition(position, type, heightMap, width, height, scale)) {
                    float scale = 0.8f + noise.perlin2D(worldX * 0.05f, worldZ * 0.05f) * 0.4f;
                    float rotation = noise.perlin2D(worldX * 0.02f, worldZ * 0.02f) * 360.0f;

                    vegetation.push_back({type, position, scale, rotation});
                }
            }

            // Retire cette position pour éviter les clusters
            biomePositions.erase(biomePositions.begin() + randomIdx);
        }
    }

    return vegetation;
}

void VegetationGenerator::setDensity(BiomeType biome, float density) {
    biomeDensities[static_cast<size_t>(biome)] = std::clamp(density, 0.0f, 1.0f);
}

VegetationType VegetationGenerator::getVegetationType(BiomeType biome, float randomValue) const {
    switch (biome) {
        case BiomeType::Plains:
            if (randomValue < 0.7f) return VegetationType::Grass;
            else if (randomValue < 0.9f) return VegetationType::Flowers;
            else return VegetationType::Trees;
        case BiomeType::Forest:
            if (randomValue < 0.8f) return VegetationType::Trees;
            else return VegetationType::Bushes;
        case BiomeType::Desert:
            if (randomValue < 0.1f) return VegetationType::Cactus;
            else return VegetationType::None;
        case BiomeType::Tundra:
            if (randomValue < 0.3f) return VegetationType::Grass;
            else return VegetationType::None;
        case BiomeType::Mountains:
            if (randomValue < 0.2f) return VegetationType::Trees;
            else return VegetationType::Bushes;
        case BiomeType::Swamp:
            if (randomValue < 0.5f) return VegetationType::Reeds;
            else if (randomValue < 0.8f) return VegetationType::Bushes;
            else return VegetationType::Trees;
        case BiomeType::Beach:
            if (randomValue < 0.2f) return VegetationType::Grass;
            else return VegetationType::None;
        default:
            return VegetationType::None;
    }
}

bool VegetationGenerator::isValidVegetationPosition(const glm::vec3& pos, VegetationType type,
                                                   const std::vector<std::vector<float>>& heightMap,
                                                   int mapWidth, int mapHeight, float scale) const {
    // Vérifie les limites
    int x = static_cast<int>(pos.x / scale);
    int z = static_cast<int>(pos.z / scale);

    if (x < 0 || x >= mapWidth || z < 0 || z >= mapHeight) {
        return false;
    }

    float terrainHeight = heightMap[z][x];

    // Vérifie que la position est sur le terrain (pas dans l'eau profonde)
    if (terrainHeight < -1.0f) {
        return false;
    }

    // Pour les arbres, vérifie qu'il y a assez d'espace
    if (type == VegetationType::Trees) {
        // Vérifie les voisins pour éviter les clusters
        for (int dx = -2; dx <= 2; ++dx) {
            for (int dz = -2; dz <= 2; ++dz) {
                if (dx == 0 && dz == 0) continue;

                int nx = x + dx;
                int nz = z + dz;

                if (nx >= 0 && nx < mapWidth && nz >= 0 && nz < mapHeight) {
                    // Si il y a déjà un arbre proche, skip
                    // Dans une vraie implémentation, on garderait une carte des positions occupées
                }
            }
        }
    }

    return true;
}

}