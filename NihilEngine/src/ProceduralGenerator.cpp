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

void ProceduralGenerator::initializeGenerators() {
    terrainGen = std::make_unique<TerrainGenerator>(seed);
    biomeGen = std::make_unique<BiomeGenerator>(seed);
    riverGen = std::make_unique<RiverGenerator>(seed);
    vegGen = std::make_unique<VegetationGenerator>(seed);
    waterGen = std::make_unique<WaterGenerator>(seed);
}

}