#pragma once

#include <vector>
#include <glm/glm.hpp>

namespace NihilEngine {

class Noise {
public:
    Noise(unsigned int seed = 0);
    ~Noise() = default;

    // Génère du bruit Perlin 2D
    float perlin2D(float x, float y) const;

    // Génère du bruit Perlin 3D
    float perlin3D(float x, float y, float z) const;

    // Génère du bruit Simplex 2D (plus rapide que Perlin)
    float simplex2D(float x, float y) const;

    // Génère du bruit fractal (multiple octaves)
    float fractal(float x, float y, int octaves = 4, float persistence = 0.5f, float scale = 1.0f) const;

    // Génère du bruit de ridged (pour montagnes)
    float ridged(float x, float y, int octaves = 4, float persistence = 0.5f, float scale = 1.0f) const;

private:
    std::vector<int> p; // Permutation table

    // Fonctions d'interpolation
    float fade(float t) const;
    float lerp(float a, float b, float t) const;
    float grad(int hash, float x, float y, float z) const;

    // Fonctions pour Simplex
    float simplexGrad(int hash, float x, float y) const;
};

}