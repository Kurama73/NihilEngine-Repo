// include/MonJeu/Chunk.h
#pragma once

#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include <NihilEngine/Mesh.h>
#include <NihilEngine/ProceduralGenerator.h>
#include "Constants.h"

namespace MonJeu {

// Type de bloc simple pour le jeu
enum class BlockType { Air, Grass, Dirt, Stone };

// Voxel de jeu
struct Voxel {
    BlockType type = BlockType::Air;
    bool active = false;
};

// Contient les meshes nécessaires pour un chunk
// (herbe intégrée directement dans le mainMesh)
struct ChunkMeshes {
    std::unique_ptr<NihilEngine::Mesh> mainMesh;
    // grassTopMeshes supprimé - herbe intégrée dans mainMesh
};

/**
 * @brief Représente une section 16x16x16 du monde de voxels.
 */
class Chunk {
public:
    static const int SIZE = 16;

    Chunk(int chunkX, int chunkZ, Constants::BiomeType biome);
    ~Chunk() = default;

    /**
     * @brief Remplit les données de voxel en utilisant le générateur procédural.
     */
    void GenerateTerrain(NihilEngine::ProceduralGenerator& generator);

    /**
     * @brief Construit les meshes visibles pour ce chunk.
     */
    ChunkMeshes CreateMeshes() const;

    // Accesseurs
    Voxel& GetVoxel(int x, int y, int z);
    const Voxel& GetVoxel(int x, int y, int z) const;
    int GetChunkX() const { return m_ChunkX; }
    int GetChunkZ() const { return m_ChunkZ; }
    Constants::BiomeType GetBiome() const { return m_Biome; }

    /**
     * @brief Détermine le biome pour une coordonnée monde (simplifié).
     */
    static Constants::BiomeType GetBiomeAt(int worldX, int worldZ);

private:
    int m_ChunkX, m_ChunkZ;
    Constants::BiomeType m_Biome;
    std::vector<Voxel> m_Voxels;

    int GetIndex(int x, int y, int z) const;

    /**
     * @brief Ajoute les faces d'un voxel aux buffers de mesh appropriés.
     */
    void AddVisibleFacesToMeshes(
        std::vector<float>& mainVertices,
        std::vector<unsigned int>& mainIndices,
        int x, int y, int z, BlockType type, const bool visible[6]
    ) const;

    /**
     * @brief Convertit un BiomeType du moteur en BiomeType du jeu.
     */
    static Constants::BiomeType convertBiomeType(NihilEngine::BiomeType engineBiome);
};

} // namespace MonJeu