// NihilEngine/include/NihilEngine/TextRenderer.h
#pragma once
#include <string>
#include <map>
#include <glm/glm.hpp>
#include <glad/glad.h>
#include <ft2build.h>
#include FT_FREETYPE_H

namespace NihilEngine {

struct Character {
    GLuint textureID;
    glm::ivec2 size;
    glm::ivec2 bearing;
    unsigned int advance;
};

class TextRenderer {
public:
    TextRenderer();
    ~TextRenderer();

    bool LoadFont(const std::string& fontPath, unsigned int fontSize);
    void RenderText(const std::string& text, float x, float y, float scale, const glm::vec3& color);

private:
    std::map<char, Character> m_Characters;
    GLuint m_VAO, m_VBO;
    GLuint m_ShaderProgram;
    FT_Library m_FT;
    FT_Face m_Face;
    bool m_FontLoaded = false;
    void InitShader();
};

}