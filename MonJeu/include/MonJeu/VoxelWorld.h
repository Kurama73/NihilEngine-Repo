#pragma once
#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>
#include <NihilEngine/Entity.h>
#include <NihilEngine/Mesh.h>
#include <NihilEngine/Physics.h>
#include <memory>

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

class Chunk {
public:
    static const int SIZE = 16;
    Chunk(int chunkX, int chunkZ);
    ~Chunk() = default;

    void GenerateTerrain();
    NihilEngine::Mesh CreateMesh() const;

    Voxel& GetVoxel(int x, int y, int z);
    const Voxel& GetVoxel(int x, int y, int z) const;

    int GetChunkX() const { return m_ChunkX; }
    int GetChunkZ() const { return m_ChunkZ; }

private:
    int m_ChunkX, m_ChunkZ;
    std::vector<Voxel> m_Voxels;

    int GetIndex(int x, int y, int z) const;
    void AddVisibleFacesToMesh(std::vector<float>& vertices, std::vector<unsigned int>& indices, int x, int y, int z, BlockType type, const bool visible[6]) const;
};

class VoxelWorld {
public:
    VoxelWorld();
    ~VoxelWorld() = default;

    void GenerateChunk(int chunkX, int chunkZ);
    void UpdateDirtyChunks();
    void Render(NihilEngine::Renderer& renderer, const NihilEngine::Camera& camera);

    bool Raycast(const glm::vec3& origin, const glm::vec3& direction, float maxDistance, NihilEngine::RaycastHit& hitResult);

    void SetVoxelActive(int worldX, int worldY, int worldZ, bool active);
    bool GetVoxelActive(int worldX, int worldY, int worldZ) const;

    bool IsPositionValid(const glm::vec3& position, float height);

    // CORRIGÉ : Ajout de GetChunks()
    const std::unordered_map<uint64_t, std::unique_ptr<NihilEngine::Entity>>& GetChunks() const {
        return m_ChunkEntities;
    }

    int GetChunkCount() const { return m_Chunks.size(); }

    static void WorldToChunk(int worldX, int worldZ, int& chunkX, int& chunkZ);

private:
    std::unordered_map<uint64_t, std::unique_ptr<Chunk>> m_Chunks;
    std::unordered_map<uint64_t, std::unique_ptr<NihilEngine::Entity>> m_ChunkEntities;
    std::vector<uint64_t> m_DirtyChunks;

    uint64_t GetChunkKey(int chunkX, int chunkZ) const;
};

} // namespace MonJeu