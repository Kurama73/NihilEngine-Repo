// include/MonJeu/VoxelWorld.h
#pragma once

#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>
#include <NihilEngine/ProceduralGenerator.h>
#include <NihilEngine/Entity.h>
#include <NihilEngine/Physics.h>
#include <NihilEngine/ChunkDataCache.h>
#include <NihilEngine/ProgressiveChunkUpdate.h>
#include <memory>
#include "Chunk.h" // Utilise le nouveau header Chunk
#include "WorldSaveManager.h" // Gestionnaire de sauvegarde

#ifdef _WIN32
#include <glad/glad.h>
#endif

namespace NihilEngine {
    class Renderer;
    class Camera;
}

namespace MonJeu {

/**
 * @brief Gère l'état global du monde, y compris tous les chunks,
 * et orchestre les systèmes de LOD et de génération du moteur.
 */
class VoxelWorld {
public:
    VoxelWorld(unsigned int seed, NihilEngine::PhysicsWorld* physicsWorld, WorldSaveManager* saveManager = nullptr);
    ~VoxelWorld() = default;

    void SetTextureAtlas(GLuint textureAtlasID) { m_TextureAtlasID = textureAtlasID; }

    // --- API de jeu ---
    void SetVoxelActive(int worldX, int worldY, int worldZ, bool active);
    bool GetVoxelActive(int worldX, int worldY, int worldZ) const;
    bool Raycast(const glm::vec3& origin, const glm::vec3& direction, float maxDistance, NihilEngine::RaycastHit& hitResult);

    /**
     * @brief Vérifie la collision d'une AABB avec les voxels du monde.
     * Utilisé par le contrôleur de personnage.
     */
    bool CheckCollision(const NihilEngine::AABB& box) const;

    // --- Cycle de vie (appelé par Game.cpp) ---
    void Render(NihilEngine::Renderer& renderer, const NihilEngine::Camera& camera);
    void UpdateDirtyChunks();
    void UpdateLOD(const glm::vec3& cameraPosition, double deltaTime);

    /**
     * @brief Génère de manière synchrone les chunks prioritaires autour d'une position.
     * Utilisé pour le spawn du joueur afin d'éviter la chute dans le vide.
     */
    void GenerateSpawnArea(const glm::vec3& position, int radiusChunks = 3);

    // --- Accesseurs ---
    NihilEngine::ProceduralGenerator& GetProceduralGenerator() { return m_ProceduralGen; }
    int GetChunkCount() const { return m_Chunks.size(); }
    static void WorldToChunk(int worldX, int worldZ, int& chunkX, int& chunkZ);

private:
    // État du jeu
    std::unordered_map<uint64_t, std::unique_ptr<Chunk>> m_Chunks;
    std::unordered_map<uint64_t, std::unique_ptr<NihilEngine::Entity>> m_ChunkEntities;
    // std::vector<std::unordered_map<uint64_t, std::unique_ptr<NihilEngine::Entity>>> m_GrassTopEntities; - COMMENTE: suppression du système d'entités d'herbe
    std::vector<uint64_t> m_DirtyChunks;
    GLuint m_TextureAtlasID = 0;

    // Systèmes Moteur
    NihilEngine::ProceduralGenerator m_ProceduralGen;
    NihilEngine::ChunkDataCache m_ChunkDataCache;
    NihilEngine::ProgressiveChunkUpdate m_ProgressiveUpdate;
    NihilEngine::PhysicsWorld* m_PhysicsWorld; // Référence au monde physique

    // Distance d'affichage
    float m_DisplayDistance;

    // Système de sauvegarde
    WorldSaveManager* m_SaveManager;

    // Logique interne
    void GenerateChunk(int chunkX, int chunkZ);

    uint64_t GetChunkKey(int chunkX, int chunkZ) const;
};

} // namespace MonJeu