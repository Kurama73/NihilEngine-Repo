#pragma once

#include <NihilEngine/Noise.h>
#include <vector>
#include <glm/glm.hpp>

namespace NihilEngine {

class TerrainGenerator {
public:
    TerrainGenerator(unsigned int seed = 0);
    ~TerrainGenerator() = default;

    // Génère une hauteur pour une position donnée
    float getHeight(float x, float z) const;

    // Génère une carte de hauteur pour une région
    std::vector<std::vector<float>> generateHeightMap(int width, int height, float scale = 1.0f) const;

    // Paramètres de génération
    void setBaseHeight(float height) { baseHeight = height; }
    void setAmplitude(float amp) { amplitude = amp; }
    void setFrequency(float freq) { frequency = freq; }
    void setOctaves(int oct) { octaves = oct; }
    void setPersistence(float pers) { persistence = pers; }

private:
    Noise noise;
    float baseHeight;
    float amplitude;
    float frequency;
    int octaves;
    float persistence;
};

}