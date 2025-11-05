// src/ChunkSerializer.cpp
#include <MonJeu/ChunkSerializer.h>
#include <iostream>
#include <cstring>

namespace MonJeu {

std::vector<uint8_t> ChunkSerializer::SerializeChunk(const Chunk& chunk) {
    std::vector<uint8_t> data(TOTAL_SIZE);

    size_t offset = 0;

    // Version
    uint32_t version = 1;
    std::memcpy(data.data() + offset, &version, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    // chunkX
    int32_t chunkX = chunk.GetChunkX();
    std::memcpy(data.data() + offset, &chunkX, sizeof(int32_t));
    offset += sizeof(int32_t);

    // chunkZ
    int32_t chunkZ = chunk.GetChunkZ();
    std::memcpy(data.data() + offset, &chunkZ, sizeof(int32_t));
    offset += sizeof(int32_t);

    // biome
    uint8_t biome = static_cast<uint8_t>(chunk.GetBiome());
    std::memcpy(data.data() + offset, &biome, sizeof(uint8_t));
    offset += sizeof(uint8_t);

    // voxelData
    for (int y = 0; y < Chunk::SIZE; ++y) {
        for (int z = 0; z < Chunk::SIZE; ++z) {
            for (int x = 0; x < Chunk::SIZE; ++x) {
                const Voxel& voxel = chunk.GetVoxel(x, y, z);

                // type
                uint8_t type = static_cast<uint8_t>(voxel.type);
                data[offset++] = type;

                // active
                uint8_t active = voxel.active ? 1 : 0;
                data[offset++] = active;
            }
        }
    }

    return data;
}

ChunkSerializer::SerializedChunk ChunkSerializer::DeserializeChunk(const std::vector<uint8_t>& data) {
    SerializedChunk result;
    size_t offset = 0;

    if (data.size() != TOTAL_SIZE) {
        throw std::runtime_error("Invalid chunk data size");
    }

    // Version
    uint32_t version;
    std::memcpy(&version, data.data() + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    if (version != 1) {
        throw std::runtime_error("Unsupported chunk data version");
    }
    result.version = version;

    // chunkX
    int32_t chunkX;
    std::memcpy(&chunkX, data.data() + offset, sizeof(int32_t));
    offset += sizeof(int32_t);
    result.chunkX = chunkX;

    // chunkZ
    int32_t chunkZ;
    std::memcpy(&chunkZ, data.data() + offset, sizeof(int32_t));
    offset += sizeof(int32_t);
    result.chunkZ = chunkZ;

    // biome
    uint8_t biome;
    std::memcpy(&biome, data.data() + offset, sizeof(uint8_t));
    offset += sizeof(uint8_t);
    result.biome = static_cast<Constants::BiomeType>(biome);

    // voxelData
    result.voxelData.resize(CHUNK_DATA_SIZE);
    std::memcpy(result.voxelData.data(), data.data() + offset, CHUNK_DATA_SIZE);

    return result;
}

void ChunkSerializer::ApplySerializedDataToChunk(Chunk& chunk, const SerializedChunk& data) {
    if (data.chunkX != chunk.GetChunkX() || data.chunkZ != chunk.GetChunkZ()) {
        throw std::runtime_error("Chunk coordinates mismatch");
    }

    size_t offset = 0;
    for (int y = 0; y < Chunk::SIZE; ++y) {
        for (int z = 0; z < Chunk::SIZE; ++z) {
            for (int x = 0; x < Chunk::SIZE; ++x) {
                Voxel& voxel = chunk.GetVoxel(x, y, z);

                // type
                uint8_t type = data.voxelData[offset++];
                voxel.type = static_cast<BlockType>(type);

                // active
                uint8_t active = data.voxelData[offset++];
                voxel.active = (active != 0);
            }
        }
    }
}

std::unique_ptr<Chunk> ChunkSerializer::CreateChunkFromSerializedData(const SerializedChunk& data) {
    auto chunk = std::make_unique<Chunk>(data.chunkX, data.chunkZ, data.biome);

    size_t offset = 0;
    for (int y = 0; y < Chunk::SIZE; ++y) {
        for (int z = 0; z < Chunk::SIZE; ++z) {
            for (int x = 0; x < Chunk::SIZE; ++x) {
                Voxel& voxel = chunk->GetVoxel(x, y, z);

                // type
                uint8_t type = data.voxelData[offset++];
                voxel.type = static_cast<BlockType>(type);

                // active
                uint8_t active = data.voxelData[offset++];
                voxel.active = (active != 0);
            }
        }
    }

    return chunk;
}

} // namespace MonJeu