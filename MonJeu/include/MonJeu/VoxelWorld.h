#pragma once

#include <vector>
#include <unordered_map>
#include <string> // Pour std::hash
#include <glm/glm.hpp>
#include <NihilEngine/Entity.h>
#include <NihilEngine/Mesh.h>
#include <NihilEngine/Physics.h> // <- Inclure pour RaycastHit
#include <memory>

// Fwd declarations
namespace NihilEngine {
    class Renderer;
    class Camera;
}

namespace MonJeu {

    // Types de blocs
    enum class BlockType {
        Air,
        Grass,
        Dirt,
        Stone
    };

    // Un voxel individuel
    struct Voxel {
        BlockType type = BlockType::Air;
        bool active = false;
    };

    // Chunk de 16x16x16 voxels
    class Chunk {
    public:
        static const int SIZE = 16;

        Chunk();
        Chunk(int chunkX, int chunkZ);
        ~Chunk() = default;

        // Génération procédurale simple
        void GenerateTerrain();

        // Créer le mesh du chunk (seulement faces visibles)
        NihilEngine::Mesh CreateMesh() const;

        // Accès aux voxels
        Voxel& GetVoxel(int x, int y, int z);
        const Voxel& GetVoxel(int x, int y, int z) const;

        // Getters
        int GetChunkX() const { return m_ChunkX; }
        int GetChunkZ() const { return m_ChunkZ; }

    private:
        int m_ChunkX, m_ChunkZ;
        std::vector<Voxel> m_Voxels; // SIZE*SIZE*SIZE

        // Helper pour indexer
        int GetIndex(int x, int y, int z) const;

        // Ajouter les faces visibles d'un cube au mesh
        void AddVisibleFacesToMesh(std::vector<float>& vertices, std::vector<unsigned int>& indices, int x, int y, int z, BlockType type, const bool visible[6]) const;
    };

    // Monde voxel gérant plusieurs chunks
    class VoxelWorld {
    public:
        VoxelWorld();
        ~VoxelWorld() = default;

        void GenerateChunk(int chunkX, int chunkZ);
        void UpdateDirtyChunks();
        void Render(NihilEngine::Renderer& renderer, const NihilEngine::Camera& camera);

        const std::unordered_map<uint64_t, Chunk>& GetChunks() const;

        static void WorldToChunk(int worldX, int worldZ, int& chunkX, int& chunkZ);

        // --- NOUVEAU RAYCAST ---
        // Utilise la structure RaycastHit du moteur
        bool Raycast(const glm::vec3& origin, const glm::vec3& direction, float maxDistance, NihilEngine::RaycastHit& result);

        void SetVoxelActive(int worldX, int worldY, int worldZ, bool active);
        bool GetVoxelActive(int worldX, int worldY, int worldZ) const;

    private:
        std::unordered_map<uint64_t, Chunk> m_Chunks; // Key: (chunkX << 32) | chunkZ
        uint64_t GetChunkKey(int chunkX, int chunkZ) const;

        // --- MEMBRES AJOUTÉS ---
        // (Ils étaient dans ton .cpp mais manquaient ici)
        std::unordered_map<uint64_t, std::unique_ptr<NihilEngine::Entity>> m_ChunkEntities;
        std::vector<uint64_t> m_DirtyChunks;
    };

}