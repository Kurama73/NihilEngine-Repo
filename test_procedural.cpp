#include <iostream>
#include <NihilEngine/ProceduralGenerator.h>

int main() {
    std::cout << "Test du système de génération procédurale..." << std::endl;

    // Crée un générateur avec un seed fixe pour des résultats reproductibles
    NihilEngine::ProceduralGenerator generator(12345);

    // Génère un petit monde de test
    auto world = generator.generateWorld(64, 64, 1.0f);

    std::cout << "Monde généré avec succès !" << std::endl;
    std::cout << "Dimensions: " << world->heightMap[0].size() << "x" << world->heightMap.size() << std::endl;
    std::cout << "Nombre de rivières: " << world->rivers.size() << std::endl;
    std::cout << "Nombre de végétations: " << world->vegetation.size() << std::endl;
    std::cout << "Nombre de corps d'eau: " << world->waterBodies.size() << std::endl;

    // Affiche quelques statistiques
    float minHeight = 1000.0f, maxHeight = -1000.0f;
    for (const auto& row : world->heightMap) {
        for (float h : row) {
            minHeight = std::min(minHeight, h);
            maxHeight = std::max(maxHeight, h);
        }
    }

    std::cout << "Hauteur minimale: " << minHeight << std::endl;
    std::cout << "Hauteur maximale: " << maxHeight << std::endl;

    return 0;
}