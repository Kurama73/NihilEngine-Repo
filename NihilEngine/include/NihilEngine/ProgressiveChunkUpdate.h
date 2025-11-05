#pragma once

#include <NihilEngine/LODSystem.h>
#include <NihilEngine/ChunkDataCache.h>
#include <glm/glm.hpp>
#include <vector>
#include <queue>
#include <unordered_map>
#include <functional>

namespace NihilEngine {

struct ChunkUpdateRequest {
    int chunkX, chunkZ;
    ChunkLODLevel targetLOD;
    double priority; // Plus la priorité est élevée, plus tôt le chunk sera mis à jour
    double requestTime;

    bool operator<(const ChunkUpdateRequest& other) const {
        return priority < other.priority; // Pour la priority_queue (max-heap)
    }
};

class ProgressiveChunkUpdate {
public:
    ProgressiveChunkUpdate();
    ~ProgressiveChunkUpdate() = default;

    // Configuration
    void setUpdateRate(int updatesPerFrame);
    void setMaxPendingUpdates(int maxPending);

    // Gestion des demandes de mise à jour
    void requestChunkUpdate(int chunkX, int chunkZ, ChunkLODLevel targetLOD, double priority = 1.0);
    void cancelChunkUpdate(int chunkX, int chunkZ);

    // Mise à jour progressive
    void updateChunks(double deltaTime,
                     const glm::vec3& cameraPosition,
                     LODSystem& lodSystem,
                     ChunkDataCache& cache,
                     std::function<void(int, int, ChunkLODLevel)> updateCallback);

    // Statistiques
    int getPendingUpdatesCount() const;
    int getProcessedUpdatesCount() const;
    void resetProcessedUpdatesCount();

private:
    // File de priorité pour les demandes de mise à jour
    std::priority_queue<ChunkUpdateRequest> m_updateQueue;

    // Map pour annuler rapidement les demandes
    std::unordered_map<uint64_t, ChunkUpdateRequest> m_pendingRequests;

    // Statistiques
    int m_updatesPerFrame;
    int m_maxPendingUpdates;
    int m_processedUpdatesCount;

    // Génération de clé pour les chunks
    uint64_t getChunkKey(int chunkX, int chunkZ) const;
};

}