#include <NihilEngine/LODMeshGenerator.h>
#include <glm/glm.hpp>
#include <glm/gtc/noise.hpp>
#include <algorithm>
#include <memory>

namespace NihilEngine {

LODMeshGenerator::LODMeshGenerator() = default;

std::unique_ptr<NihilEngine::Mesh> LODMeshGenerator::generateMesh(
    int chunkX, int chunkZ,
    const std::vector<std::vector<float>>& heightMap,
    const std::vector<std::vector<int>>& biomeMap,
    ChunkLODLevel lodLevel
) {
    LODMesh lodMesh;
    LODParams params = getLODParams(lodLevel);

    // Pour les LOD moyens et élevés, utiliser la génération voxel-based
    if (lodLevel == ChunkLODLevel::HIGH_DETAIL || lodLevel == ChunkLODLevel::MEDIUM_DETAIL) {
        generateVoxelBasedMesh(lodMesh, chunkX, chunkZ, heightMap, biomeMap, params);
    } else {
        // Pour les LOD faibles, utiliser la génération simplifiée mais améliorée
        generateDetailedMesh(lodMesh, chunkX, chunkZ, heightMap, biomeMap, params);
    }

    return convertToMesh(lodMesh);
}

std::unique_ptr<NihilEngine::Mesh> LODMeshGenerator::generateSimplifiedMesh(
    const SimplifiedChunkData& chunkData
) {
    LODMesh lodMesh;
    generateSimplifiedMeshImpl(lodMesh, chunkData);
    return convertToMesh(lodMesh);
}

LODMeshGenerator::LODParams LODMeshGenerator::getLODParams(ChunkLODLevel level) const {
    switch (level) {
        case ChunkLODLevel::HIGH_DETAIL:
            return {1, 1.0f, true};  // Résolution complète
        case ChunkLODLevel::MEDIUM_DETAIL:
            return {2, 1.0f, true}; // Résolution 1/2
        case ChunkLODLevel::LOW_DETAIL:
            return {4, 0.8f, false}; // Résolution 1/4
        case ChunkLODLevel::VERY_LOW_DETAIL:
            return {8, 0.5f, false}; // Résolution 1/8
        default:
            return {4, 1.0f, false};
    }
}

void LODMeshGenerator::generateVoxelBasedMesh(
    LODMesh& mesh,
    int chunkX, int chunkZ,
    const std::vector<std::vector<float>>& heightMap,
    const std::vector<std::vector<int>>& biomeMap,
    const LODParams& params
) {
    const int chunkSize = 16;
    const float voxelSize = 1.0f;

    int width = heightMap.empty() ? 0 : static_cast<int>(heightMap[0].size());
    int height = static_cast<int>(heightMap.size());

    if (width == 0 || height == 0) return;

    // Pour les LOD, on réduit la résolution en sautant des voxels
    int step = params.vertexStep;

    // Génération des vertices - échantillonnage du terrain réel
    for (int z = 0; z < height; z += step) {
        for (int x = 0; x < width; x += step) {
            float worldX = chunkX * chunkSize + x;
            float worldZ = chunkZ * chunkSize + z;
            float terrainHeight = heightMap[z][x];

            // Position du vertex
            glm::vec3 position(worldX, terrainHeight, worldZ);

            // Normal simplifiée (vers le haut pour le terrain)
            glm::vec3 normal(0.0f, 1.0f, 0.0f);

            // Coordonnées de texture
            glm::vec2 texCoord(
                static_cast<float>(x) / (width - 1),
                static_cast<float>(z) / (height - 1)
            );

            // Couleur basée sur le biome
            int biomeId = biomeMap[z][x];
            glm::vec3 color = getBiomeColor(biomeId);

            addVertex(mesh, position, normal, texCoord, color);
        }
    }

    // Génération des triangles
    int gridWidth = (width + step - 1) / step;  // Nombre de points en X
    int gridHeight = (height + step - 1) / step; // Nombre de points en Z

    for (int z = 0; z < gridHeight - 1; ++z) {
        for (int x = 0; x < gridWidth - 1; ++x) {
            unsigned int topLeft = z * gridWidth + x;
            unsigned int topRight = topLeft + 1;
            unsigned int bottomLeft = (z + 1) * gridWidth + x;
            unsigned int bottomRight = bottomLeft + 1;

            // Premier triangle
            addTriangle(mesh, topLeft, bottomLeft, topRight);
            // Deuxième triangle
            addTriangle(mesh, topRight, bottomLeft, bottomRight);
        }
    }
}

void LODMeshGenerator::generateDetailedMesh(
    LODMesh& mesh,
    int chunkX, int chunkZ,
    const std::vector<std::vector<float>>& heightMap,
    const std::vector<std::vector<int>>& biomeMap,
    const LODParams& params
) {
    const int chunkSize = 16;
    const float vertexSpacing = 1.0f;

    int width = heightMap.empty() ? 0 : static_cast<int>(heightMap[0].size());
    int height = static_cast<int>(heightMap.size());

    if (width == 0 || height == 0) return;

    // Génération des vertices
    for (int z = 0; z < height; z += params.vertexStep) {
        for (int x = 0; x < width; x += params.vertexStep) {
            float worldX = chunkX * chunkSize + x * vertexSpacing;
            float worldZ = chunkZ * chunkSize + z * vertexSpacing;
            float h = heightMap[z][x] * params.heightScale;

            glm::vec3 position(worldX, h, worldZ);
            glm::vec3 normal = calculateNormal(heightMap, x, z, width, height);
            glm::vec2 texCoord(static_cast<float>(x) / width, static_cast<float>(z) / height);

            int biomeId = (z < static_cast<int>(biomeMap.size()) && x < static_cast<int>(biomeMap[z].size()))
                         ? biomeMap[z][x] : 0;
            glm::vec3 color = getBiomeColor(biomeId);

            addVertex(mesh, position, normal, texCoord, color);
        }
    }

    // Génération des indices (triangles)
    int verticesPerRow = (width + params.vertexStep - 1) / params.vertexStep;

    for (int z = 0; z < verticesPerRow - 1; ++z) {
        for (int x = 0; x < verticesPerRow - 1; ++x) {
            unsigned int topLeft = z * verticesPerRow + x;
            unsigned int topRight = topLeft + 1;
            unsigned int bottomLeft = (z + 1) * verticesPerRow + x;
            unsigned int bottomRight = bottomLeft + 1;

            // Premier triangle
            addTriangle(mesh, topLeft, bottomLeft, topRight);
            // Deuxième triangle
            addTriangle(mesh, topRight, bottomLeft, bottomRight);
        }
    }
}

void LODMeshGenerator::generateSimplifiedMeshImpl(LODMesh& mesh, const SimplifiedChunkData& chunkData) {
    // Au lieu de créer un plan plat, on utilise les données réelles du chunk
    // pour créer un mesh qui représente la topographie réelle

    const int chunkSize = 16;
    const int lodResolution = 4; // 4x4 échantillonnage pour LOD lointain
    const float step = static_cast<float>(chunkSize) / lodResolution;

    // Échantillonnage du terrain à intervalles réguliers
    for (int z = 0; z < lodResolution; ++z) {
        for (int x = 0; x < lodResolution; ++x) {
            // Position dans le chunk
            float localX = x * step;
            float localZ = z * step;

            // Position mondiale
            float worldX = chunkData.position.x + localX;
            float worldZ = chunkData.position.z + localZ;

            // Pour les LOD lointains, on utilise une approximation basée sur les données disponibles
            // Ici on pourrait utiliser une fonction de bruit ou interpoler depuis les données du chunk
            float height = chunkData.minHeight + 0.5f * (chunkData.maxHeight - chunkData.minHeight);

            // Essayer d'estimer la hauteur basée sur la position relative
            float centerX = chunkData.position.x + chunkSize * 0.5f;
            float centerZ = chunkData.position.z + chunkSize * 0.5f;
            float distFromCenter = glm::distance(glm::vec2(worldX, worldZ), glm::vec2(centerX, centerZ));
            float heightVariation = (chunkData.maxHeight - chunkData.minHeight) * (1.0f - distFromCenter / (chunkSize * 0.7f));

            height = chunkData.minHeight + heightVariation;

            glm::vec3 position(worldX, height, worldZ);
            glm::vec3 normal(0.0f, 1.0f, 0.0f);
            glm::vec2 texCoord(static_cast<float>(x) / (lodResolution - 1), static_cast<float>(z) / (lodResolution - 1));

            addVertex(mesh, position, normal, texCoord, chunkData.color);
        }
    }

    // Génération des triangles
    for (int z = 0; z < lodResolution - 1; ++z) {
        for (int x = 0; x < lodResolution - 1; ++x) {
            unsigned int topLeft = z * lodResolution + x;
            unsigned int topRight = topLeft + 1;
            unsigned int bottomLeft = (z + 1) * lodResolution + x;
            unsigned int bottomRight = bottomLeft + 1;

            addTriangle(mesh, topLeft, bottomLeft, topRight);
            addTriangle(mesh, topRight, bottomLeft, bottomRight);
        }
    }
}

glm::vec3 LODMeshGenerator::getBiomeColor(int biomeId) const {
    // Couleurs basées sur les biomes (à adapter selon vos biomes)
    switch (biomeId) {
        case 0: return glm::vec3(0.2f, 0.6f, 0.2f); // Forêt
        case 1: return glm::vec3(0.8f, 0.8f, 0.4f); // Plaine
        case 2: return glm::vec3(0.4f, 0.4f, 0.8f); // Océan
        case 3: return glm::vec3(0.6f, 0.4f, 0.2f); // Désert
        case 4: return glm::vec3(0.9f, 0.9f, 0.9f); // Montagne
        default: return glm::vec3(0.5f, 0.5f, 0.5f); // Défaut
    }
}

glm::vec3 LODMeshGenerator::calculateNormal(
    const std::vector<std::vector<float>>& heightMap,
    int x, int z, int width, int height
) const {
    // Calcul de normale simple basé sur les voisins
    float h = heightMap[z][x];

    float hx = (x > 0) ? heightMap[z][x-1] : h;
    float hX = (x < width-1) ? heightMap[z][x+1] : h;
    float hz = (z > 0) ? heightMap[z-1][x] : h;
    float hZ = (z < height-1) ? heightMap[z+1][x] : h;

    glm::vec3 normal(hx - hX, 2.0f, hz - hZ);
    return glm::normalize(normal);
}

void LODMeshGenerator::addVertex(LODMesh& mesh, const glm::vec3& position,
                                const glm::vec3& normal, const glm::vec2& texCoord,
                                const glm::vec3& color) {
    // Position (3 floats)
    mesh.vertices.push_back(position.x);
    mesh.vertices.push_back(position.y);
    mesh.vertices.push_back(position.z);

    // Normale (3 floats)
    mesh.normals.push_back(normal.x);
    mesh.normals.push_back(normal.y);
    mesh.normals.push_back(normal.z);

    // Coordonnées de texture (2 floats)
    mesh.texCoords.push_back(texCoord.x);
    mesh.texCoords.push_back(texCoord.y);

    // Couleur (4 floats - RGBA). Le pipeline Mesh attend 4 composantes pour la couleur.
    mesh.colors.push_back(color.r);
    mesh.colors.push_back(color.g);
    mesh.colors.push_back(color.b);
    mesh.colors.push_back(1.0f); // alpha
}

void LODMeshGenerator::addTriangle(LODMesh& mesh, unsigned int i1, unsigned int i2, unsigned int i3) {
    mesh.indices.push_back(i1);
    mesh.indices.push_back(i2);
    mesh.indices.push_back(i3);
}

std::unique_ptr<NihilEngine::Mesh> LODMeshGenerator::convertToMesh(const LODMesh& lodMesh) {
    // Assembler les données pour Mesh
    // Format: position (3), normal (3), texCoord (2), color (4) = 12 floats par vertex
    std::vector<float> vertices;
    vertices.reserve(lodMesh.vertices.size() / 3 * 12);

    size_t vertexCount = lodMesh.vertices.size() / 3;
    for (size_t i = 0; i < vertexCount; ++i) {
        // Position (3 floats)
        vertices.push_back(lodMesh.vertices[i * 3]);
        vertices.push_back(lodMesh.vertices[i * 3 + 1]);
        vertices.push_back(lodMesh.vertices[i * 3 + 2]);

        // Normal (3 floats)
        if (i * 3 < lodMesh.normals.size()) {
            vertices.push_back(lodMesh.normals[i * 3]);
            vertices.push_back(lodMesh.normals[i * 3 + 1]);
            vertices.push_back(lodMesh.normals[i * 3 + 2]);
        } else {
            // Normal par défaut
            vertices.push_back(0.0f);
            vertices.push_back(1.0f);
            vertices.push_back(0.0f);
        }

        // TexCoord (2 floats)
        if (i * 2 < lodMesh.texCoords.size()) {
            vertices.push_back(lodMesh.texCoords[i * 2]);
            vertices.push_back(lodMesh.texCoords[i * 2 + 1]);
        } else {
            // TexCoord par défaut
            vertices.push_back(0.0f);
            vertices.push_back(0.0f);
        }

        // Color (4 floats)
        if (i * 4 < lodMesh.colors.size()) {
            vertices.push_back(lodMesh.colors[i * 4 + 0]);
            vertices.push_back(lodMesh.colors[i * 4 + 1]);
            vertices.push_back(lodMesh.colors[i * 4 + 2]);
            vertices.push_back(lodMesh.colors[i * 4 + 3]);
        } else {
            // Couleur par défaut (avec alpha)
            vertices.push_back(0.5f);
            vertices.push_back(0.5f);
            vertices.push_back(0.5f);
            vertices.push_back(1.0f);
        }
    }

    // Attributs pour Mesh
    std::vector<NihilEngine::VertexAttribute> attributes = {
        NihilEngine::VertexAttribute::Position,
        NihilEngine::VertexAttribute::Normal,
        NihilEngine::VertexAttribute::TexCoord,
        NihilEngine::VertexAttribute::Color
    };

    return std::make_unique<NihilEngine::Mesh>(vertices, lodMesh.indices, attributes);
}

}