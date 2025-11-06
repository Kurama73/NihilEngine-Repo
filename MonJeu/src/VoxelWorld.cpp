// src/VoxelWorld.cpp
#include <MonJeu/VoxelWorld.h>
#include <MonJeu/Constants.h>
#include <NihilEngine/Renderer.h>
#include <NihilEngine/Camera.h>
#include <NihilEngine/Performance.h>
#include <algorithm>
#include <iostream>

namespace MonJeu {

// ==============================================================================
// MODIFICATION 1: Le Constructeur
// Nous disons au système de LOD que la seule distance d'affichage
// est le "HIGH_DETAIL".
// ==============================================================================
VoxelWorld::VoxelWorld(unsigned int seed, NihilEngine::PhysicsWorld* physicsWorld, WorldSaveManager* saveManager)
    : m_ProceduralGen(seed),
      m_PhysicsWorld(physicsWorld),
      m_SaveManager(saveManager)
{
    // Distance d'affichage
    m_DisplayDistance = 384.0f;

    m_ProgressiveUpdate.setUpdateRate(4); // Augmenter un peu le nombre de chunks traités par frame
    m_ProgressiveUpdate.setMaxPendingUpdates(200);
}

// Génère de manière synchrone les chunks prioritaires autour d'une position (pour le spawn)
void VoxelWorld::GenerateSpawnArea(const glm::vec3& position, int radiusChunks) {
    std::cout << "[VoxelWorld] Generating spawn area around (" << position.x << ", " << position.z << ") with radius " << radiusChunks << " chunks..." << std::endl;

    int centerChunkX, centerChunkZ;
    WorldToChunk(static_cast<int>(position.x), static_cast<int>(position.z), centerChunkX, centerChunkZ);

    // Générer les chunks dans l'ordre de priorité :
    // 1. Chunk central (où le joueur se trouve)
    // 2. Chunks dans le champ de vision (devant)
    // 3. Chunks sur les côtés
    // 4. Chunks derrière

    std::vector<std::pair<int, int>> priorityOrder;

    // Chunk central en premier
    priorityOrder.emplace_back(0, 0);

    // Ensuite les chunks dans un rayon croissant, en priorisant devant
    for (int radius = 1; radius <= radiusChunks; ++radius) {
        // Devant (négatif en Z dans le système de coordonnées du jeu)
        for (int dz = -radius; dz <= -1; ++dz) {
            for (int dx = -radius; dx <= radius; ++dx) {
                if (abs(dx) == radius || abs(dz) == radius) {
                    priorityOrder.emplace_back(dx, dz);
                }
            }
        }

        // Côtés (gauche et droite)
        for (int dx = -radius; dx <= radius; ++dx) {
            if (dx != 0) {
                priorityOrder.emplace_back(dx, 0); // Même ligne Z
            }
        }

        // Derrière (positif en Z)
        for (int dz = 1; dz <= radius; ++dz) {
            for (int dx = -radius; dx <= radius; ++dx) {
                if (abs(dx) == radius || dz == radius) {
                    priorityOrder.emplace_back(dx, dz);
                }
            }
        }
    }

    // Générer les chunks dans l'ordre de priorité
    for (const auto& [dx, dz] : priorityOrder) {
        int chunkX = centerChunkX + dx;
        int chunkZ = centerChunkZ + dz;

        uint64_t key = GetChunkKey(chunkX, chunkZ);

        // Vérifier si le chunk existe déjà
        if (m_Chunks.find(key) != m_Chunks.end()) continue;

        std::cout << "[VoxelWorld] Generating chunk (" << chunkX << ", " << chunkZ << ") for spawn area..." << std::endl;

        // Générer le chunk de manière synchrone
        GenerateChunk(chunkX, chunkZ);
    }

    std::cout << "[VoxelWorld] Spawn area generation completed." << std::endl;
}

// Genère un chunk de voxels (Haut detail)
void VoxelWorld::GenerateChunk(int chunkX, int chunkZ) {
    uint64_t key = GetChunkKey(chunkX, chunkZ);
    if (m_Chunks.find(key) != m_Chunks.end()) return;

    std::unique_ptr<Chunk> chunk;

    // Essaie de charger le chunk depuis la sauvegarde
    if (m_SaveManager) {
        chunk = m_SaveManager->LoadChunk(chunkX, chunkZ);
    }

    // Genère proceduralement si pas de sauvegarde
    if (!chunk) {
        Constants::BiomeType biome = Chunk::GetBiomeAt(chunkX * Chunk::SIZE, chunkZ * Chunk::SIZE);
        chunk = std::make_unique<Chunk>(chunkX, chunkZ, biome);
        chunk->GenerateTerrain(m_ProceduralGen);
    }

    auto meshes = chunk->CreateMeshes();

    // Entite principale
    auto mainEntity = std::make_unique<NihilEngine::Entity>(
        std::move(*meshes.mainMesh),
        glm::vec3(chunk->GetChunkX() * Chunk::SIZE, 0.0f, chunk->GetChunkZ() * Chunk::SIZE)
    );

    // Entites d'herbe - SUPPRIMÉ : Trop lourd, à remplacer par une meilleure approche
    // std::vector<std::unique_ptr<NihilEngine::Entity>> grassTopEntities;
    // for (int i = 0; i < 5; ++i) {
    //     auto grassTopEntity = std::make_unique<NihilEngine::Entity>(
    //         std::move(*meshes.grassTopMeshes[i]),
    //         glm::vec3(chunk->GetChunkX() * Chunk::SIZE, 0.0f, chunk->GetChunkZ() * Chunk::SIZE)
    //     );
    //     grassTopEntities.push_back(std::move(grassTopEntity));
    // }

    // Appliquer les materiaux (après le move)
    if (m_TextureAtlasID != 0) {
        NihilEngine::Material mainMaterial;
        mainMaterial.textureID = m_TextureAtlasID;
        mainMaterial.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f); // Couleur blanche neutre
        mainEntity->SetMaterial(mainMaterial);

        // for (int i = 0; i < 5; ++i) {
        //     NihilEngine::Material grassTopMaterial;
        //     grassTopMaterial.textureID = m_TextureAtlasID;
        //     // OPTIMISATION : Couleur par défaut unique pour toute l'herbe (au lieu de 5 couleurs de biome)
        //     grassTopMaterial.color = glm::vec4(0.4f, 0.8f, 0.2f, 1.0f); // Vert d'herbe standard
        //     grassTopEntities[i]->SetMaterial(grassTopMaterial);
        // }
    }

