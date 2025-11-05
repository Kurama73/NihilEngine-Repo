#pragma once

#include <glm/glm.hpp>

namespace MonJeu {
    namespace Constants {
        // Constantes du joueur
        constexpr float PLAYER_SPEED = 5.0f;
        constexpr float GRAVITY = 40.0f;
        constexpr float JUMP_FORCE = 10.0f;
        constexpr float PLAYER_HEIGHT = 1.8f;        //
        constexpr float PLAYER_WIDTH = 0.6f;         // Largeur du joueur pour les collisions
        constexpr float MOUSE_SENSITIVITY = 0.1f;    //
        constexpr float MAX_PITCH = 89.0f;
        constexpr float EYE_HEIGHT = 0.9f;           // (Hauteur des yeux par rapport au centre du joueur)
        constexpr float FLY_SPEED = 5.0f;

        // Constantes de collision et d'interaction
        constexpr float RAYCAST_MAX_DISTANCE = 10.0f; //
        constexpr float COLLISION_RADIUS = 0.1f;      // (Largeur du joueur)
        constexpr float RAYCAST_DISTANCE = 6.0f;      // (Distance pour poser/casser blocs)

        // Constantes de génération du monde
        constexpr int CHUNK_SIZE = 16;                //
        constexpr int BASE_HEIGHT = 8;
        constexpr float TERRAIN_FREQUENCY = 0.1f;
        constexpr int TERRAIN_AMPLITUDE = 4;

        // Constantes de rendu
        constexpr float LINE_WIDTH = 2.0f;

        // Biomes (Spécifique au jeu)
        enum class BiomeType {
            Plains,
            Forest,
            Desert,
            Tundra,
            Swamp
        };

        struct Biome {
            BiomeType type;
            glm::vec3 grassColor; // Couleur pour l'herbe de ce biome
        };

        // Table de correspondance pour les 5 biomes
        const Biome BIOMES[] = {
            {BiomeType::Plains, glm::vec3(0.4f, 0.8f, 0.2f)},  // Vert
            {BiomeType::Forest, glm::vec3(0.3f, 0.7f, 0.2f)},  // Vert foncé
            {BiomeType::Desert, glm::vec3(0.8f, 0.8f, 0.2f)},  // Jaunâtre
            {BiomeType::Tundra, glm::vec3(0.6f, 0.8f, 0.6f)},  // Vert clair
            {BiomeType::Swamp, glm::vec3(0.2f, 0.6f, 0.3f)}    // Vert-bleu
        };
    }
}