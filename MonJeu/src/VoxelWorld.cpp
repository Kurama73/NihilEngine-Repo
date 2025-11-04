#include <MonJeu/VoxelWorld.h>
#include <MonJeu/Constants.h>
#include <algorithm>
#include <cmath>
#include <NihilEngine/Physics.h>
#include <NihilEngine/Renderer.h>
#include <NihilEngine/Camera.h>
#include <iostream>

namespace MonJeu {
Chunk::Chunk(int chunkX, int chunkZ)
    : m_ChunkX(chunkX), m_ChunkZ(chunkZ), m_Voxels(SIZE * SIZE * SIZE) {}

void Chunk::GenerateTerrain() {
    for (int x = 0; x < SIZE; ++x) {
        for (int z = 0; z < SIZE; ++z) {
            int worldX = m_ChunkX * SIZE + x;
            int worldZ = m_ChunkZ * SIZE + z;
            float distance = std::sqrt(static_cast<float>(worldX * worldX + worldZ * worldZ));
            int height = Constants::BASE_HEIGHT + static_cast<int>(Constants::TERRAIN_AMPLITUDE * std::sin(distance * Constants::TERRAIN_FREQUENCY));
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

NihilEngine::Mesh Chunk::CreateMesh() const {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

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

                AddVisibleFacesToMesh(vertices, indices, x, y, z, voxel.type, visible);
            }
        }
    }
    return NihilEngine::Mesh(vertices, indices, {NihilEngine::VertexAttribute::Position, NihilEngine::VertexAttribute::Normal, NihilEngine::VertexAttribute::TexCoord});
}

void Chunk::AddVisibleFacesToMesh(std::vector<float>& vertices, std::vector<unsigned int>& indices, int x, int y, int z, BlockType type, const bool visible[6]) const {
    unsigned int vertexOffset = vertices.size() / 8;
    float px = static_cast<float>(x), py = static_cast<float>(y), pz = static_cast<float>(z);

    float side_u_min = 0.0f, side_u_max = 1.0f;
    float top_u_min = 0.0f, top_u_max = 1.0f;
    float bottom_u_min = 0.0f, bottom_u_max = 1.0f;
    float v_min = 0.0f, v_max = 1.0f;

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

    // Front face (+Z)
    if (visible[0]) {
        float u_min = side_u_min, u_max = side_u_max;
        glm::vec3 normal = {0.0f, 0.0f, 1.0f};
        vertices.insert(vertices.end(), {
            px, py, pz + 1,  normal.x, normal.y, normal.z, u_min, v_min,
            px + 1, py, pz + 1,  normal.x, normal.y, normal.z, u_max, v_min,
            px + 1, py + 1, pz + 1,  normal.x, normal.y, normal.z, u_max, v_max,
            px, py + 1, pz + 1,  normal.x, normal.y, normal.z, u_min, v_max
        });
        indices.insert(indices.end(), {
            vertexOffset + 0, vertexOffset + 1, vertexOffset + 2, vertexOffset + 2, vertexOffset + 3, vertexOffset + 0
        });
        vertexOffset += 4;
    }

    // Back face (-Z)
    if (visible[1]) {
        float u_min = side_u_min, u_max = side_u_max;
        glm::vec3 normal = {0.0f, 0.0f, -1.0f};
        vertices.insert(vertices.end(), {
            px + 1, py, pz,  normal.x, normal.y, normal.z, u_min, v_min,
            px, py, pz,  normal.x, normal.y, normal.z, u_max, v_min,
            px, py + 1, pz,  normal.x, normal.y, normal.z, u_max, v_max,
            px + 1, py + 1, pz,  normal.x, normal.y, normal.z, u_min, v_max
        });
        indices.insert(indices.end(), {
            vertexOffset + 0, vertexOffset + 1, vertexOffset + 2, vertexOffset + 2, vertexOffset + 3, vertexOffset + 0
        });
        vertexOffset += 4;
    }

    // Left face (-X)
    if (visible[2]) {
        float u_min = side_u_min, u_max = side_u_max;
        glm::vec3 normal = {-1.0f, 0.0f, 0.0f};
        vertices.insert(vertices.end(), {
            px, py, pz + 1,  normal.x, normal.y, normal.z, u_min, v_min,
            px, py, pz,  normal.x, normal.y, normal.z, u_max, v_min,
            px, py + 1, pz,  normal.x, normal.y, normal.z, u_max, v_max,
            px, py + 1, pz + 1,  normal.x, normal.y, normal.z, u_min, v_max
        });
        indices.insert(indices.end(), {
            vertexOffset + 0, vertexOffset + 1, vertexOffset + 2, vertexOffset + 2, vertexOffset + 3, vertexOffset + 0
        });
        vertexOffset += 4;
    }

    // Right face (+X)
    if (visible[3]) {
        float u_min = side_u_min, u_max = side_u_max;
        glm::vec3 normal = {1.0f, 0.0f, 0.0f};
        vertices.insert(vertices.end(), {
            px + 1, py, pz,  normal.x, normal.y, normal.z, u_min, v_min,
            px + 1, py, pz + 1,  normal.x, normal.y, normal.z, u_max, v_min,
            px + 1, py + 1, pz + 1,  normal.x, normal.y, normal.z, u_max, v_max,
            px + 1, py + 1, pz,  normal.x, normal.y, normal.z, u_min, v_max
        });
        indices.insert(indices.end(), {
            vertexOffset + 0, vertexOffset + 1, vertexOffset + 2, vertexOffset + 2, vertexOffset + 3, vertexOffset + 0
        });
        vertexOffset += 4;
    }

    // Top face (+Y)
    if (visible[4]) {
        float u_min = (type == BlockType::Grass) ? top_u_min : side_u_min;
        float u_max = (type == BlockType::Grass) ? top_u_max : side_u_max;
        glm::vec3 normal = {0.0f, 1.0f, 0.0f};
        vertices.insert(vertices.end(), {
            px, py + 1, pz,  normal.x, normal.y, normal.z, u_min, v_min,
            px, py + 1, pz + 1,  normal.x, normal.y, normal.z, u_min, v_max,
            px + 1, py + 1, pz + 1,  normal.x, normal.y, normal.z, u_max, v_max,
            px + 1, py + 1, pz,  normal.x, normal.y, normal.z, u_max, v_min
        });
        indices.insert(indices.end(), {
            vertexOffset + 0, vertexOffset + 1, vertexOffset + 2, vertexOffset + 2, vertexOffset + 3, vertexOffset + 0
        });
        vertexOffset += 4;
    }

    // Bottom face (-Y)
    if (visible[5]) {
        float u_min = (type == BlockType::Grass) ? bottom_u_min : side_u_min;
        float u_max = (type == BlockType::Grass) ? bottom_u_max : side_u_max;
        glm::vec3 normal = {0.0f, -1.0f, 0.0f};
        vertices.insert(vertices.end(), {
            px, py, pz,  normal.x, normal.y, normal.z, u_min, v_min,
            px + 1, py, pz,  normal.x, normal.y, normal.z, u_max, v_min,
            px + 1, py, pz + 1,  normal.x, normal.y, normal.z, u_max, v_max,
            px, py, pz + 1,  normal.x, normal.y, normal.z, u_min, v_max
        });
        indices.insert(indices.end(), {
            vertexOffset + 0, vertexOffset + 1, vertexOffset + 2, vertexOffset + 2, vertexOffset + 3, vertexOffset + 0
        });
        vertexOffset += 4;
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

VoxelWorld::VoxelWorld() {}

void VoxelWorld::GenerateChunk(int chunkX, int chunkZ) {
    uint64_t key = GetChunkKey(chunkX, chunkZ);
    if (m_Chunks.find(key) != m_Chunks.end()) return;

    auto chunk = std::make_unique<Chunk>(chunkX, chunkZ);
    chunk->GenerateTerrain();

    auto chunkEntity = std::make_unique<NihilEngine::Entity>(
        std::move(chunk->CreateMesh()),
        glm::vec3(
            chunk->GetChunkX() * Chunk::SIZE,
            0.0f,
            chunk->GetChunkZ() * Chunk::SIZE
        )
    );

    m_Chunks[key] = std::move(chunk);
    m_ChunkEntities[key] = std::move(chunkEntity);
}

void VoxelWorld::UpdateDirtyChunks() {
    std::sort(m_DirtyChunks.begin(), m_DirtyChunks.end());
    m_DirtyChunks.erase(std::unique(m_DirtyChunks.begin(), m_DirtyChunks.end()), m_DirtyChunks.end());

    for (uint64_t key : m_DirtyChunks) {
        if (m_Chunks.find(key) != m_Chunks.end()) {
            const Chunk& chunk = *m_Chunks[key];
            NihilEngine::Mesh newMesh = chunk.CreateMesh();
            m_ChunkEntities[key]->SetMesh(std::move(newMesh));
            std::cout << "Updated mesh for chunk " << chunk.GetChunkX() << ", " << chunk.GetChunkZ() << std::endl;
        }
    }
    m_DirtyChunks.clear();
}

void VoxelWorld::Render(NihilEngine::Renderer& renderer, const NihilEngine::Camera& camera)
{
    for (auto const& [key, val] : m_ChunkEntities)
    {
        renderer.DrawEntity(*val, camera);
    }
}

bool VoxelWorld::Raycast(const glm::vec3& origin, const glm::vec3& direction, float maxDistance, NihilEngine::RaycastHit& result) {
    return NihilEngine::RaycastVoxel(origin, direction, maxDistance,
        [this](const glm::ivec3& pos) {
            return this->GetVoxelActive(pos.x, pos.y, pos.z);
        },
        result
    );
}

void VoxelWorld::WorldToChunk(int worldX, int worldZ, int& chunkX, int& chunkZ) {
    chunkX = worldX / Chunk::SIZE;
    chunkZ = worldZ / Chunk::SIZE;
    if (worldX < 0) chunkX = (worldX - Chunk::SIZE + 1) / Chunk::SIZE;
    if (worldZ < 0) chunkZ = (worldZ - Chunk::SIZE + 1) / Chunk::SIZE;
}

bool VoxelWorld::GetVoxelActive(int worldX, int worldY, int worldZ) const {
    int chunkX, chunkZ;
    WorldToChunk(worldX, worldZ, chunkX, chunkZ);
    uint64_t key = GetChunkKey(chunkX, chunkZ);

    if (m_Chunks.find(key) != m_Chunks.end()) {
        int localX = worldX - chunkX * Chunk::SIZE;
        int localZ = worldZ - chunkZ * Chunk::SIZE;
        int localY = worldY;
        if (localX >= 0 && localX < Chunk::SIZE && localY >= 0 && localY < Chunk::SIZE && localZ >= 0 && localZ < Chunk::SIZE) {
            return m_Chunks.at(key)->GetVoxel(localX, localY, localZ).active;
        }
    }
    return false;
}

void VoxelWorld::SetVoxelActive(int worldX, int worldY, int worldZ, bool active) {
    int chunkX, chunkZ;
    WorldToChunk(worldX, worldZ, chunkX, chunkZ);
    uint64_t key = GetChunkKey(chunkX, chunkZ);

    if (m_Chunks.find(key) != m_Chunks.end()) {
        int localX = worldX - chunkX * Chunk::SIZE;
        int localZ = worldZ - chunkZ * Chunk::SIZE;
        int localY = worldY;
        if (localX >= 0 && localX < Chunk::SIZE && localY >= 0 && localY < Chunk::SIZE && localZ >= 0 && localZ < Chunk::SIZE) {
            m_Chunks[key]->GetVoxel(localX, localY, localZ).active = active;
            if (active) {
                m_Chunks[key]->GetVoxel(localX, localY, localZ).type = BlockType::Grass;
            }
            m_DirtyChunks.push_back(key);
            if (localX == 0) m_DirtyChunks.push_back(GetChunkKey(chunkX - 1, chunkZ));
            if (localX == Chunk::SIZE - 1) m_DirtyChunks.push_back(GetChunkKey(chunkX + 1, chunkZ));
            if (localZ == 0) m_DirtyChunks.push_back(GetChunkKey(chunkX, chunkZ - 1));
            if (localZ == Chunk::SIZE - 1) m_DirtyChunks.push_back(GetChunkKey(chunkX, chunkZ + 1));
        }
    }
}

uint64_t VoxelWorld::GetChunkKey(int chunkX, int chunkZ) const {
    return (static_cast<uint64_t>(static_cast<int32_t>(chunkX)) << 32) | (static_cast<uint64_t>(static_cast<int32_t>(chunkZ)) & 0xFFFFFFFF);
}

bool VoxelWorld::IsPositionValid(const glm::vec3& position, float height) {
    for (float y = position.y - height * 0.5f; y < position.y + height * 0.5f; y += 0.5f) {
        glm::ivec3 blockPos = glm::floor(glm::vec3(position.x, y, position.z));
        if (GetVoxelActive(blockPos.x, blockPos.y, blockPos.z)) {
            return false;
        }
    }
    return true;
}
} // namespace MonJeu