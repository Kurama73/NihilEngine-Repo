#pragma once

namespace NihilEngine {
    namespace Constants {
        // Camera constants
        constexpr float DEFAULT_FOV = 45.0f;
        constexpr float DEFAULT_ASPECT = 1.0f;
        constexpr float DEFAULT_NEAR_PLANE = 0.1f;
        constexpr float DEFAULT_FAR_PLANE = 100.0f;
        constexpr float MAX_PITCH = 89.0f;
        constexpr float FORWARD_Z_DEFAULT = 1.0f;

        // Renderer constants
        constexpr float LIGHT_POS_X = 20.0f;
        constexpr float LIGHT_POS_Y = 50.0f;
        constexpr float LIGHT_POS_Z = 20.0f;
        constexpr float AMBIENT_STRENGTH = 0.3f;
        constexpr float CROSSHAIR_SIZE = 10.0f;
        constexpr float LINE_WIDTH_DEFAULT = 2.0f;

        // Physics constants
        constexpr int AXIS_COUNT = 3;

        // Window constants
        constexpr int VSYNC_ENABLED = 1;
        constexpr int VSYNC_DISABLED = 0;

        // TextRenderer constants
        constexpr float TEXT_Z_POS = 0.0f;
        constexpr float TEXT_W_POS = 1.0f;
        constexpr float TEXT_UV_MIN = 0.0f;
        constexpr float TEXT_UV_MAX = 1.0f;
        constexpr int ASCII_START = 0;
        constexpr int ASCII_END = 128;
    }
}