#pragma once

#include <glm/glm.hpp>

namespace MonJeu {
    namespace Constants {
        // Player constants
        constexpr float PLAYER_SPEED = 5.0f;
        constexpr float GRAVITY = 40.0f;
        constexpr float JUMP_FORCE = 10.0f;
        constexpr float PLAYER_HEIGHT = 1.8f;
        constexpr float MOUSE_SENSITIVITY = 0.1f;
        constexpr float MAX_PITCH = 89.0f;
        constexpr float EYE_HEIGHT = 0.9f;
        constexpr float FLY_SPEED = 5.0f;
        constexpr float FOV_DEGREES = 60.0f;
        constexpr float FOV_DISTANCE = 5.0f;
        constexpr float RAYCAST_MAX_DISTANCE = 10.0f;
        constexpr float COLLISION_OFFSET = 0.05f;
        constexpr float COLLISION_RADIUS = 0.1f;

        // World generation
        constexpr int CHUNK_SIZE = 16;
        constexpr int BASE_HEIGHT = 8;
        constexpr float TERRAIN_FREQUENCY = 0.1f;
        constexpr int TERRAIN_AMPLITUDE = 4;
        constexpr int WORLD_GENERATION_RANGE = 3;

        // Rendering
        constexpr float LINE_WIDTH = 2.0f;

        // Biomes
        enum class BiomeType {
            Plains,
            Forest,
            Desert,
            Tundra,
            Swamp
        };

        struct Biome {
            BiomeType type;
            glm::vec3 grassColor;
        };

        const Biome BIOMES[] = {
            {BiomeType::Plains, glm::vec3(0.4f, 0.8f, 0.2f)},  // Green
            {BiomeType::Forest, glm::vec3(0.3f, 0.7f, 0.2f)},  // Darker green
            {BiomeType::Desert, glm::vec3(0.8f, 0.8f, 0.2f)},  // Yellowish
            {BiomeType::Tundra, glm::vec3(0.6f, 0.8f, 0.6f)},  // Light green
            {BiomeType::Swamp, glm::vec3(0.2f, 0.6f, 0.3f)}    // Teal green
        };

        // Input
        constexpr float RAYCAST_DISTANCE = 6.0f;
    }
}