    m_Chunks[key] = std::move(chunk);
    m_ChunkEntities[key] = std::move(mainEntity);
    // for (int i = 0; i < 5; ++i) {
    //     m_GrassTopEntities[i][key] = std::move(grassTopEntities[i]);
    // }
}

void VoxelWorld::UpdateDirtyChunks() {
    // [Logique de UpdateDirtyChunks - Inchangee]
    std::sort(m_DirtyChunks.begin(), m_DirtyChunks.end());
    m_DirtyChunks.erase(std::unique(m_DirtyChunks.begin(), m_DirtyChunks.end()), m_DirtyChunks.end());

    for (uint64_t key : m_DirtyChunks) {
        if (m_Chunks.find(key) != m_Chunks.end()) {
            const Chunk& chunk = *m_Chunks[key];
            auto meshes = chunk.CreateMeshes();

            m_ChunkEntities[key]->SetMesh(std::move(*meshes.mainMesh));
            // for (int i = 0; i < 5; ++i) {
            //     m_GrassTopEntities[i][key]->SetMesh(std::move(*meshes.grassTopMeshes[i]));
            // } - COMMENTE: suppression du système d'entités d'herbe

            if (m_TextureAtlasID != 0) {
                NihilEngine::Material mainMaterial;
                mainMaterial.textureID = m_TextureAtlasID;
                mainMaterial.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
                m_ChunkEntities[key]->SetMaterial(mainMaterial);

                // for (int i = 0; i < 5; ++i) {
                //     NihilEngine::Material grassTopMaterial;
                //     grassTopMaterial.textureID = m_TextureAtlasID;
                //     const Constants::Biome& biomeData = Constants::BIOMES[i];
                //     grassTopMaterial.color = glm::vec4(biomeData.grassColor, 1.0f);
                //     m_GrassTopEntities[i][key]->SetMaterial(grassTopMaterial);
                // } - COMMENTE: suppression du système d'entités d'herbe
            }

            // Sauvegarde automatique du chunk modifie
            if (m_SaveManager) {
                m_SaveManager->SaveChunk(chunk);
                std::cout << "[VoxelWorld] Chunk sauvegarde: (" << chunk.GetChunkX() << ", " << chunk.GetChunkZ() << ")" << std::endl;
            }
        }
    }
    m_DirtyChunks.clear();
}

