#include <NihilEngine/ProceduralGenerator.h>

namespace NihilEngine {

ProceduralGenerator::ProceduralGenerator(unsigned int seed) : seed(seed) {
    initializeGenerators();
}

std::unique_ptr<ProceduralWorld> ProceduralGenerator::generateWorld(int width, int height, float scale) {
    auto world = std::make_unique<ProceduralWorld>();

    // 1. Génère le terrain de base
    world->heightMap = terrainGen->generateHeightMap(width, height, scale);

    // 2. Génère les biomes
    world->biomeMap = biomeGen->generateBiomeMap(width, height, scale, world->heightMap);

    // 3. Génère les rivières et modifie le terrain
    world->rivers = riverGen->generateRivers(width, height, scale, *terrainGen);
    riverGen->carveRivers(world->heightMap, world->rivers, scale);

    // 4. Génère les corps d'eau
    world->waterBodies = waterGen->generateWaterBodies(width, height, scale, world->heightMap);
    waterGen->applyWaterLevels(world->heightMap, world->waterBodies, scale);

    // 5. Génère la végétation
    world->vegetation = vegGen->generateVegetation(width, height, scale, world->biomeMap, world->heightMap);

    return world;
}

void ProceduralGenerator::setSeed(unsigned int newSeed) {
    seed = newSeed;
    initializeGenerators();
}

void ProceduralGenerator::setVoxelGenerationSettings(const VoxelGenerationSettings& settings) {
    m_VoxelSettings = settings;
    if (terrainGen) {
        terrainGen->setBaseHeight(m_VoxelSettings.baseHeight);
        terrainGen->setAmplitude(m_VoxelSettings.amplitude);
        terrainGen->setFrequency(m_VoxelSettings.frequency);
        terrainGen->setOctaves(m_VoxelSettings.octaves);
        terrainGen->setPersistence(m_VoxelSettings.persistence);
    }
}

void ProceduralGenerator::setTerrainGpuPreferred(bool enabled) {
    if (terrainGen) {
        terrainGen->SetGpuPreferred(enabled);
    }
}

bool ProceduralGenerator::isTerrainGpuPreferred() const {
    return terrainGen ? terrainGen->IsGpuPreferred() : false;
}

bool ProceduralGenerator::isTerrainGpuAvailable() const {
    return terrainGen ? terrainGen->IsGpuComputeAvailable() : false;
}

bool ProceduralGenerator::wasLastTerrainGenerationGpu() const {
    return terrainGen ? terrainGen->WasLastGenerationGpu() : false;
}

double ProceduralGenerator::getLastTerrainGenerationMs() const {
    return terrainGen ? terrainGen->GetLastGenerationMs() : 0.0;
}

void ProceduralGenerator::initializeGenerators() {
    terrainGen = std::make_unique<TerrainGenerator>(seed);
    biomeGen = std::make_unique<BiomeGenerator>(seed);
    riverGen = std::make_unique<RiverGenerator>(seed);
    vegGen = std::make_unique<VegetationGenerator>(seed);
    waterGen = std::make_unique<WaterGenerator>(seed);

    // Apply world-profile defaults in the engine so game code stays lightweight.
    setVoxelGenerationSettings(m_VoxelSettings);
}

}