#pragma once

#include <NihilEngine/Noise.h>
#include <NihilEngine/TerrainGenerator.h>
#include <vector>
#include <glm/glm.hpp>

namespace NihilEngine {

struct RiverPoint {
    glm::vec2 position;
    float width;
    float depth;
};

class RiverGenerator {
public:
    RiverGenerator(unsigned int seed = 0);
    ~RiverGenerator() = default;

    // Génère une rivière à partir d'un point source
    std::vector<RiverPoint> generateRiver(const glm::vec2& startPos, const TerrainGenerator& terrainGen, float maxLength = 1000.0f);

    // Génère plusieurs rivières dans une région
    std::vector<std::vector<RiverPoint>> generateRivers(int width, int height, float scale, const TerrainGenerator& terrainGen, int numRivers = 5);

    // Modifie la heightmap pour inclure les rivières
    void carveRivers(std::vector<std::vector<float>>& heightMap, const std::vector<std::vector<RiverPoint>>& rivers, float scale);

private:
    Noise noise;

    // Trouve le point le plus bas adjacent
    glm::vec2 findLowestNeighbor(const glm::vec2& pos, const TerrainGenerator& terrainGen, float stepSize = 1.0f) const;

    // Vérifie si une position est valide (dans les limites)
    bool isValidPosition(const glm::vec2& pos, int width, int height, float scale) const;
};

}