void VoxelWorld::Render(NihilEngine::Renderer& renderer, const NihilEngine::Camera& camera) {
    // [Logique de Render - Inchangee]
    glm::vec3 camPos = camera.GetPosition();
    float maxRenderDist = m_DisplayDistance;
    float maxRenderDistSq = maxRenderDist * maxRenderDist;

    // Mesurer le rendu des entités principales
    NihilEngine::PerformanceMonitor::getInstance().startSection("Render_MainEntities");
    for (auto const& [key, val] : m_ChunkEntities)
    {
        glm::vec3 chunkPos = val->GetPosition() + glm::vec3(Chunk::SIZE * 0.5f, 0.0f, Chunk::SIZE * 0.5f);
        float distSq = glm::dot(camPos - chunkPos, camPos - chunkPos);

        if (distSq <= maxRenderDistSq) {
            renderer.DrawEntity(*val, camera);
        }
    }
    NihilEngine::PerformanceMonitor::getInstance().endSection("Render_MainEntities");

    // Mesurer le rendu des entités d'herbe
    NihilEngine::PerformanceMonitor::getInstance().startSection("Render_GrassEntities");
    // SUPPRIMÉ : Système d'entités d'herbe trop lourd - à remplacer par une meilleure approche
    NihilEngine::PerformanceMonitor::getInstance().endSection("Render_GrassEntities");
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
    // [Logique de SetVoxelActive - Inchangee]
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

bool VoxelWorld::CheckCollision(const NihilEngine::AABB& box) const {
    // [Logique de CheckCollision - Inchangee]
    glm::ivec3 min = glm::floor(box.min);
    glm::ivec3 max = glm::floor(box.max);

    for (int y = min.y; y <= max.y; ++y) {
        for (int z = min.z; z <= max.z; ++z) {
            for (int x = min.x; x <= max.x; ++x) {
                if (GetVoxelActive(x, y, z)) {
                    return true;
                }
            }
        }
    }
    return false;
}

// --- Logique LOD ---

void VoxelWorld::UpdateLOD(const glm::vec3& cameraPosition, double deltaTime) {
    // [Logique de UpdateLOD - Inchangee]
    int camChunkX, camChunkZ;
    WorldToChunk(static_cast<int>(cameraPosition.x), static_cast<int>(cameraPosition.z), camChunkX, camChunkZ);

    glm::vec3 camPos = cameraPosition;
    float maxRenderDistSq = m_DisplayDistance * m_DisplayDistance;

    int displayDistChunks = static_cast<int>(m_DisplayDistance / Chunk::SIZE) + 1;

    // 1. Demander les chunks necessaires
    for (int z = camChunkZ - displayDistChunks; z <= camChunkZ + displayDistChunks; ++z) {
        for (int x = camChunkX - displayDistChunks; x <= camChunkX + displayDistChunks; ++x) {

            glm::vec3 chunkCenter(x * Chunk::SIZE + Chunk::SIZE / 2.0f, 0.0f, z * Chunk::SIZE + Chunk::SIZE / 2.0f);

            float distSq = glm::dot(camPos - chunkCenter, camPos - chunkCenter);
            if (distSq <= maxRenderDistSq) {
                uint64_t key = GetChunkKey(x, z);

                if (m_ChunkEntities.find(key) == m_ChunkEntities.end()) {
                    float distance = glm::distance(glm::vec2(cameraPosition.x, cameraPosition.z), glm::vec2(chunkCenter.x, chunkCenter.z));
                    double priority = 1000.0 / (distance + 1.0);
                    m_ProgressiveUpdate.requestChunkUpdate(x, z, priority);
                }
            }
        }
    }

    // 2. Traiter la file d'attente
    m_ProgressiveUpdate.updateChunks(deltaTime, cameraPosition, m_ChunkDataCache,
        [this](int chunkX, int chunkZ) {
            this->GenerateChunk(chunkX, chunkZ);
        });

    // 3. Decharger les chunks
    for (auto it = m_ChunkEntities.begin(); it != m_ChunkEntities.end(); ) {
        uint64_t key = it->first;
        int32_t chunkX = (key >> 32);
        int32_t chunkZ = (key & 0xFFFFFFFF);

        glm::vec3 chunkCenter(chunkX * Chunk::SIZE + Chunk::SIZE / 2.0f, 0.0f, chunkZ * Chunk::SIZE + Chunk::SIZE / 2.0f);

        float distSq = glm::dot(camPos - chunkCenter, camPos - chunkCenter);
        if (distSq > maxRenderDistSq) {
            m_ProgressiveUpdate.cancelChunkUpdate(chunkX, chunkZ);
            m_Chunks.erase(key);
            // for (auto& grassMap : m_GrassTopEntities) {
            //     grassMap.erase(key);
            // } - COMMENTE: suppression du système d'entités d'herbe
            it = m_ChunkEntities.erase(it);
        } else {
            ++it;
        }
    }

    m_ChunkDataCache.cleanupOldData(0.0, 300.0);
}

} // namespace MonJeu