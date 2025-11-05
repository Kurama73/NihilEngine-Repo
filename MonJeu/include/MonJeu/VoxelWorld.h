#pragma once
#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>
#include <NihilEngine/ProceduralGenerator.h>
#include <NihilEngine/Entity.h>
#include <NihilEngine/Mesh.h>
#include <NihilEngine/Physics.h>
#include <memory>
#include "Constants.h"
#include <NihilEngine/LODSystem.h>
#include <NihilEngine/ChunkDataCache.h>
#include <NihilEngine/ProgressiveChunkUpdate.h>
#include <NihilEngine/LODMeshGenerator.h>

// Include OpenGL for GLuint
#ifdef _WIN32
#include <glad/glad.h>
#endif

namespace NihilEngine {
    class Renderer;
    class Camera;
}

namespace MonJeu {

enum class BlockType { Air, Grass, Dirt, Stone };

struct Voxel {
    BlockType type = BlockType::Air;
    bool active = false;
};

struct ChunkMeshes {
    std::unique_ptr<NihilEngine::Mesh> mainMesh;      // Toutes les faces sauf le top des blocs d'herbe
    std::vector<std::unique_ptr<NihilEngine::Mesh>> grassTopMeshes;  // Un mesh par biome pour les faces top d'herbe
};

class Chunk {
public:
    static const int SIZE = 16;
    Chunk(int chunkX, int chunkZ, Constants::BiomeType biome);
    ~Chunk() = default;

    // Modifié : Prend le générateur pour une génération dynamique
    void GenerateTerrain(NihilEngine::ProceduralGenerator& generator);
    ChunkMeshes CreateMeshes() const;

    Voxel& GetVoxel(int x, int y, int z);
    const Voxel& GetVoxel(int x, int y, int z) const;

    int GetChunkX() const { return m_ChunkX; }
    int GetChunkZ() const { return m_ChunkZ; }

    Constants::BiomeType GetBiome() const { return m_Biome; }

    static Constants::BiomeType GetBiomeAt(int worldX, int worldZ);

private:
    int m_ChunkX, m_ChunkZ;
    Constants::BiomeType m_Biome;
    std::vector<Voxel> m_Voxels;

    int GetIndex(int x, int y, int z) const;
    void AddVisibleFacesToMeshes(std::vector<float>& mainVertices, std::vector<unsigned int>& mainIndices,
                                std::vector<std::vector<float>>& grassTopVertices, std::vector<std::vector<unsigned int>>& grassTopIndices,
                                int x, int y, int z, BlockType type, const bool visible[6]) const;

    static Constants::BiomeType convertBiomeType(NihilEngine::BiomeType engineBiome);
};

class VoxelWorld {
public:
    VoxelWorld(unsigned int seed = 12345);
    ~VoxelWorld() = default;

    void SetTextureAtlas(GLuint textureAtlasID) { m_TextureAtlasID = textureAtlasID; }

    void GenerateChunk(int chunkX, int chunkZ);
    void UpdateDirtyChunks();
    void Render(NihilEngine::Renderer& renderer, const NihilEngine::Camera& camera);

    bool Raycast(const glm::vec3& origin, const glm::vec3& direction, float maxDistance, NihilEngine::RaycastHit& hitResult);

    void SetVoxelActive(int worldX, int worldY, int worldZ, bool active);
    bool GetVoxelActive(int worldX, int worldY, int worldZ) const;

    bool IsPositionValid(const glm::vec3& position, float height);

    const std::unordered_map<uint64_t, std::unique_ptr<NihilEngine::Entity>>& GetChunks() const {
        return m_ChunkEntities;
    }

    int GetChunkCount() const { return m_Chunks.size(); }

    static void WorldToChunk(int worldX, int worldZ, int& chunkX, int& chunkZ);

    // Méthodes LOD
    void UpdateLOD(const glm::vec3& cameraPosition, double deltaTime);
    void GenerateChunkLOD(int chunkX, int chunkZ, NihilEngine::ChunkLODLevel lodLevel);
    NihilEngine::ChunkLODLevel GetChunkLOD(int chunkX, int chunkZ, const glm::vec3& cameraPosition) const;
    void UpdateChunkMeshLOD(int chunkX, int chunkZ, NihilEngine::ChunkLODLevel newLOD);

    // Ajouté : Pour que main.cpp puisse trouver la hauteur de spawn
    NihilEngine::ProceduralGenerator& GetProceduralGenerator() { return m_ProceduralGen; }

private:
    std::unordered_map<uint64_t, std::unique_ptr<Chunk>> m_Chunks;
    std::unordered_map<uint64_t, std::unique_ptr<NihilEngine::Entity>> m_ChunkEntities;
    std::vector<std::unordered_map<uint64_t, std::unique_ptr<NihilEngine::Entity>>> m_GrassTopEntities; // Un map par biome
    std::vector<uint64_t> m_DirtyChunks;
    GLuint m_TextureAtlasID = 0;

    // Système de génération procédurale
    NihilEngine::ProceduralGenerator m_ProceduralGen;
    // Supprimé : m_WorldData n'est plus statique
    // Supprimé : m_ProceduralGenerator (doublon)

    // Système LOD
    NihilEngine::LODSystem m_LODSystem;
    NihilEngine::ChunkDataCache m_ChunkDataCache;
    NihilEngine::ProgressiveChunkUpdate m_ProgressiveUpdate;
    NihilEngine::LODMeshGenerator m_LODMeshGenerator;

    // Ajouté : Pour suivre le LOD de chaque chunk
    std::unordered_map<uint64_t, NihilEngine::ChunkLODLevel> m_ChunkLODLevels;


    uint64_t GetChunkKey(int chunkX, int chunkZ) const;
};

} // namespace MonJeu