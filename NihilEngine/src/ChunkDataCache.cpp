#include <NihilEngine/ChunkDataCache.h>
#include <algorithm>
#include <limits>

namespace NihilEngine {

ChunkDataCache::ChunkDataCache() = default;

void ChunkDataCache::updateChunkData(int chunkX, int chunkZ, const SimplifiedChunkData& data) {
    uint64_t key = getCacheKey(chunkX, chunkZ);
    m_cache[key] = data;
}

const SimplifiedChunkData* ChunkDataCache::getChunkData(int chunkX, int chunkZ) const {
    uint64_t key = getCacheKey(chunkX, chunkZ);
    auto it = m_cache.find(key);
    if (it != m_cache.end()) {
        return &it->second;
    }
    return nullptr;
}

bool ChunkDataCache::isChunkDataUpToDate(int chunkX, int chunkZ, double currentTime, double maxAge) const {
    const SimplifiedChunkData* data = getChunkData(chunkX, chunkZ);
    if (!data) return false;

    return (currentTime - data->lastUpdateTime) <= maxAge;
}

void ChunkDataCache::cleanupOldData(double currentTime, double maxAge) {
    for (auto it = m_cache.begin(); it != m_cache.end(); ) {
        if ((currentTime - it->second.lastUpdateTime) > maxAge) {
            it = m_cache.erase(it);
        } else {
            ++it;
        }
    }
}

SimplifiedChunkData ChunkDataCache::generateSimplifiedData(
    int chunkX, int chunkZ,
    const std::vector<std::vector<float>>& heightMap,
    const std::vector<std::vector<int>>& biomeMap,
    ChunkLODLevel targetLOD
) {
    SimplifiedChunkData data;
    data.position = glm::vec3(chunkX * 16.0f, 0.0f, chunkZ * 16.0f); // 16 = chunk size
    data.lodLevel = targetLOD;

    // Calcul des statistiques de hauteur
    float minH = std::numeric_limits<float>::max();
    float maxH = std::numeric_limits<float>::min();
    float sumH = 0.0f;
    int count = 0;

    // Compte des biomes
    std::unordered_map<int, int> biomeCounts;

    // --- CORRECTION ICI ---
    // Les cartes 'heightMap' et 'biomeMap' sont locales (16x16).
    // Nous devons itérer de 0 à la taille de la carte, pas utiliser les coordonnées globales.

    int mapHeight = static_cast<int>(heightMap.size());
    if (mapHeight == 0) return data; // Sécurité
    int mapWidth = static_cast<int>(heightMap[0].size());
    if (mapWidth == 0) return data; // Sécurité

    for (int z = 0; z < mapHeight; ++z) {
        for (int x = 0; x < mapWidth; ++x) {
            float h = heightMap[z][x];
            minH = std::min(minH, h);
            maxH = std::max(maxH, h);
            sumH += h;
            count++;

            // Compte les biomes
            if (z < static_cast<int>(biomeMap.size()) && x < static_cast<int>(biomeMap[z].size())) {
                biomeCounts[biomeMap[z][x]]++;
            }
        }
    }
    // --- FIN DE LA CORRECTION ---


    if (count > 0) {
        data.averageHeight = sumH / count;
        data.minHeight = minH;
        data.maxHeight = maxH;

        // Trouve le biome dominant
        int maxCount = 0;
        for (const auto& pair : biomeCounts) {
            if (pair.second > maxCount) {
                maxCount = pair.second;
                data.dominantBiome = pair.first;
            }
        }

        // Couleur basée sur le biome et la hauteur
        // TODO: Mapper les biomes aux couleurs
        data.color = glm::vec3(0.5f, 0.5f, 0.5f); // Couleur par défaut

        // Échelle basée sur la variation de hauteur
        float heightRange = maxH - minH;
        data.scale = glm::vec3(16.0f, std::max(heightRange, 1.0f), 16.0f);
    }

    return data;
}

uint64_t ChunkDataCache::getCacheKey(int chunkX, int chunkZ) {
    // Combine chunkX et chunkZ en une clé 64-bit
    return (static_cast<uint64_t>(chunkX) << 32) | static_cast<uint64_t>(chunkZ);
}

}