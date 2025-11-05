#include <NihilEngine/Mesh.h>
#include <NihilEngine/LODSystem.h>
#include <NihilEngine/ChunkDataCache.h>
#include <vector>
#include <memory>

namespace NihilEngine {

struct LODMesh {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    std::vector<float> normals;
    std::vector<float> texCoords;
    std::vector<float> colors;

    void clear() {
        vertices.clear();
        indices.clear();
        normals.clear();
        texCoords.clear();
        colors.clear();
    }

    bool empty() const {
        return vertices.empty();
    }
};

class LODMeshGenerator {
public:
    LODMeshGenerator();
    ~LODMeshGenerator() = default;

    // Génération de mesh pour différents niveaux de LOD
    std::unique_ptr<NihilEngine::Mesh> generateMesh(
        int chunkX, int chunkZ,
        const std::vector<std::vector<float>>& heightMap,
        const std::vector<std::vector<int>>& biomeMap,
        ChunkLODLevel lodLevel
    );

    // Génération de mesh simplifié à partir des données cachées
    std::unique_ptr<NihilEngine::Mesh> generateSimplifiedMesh(
        const SimplifiedChunkData& chunkData
    );

private:
    // Paramètres de génération par LOD
    struct LODParams {
        int vertexStep;     // Pas entre les vertices (1 = haute résolution, 2 = demi-résolution, etc.)
        float heightScale;  // Échelle de hauteur
        bool smoothNormals; // Lisser les normales
    };

    LODParams getLODParams(ChunkLODLevel level) const;

    // Génération de mesh détaillé
    void generateDetailedMesh(LODMesh& mesh,
                             int chunkX, int chunkZ,
                             const std::vector<std::vector<float>>& heightMap,
                             const std::vector<std::vector<int>>& biomeMap,
                             const LODParams& params);

    // Génération de mesh simplifié
    void generateSimplifiedMeshImpl(LODMesh& mesh, const SimplifiedChunkData& chunkData);

    // Utilitaires
    glm::vec3 getBiomeColor(int biomeId) const;
    glm::vec3 calculateNormal(const std::vector<std::vector<float>>& heightMap,
                             int x, int z, int width, int height) const;
    void addVertex(LODMesh& mesh, const glm::vec3& position,
                  const glm::vec3& normal, const glm::vec2& texCoord,
                  const glm::vec3& color);
    void addTriangle(LODMesh& mesh, unsigned int i1, unsigned int i2, unsigned int i3);
    std::unique_ptr<NihilEngine::Mesh> convertToMesh(const LODMesh& lodMesh);
};

}