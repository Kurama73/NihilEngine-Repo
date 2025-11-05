#include <NihilEngine/TextureManager.h>
#include <iostream>
#include <filesystem>
#include <vector>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace NihilEngine {

TextureManager::TextureManager() {}

TextureManager::~TextureManager() {
    for (auto& pair : m_Textures) {
        glDeleteTextures(1, &pair.second.id);
        if (pair.second.data) {
            stbi_image_free(pair.second.data);
        }
    }
}

TextureManager& TextureManager::getInstance() {
    static TextureManager instance;
    return instance;
}

bool TextureManager::loadMinecraftTexturePack(const std::string& packPath) {
    m_PackPath = packPath;
    std::filesystem::path assetsPath = std::filesystem::path(packPath) / "assets" / "minecraft" / "textures";

    if (!std::filesystem::exists(assetsPath)) {
        std::cerr << "Minecraft texture pack path not found: " << assetsPath << std::endl;
        return false;
    }

    // Load common block textures
    std::vector<std::string> blockTextures = {
        "block/grass_block_top.png",
        "block/grass_block_side.png",
        "block/dirt.png",
        "block/stone.png",
        "block/cobblestone.png",
        "block/oak_log.png",
        "block/oak_leaves.png",
        "block/water_still.png"
    };

    for (const auto& texPath : blockTextures) {
        std::filesystem::path fullPath = assetsPath / texPath;
        if (std::filesystem::exists(fullPath)) {
            TextureInfo info;
            if (loadTextureFromFile(fullPath.string(), info)) {
                std::string name = texPath.substr(0, texPath.find_last_of('.'));
                m_Textures[name] = info;
                std::cout << "Loaded texture: " << name << std::endl;
            }
        }
    }

    return true;
}

