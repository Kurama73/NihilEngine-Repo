#pragma once
#include <string>
#include <unordered_map>
#include <glad/glad.h>
#include <glm/glm.hpp>

namespace NihilEngine {

class TextureManager {
public:
    static TextureManager& getInstance();

    bool loadMinecraftTexturePack(const std::string& packPath);
    void createFallbackTextures();
    GLuint createTextureAtlas();
    GLuint getTexture(const std::string& name) const;
    glm::vec2 getTextureUV(const std::string& name, int face = 0) const; // For cube textures

    void bindTexture(GLuint textureID);
    void unbindTexture();

private:
    TextureManager();
    ~TextureManager();

    struct TextureInfo {
        GLuint id;
        glm::vec2 uvMin;
        glm::vec2 uvMax;
        int width, height, channels;
        unsigned char* data;
        bool isFallback;
    };

    std::unordered_map<std::string, TextureInfo> m_Textures;
    std::string m_PackPath;

    bool loadTextureFromFile(const std::string& filepath, TextureInfo& info);
    bool parseTextureAtlas(const std::string& atlasPath);

    // === NOUVEAU : Colorisation de l'herbe ===
    void ColorizeGrassTop(unsigned char* data, int width, int height, int channels);
    // ========================================
};

} // namespace NihilEngine