#include <MonJeu/VoxelWorld.h>
#include <algorithm>
#include <cmath>

namespace MonJeu {

    Chunk::Chunk() : m_ChunkX(0), m_ChunkZ(0), m_Voxels(SIZE * SIZE * SIZE) {
    }

    Chunk::Chunk(int chunkX, int chunkZ)
        : m_ChunkX(chunkX), m_ChunkZ(chunkZ), m_Voxels(SIZE * SIZE * SIZE) {
    }

    void Chunk::GenerateTerrain() {
        for (int x = 0; x < SIZE; ++x) {
            for (int z = 0; z < SIZE; ++z) {
                // Coordonnées monde
                int worldX = m_ChunkX * SIZE + x;
                int worldZ = m_ChunkZ * SIZE + z;

                // Hauteur simple basée sur distance du centre
                float distance = std::sqrt(worldX * worldX + worldZ * worldZ);
                int height = 8 + static_cast<int>(4 * std::sin(distance * 0.1f));

                for (int y = 0; y < SIZE; ++y) {
                    int worldY = y;

                    Voxel& voxel = GetVoxel(x, y, z);
                    if (worldY < height - 3) {
                        voxel.type = BlockType::Stone;
                        voxel.active = true;
                    } else if (worldY < height) {
                        voxel.type = BlockType::Dirt;
                        voxel.active = true;
                    } else if (worldY == height) {
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

        // Générer le mesh basé sur les voxels actifs avec face culling
        for (int x = 0; x < SIZE; ++x) {
            for (int y = 0; y < SIZE; ++y) {
                for (int z = 0; z < SIZE; ++z) {
                    const Voxel& voxel = GetVoxel(x, y, z);
                    if (!voxel.active) continue;

                    // Vérifier les faces visibles
                    bool visible[6] = {true, true, true, true, true, true}; // Front, Back, Left, Right, Top, Bottom

                    // Vérifier adjacent +X (Right)
                    if (x + 1 < SIZE && GetVoxel(x + 1, y, z).active) visible[3] = false;
                    // -X (Left)
                    if (x - 1 >= 0 && GetVoxel(x - 1, y, z).active) visible[2] = false;
                    // +Y (Top)
                    if (y + 1 < SIZE && GetVoxel(x, y + 1, z).active) visible[4] = false;
                    // -Y (Bottom)
                    if (y - 1 >= 0 && GetVoxel(x, y - 1, z).active) visible[5] = false;
                    // +Z (Front)
                    if (z + 1 < SIZE && GetVoxel(x, y, z + 1).active) visible[0] = false;
                    // -Z (Back)
                    if (z - 1 >= 0 && GetVoxel(x, y, z - 1).active) visible[1] = false;

                    // Ajouter les faces visibles
                    AddVisibleFacesToMesh(vertices, indices, x, y, z, voxel.type, visible);
                }
            }
        }

        return NihilEngine::Mesh(vertices, indices);
    }

    void Chunk::AddVisibleFacesToMesh(std::vector<float>& vertices, std::vector<unsigned int>& indices, int x, int y, int z, BlockType type, const bool visible[6]) const {
        // Couleurs par type
        glm::vec3 color;
        switch (type) {
            case BlockType::Grass: color = glm::vec3(0.0f, 1.0f, 0.0f); break; // Vert
            case BlockType::Dirt: color = glm::vec3(0.6f, 0.3f, 0.0f); break; // Marron
            case BlockType::Stone: color = glm::vec3(0.5f, 0.5f, 0.5f); break; // Gris
            default: color = glm::vec3(1.0f, 1.0f, 1.0f); break; // Blanc
        }

        float half = 0.5f; // Blocs collés
        unsigned int vertexOffset = vertices.size() / 6; // 6 floats par vertex

        float offset = (SIZE - 1) / 2.0f; // Pour centrer l'entity
        float px = x;
        float py = y;
        float pz = z;

        // Front face
        if (visible[0]) {
            vertices.insert(vertices.end(), {
                px - half, py - half, pz + half, color.r, color.g, color.b,
                px + half, py - half, pz + half, color.r, color.g, color.b,
                px + half, py + half, pz + half, color.r, color.g, color.b,
                px - half, py + half, pz + half, color.r, color.g, color.b
            });
            indices.insert(indices.end(), {
                vertexOffset + 0, vertexOffset + 1, vertexOffset + 2, vertexOffset + 2, vertexOffset + 3, vertexOffset + 0
            });
            vertexOffset += 4;
        }

        // Back face
        if (visible[1]) {
            vertices.insert(vertices.end(), {
                px - half, py - half, pz - half, color.r, color.g, color.b,
                px - half, py + half, pz - half, color.r, color.g, color.b,
                px + half, py + half, pz - half, color.r, color.g, color.b,
                px + half, py - half, pz - half, color.r, color.g, color.b
            });
            indices.insert(indices.end(), {
                vertexOffset + 0, vertexOffset + 1, vertexOffset + 2, vertexOffset + 2, vertexOffset + 3, vertexOffset + 0
            });
            vertexOffset += 4;
        }

        // Left face
        if (visible[2]) {
            vertices.insert(vertices.end(), {
                px - half, py + half, pz + half, color.r, color.g, color.b,
                px - half, py + half, pz - half, color.r, color.g, color.b,
                px - half, py - half, pz - half, color.r, color.g, color.b,
                px - half, py - half, pz + half, color.r, color.g, color.b
            });
            indices.insert(indices.end(), {
                vertexOffset + 0, vertexOffset + 1, vertexOffset + 2, vertexOffset + 2, vertexOffset + 3, vertexOffset + 0
            });
            vertexOffset += 4;
        }

        // Right face
        if (visible[3]) {
            vertices.insert(vertices.end(), {
                px + half, py + half, pz + half, color.r, color.g, color.b,
                px + half, py - half, pz + half, color.r, color.g, color.b,
                px + half, py - half, pz - half, color.r, color.g, color.b,
                px + half, py + half, pz - half, color.r, color.g, color.b
            });
            indices.insert(indices.end(), {
                vertexOffset + 0, vertexOffset + 1, vertexOffset + 2, vertexOffset + 2, vertexOffset + 3, vertexOffset + 0
            });
            vertexOffset += 4;
        }

        // Top face
        if (visible[4]) {
            vertices.insert(vertices.end(), {
                px - half, py + half, pz - half, color.r, color.g, color.b,
                px - half, py + half, pz + half, color.r, color.g, color.b,
                px + half, py + half, pz + half, color.r, color.g, color.b,
                px + half, py + half, pz - half, color.r, color.g, color.b
            });
            indices.insert(indices.end(), {
                vertexOffset + 0, vertexOffset + 1, vertexOffset + 2, vertexOffset + 2, vertexOffset + 3, vertexOffset + 0
            });
            vertexOffset += 4;
        }

        // Bottom face
        if (visible[5]) {
            vertices.insert(vertices.end(), {
                px - half, py - half, pz - half, color.r, color.g, color.b,
                px + half, py - half, pz - half, color.r, color.g, color.b,
                px + half, py - half, pz + half, color.r, color.g, color.b,
                px - half, py - half, pz + half, color.r, color.g, color.b
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

    // VoxelWorld

    VoxelWorld::VoxelWorld() {
    }

    void VoxelWorld::GenerateChunk(int chunkX, int chunkZ) {
        uint64_t key = GetChunkKey(chunkX, chunkZ);
        if (m_Chunks.find(key) != m_Chunks.end()) return;

        Chunk chunk(chunkX, chunkZ);
        chunk.GenerateTerrain();
        m_Chunks[key] = std::move(chunk);
    }

    const std::unordered_map<uint64_t, Chunk>& VoxelWorld::GetChunks() const {
        return m_Chunks;
    }

    void VoxelWorld::WorldToChunk(int worldX, int worldZ, int& chunkX, int& chunkZ) {
        chunkX = worldX / Chunk::SIZE;
        chunkZ = worldZ / Chunk::SIZE;
    }

    bool VoxelWorld::Raycast(const glm::vec3& origin, const glm::vec3& direction, float maxDistance, RaycastResult& result) {
        glm::vec3 pos = origin;
        glm::vec3 dir = glm::normalize(direction);
        float distance = 0.0f;

        while (distance < maxDistance) {
            // Convertir en coordonnées monde entières
            int worldX = static_cast<int>(std::round(pos.x));
            int worldY = static_cast<int>(std::round(pos.y));
            int worldZ = static_cast<int>(std::round(pos.z));

            // Trouver le chunk
            int chunkX, chunkZ;
            VoxelWorld::WorldToChunk(worldX, worldZ, chunkX, chunkZ);
            uint64_t key = GetChunkKey(chunkX, chunkZ);

            if (m_Chunks.find(key) != m_Chunks.end()) {
                // Coordonnées locales dans le chunk
                int localX = worldX - chunkX * Chunk::SIZE;
                int localZ = worldZ - chunkZ * Chunk::SIZE;

                if (localX >= 0 && localX < Chunk::SIZE && worldY >= 0 && worldY < Chunk::SIZE && localZ >= 0 && localZ < Chunk::SIZE) {
                    const Voxel& voxel = m_Chunks.at(key).GetVoxel(localX, worldY, localZ);
                    if (voxel.active) {
                        // Trouvé un voxel actif
                        result.hit = true;
                        result.chunkX = chunkX;
                        result.chunkZ = chunkZ;
                        result.x = localX;
                        result.y = worldY;
                        result.z = localZ;
                        // Calculer la normale approximative (direction opposée au mouvement)
                        result.normal = -glm::normalize(dir);
                        return true;
                    }
                }
            }

            // Avancer
            pos += dir * 0.1f;
            distance += 0.1f;
        }

        result.hit = false;
        return false;
    }

    void VoxelWorld::SetVoxelActive(int worldX, int worldY, int worldZ, bool active) {
        int chunkX, chunkZ;
        VoxelWorld::WorldToChunk(worldX, worldZ, chunkX, chunkZ);
        uint64_t key = GetChunkKey(chunkX, chunkZ);

        if (m_Chunks.find(key) != m_Chunks.end()) {
            int localX = worldX - chunkX * Chunk::SIZE;
            int localZ = worldZ - chunkZ * Chunk::SIZE;

            if (localX >= 0 && localX < Chunk::SIZE && worldY >= 0 && worldY < Chunk::SIZE && localZ >= 0 && localZ < Chunk::SIZE) {
                m_Chunks[key].GetVoxel(localX, worldY, localZ).active = active;
            }
        }
    }

    uint64_t VoxelWorld::GetChunkKey(int chunkX, int chunkZ) const {
        return (static_cast<uint64_t>(chunkX) << 32) | static_cast<uint64_t>(chunkZ);
    }

}