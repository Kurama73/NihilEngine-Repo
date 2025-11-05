#include <MonJeu/VoxelWorld.h>
#include <MonJeu/Constants.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <NihilEngine/Physics.h>
#include <NihilEngine/Renderer.h>
#include <NihilEngine/Camera.h>
#include <NihilEngine/ProceduralGenerator.h>
#include <iostream>

namespace MonJeu {
Chunk::Chunk(int chunkX, int chunkZ, Constants::BiomeType biome)
    : m_ChunkX(chunkX), m_ChunkZ(chunkZ), m_Biome(biome), m_Voxels(SIZE * SIZE * SIZE) {}

// Modifié : Utilise le générateur procédural directement
void Chunk::GenerateTerrain(NihilEngine::ProceduralGenerator& generator) {
    NihilEngine::TerrainGenerator& terrainGen = generator.getTerrainGenerator();
    NihilEngine::BiomeGenerator& biomeGen = generator.getBiomeGenerator();

    for (int x = 0; x < SIZE; ++x) {
        for (int z = 0; z < SIZE; ++z) {
            int worldX = m_ChunkX * SIZE + x;
            int worldZ = m_ChunkZ * SIZE + z;

            // Obtient les données directement du générateur
            float terrainHeight = terrainGen.getHeight(static_cast<float>(worldX), static_cast<float>(worldZ));
            int height = static_cast<int>(terrainHeight);

            // Détermine le biome
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

ChunkMeshes Chunk::CreateMeshes() const {
    std::vector<float> mainVertices;
    std::vector<unsigned int> mainIndices;
    std::vector<std::vector<float>> grassTopVertices(5);  // Un vector par biome
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

    // Créer un mesh par biome pour les faces top d'herbe
    for (int i = 0; i < 5; ++i) {
        if (!grassTopVertices[i].empty()) {
            meshes.grassTopMeshes.emplace_back(std::make_unique<NihilEngine::Mesh>(grassTopVertices[i], grassTopIndices[i],
                std::vector<NihilEngine::VertexAttribute>{NihilEngine::VertexAttribute::Position, NihilEngine::VertexAttribute::Normal, NihilEngine::VertexAttribute::TexCoord}));
        } else {
            // Mesh vide pour les biomes sans blocs d'herbe
            meshes.grassTopMeshes.emplace_back(std::make_unique<NihilEngine::Mesh>(std::vector<float>{}, std::vector<unsigned int>{},
                std::vector<NihilEngine::VertexAttribute>{NihilEngine::VertexAttribute::Position, NihilEngine::VertexAttribute::Normal, NihilEngine::VertexAttribute::TexCoord}));
        }
    }
    return meshes;
}

void Chunk::AddVisibleFacesToMeshes(std::vector<float>& mainVertices, std::vector<unsigned int>& mainIndices,
                                   std::vector<std::vector<float>>& grassTopVertices, std::vector<std::vector<unsigned int>>& grassTopIndices,
                                   int x, int y, int z, BlockType type, const bool visible[6]) const {
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

    // Calculer le biome pour ce bloc
    int worldX = m_ChunkX * SIZE + x;
    int worldZ = m_ChunkZ * SIZE + z;
    Constants::BiomeType blockBiome = GetBiomeAt(worldX, worldZ);
    int biomeIndex = static_cast<int>(blockBiome);

    // Front face (+Z)
    if (visible[0]) {
        float u_min = side_u_min, u_max = side_u_max;
        glm::vec3 normal = {0.0f, 0.0f, 1.0f};
        std::array<float, 32> faceVertices = {
            px, py, pz + 1,  normal.x, normal.y, normal.z, u_min, v_min,
            px + 1, py, pz + 1,  normal.x, normal.y, normal.z, u_max, v_min,
            px + 1, py + 1, pz + 1,  normal.x, normal.y, normal.z, u_max, v_max,
            px, py + 1, pz + 1,  normal.x, normal.y, normal.z, u_min, v_max
        };
        std::array<unsigned int, 6> faceIndices = {0, 1, 2, 2, 3, 0};
        addFace(mainVertices, mainIndices, mainVertexOffset, faceVertices, faceIndices);
    }

    // Back face (-Z)
    if (visible[1]) {
        float u_min = side_u_min, u_max = side_u_max;
        glm::vec3 normal = {0.0f, 0.0f, -1.0f};
        std::array<float, 32> faceVertices = {
            px + 1, py, pz,  normal.x, normal.y, normal.z, u_min, v_min,
            px, py, pz,  normal.x, normal.y, normal.z, u_max, v_min,
            px, py + 1, pz,  normal.x, normal.y, normal.z, u_max, v_max,
            px + 1, py + 1, pz,  normal.x, normal.y, normal.z, u_min, v_max
        };
        std::array<unsigned int, 6> faceIndices = {0, 1, 2, 2, 3, 0};
        addFace(mainVertices, mainIndices, mainVertexOffset, faceVertices, faceIndices);
    }

    // Left face (-X)
    if (visible[2]) {
        float u_min = side_u_min, u_max = side_u_max;
        glm::vec3 normal = {-1.0f, 0.0f, 0.0f};
        std::array<float, 32> faceVertices = {
            px, py, pz + 1,  normal.x, normal.y, normal.z, u_min, v_min,
            px, py, pz,  normal.x, normal.y, normal.z, u_max, v_min,
            px, py + 1, pz,  normal.x, normal.y, normal.z, u_max, v_max,
            px, py + 1, pz + 1,  normal.x, normal.y, normal.z, u_min, v_max
        };
        std::array<unsigned int, 6> faceIndices = {0, 1, 2, 2, 3, 0};
        addFace(mainVertices, mainIndices, mainVertexOffset, faceVertices, faceIndices);
    }

    // Right face (+X)
    if (visible[3]) {
        float u_min = side_u_min, u_max = side_u_max;
        glm::vec3 normal = {1.0f, 0.0f, 0.0f};
        std::array<float, 32> faceVertices = {
            px + 1, py, pz,         normal.x, normal.y, normal.z, u_min, v_min,
            px + 1, py, pz + 1,     normal.x, normal.y, normal.z, u_max, v_min,
            px + 1, py + 1, pz + 1, normal.x, normal.y, normal.z, u_max, v_max,
            px + 1, py + 1, pz,     normal.x, normal.y, normal.z, u_min, v_max
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
            // Face top d'herbe va dans le mesh du biome approprié
            unsigned int biomeVertexOffset = grassTopVertices[biomeIndex].size() / 8;
            addFace(grassTopVertices[biomeIndex], grassTopIndices[biomeIndex], biomeVertexOffset, faceVertices, faceIndices);
        } else {
            // Autres faces top vont dans mainMesh
            addFace(mainVertices, mainIndices, mainVertexOffset, faceVertices, faceIndices);
        }
    }

    // Bottom face (-Y)
    if (visible[5]) {
        float u_min = bottom_u_min, u_max = bottom_u_max;
        glm::vec3 normal = {0.0f, -1.0f, 0.0f};
        std::array<float, 32> faceVertices = {
            px, py, pz,         normal.x, normal.y, normal.z, u_min, v_min,
            px + 1, py, pz,     normal.x, normal.y, normal.z, u_max, v_min,
            px + 1, py, pz + 1, normal.x, normal.y, normal.z, u_max, v_max,
            px, py, pz + 1,     normal.x, normal.y, normal.z, u_min, v_max
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

// Modifié : Constructeur ne pré-génère plus le monde
VoxelWorld::VoxelWorld(unsigned int seed)
    : m_ProceduralGen(seed),
      m_GrassTopEntities(5)
{
    // Initialiser le système LOD
    NihilEngine::LODSettings lodSettings;

    // Distances en unités monde (1 chunk = 16 blocs)
    // Configuration équilibrée pour de bonnes performances
    lodSettings.calculationDistance = 320.0f;   // ~20 chunks de calcul
    lodSettings.displayDistance = 256.0f;       // ~16 chunks d'affichage
    lodSettings.generationDistance = 288.0f;    // ~18 chunks de génération

    lodSettings.highDetailDistance = 64.0f;     // ~4 chunks en détail max (voxels)
    lodSettings.mediumDetailDistance = 128.0f;  // ~8 chunks (heightmap détaillée)
    lodSettings.lowDetailDistance = 192.0f;     // ~12 chunks (grille simplifiée)
    lodSettings.veryLowDetailDistance = 256.0f; // ~16 chunks (très simplifié)

    m_LODSystem.setSettings(lodSettings);

    // Configurer les mises à jour progressives
    m_ProgressiveUpdate.setUpdateRate(3); // 3 chunks par frame (équilibré)
    m_ProgressiveUpdate.setMaxPendingUpdates(100); // Limite raisonnable
}

// Modifié : Utilise le générateur de terrain du chunk
void VoxelWorld::GenerateChunk(int chunkX, int chunkZ) {
    uint64_t key = GetChunkKey(chunkX, chunkZ);
    if (m_Chunks.find(key) != m_Chunks.end()) return;

    Constants::BiomeType biome = Chunk::GetBiomeAt(chunkX * Chunk::SIZE, chunkZ * Chunk::SIZE);
    auto chunk = std::make_unique<Chunk>(chunkX, chunkZ, biome);

    // Modifié : Passe le générateur
    chunk->GenerateTerrain(m_ProceduralGen);

    auto meshes = chunk->CreateMeshes();

    // Entité principale pour toutes les faces sauf le top des blocs d'herbe
    auto mainEntity = std::make_unique<NihilEngine::Entity>(
        std::move(*meshes.mainMesh),
        glm::vec3(
            chunk->GetChunkX() * Chunk::SIZE,
            0.0f,
            chunk->GetChunkZ() * Chunk::SIZE
        )
    );

    // Créer une entité par biome pour les faces top d'herbe
    std::vector<std::unique_ptr<NihilEngine::Entity>> grassTopEntities;
    for (int i = 0; i < 5; ++i) {
        auto grassTopEntity = std::make_unique<NihilEngine::Entity>(
            std::move(*meshes.grassTopMeshes[i]),
            glm::vec3(
                chunk->GetChunkX() * Chunk::SIZE,
                0.0f,
                chunk->GetChunkZ() * Chunk::SIZE
            )
        );

        // Appliquer la couleur du biome
        if (m_TextureAtlasID != 0) {
            NihilEngine::Material grassTopMaterial;
            grassTopMaterial.textureID = m_TextureAtlasID;
            const Constants::Biome& biomeData = Constants::BIOMES[i];
            grassTopMaterial.color = glm::vec4(biomeData.grassColor, 1.0f);
            grassTopEntity->SetMaterial(grassTopMaterial);
        }

        grassTopEntities.push_back(std::move(grassTopEntity));
    }

    // Définir le matériau pour l'entité principale (couleur blanche)
    if (m_TextureAtlasID != 0) {
        NihilEngine::Material mainMaterial;
        mainMaterial.textureID = m_TextureAtlasID;
        mainMaterial.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
        mainEntity->SetMaterial(mainMaterial);
    }

    m_Chunks[key] = std::move(chunk);
    m_ChunkEntities[key] = std::move(mainEntity);

    // Stocker les entités des faces top d'herbe par biome
    for (int i = 0; i < 5; ++i) {
        m_GrassTopEntities[i][key] = std::move(grassTopEntities[i]);
    }
}

void VoxelWorld::UpdateDirtyChunks() {
    std::sort(m_DirtyChunks.begin(), m_DirtyChunks.end());
    m_DirtyChunks.erase(std::unique(m_DirtyChunks.begin(), m_DirtyChunks.end()), m_DirtyChunks.end());

    for (uint64_t key : m_DirtyChunks) {
        if (m_Chunks.find(key) != m_Chunks.end()) {
            const Chunk& chunk = *m_Chunks[key];
            auto meshes = chunk.CreateMeshes();

            // Mettre à jour l'entité principale
            m_ChunkEntities[key]->SetMesh(std::move(*meshes.mainMesh));

            // Mettre à jour les entités des faces top d'herbe par biome
            for (int i = 0; i < 5; ++i) {
                m_GrassTopEntities[i][key]->SetMesh(std::move(*meshes.grassTopMeshes[i]));
            }

            // Re-définir les matériaux
            if (m_TextureAtlasID != 0) {
                // Matériau principal (blanc)
                NihilEngine::Material mainMaterial;
                mainMaterial.textureID = m_TextureAtlasID;
                mainMaterial.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
                m_ChunkEntities[key]->SetMaterial(mainMaterial);

                // Matériaux des faces top d'herbe (un par biome)
                for (int i = 0; i < 5; ++i) {
                    NihilEngine::Material grassTopMaterial;
                    grassTopMaterial.textureID = m_TextureAtlasID;
                    const Constants::Biome& biomeData = Constants::BIOMES[i];
                    grassTopMaterial.color = glm::vec4(biomeData.grassColor, 1.0f);
                    m_GrassTopEntities[i][key]->SetMaterial(grassTopMaterial);
                }
            }

            // std::cout << "Updated mesh for chunk " << chunk.GetChunkX() << ", " << chunk.GetChunkZ() << std::endl;
        }
    }
    m_DirtyChunks.clear();
}

void VoxelWorld::Render(NihilEngine::Renderer& renderer, const NihilEngine::Camera& camera)
{
    glm::vec3 camPos = camera.GetPosition();
    float maxRenderDist = m_LODSystem.getSettings().displayDistance;
    float maxRenderDistSq = maxRenderDist * maxRenderDist;

    // Rendre les entités principales (toutes les faces sauf top d'herbe)
    for (auto const& [key, val] : m_ChunkEntities)
    {
        // Culling basique par distance
        glm::vec3 chunkPos = val->GetPosition() + glm::vec3(Chunk::SIZE * 0.5f, 0.0f, Chunk::SIZE * 0.5f);
        float distSq = glm::dot(camPos - chunkPos, camPos - chunkPos);

        if (distSq <= maxRenderDistSq) {
            renderer.DrawEntity(*val, camera);
        }
    }

    // Rendre les entités des faces top d'herbe pour chaque biome
    for (int i = 0; i < 5; ++i) {
        for (auto const& [key, val] : m_GrassTopEntities[i])
        {
            glm::vec3 chunkPos = val->GetPosition() + glm::vec3(Chunk::SIZE * 0.5f, 0.0f, Chunk::SIZE * 0.5f);
            float distSq = glm::dot(camPos - chunkPos, camPos - chunkPos);

            if (distSq <= maxRenderDistSq) {
                renderer.DrawEntity(*val, camera);
            }
        }
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

Constants::BiomeType Chunk::GetBiomeAt(int worldX, int worldZ) {
    // Simple biome assignment based on position
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

// Méthodes LOD

// Modifié : Logique de demande de chargement/déchargement ajoutée
void VoxelWorld::UpdateLOD(const glm::vec3& cameraPosition, double deltaTime) {
    int camChunkX, camChunkZ;
    WorldToChunk(static_cast<int>(cameraPosition.x), static_cast<int>(cameraPosition.z), camChunkX, camChunkZ);

    // Convertir la distance monde en distance chunks (1 chunk = 16 blocs)
    int displayDistChunks = static_cast<int>(m_LODSystem.getSettings().displayDistance / Chunk::SIZE) + 1;

    // 1. Demander les chunks nécessaires
    for (int z = camChunkZ - displayDistChunks; z <= camChunkZ + displayDistChunks; ++z) {
        for (int x = camChunkX - displayDistChunks; x <= camChunkX + displayDistChunks; ++x) {

            glm::vec3 chunkCenter(x * Chunk::SIZE + Chunk::SIZE / 2.0f, 0.0f, z * Chunk::SIZE + Chunk::SIZE / 2.0f);

            // Vérifie la distance d'affichage circulaire
            if (m_LODSystem.shouldDisplay(chunkCenter, cameraPosition)) {
                NihilEngine::ChunkLODLevel requiredLOD = GetChunkLOD(x, z, cameraPosition);
                uint64_t key = GetChunkKey(x, z);

                auto it = m_ChunkLODLevels.find(key);
                bool needsUpdate = false;

                if (it == m_ChunkLODLevels.end()) {
                    // Nouveau chunk, il faut le charger
                    needsUpdate = true;
                } else if (it->second != requiredLOD) {
                    // Le LOD a changé, il faut le mettre à jour
                    needsUpdate = true;
                }

                if (needsUpdate) {
                    float distance = glm::distance(glm::vec2(cameraPosition.x, cameraPosition.z), glm::vec2(chunkCenter.x, chunkCenter.z));
                    double priority = 1000.0 / (distance + 1.0); // Plus c'est proche, plus la priorité est haute
                    m_ProgressiveUpdate.requestChunkUpdate(x, z, requiredLOD, priority);
                }
            }
        }
    }

    // 2. Traiter la file d'attente de mise à jour
    m_ProgressiveUpdate.updateChunks(deltaTime, cameraPosition, m_LODSystem, m_ChunkDataCache,
        [this](int chunkX, int chunkZ, NihilEngine::ChunkLODLevel targetLOD) {
            this->GenerateChunkLOD(chunkX, chunkZ, targetLOD);
        });

    // 3. Décharger les chunks trop lointains
    for (auto it = m_ChunkEntities.begin(); it != m_ChunkEntities.end(); ) {
        uint64_t key = it->first;
        int32_t chunkX = (key >> 32);
        int32_t chunkZ = (key & 0xFFFFFFFF);

        glm::vec3 chunkCenter(chunkX * Chunk::SIZE + Chunk::SIZE / 2.0f, 0.0f, chunkZ * Chunk::SIZE + Chunk::SIZE / 2.0f);

        if (!m_LODSystem.shouldDisplay(chunkCenter, cameraPosition)) {
            // Décharger ce chunk
            m_ProgressiveUpdate.cancelChunkUpdate(chunkX, chunkZ);
            m_ChunkLODLevels.erase(key);
            m_Chunks.erase(key);
            for (auto& grassMap : m_GrassTopEntities) {
                grassMap.erase(key);
            }
            it = m_ChunkEntities.erase(it); // Efface l'entité principale
        } else {
            ++it;
        }
    }

    // Nettoyer les anciennes données de cache
    m_ChunkDataCache.cleanupOldData(0.0, 300.0); // 5 minutes
}

// Modifié : Corrigé la génération de LOD faible
void VoxelWorld::GenerateChunkLOD(int chunkX, int chunkZ, NihilEngine::ChunkLODLevel lodLevel) {
    uint64_t key = GetChunkKey(chunkX, chunkZ);

    // Confirme que ce chunk est en cours de traitement ou traité
    m_ChunkLODLevels[key] = lodLevel;

    // Supprime les anciennes entités pour ce chunk (au cas où on change de LOD)
    m_ChunkEntities.erase(key);
    for (auto& grassMap : m_GrassTopEntities) {
        grassMap.erase(key);
    }

    if (lodLevel == NihilEngine::ChunkLODLevel::HIGH_DETAIL) {
        // Génération complète du chunk (voxels)
        GenerateChunk(chunkX, chunkZ);

    } else {
        // Générer un mesh LOD basé sur la heightmap

        const int size = Chunk::SIZE;
        float scale = 1.0f; // Doit correspondre à votre échelle de monde
        std::vector<std::vector<float>> heightMap(size, std::vector<float>(size));
        std::vector<std::vector<int>> biomeMapInt(size, std::vector<int>(size));

        auto& terrainGen = m_ProceduralGen.getTerrainGenerator();
        auto& biomeGen = m_ProceduralGen.getBiomeGenerator();

        for (int z = 0; z < size; ++z) {
            for (int x = 0; x < size; ++x) {
                float worldX = (chunkX * size + x) * scale;
                float worldZ = (chunkZ * size + z) * scale;
                float h = terrainGen.getHeight(worldX, worldZ);
                heightMap[z][x] = h;
                auto biome = biomeGen.getBiome(worldX, worldZ, h);
                biomeMapInt[z][x] = static_cast<int>(biome);
            }
        }

        // Choisir entre mesh détaillé LOD ou mesh simplifié
        std::unique_ptr<NihilEngine::Mesh> lodMesh;

        if (lodLevel == NihilEngine::ChunkLODLevel::MEDIUM_DETAIL) {
            // Utiliser generateMesh avec un LOD moyen (terrain détaillé mais sampé)
            lodMesh = m_LODMeshGenerator.generateMesh(chunkX, chunkZ, heightMap, biomeMapInt, lodLevel);
        } else {
            // Pour LOW_DETAIL et VERY_LOW_DETAIL, utiliser le mesh simplifié
            NihilEngine::SimplifiedChunkData simplifiedData = m_ChunkDataCache.generateSimplifiedData(
                chunkX, chunkZ, heightMap, biomeMapInt, lodLevel);
            m_ChunkDataCache.updateChunkData(chunkX, chunkZ, simplifiedData);
            lodMesh = m_LODMeshGenerator.generateSimplifiedMesh(simplifiedData);
        }

        // Créer une entité avec le mesh LOD
        auto entity = std::make_unique<NihilEngine::Entity>(
            std::move(*lodMesh),
            glm::vec3(chunkX * Chunk::SIZE, 0.0f, chunkZ * Chunk::SIZE)
        );

        // Appliquer une couleur simple basée sur le biome dominant
        NihilEngine::Material lodMaterial;
        lodMaterial.color = glm::vec4(0.3f, 0.6f, 0.3f, 1.0f); // Vert par défaut
        entity->SetMaterial(lodMaterial);

        m_ChunkEntities[key] = std::move(entity);
    }
}

NihilEngine::ChunkLODLevel VoxelWorld::GetChunkLOD(int chunkX, int chunkZ, const glm::vec3& cameraPosition) const {
    glm::vec3 chunkCenter(chunkX * Chunk::SIZE + Chunk::SIZE / 2.0f, 0.0f, chunkZ * Chunk::SIZE + Chunk::SIZE / 2.0f);
    return m_LODSystem.getLODLevel(chunkCenter, cameraPosition);
}

void VoxelWorld::UpdateChunkMeshLOD(int chunkX, int chunkZ, NihilEngine::ChunkLODLevel newLOD) {
    uint64_t key = GetChunkKey(chunkX, chunkZ);

    // Si le chunk existe déjà avec un LOD différent, le régénérer
    if (m_Chunks.find(key) != m_Chunks.end() || m_ChunkEntities.find(key) != m_ChunkEntities.end()) {
        // Demande une mise à jour
         m_ProgressiveUpdate.requestChunkUpdate(chunkX, chunkZ, newLOD, 1000.0); // Priorité haute
    } else {
        // Demander une génération LOD
         m_ProgressiveUpdate.requestChunkUpdate(chunkX, chunkZ, newLOD, 100.0); // Priorité normale
    }
}

} // namespace MonJeu