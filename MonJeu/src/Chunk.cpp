// src/Chunk.cpp
#include <MonJeu/Chunk.h>
#include <array>
#include <cmath>

namespace MonJeu {

Chunk::Chunk(int chunkX, int chunkZ, Constants::BiomeType biome)
    : m_ChunkX(chunkX), m_ChunkZ(chunkZ), m_Biome(biome), m_Voxels(SIZE * SIZE * SIZE) {}

// Logique de génération de terrain (extraite de VoxelWorld.cpp)
void Chunk::GenerateTerrain(NihilEngine::ProceduralGenerator& generator) {
    NihilEngine::TerrainGenerator& terrainGen = generator.getTerrainGenerator();
    NihilEngine::BiomeGenerator& biomeGen = generator.getBiomeGenerator();

    for (int x = 0; x < SIZE; ++x) {
        for (int z = 0; z < SIZE; ++z) {
            int worldX = m_ChunkX * SIZE + x;
            int worldZ = m_ChunkZ * SIZE + z;

            float terrainHeight = terrainGen.getHeight(static_cast<float>(worldX), static_cast<float>(worldZ));
            int height = static_cast<int>(terrainHeight);

            NihilEngine::BiomeType biome = biomeGen.getBiome(static_cast<float>(worldX), static_cast<float>(worldZ), terrainHeight);
            m_Biome = convertBiomeType(biome);

            for (int y = 0; y < SIZE; ++y) {
                Voxel& voxel = GetVoxel(x, y, z);
                if (y < height - 3) {
                    voxel.type = BlockType::Stone;
                    voxel.active = true;
                } else if (y < height) {
                    voxel.type = BlockType::Dirt;
                    voxel.active = true;
                } else if (y == height) {
                    voxel.type = BlockType::Grass;
                    voxel.active = true;
                } else {
                    voxel.type = BlockType::Air;
                    voxel.active = false;
                }
            }
        }
    }
}

Constants::BiomeType Chunk::convertBiomeType(NihilEngine::BiomeType engineBiome) {
    switch (engineBiome) {
        case NihilEngine::BiomeType::Plains: return Constants::BiomeType::Plains;
        case NihilEngine::BiomeType::Forest: return Constants::BiomeType::Forest;
        case NihilEngine::BiomeType::Desert: return Constants::BiomeType::Desert;
        case NihilEngine::BiomeType::Tundra: return Constants::BiomeType::Tundra;
        case NihilEngine::BiomeType::Swamp: return Constants::BiomeType::Swamp;
        default: return Constants::BiomeType::Plains;
    }
}

// Logique de création de mesh (extraite de VoxelWorld.cpp)
ChunkMeshes Chunk::CreateMeshes() const {
    std::vector<float> mainVertices;
    std::vector<unsigned int> mainIndices;
    std::vector<std::vector<float>> grassTopVertices(5);
    std::vector<std::vector<unsigned int>> grassTopIndices(5);

    for (int x = 0; x < SIZE; ++x) {
        for (int y = 0; y < SIZE; ++y) {
            for (int z = 0; z < SIZE; ++z) {
                const Voxel& voxel = GetVoxel(x, y, z);
                if (!voxel.active) continue;

                bool visible[6] = {true, true, true, true, true, true}; // +Z, -Z, -X, +X, +Y, -Y

                if (z + 1 < SIZE) visible[0] = !GetVoxel(x, y, z + 1).active;
                if (z - 1 >= 0) visible[1] = !GetVoxel(x, y, z - 1).active;
                if (x - 1 >= 0) visible[2] = !GetVoxel(x - 1, y, z).active;
                if (x + 1 < SIZE) visible[3] = !GetVoxel(x + 1, y, z).active;
                if (y + 1 < SIZE) visible[4] = !GetVoxel(x, y + 1, z).active;
                if (y - 1 >= 0) visible[5] = !GetVoxel(x, y - 1, z).active;

                AddVisibleFacesToMeshes(mainVertices, mainIndices, grassTopVertices, grassTopIndices, x, y, z, voxel.type, visible);
            }
        }
    }

    ChunkMeshes meshes;
    meshes.mainMesh = std::make_unique<NihilEngine::Mesh>(mainVertices, mainIndices, std::vector<NihilEngine::VertexAttribute>{NihilEngine::VertexAttribute::Position, NihilEngine::VertexAttribute::Normal, NihilEngine::VertexAttribute::TexCoord});

    for (int i = 0; i < 5; ++i) {
        if (!grassTopVertices[i].empty()) {
            meshes.grassTopMeshes.emplace_back(std::make_unique<NihilEngine::Mesh>(grassTopVertices[i], grassTopIndices[i],
                std::vector<NihilEngine::VertexAttribute>{NihilEngine::VertexAttribute::Position, NihilEngine::VertexAttribute::Normal, NihilEngine::VertexAttribute::TexCoord}));
        } else {
            // ==================================================================
            // CORRECTION : Créer un mesh vide (le constructeur du moteur le gère)
            // ==================================================================
            meshes.grassTopMeshes.emplace_back(std::make_unique<NihilEngine::Mesh>(std::vector<float>{}, std::vector<unsigned int>{},
                std::vector<NihilEngine::VertexAttribute>{NihilEngine::VertexAttribute::Position, NihilEngine::VertexAttribute::Normal, NihilEngine::VertexAttribute::TexCoord}));
        }
    }
    return meshes;
}

// Logique d'ajout de faces (extraite de VoxelWorld.cpp)
void Chunk::AddVisibleFacesToMeshes(std::vector<float>& mainVertices, std::vector<unsigned int>& mainIndices,
                                   std::vector<std::vector<float>>& grassTopVertices, std::vector<std::vector<unsigned int>>& grassTopIndices,
                                   int x, int y, int z, BlockType type, const bool visible[6]) const {

    // [Logique de AddVisibleFacesToMeshes - Inchangée]
    float px = static_cast<float>(x), py = static_cast<float>(y), pz = static_cast<float>(z);

    float side_u_min = 0.0f, side_u_max = 1.0f;
    float top_u_min = 0.0f, top_u_max = 1.0f;
    float bottom_u_min = 0.0f, bottom_u_max = 1.0f;
    float v_min = 0.0f, v_max = 0.5f;

    switch (type) {
        case BlockType::Grass:
            top_u_min = 0.0f; top_u_max = 0.25f;
            side_u_min = 0.25f; side_u_max = 0.5f;
            bottom_u_min = 0.5f; bottom_u_max = 0.75f;
            break;
        case BlockType::Dirt:
            side_u_min = top_u_min = bottom_u_min = 0.5f;
            side_u_max = top_u_max = bottom_u_max = 0.75f;
            break;
        case BlockType::Stone:
            side_u_min = top_u_min = bottom_u_min = 0.75f;
            side_u_max = top_u_max = bottom_u_max = 1.0f;
            break;
        default:
            return;
    }

    auto addFace = [](std::vector<float>& vertices, std::vector<unsigned int>& indices, unsigned int& vertexOffset,
                      const std::array<float, 32>& faceVertices, const std::array<unsigned int, 6>& faceIndices) {
        vertices.insert(vertices.end(), faceVertices.begin(), faceVertices.end());
        for (unsigned int idx : faceIndices) {
            indices.push_back(vertexOffset + idx);
        }
        vertexOffset += 4;
    };

    unsigned int mainVertexOffset = mainVertices.size() / 8;

    int worldX = m_ChunkX * SIZE + x;
    int worldZ = m_ChunkZ * SIZE + z;
    Constants::BiomeType blockBiome = GetBiomeAt(worldX, worldZ);
    int biomeIndex = static_cast<int>(blockBiome);

    // Front face (+Z)
    if (visible[0]) {
        glm::vec3 normal = {0.0f, 0.0f, 1.0f};
        std::array<float, 32> faceVertices = {
            px, py, pz + 1,  normal.x, normal.y, normal.z, side_u_min, v_min,
            px + 1, py, pz + 1,  normal.x, normal.y, normal.z, side_u_max, v_min,
            px + 1, py + 1, pz + 1,  normal.x, normal.y, normal.z, side_u_max, v_max,
            px, py + 1, pz + 1,  normal.x, normal.y, normal.z, side_u_min, v_max
        };
        std::array<unsigned int, 6> faceIndices = {0, 1, 2, 2, 3, 0};
        addFace(mainVertices, mainIndices, mainVertexOffset, faceVertices, faceIndices);
    }

    // Back face (-Z)
    if (visible[1]) {
        glm::vec3 normal = {0.0f, 0.0f, -1.0f};
        std::array<float, 32> faceVertices = {
            px + 1, py, pz,  normal.x, normal.y, normal.z, side_u_min, v_min,
            px, py, pz,  normal.x, normal.y, normal.z, side_u_max, v_min,
            px, py + 1, pz,  normal.x, normal.y, normal.z, side_u_max, v_max,
            px + 1, py + 1, pz,  normal.x, normal.y, normal.z, side_u_min, v_max
        };
        std::array<unsigned int, 6> faceIndices = {0, 1, 2, 2, 3, 0};
        addFace(mainVertices, mainIndices, mainVertexOffset, faceVertices, faceIndices);
    }

    // Left face (-X)
    if (visible[2]) {
        glm::vec3 normal = {-1.0f, 0.0f, 0.0f};
        std::array<float, 32> faceVertices = {
            px, py, pz + 1,  normal.x, normal.y, normal.z, side_u_min, v_min,
            px, py, pz,  normal.x, normal.y, normal.z, side_u_max, v_min,
            px, py + 1, pz,  normal.x, normal.y, normal.z, side_u_max, v_max,
            px, py + 1, pz + 1,  normal.x, normal.y, normal.z, side_u_min, v_max
        };
        std::array<unsigned int, 6> faceIndices = {0, 1, 2, 2, 3, 0};
        addFace(mainVertices, mainIndices, mainVertexOffset, faceVertices, faceIndices);
    }

    // Right face (+X)
    if (visible[3]) {
        glm::vec3 normal = {1.0f, 0.0f, 0.0f};
        std::array<float, 32> faceVertices = {
            px + 1, py, pz,         normal.x, normal.y, normal.z, side_u_min, v_min,
            px + 1, py, pz + 1,     normal.x, normal.y, normal.z, side_u_max, v_min,
            px + 1, py + 1, pz + 1, normal.x, normal.y, normal.z, side_u_max, v_max,
            px + 1, py + 1, pz,     normal.x, normal.y, normal.z, side_u_min, v_max
        };
        std::array<unsigned int, 6> faceIndices = {0, 1, 2, 2, 3, 0};
        addFace(mainVertices, mainIndices, mainVertexOffset, faceVertices, faceIndices);
    }

    // Top face (+Y)
    if (visible[4]) {
        float u_min = (type == BlockType::Grass) ? top_u_min : side_u_min;
        float u_max = (type == BlockType::Grass) ? top_u_max : side_u_max;
        glm::vec3 normal = {0.0f, 1.0f, 0.0f};
        std::array<float, 32> faceVertices = {
            px, py + 1, pz + 1,     normal.x, normal.y, normal.z, u_min, v_min,
            px + 1, py + 1, pz + 1, normal.x, normal.y, normal.z, u_max, v_min,
            px + 1, py + 1, pz,     normal.x, normal.y, normal.z, u_max, v_max,
            px, py + 1, pz,         normal.x, normal.y, normal.z, u_min, v_max
        };
        std::array<unsigned int, 6> faceIndices = {0, 1, 2, 2, 3, 0};

        if (type == BlockType::Grass) {
            unsigned int biomeVertexOffset = grassTopVertices[biomeIndex].size() / 8;
            addFace(grassTopVertices[biomeIndex], grassTopIndices[biomeIndex], biomeVertexOffset, faceVertices, faceIndices);
        } else {
            addFace(mainVertices, mainIndices, mainVertexOffset, faceVertices, faceIndices);
        }
    }

    // Bottom face (-Y)
    if (visible[5]) {
        glm::vec3 normal = {0.0f, -1.0f, 0.0f};
        std::array<float, 32> faceVertices = {
            px, py, pz,         normal.x, normal.y, normal.z, bottom_u_min, v_min,
            px + 1, py, pz,     normal.x, normal.y, normal.z, bottom_u_max, v_min,
            px + 1, py, pz + 1, normal.x, normal.y, normal.z, bottom_u_max, v_max,
            px, py, pz + 1,     normal.x, normal.y, normal.z, bottom_u_min, v_max
        };
        std::array<unsigned int, 6> faceIndices = {0, 1, 2, 2, 3, 0};
        addFace(mainVertices, mainIndices, mainVertexOffset, faceVertices, faceIndices);
    }
}

Voxel& Chunk::GetVoxel(int x, int y, int z) {
    return m_Voxels[GetIndex(x, y, z)];
}

const Voxel& Chunk::GetVoxel(int x, int y, int z) const {
    return m_Voxels[GetIndex(x, y, z)];
}

int Chunk::GetIndex(int x, int y, int z) const {
    return x + y * SIZE + z * SIZE * SIZE;
}

Constants::BiomeType Chunk::GetBiomeAt(int worldX, int worldZ) {
    // Logique de biome simplifiée (extraite de VoxelWorld.cpp)
    float distance = std::sqrt(static_cast<float>(worldX * worldX + worldZ * worldZ));
    if (distance < 20) {
        return Constants::BiomeType::Plains;
    } else if (worldX > 0) {
        return Constants::BiomeType::Forest;
    } else if (worldZ > 0) {
        return Constants::BiomeType::Desert;
    } else {
        return Constants::BiomeType::Tundra;
    }
}

} // namespace MonJeu