#include <NihilEngine/ProgressiveChunkUpdate.h>
#include <algorithm>
#include <chrono>

namespace NihilEngine {

ProgressiveChunkUpdate::ProgressiveChunkUpdate()
    : m_updatesPerFrame(2), m_maxPendingUpdates(100), m_processedUpdatesCount(0) {
}

void ProgressiveChunkUpdate::setUpdateRate(int updatesPerFrame) {
    m_updatesPerFrame = std::max(1, updatesPerFrame);
}

void ProgressiveChunkUpdate::setMaxPendingUpdates(int maxPending) {
    m_maxPendingUpdates = std::max(10, maxPending);
}

void ProgressiveChunkUpdate::requestChunkUpdate(int chunkX, int chunkZ, double priority) {
    uint64_t key = getChunkKey(chunkX, chunkZ);

    // Vérifie si une demande existe déjà
    auto it = m_pendingRequests.find(key);
    if (it != m_pendingRequests.end()) {
        // Met à jour la priorité si elle est plus élevée
        if (priority > it->second.priority) {
            it->second.priority = priority;
            it->second.requestTime = std::chrono::duration<double>(
                std::chrono::high_resolution_clock::now().time_since_epoch()).count();
        }
        return;
    }

    // Limite le nombre de demandes en attente
    if (static_cast<int>(m_pendingRequests.size()) >= m_maxPendingUpdates) {
        // Supprime la demande la moins prioritaire
        auto oldestIt = std::min_element(m_pendingRequests.begin(), m_pendingRequests.end(),
            [](const auto& a, const auto& b) { return a.second.priority < b.second.priority; });
        if (oldestIt != m_pendingRequests.end()) {
            m_pendingRequests.erase(oldestIt);
        }
    }

    ChunkUpdateRequest request;
    request.chunkX = chunkX;
    request.chunkZ = chunkZ;
    request.priority = priority;
    request.requestTime = std::chrono::duration<double>(
        std::chrono::high_resolution_clock::now().time_since_epoch()).count();

    m_updateQueue.push(request);
    m_pendingRequests[key] = request;
}

void ProgressiveChunkUpdate::cancelChunkUpdate(int chunkX, int chunkZ) {
    uint64_t key = getChunkKey(chunkX, chunkZ);
    m_pendingRequests.erase(key);

    // Reconstruit la file de priorité sans cette demande
    std::priority_queue<ChunkUpdateRequest> newQueue;
    while (!m_updateQueue.empty()) {
        ChunkUpdateRequest req = m_updateQueue.top();
        m_updateQueue.pop();
        uint64_t reqKey = getChunkKey(req.chunkX, req.chunkZ);
        if (reqKey != key) {
            newQueue.push(req);
        }
    }
    m_updateQueue = std::move(newQueue);
}

void ProgressiveChunkUpdate::updateChunks(double deltaTime,
                                         const glm::vec3& cameraPosition,
                                         ChunkDataCache& cache,
                                         std::function<void(int, int)> updateCallback) {
    int updatesThisFrame = 0;

    while (!m_updateQueue.empty() && updatesThisFrame < m_updatesPerFrame) {
        ChunkUpdateRequest request = m_updateQueue.top();
        m_updateQueue.pop();

        uint64_t key = getChunkKey(request.chunkX, request.chunkZ);

        // Vérifie si la demande est toujours valide
        auto it = m_pendingRequests.find(key);
        if (it == m_pendingRequests.end()) {
            continue; // Demande annulée
        }

        // Effectue la mise à jour
        updateCallback(request.chunkX, request.chunkZ);

        // Supprime de la liste des demandes en attente
        m_pendingRequests.erase(key);

        updatesThisFrame++;
        m_processedUpdatesCount++;
    }
}

int ProgressiveChunkUpdate::getPendingUpdatesCount() const {
    return static_cast<int>(m_pendingRequests.size());
}

int ProgressiveChunkUpdate::getProcessedUpdatesCount() const {
    return m_processedUpdatesCount;
}

void ProgressiveChunkUpdate::resetProcessedUpdatesCount() {
    m_processedUpdatesCount = 0;
}

uint64_t ProgressiveChunkUpdate::getChunkKey(int chunkX, int chunkZ) const {
    return (static_cast<uint64_t>(chunkX) << 32) | static_cast<uint64_t>(chunkZ);
}

}