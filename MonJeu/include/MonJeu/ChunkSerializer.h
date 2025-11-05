// include/MonJeu/ChunkSerializer.h
#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include "Chunk.h"

namespace MonJeu {

/**
 * @brief Gère la sérialisation et désérialisation des données de chunk.
 * Format binaire simple pour la performance.
 */
class ChunkSerializer {
public:
    /**
     * @brief Structure pour les données sérialisées d'un chunk
     */
    struct SerializedChunk {
        int chunkX;
        int chunkZ;
        Constants::BiomeType biome;
        std::vector<uint8_t> voxelData; // Données compactées des voxels
        uint32_t version = 1; // Version du format pour la compatibilité future
    };

    /**
     * @brief Sérialise un chunk en données binaires
     */
    static std::vector<uint8_t> SerializeChunk(const Chunk& chunk);

    /**
     * @brief Désérialise des données binaires en structure SerializedChunk
     */
    static SerializedChunk DeserializeChunk(const std::vector<uint8_t>& data);

    /**
     * @brief Applique les données désérialisées à un chunk existant
     */
    static void ApplySerializedDataToChunk(Chunk& chunk, const SerializedChunk& data);

    /**
     * @brief Crée un nouveau chunk à partir des données sérialisées
     */
    static std::unique_ptr<Chunk> CreateChunkFromSerializedData(const SerializedChunk& data);

private:
    // Format binaire:
    // - Version (uint32_t)
    // - chunkX (int32_t)
    // - chunkZ (int32_t)
    // - biome (uint8_t)
    // - voxelData: pour chaque voxel (16*16*16):
    //   - type (uint8_t: 0=Air, 1=Grass, 2=Dirt, 3=Stone)
    //   - active (uint8_t: 0=false, 1=true)

    static constexpr size_t HEADER_SIZE = sizeof(uint32_t) + sizeof(int32_t) * 2 + sizeof(uint8_t);
    static constexpr size_t VOXEL_SIZE = 2; // type + active
    static constexpr size_t CHUNK_DATA_SIZE = Chunk::SIZE * Chunk::SIZE * Chunk::SIZE * VOXEL_SIZE;
    static constexpr size_t TOTAL_SIZE = HEADER_SIZE + CHUNK_DATA_SIZE;
};

} // namespace MonJeu