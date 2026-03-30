#pragma once

#include <NihilEngine/Noise.h>
#include <glad/glad.h>
#include <vector>
#include <glm/glm.hpp>

namespace NihilEngine {

class TerrainGenerator {
public:
    TerrainGenerator(unsigned int seed = 0);
    ~TerrainGenerator();

    // Génère une hauteur pour une position donnée
    float getHeight(float x, float z) const;

    // Génère une carte de hauteur pour une région
    std::vector<std::vector<float>> generateHeightMap(int width, int height, float scale = 1.0f) const;

    // Génère une carte de hauteur pour une région monde donnée (batch GPU si possible).
    std::vector<std::vector<float>> generateHeightMapRegion(int startX, int startZ, int width, int height, float scale = 1.0f) const;

    // Paramètres de génération
    void setBaseHeight(float height) { baseHeight = height; }
    void setAmplitude(float amp) { amplitude = amp; }
    void setFrequency(float freq) { frequency = freq; }
    void setOctaves(int oct) { octaves = oct; }
    void setPersistence(float pers) { persistence = pers; }

    // Contrôle/runtime stats du backend terrain.
    void SetGpuPreferred(bool enabled) { m_UseGpuPreferred = enabled; }
    bool IsGpuPreferred() const { return m_UseGpuPreferred; }
    bool IsGpuComputeAvailable() const;
    bool WasLastGenerationGpu() const { return m_LastGenerationUsedGpu; }
    double GetLastGenerationMs() const { return m_LastGenerationMs; }

private:
    bool InitGpuCompute() const;
    bool GenerateHeightMapRegionGpu(int startX, int startZ, int width, int height, float scale, std::vector<float>& outHeights) const;

    Noise noise;
    unsigned int m_Seed;
    float baseHeight;
    float amplitude;
    float frequency;
    int octaves;
    float persistence;

    mutable bool m_GpuInitAttempted = false;
    mutable bool m_GpuAvailable = false;
    mutable bool m_UseGpuPreferred = false;
    mutable bool m_LastGenerationUsedGpu = false;
    mutable double m_LastGenerationMs = 0.0;
    mutable GLuint m_ComputeProgram = 0;
    mutable GLuint m_HeightSsbo = 0;
};

}