bool TextureManager::loadTextureFromFile(const std::string& filepath, TextureInfo& info) {
    int width, height, channels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(filepath.c_str(), &width, &height, &channels, 0);

    if (!data) {
        std::cerr << "Failed to load texture: " << filepath << std::endl;
        return false;
    }

    GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;

    glGenTextures(1, &info.id);
    glBindTexture(GL_TEXTURE_2D, info.id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    info.data = data;
    // stbi_image_free(data); // Keep data for atlas creation

    info.uvMin = glm::vec2(0.0f, 0.0f);
    info.uvMax = glm::vec2(1.0f, 1.0f);
    info.width = width;
    info.height = height;
    info.channels = channels;

    return true;
}

GLuint TextureManager::getTexture(const std::string& name) const {
    auto it = m_Textures.find(name);
    return (it != m_Textures.end()) ? it->second.id : 0;
}

glm::vec2 TextureManager::getTextureUV(const std::string& name, int face) const {
    auto it = m_Textures.find(name);
    if (it != m_Textures.end()) {
        return glm::vec2(it->second.uvMin.x, it->second.uvMin.y); // Simplified
    }
    return glm::vec2(0.0f, 0.0f);
}

void TextureManager::bindTexture(GLuint textureID) {
    glBindTexture(GL_TEXTURE_2D, textureID);
}

void TextureManager::unbindTexture() {
    glBindTexture(GL_TEXTURE_2D, 0);
}

void TextureManager::createFallbackTextures() {
    // Create simple colored textures for each block type
    struct FallbackTexture {
        std::string name;
        glm::vec3 color;
    };

    std::vector<FallbackTexture> fallbackTextures = {
        {"block/grass_block_top", glm::vec3(0.2f, 0.8f, 0.2f)},    // Green
        {"block/grass_block_side", glm::vec3(0.4f, 0.6f, 0.2f)},   // Green-brown
        {"block/dirt", glm::vec3(0.6f, 0.4f, 0.2f)},               // Brown
        {"block/stone", glm::vec3(0.5f, 0.5f, 0.5f)},              // Gray
        {"block/cobblestone", glm::vec3(0.4f, 0.4f, 0.4f)},        // Dark gray
        {"block/oak_log", glm::vec3(0.4f, 0.3f, 0.1f)},            // Brown
        {"block/oak_leaves", glm::vec3(0.1f, 0.5f, 0.1f)},         // Dark green
        {"block/water_still", glm::vec3(0.2f, 0.4f, 0.8f)}         // Blue
    };

    for (const auto& fallback : fallbackTextures) {
        TextureInfo info;
        info.width = 16;
        info.height = 16;
        info.uvMin = glm::vec2(0.0f, 0.0f);
        info.uvMax = glm::vec2(1.0f, 1.0f);

        // Create a simple colored texture
        std::vector<unsigned char> pixels(16 * 16 * 4);
        for (int i = 0; i < 16 * 16; ++i) {
            pixels[i * 4 + 0] = static_cast<unsigned char>(fallback.color.r * 255);
            pixels[i * 4 + 1] = static_cast<unsigned char>(fallback.color.g * 255);
            pixels[i * 4 + 2] = static_cast<unsigned char>(fallback.color.b * 255);
            pixels[i * 4 + 3] = 255; // Alpha
        }

        glGenTextures(1, &info.id);
        glBindTexture(GL_TEXTURE_2D, info.id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        m_Textures[fallback.name] = info;
    }

    std::cout << "   [OK] " << fallbackTextures.size() << " textures de fallback creees" << std::endl;
}

GLuint TextureManager::createTextureAtlas() {
    // Create a simple 2x4 texture atlas (8 textures)
    const int ATLAS_WIDTH = 128;  // 4 textures * 32px each
    const int ATLAS_HEIGHT = 64;  // 2 rows * 32px each
    const int TEX_SIZE = 32;

    std::vector<unsigned char> atlasPixels(ATLAS_WIDTH * ATLAS_HEIGHT * 4, 0);

    // Define texture positions in atlas (normalized coordinates)
    struct AtlasEntry {
        std::string textureName;
        int x, y; // Position in atlas (0-3 for x, 0-1 for y)
    };

    std::vector<AtlasEntry> atlasLayout = {
        {"block/grass_block_top", 0, 0},
        {"block/grass_block_side", 1, 0},
        {"block/dirt", 2, 0},
        {"block/stone", 3, 0},
        {"block/cobblestone", 0, 1},
        {"block/oak_log", 1, 1},
        {"block/oak_leaves", 2, 1},
        {"block/water_still", 3, 1}
    };

    // Copy each texture to the atlas
    for (const auto& entry : atlasLayout) {
        auto it = m_Textures.find(entry.textureName);
        if (it != m_Textures.end()) {
            const TextureInfo& texInfo = it->second;
            int atlasX = entry.x * TEX_SIZE;
            int atlasY = entry.y * TEX_SIZE;

            // For fallback textures (16x16), scale up to 32x32
            for (int y = 0; y < TEX_SIZE; ++y) {
                for (int x = 0; x < TEX_SIZE; ++x) {
                    int srcX = (x * texInfo.width) / TEX_SIZE;
                    int srcY = (y * texInfo.height) / TEX_SIZE;

                    int atlasIndex = ((atlasY + y) * ATLAS_WIDTH + (atlasX + x)) * 4;

                    // Read pixel from source texture data
                    int srcIndex = (srcY * texInfo.width + srcX) * texInfo.channels;
                    unsigned char r = texInfo.data[srcIndex];
                    unsigned char g = texInfo.data[srcIndex + 1];
                    unsigned char b = texInfo.data[srcIndex + 2];
                    unsigned char a = (texInfo.channels == 4) ? texInfo.data[srcIndex + 3] : 255;

                    atlasPixels[atlasIndex + 0] = r;
                    atlasPixels[atlasIndex + 1] = g;
                    atlasPixels[atlasIndex + 2] = b;
                    atlasPixels[atlasIndex + 3] = a;
                }
            }
        }
    }

    // Create the atlas texture
    GLuint atlasTexture;
    glGenTextures(1, &atlasTexture);
    glBindTexture(GL_TEXTURE_2D, atlasTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ATLAS_WIDTH, ATLAS_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, atlasPixels.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    std::cout << "[OK] Texture atlas cree (" << ATLAS_WIDTH << "x" << ATLAS_HEIGHT << ")" << std::endl;
    return atlasTexture;
}

}