#pragma once

#include <NihilEngine/TerrainGenerator.h>
#include <NihilEngine/BiomeGenerator.h>
#include <NihilEngine/RiverGenerator.h>
#include <NihilEngine/VegetationGenerator.h>
#include <NihilEngine/WaterGenerator.h>
#include <vector>
#include <memory>

namespace NihilEngine {

struct ProceduralWorld {
    std::vector<std::vector<float>> heightMap;
    std::vector<std::vector<BiomeType>> biomeMap;
    std::vector<std::vector<RiverPoint>> rivers;
    std::vector<VegetationInstance> vegetation;
    std::vector<WaterBody> waterBodies;
};

class ProceduralGenerator {
public:
    ProceduralGenerator(unsigned int seed = 0);
    ~ProceduralGenerator() = default;

    // Génère un monde procédural complet
    std::unique_ptr<ProceduralWorld> generateWorld(int width, int height, float scale = 1.0f);

    // Accès aux générateurs individuels pour configuration
    TerrainGenerator& getTerrainGenerator() { return *terrainGen; }
    BiomeGenerator& getBiomeGenerator() { return *biomeGen; }
    RiverGenerator& getRiverGenerator() { return *riverGen; }
    VegetationGenerator& getVegetationGenerator() { return *vegGen; }
    WaterGenerator& getWaterGenerator() { return *waterGen; }

    // Paramètres globaux
    void setSeed(unsigned int seed);
    unsigned int getSeed() const { return seed; }

private:
    unsigned int seed;
    std::unique_ptr<TerrainGenerator> terrainGen;
    std::unique_ptr<BiomeGenerator> biomeGen;
    std::unique_ptr<RiverGenerator> riverGen;
    std::unique_ptr<VegetationGenerator> vegGen;
    std::unique_ptr<WaterGenerator> waterGen;

    void initializeGenerators();
};

}