#pragma once

#include <NihilEngine/TerrainGenerator.h>
#include <NihilEngine/BiomeGenerator.h>
#include <NihilEngine/RiverGenerator.h>
#include <NihilEngine/VegetationGenerator.h>
#include <NihilEngine/WaterGenerator.h>
#include <vector>
#include <memory>

namespace NihilEngine {

struct VoxelGenerationSettings {
    float baseHeight = 52.0f;
    float amplitude = 34.0f;
    float frequency = 0.0045f;
    int octaves = 5;
    float persistence = 0.52f;
    int seaLevel = 50;
    int soilDepth = 4;
};

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

    void setVoxelGenerationSettings(const VoxelGenerationSettings& settings);
    const VoxelGenerationSettings& getVoxelGenerationSettings() const { return m_VoxelSettings; }

    // Contrôle calcul terrain GPU
    void setTerrainGpuPreferred(bool enabled);
    bool isTerrainGpuPreferred() const;
    bool isTerrainGpuAvailable() const;
    bool wasLastTerrainGenerationGpu() const;
    double getLastTerrainGenerationMs() const;

private:
    unsigned int seed;
    std::unique_ptr<TerrainGenerator> terrainGen;
    std::unique_ptr<BiomeGenerator> biomeGen;
    std::unique_ptr<RiverGenerator> riverGen;
    std::unique_ptr<VegetationGenerator> vegGen;
    std::unique_ptr<WaterGenerator> waterGen;
    VoxelGenerationSettings m_VoxelSettings;

    void initializeGenerators();
};

}