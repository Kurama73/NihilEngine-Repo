#include <MonJeu/VoxelWorld.h>
#include <algorithm>
#include <cmath>
#include <NihilEngine/Physics.h>
#include <NihilEngine/Renderer.h>
#include <NihilEngine/Camera.h>
#include <iostream> // Ajouté pour le std::cout

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

    // --- Fonction AddVisibleFacesToMesh (MODIFIÉE) ---
    // Elle génère maintenant 8 floats par vertex (Pos, Normal, UV)
    void Chunk::AddVisibleFacesToMesh(std::vector<float>& vertices, std::vector<unsigned int>& indices, int x, int y, int z, BlockType type, const bool visible[6]) const {

        // On n'utilise plus les couleurs, on passe les normales et les UVs
        // (Les couleurs viendront des textures ou du shader)

        float half = 0.5f; // Blocs collés

        // ATTENTION: 8 floats par vertex maintenant!
        unsigned int vertexOffset = vertices.size() / 8;

        float px = x;
        float py = y;
        float pz = z;

        // Coordonnées UV (pour l'instant, on utilise 0,0 à 1,1 pour toutes)
        float u_min = 0.0f;
        float u_max = 1.0f;
        float v_min = 0.0f;
        float v_max = 1.0f;

        // TODO: Plus tard, tu changeras ces UVs en fonction du BlockType
        // pour utiliser un "Texture Atlas"
        // if (type == BlockType::Grass) { u_min = 0.0; u_max = 0.25; ... }
        // if (type == BlockType::Dirt)  { u_min = 0.25; u_max = 0.5; ... }

        // Front face (+Z)
        if (visible[0]) {
            glm::vec3 normal = {0.0f, 0.0f, 1.0f};
            vertices.insert(vertices.end(), {
                // Pos (x, y, z)          // Normal (nx, ny, nz)     // UV (u, v)
                px - half, py - half, pz + half,  normal.x, normal.y, normal.z, u_min, v_min,
                px + half, py - half, pz + half,  normal.x, normal.y, normal.z, u_max, v_min,
                px + half, py + half, pz + half,  normal.x, normal.y, normal.z, u_max, v_max,
                px - half, py + half, pz + half,  normal.x, normal.y, normal.z, u_min, v_max
            });
            indices.insert(indices.end(), {
                vertexOffset + 0, vertexOffset + 1, vertexOffset + 2, vertexOffset + 2, vertexOffset + 3, vertexOffset + 0
            });
            vertexOffset += 4;
        }

        // Back face (-Z)
        if (visible[1]) {
            glm::vec3 normal = {0.0f, 0.0f, -1.0f};
            vertices.insert(vertices.end(), {
                px - half, py - half, pz - half,  normal.x, normal.y, normal.z, u_min, v_min,
                px - half, py + half, pz - half,  normal.x, normal.y, normal.z, u_min, v_max,
                px + half, py + half, pz - half,  normal.x, normal.y, normal.z, u_max, v_max,
                px + half, py - half, pz - half,  normal.x, normal.y, normal.z, u_max, v_min
            });
            indices.insert(indices.end(), {
                vertexOffset + 0, vertexOffset + 1, vertexOffset + 2, vertexOffset + 2, vertexOffset + 3, vertexOffset + 0
            });
            vertexOffset += 4;
        }

        // Left face (-X)
        if (visible[2]) {
            glm::vec3 normal = {-1.0f, 0.0f, 0.0f};
            vertices.insert(vertices.end(), {
                px - half, py + half, pz + half,  normal.x, normal.y, normal.z, u_min, v_max,
                px - half, py + half, pz - half,  normal.x, normal.y, normal.z, u_max, v_max,
                px - half, py - half, pz - half,  normal.x, normal.y, normal.z, u_max, v_min,
                px - half, py - half, pz + half,  normal.x, normal.y, normal.z, u_min, v_min
            });
            indices.insert(indices.end(), {
                vertexOffset + 0, vertexOffset + 1, vertexOffset + 2, vertexOffset + 2, vertexOffset + 3, vertexOffset + 0
            });
            vertexOffset += 4;
        }

        // Right face (+X)
        if (visible[3]) {
            glm::vec3 normal = {1.0f, 0.0f, 0.0f};
            vertices.insert(vertices.end(), {
                px + half, py + half, pz + half,  normal.x, normal.y, normal.z, u_min, v_max,
                px + half, py - half, pz + half,  normal.x, normal.y, normal.z, u_min, v_min,
                px + half, py - half, pz - half,  normal.x, normal.y, normal.z, u_max, v_min,
                px + half, py + half, pz - half,  normal.x, normal.y, normal.z, u_max, v_max
            });
            indices.insert(indices.end(), {
                vertexOffset + 0, vertexOffset + 1, vertexOffset + 2, vertexOffset + 2, vertexOffset + 3, vertexOffset + 0
            });
            vertexOffset += 4;
        }

        // Top face (+Y)
        if (visible[4]) {
            glm::vec3 normal = {0.0f, 1.0f, 0.0f};
            vertices.insert(vertices.end(), {
                px - half, py + half, pz - half,  normal.x, normal.y, normal.z, u_min, v_min,
                px - half, py + half, pz + half,  normal.x, normal.y, normal.z, u_min, v_max,
                px + half, py + half, pz + half,  normal.x, normal.y, normal.z, u_max, v_max,
                px + half, py + half, pz - half,  normal.x, normal.y, normal.z, u_max, v_min
            });
            indices.insert(indices.end(), {
                vertexOffset + 0, vertexOffset + 1, vertexOffset + 2, vertexOffset + 2, vertexOffset + 3, vertexOffset + 0
            });
            vertexOffset += 4;
        }

        // Bottom face (-Y)
        if (visible[5]) {
            glm::vec3 normal = {0.0f, -1.0f, 0.0f};
            vertices.insert(vertices.end(), {
                px - half, py - half, pz - half,  normal.x, normal.y, normal.z, u_min, v_min,
                px + half, py - half, pz - half,  normal.x, normal.y, normal.z, u_max, v_min,
                px + half, py - half, pz + half,  normal.x, normal.y, normal.z, u_max, v_max,
                px - half, py - half, pz + half,  normal.x, normal.y, normal.z, u_min, v_max
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

        // 1. Créer les données du chunk
        Chunk chunk(chunkX, chunkZ);
        chunk.GenerateTerrain();

        // 2. Créer l'entité (le mesh)
        // --- LA CORRECTION EST ICI ---
        // Le mesh local est déjà de 0 à 15.
        // On positionne l'entité à l'origine du chunk (ex: 16, 0, 0)
        // On supprime le décalage de -7.5
        auto chunkEntity = std::make_unique<NihilEngine::Entity>(
            std::move(chunk.CreateMesh()),
            glm::vec3(
                chunk.GetChunkX() * Chunk::SIZE,
                0.0f,
                chunk.GetChunkZ() * Chunk::SIZE
            )
        );
        // -----------------------------

        // 3. Stocker les deux
        m_Chunks[key] = std::move(chunk);
        m_ChunkEntities[key] = std::move(chunkEntity);
    }

    void VoxelWorld::UpdateDirtyChunks() {
        // Simple dé-duplication
        std::sort(m_DirtyChunks.begin(), m_DirtyChunks.end());
        m_DirtyChunks.erase(std::unique(m_DirtyChunks.begin(), m_DirtyChunks.end()), m_DirtyChunks.end());

        for (uint64_t key : m_DirtyChunks) {
            if (m_Chunks.find(key) != m_Chunks.end()) {
                const Chunk& chunk = m_Chunks[key];

                // Recréer le mesh (cher)
                NihilEngine::Mesh newMesh = chunk.CreateMesh();

                // Mettre à jour l'entité existante
                m_ChunkEntities[key]->SetMesh(std::move(newMesh));

                std::cout << "Updated mesh for chunk " << chunk.GetChunkX() << ", " << chunk.GetChunkZ() << std::endl;
            }
        }
        m_DirtyChunks.clear(); // Vider la liste
    }

    void VoxelWorld::Render(NihilEngine::Renderer& renderer, const NihilEngine::Camera& camera) {
        for (const auto& pair : m_ChunkEntities) {
            const auto& entity = pair.second;
            // On appelle le nouveau DrawMesh (sans colorMultiplier)
            renderer.DrawMesh(
                entity->GetMesh(),
                camera,
                entity->GetModelMatrix()
            );
        }
    }

    bool VoxelWorld::Raycast(const glm::vec3& origin, const glm::vec3& direction, float maxDistance, NihilEngine::RaycastHit& result) {
        // Utiliser le "::" global pour appeler la fonction statique
        return NihilEngine::RaycastVoxel(origin, direction, maxDistance,
            // C'est la "callback"
            [this](const glm::ivec3& pos) {
                return this->GetVoxelActive(pos.x, pos.y, pos.z);
            },
            result
        );
    }

    const std::unordered_map<uint64_t, Chunk>& VoxelWorld::GetChunks() const {
        return m_Chunks;
    }

    void VoxelWorld::WorldToChunk(int worldX, int worldZ, int& chunkX, int& chunkZ) {
        chunkX = worldX / Chunk::SIZE;
        chunkZ = worldZ / Chunk::SIZE;
        // Gérer les coordonnées négatives correctement
        if (worldX < 0) chunkX = (worldX - Chunk::SIZE + 1) / Chunk::SIZE;
        if (worldZ < 0) chunkZ = (worldZ - Chunk::SIZE + 1) / Chunk::SIZE;
    }

    bool VoxelWorld::GetVoxelActive(int worldX, int worldY, int worldZ) const {
        int chunkX, chunkZ;
        VoxelWorld::WorldToChunk(worldX, worldZ, chunkX, chunkZ);
        uint64_t key = GetChunkKey(chunkX, chunkZ);

        if (m_Chunks.find(key) != m_Chunks.end()) {
            // Coords locales
            int localX = worldX - chunkX * Chunk::SIZE;
            int localZ = worldZ - chunkZ * Chunk::SIZE;
            int localY = worldY;

            if (localX >= 0 && localX < Chunk::SIZE && localY >= 0 && localY < Chunk::SIZE && localZ >= 0 && localZ < Chunk::SIZE) {
                return m_Chunks.at(key).GetVoxel(localX, localY, localZ).active;
            }
        }
        return false;
    }

    void VoxelWorld::SetVoxelActive(int worldX, int worldY, int worldZ, bool active) {
        int chunkX, chunkZ;
        VoxelWorld::WorldToChunk(worldX, worldZ, chunkX, chunkZ);
        uint64_t key = GetChunkKey(chunkX, chunkZ);

        if (m_Chunks.find(key) != m_Chunks.end()) {
            int localX = worldX - chunkX * Chunk::SIZE;
            int localZ = worldZ - chunkZ * Chunk::SIZE;
            int localY = worldY;

            if (localX >= 0 && localX < Chunk::SIZE && localY >= 0 && localY < Chunk::SIZE && localZ >= 0 && localZ < Chunk::SIZE) {
                // 1. Modifier la donnée
                m_Chunks[key].GetVoxel(localX, localY, localZ).active = active;

                // 2. Marquer ce chunk pour mise à jour
                m_DirtyChunks.push_back(key);

                // 3. AMÉLIORATION : Si on est sur une bordure,
                // marquer aussi le chunk voisin comme "dirty" !
                if (localX == 0) m_DirtyChunks.push_back(GetChunkKey(chunkX - 1, chunkZ));
                if (localX == Chunk::SIZE - 1) m_DirtyChunks.push_back(GetChunkKey(chunkX + 1, chunkZ));
                if (localZ == 0) m_DirtyChunks.push_back(GetChunkKey(chunkX, chunkZ - 1));
                if (localZ == Chunk::SIZE - 1) m_DirtyChunks.push_back(GetChunkKey(chunkX, chunkZ + 1));
            }
        }
    }

    uint64_t VoxelWorld::GetChunkKey(int chunkX, int chunkZ) const {
        // Combine deux int32 en un uint64
        return (static_cast<uint64_t>(static_cast<int32_t>(chunkX)) << 32) | (static_cast<uint64_t>(static_cast<int32_t>(chunkZ)) & 0xFFFFFFFF);
    }
}