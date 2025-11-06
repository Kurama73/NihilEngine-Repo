#pragma once

#include <unordered_map>
#include <memory>
#include <vector>
#include <glm/glm.hpp>

namespace NihilEngine {

// Structure pour stocker les données simplifiées d'un chunk lointain
struct SimplifiedChunkData {
    glm::vec3 position;
    float averageHeight = 0.0f;
    float minHeight = 0.0f;
    float maxHeight = 0.0f;
    int dominantBiome = 0; // Index du biome dominant

    // Pour LOD très faible : représenter le chunk comme un seul bloc
    glm::vec3 color; // Couleur moyenne du chunk
    glm::vec3 scale; // Échelle du bloc représentatif

    // Timestamp de dernière mise à jour
    double lastUpdateTime = 0.0;
};

class ChunkDataCache {
public:
    ChunkDataCache();
    ~ChunkDataCache() = default;

    // Ajoute ou met à jour les données simplifiées d'un chunk
    void updateChunkData(int chunkX, int chunkZ, const SimplifiedChunkData& data);

    // Récupère les données simplifiées d'un chunk
    const SimplifiedChunkData* getChunkData(int chunkX, int chunkZ) const;

    // Vérifie si les données d'un chunk sont à jour
    bool isChunkDataUpToDate(int chunkX, int chunkZ, double currentTime, double maxAge) const;

    // Nettoie les anciennes données
    void cleanupOldData(double currentTime, double maxAge);

    // Génère des données simplifiées à partir de données détaillées
    SimplifiedChunkData generateSimplifiedData(
        int chunkX, int chunkZ,
        const std::vector<std::vector<float>>& heightMap,
        const std::vector<std::vector<int>>& biomeMap
    );

    // Obtient la clé de cache pour un chunk
    static uint64_t getCacheKey(int chunkX, int chunkZ);

private:
    std::unordered_map<uint64_t, SimplifiedChunkData> m_cache;

    // Statistiques
    size_t m_cacheHits = 0;
    size_t m_cacheMisses = 0;
};

}