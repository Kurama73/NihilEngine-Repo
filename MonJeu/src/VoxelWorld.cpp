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
      m_SaveManager(saveManager),
      m_LODMeshGenerator()
{
    // Configuration du système LOD (CORRIGÉE)
    NihilEngine::LODSettings lodSettings;

    // Définissez vos distances d'affichage
    float highDetailDist = 96.0f;    // ~6 chunks (Maillage Voxel complet)
    float mediumDetailDist = 192.0f; // ~12 chunks (Maillage simplifié 1/2)
    float lowDetailDist = 384.0f;    // ~24 chunks (Maillage simplifié 1/4)
    float displayDistance = 384.0f;  // Distance d'affichage max

    // *** LA CORRECTION EST ICI ***
    // Utiliser des distances échelonnées
    lodSettings.displayDistance = displayDistance;
    lodSettings.highDetailDistance = highDetailDist;
    lodSettings.mediumDetailDistance = mediumDetailDist;
    lodSettings.lowDetailDistance = lowDetailDist;
    lodSettings.veryLowDetailDistance = displayDistance; // VERY_LOW = tout le reste

    // La génération doit être un peu plus loin
    lodSettings.generationDistance = displayDistance + 32.0f;
    lodSettings.calculationDistance = displayDistance + 64.0f;

    m_LODSystem.setSettings(lodSettings);

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
        GenerateChunkLOD(chunkX, chunkZ, NihilEngine::ChunkLODLevel::HIGH_DETAIL);

        // Marquer comme généré au niveau HIGH_DETAIL
        m_ChunkLODLevels[key] = NihilEngine::ChunkLODLevel::HIGH_DETAIL;
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
    float maxRenderDist = m_LODSystem.getSettings().displayDistance;
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

    int displayDistChunks = static_cast<int>(m_LODSystem.getSettings().displayDistance / Chunk::SIZE) + 1;

    // 1. Demander les chunks necessaires
    for (int z = camChunkZ - displayDistChunks; z <= camChunkZ + displayDistChunks; ++z) {
        for (int x = camChunkX - displayDistChunks; x <= camChunkX + displayDistChunks; ++x) {

            glm::vec3 chunkCenter(x * Chunk::SIZE + Chunk::SIZE / 2.0f, 0.0f, z * Chunk::SIZE + Chunk::SIZE / 2.0f);

            if (m_LODSystem.shouldDisplay(chunkCenter, cameraPosition)) {
                // NOTE: Grâce à la MODIFICATION 1, 'requiredLOD' sera TOUJOURS 'HIGH_DETAIL'
                NihilEngine::ChunkLODLevel requiredLOD = GetChunkLOD(x, z, cameraPosition);
                uint64_t key = GetChunkKey(x, z);

                auto it = m_ChunkLODLevels.find(key);
                bool needsUpdate = false;

                if (it == m_ChunkLODLevels.end()) {
                    needsUpdate = true;
                } else if (it->second != requiredLOD) {
                    needsUpdate = true;
                }

                if (needsUpdate) {
                    float distance = glm::distance(glm::vec2(cameraPosition.x, cameraPosition.z), glm::vec2(chunkCenter.x, chunkCenter.z));
                    double priority = 1000.0 / (distance + 1.0);
                    m_ProgressiveUpdate.requestChunkUpdate(x, z, requiredLOD, priority);
                }
            }
        }
    }

    // 2. Traiter la file d'attente
    m_ProgressiveUpdate.updateChunks(deltaTime, cameraPosition, m_LODSystem, m_ChunkDataCache,
        [this](int chunkX, int chunkZ, NihilEngine::ChunkLODLevel targetLOD) {
            this->GenerateChunkLOD(chunkX, chunkZ, targetLOD);
        });

    // 3. Decharger les chunks
    for (auto it = m_ChunkEntities.begin(); it != m_ChunkEntities.end(); ) {
        uint64_t key = it->first;
        int32_t chunkX = (key >> 32);
        int32_t chunkZ = (key & 0xFFFFFFFF);

        glm::vec3 chunkCenter(chunkX * Chunk::SIZE + Chunk::SIZE / 2.0f, 0.0f, chunkZ * Chunk::SIZE + Chunk::SIZE / 2.0f);

        if (!m_LODSystem.shouldDisplay(chunkCenter, cameraPosition)) {
            m_ProgressiveUpdate.cancelChunkUpdate(chunkX, chunkZ);
            m_ChunkLODLevels.erase(key);
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

//
// ==============================================================================
// MODIFICATION 2: La Generation de LOD
// Nous supprimons tout le code qui n'est PAS pour 'HIGH_DETAIL'.
// ==============================================================================
//
void VoxelWorld::GenerateChunkLOD(int chunkX, int chunkZ, NihilEngine::ChunkLODLevel lodLevel) {
    uint64_t key = GetChunkKey(chunkX, chunkZ);
    m_ChunkLODLevels[key] = lodLevel;

    // Efface les anciens meshes pour ce chunk
    m_ChunkEntities.erase(key);
    // (L'ancien système d'herbe est déjà commenté, c'est bien)

    // *** LA LOGIQUE DE LOD EST ICI ***

    // 1. LOD ÉLEVÉ : Génération Voxel complète (ce que vous faites déjà)
    if (lodLevel == NihilEngine::ChunkLODLevel::HIGH_DETAIL) {
        GenerateChunk(chunkX, chunkZ); // Cette fonction crée le maillage voxel
        return; // Terminé pour ce chunk
    }

    // 2. LODs MOYENS/FAIBLES : Génération de maillage simplifié
    // Pour les LODs simplifiés, nous n'avons pas besoin des données voxel complètes,
    // juste de la carte de hauteur (heightmap) et des biomes.

    // A. Obtenir les données du terrain (simulé, car nous n'avons pas les vrais voxels)
    // Nous devons générer une heightmap locale pour ce chunk (16x16)
    // NOTE : Idéalement, cela devrait venir d'un cache (ChunkDataCache)

    std::vector<std::vector<float>> heightMap(Chunk::SIZE, std::vector<float>(Chunk::SIZE));
    std::vector<std::vector<int>> biomeMap(Chunk::SIZE, std::vector<int>(Chunk::SIZE));

    NihilEngine::TerrainGenerator& terrainGen = m_ProceduralGen.getTerrainGenerator();
    NihilEngine::BiomeGenerator& biomeGen = m_ProceduralGen.getBiomeGenerator();

    for (int z = 0; z < Chunk::SIZE; ++z) {
        for (int x = 0; x < Chunk::SIZE; ++x) {
            int worldX = chunkX * Chunk::SIZE + x;
            int worldZ = chunkZ * Chunk::SIZE + z;

            float height = terrainGen.getHeight(static_cast<float>(worldX), static_cast<float>(worldZ));
            heightMap[z][x] = height;

            NihilEngine::BiomeType biome = biomeGen.getBiome(static_cast<float>(worldX), static_cast<float>(worldZ), height);
            biomeMap[z][x] = static_cast<int>(biome);
        }
    }

    // B. Générer le maillage LOD simplifié
    std::unique_ptr<NihilEngine::Mesh> lodMesh = m_LODMeshGenerator.generateMesh(
        chunkX, chunkZ,
        heightMap,
        biomeMap,
        lodLevel
    );

    if (lodMesh && lodMesh->GetIndexCount() > 0) {
        // C. Créer l'entité
        auto lodEntity = std::make_unique<NihilEngine::Entity>(
            std::move(*lodMesh),
            glm::vec3(chunkX * Chunk::SIZE, 0.0f, chunkZ * Chunk::SIZE)
        );

        // D. Appliquer la texture (l'atlas)
        if (m_TextureAtlasID != 0) {
            NihilEngine::Material lodMaterial;
            lodMaterial.textureID = m_TextureAtlasID;
            lodMaterial.color = glm::vec4(1.0f); // Le shader de LOD devrait utiliser les couleurs de vertex
            lodEntity->SetMaterial(lodMaterial);
        }

        m_ChunkEntities[key] = std::move(lodEntity);
    }
}

NihilEngine::ChunkLODLevel VoxelWorld::GetChunkLOD(int chunkX, int chunkZ, const glm::vec3& cameraPosition) const {
    glm::vec3 chunkCenter(chunkX * Chunk::SIZE + Chunk::SIZE / 2.0f, 0.0f, chunkZ * Chunk::SIZE + Chunk::SIZE / 2.0f);
    return m_LODSystem.getLODLevel(chunkCenter, cameraPosition);
}

} // namespace MonJeu