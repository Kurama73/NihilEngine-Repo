// NihilEngine/src/TextRenderer.cpp
#include <NihilEngine/TextRenderer.h>
#include <NihilEngine/Constants.h>
#include <glad/glad.h>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>

namespace NihilEngine {

    // ============================================================
    // SHADERS
    // ============================================================

    const char* textVertexShader = R"(
        #version 330 core
        layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>
        out vec2 TexCoords;

        uniform mat4 projection;

        void main() {
            gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
            TexCoords = vertex.zw;
        }
    )";

    const char* textFragmentShader = R"(
        #version 330 core
        in vec2 TexCoords;
        out vec4 color;

        uniform sampler2D text;
        uniform vec3 textColor;

        void main() {
            float alpha = texture(text, TexCoords).r;
            color = vec4(textColor, alpha);
        }
    )";

    TextRenderer::TextRenderer() : m_FT(nullptr), m_Face(nullptr) {
        if (FT_Init_FreeType(&m_FT)) {
            std::cerr << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
            m_FT = nullptr;
        }

        glGenVertexArrays(1, &m_VAO);
        glGenBuffers(1, &m_VBO);
        glBindVertexArray(m_VAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        InitShader();
    }

    TextRenderer::~TextRenderer() {
        for (auto& pair : m_Characters) {
            glDeleteTextures(1, &pair.second.textureID);
        }
        glDeleteProgram(m_ShaderProgram);
        if (m_Face) FT_Done_Face(m_Face);
        if (m_FT) FT_Done_FreeType(m_FT);
    }

    void TextRenderer::InitShader() {
        GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &textVertexShader, nullptr);
        glCompileShader(vertex);

        GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &textFragmentShader, nullptr);
        glCompileShader(fragment);

        m_ShaderProgram = glCreateProgram();
        glAttachShader(m_ShaderProgram, vertex);
        glAttachShader(m_ShaderProgram, fragment);
        glLinkProgram(m_ShaderProgram);

        glDeleteShader(vertex);
        glDeleteShader(fragment);
    }

    bool TextRenderer::LoadFont(const std::string& fontPath, unsigned int fontSize) {
        // Vérifier si le fichier existe
        std::ifstream fontFile(fontPath, std::ios::binary | std::ios::ate);
        if (!fontFile.is_open()) {
            std::cerr << "ERROR::FREETYPE: Font file does not exist or cannot be opened: " << fontPath << std::endl;
            m_FontLoaded = false;
            return false;
        }
        fontFile.close();

        if (FT_New_Face(m_FT, fontPath.c_str(), 0, &m_Face)) {
            std::cerr << "ERROR::FREETYPE: Failed to load font: " << fontPath << std::endl;
            m_FontLoaded = false;
            return false;
        }

        if (!m_Face) {
            std::cerr << "ERROR::FREETYPE: m_Face is null after FT_New_Face" << std::endl;
            m_FontLoaded = false;
            return false;
        }

        FT_Set_Pixel_Sizes(m_Face, 0, fontSize);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        m_Characters.clear();

        for (unsigned char c = Constants::ASCII_START; c < Constants::ASCII_END; ++c) {
            if (FT_Load_Char(m_Face, c, FT_LOAD_RENDER)) {
                std::cerr << "ERROR::FREETYPE: Failed to load Glyph '" << c << "'" << std::endl;
                continue;
            }

            GLuint texture;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(
                GL_TEXTURE_2D, 0, GL_RED,
                m_Face->glyph->bitmap.width,
                m_Face->glyph->bitmap.rows,
                0, GL_RED, GL_UNSIGNED_BYTE,
                m_Face->glyph->bitmap.buffer
            );

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            Character character = {
                texture,
                glm::ivec2(m_Face->glyph->bitmap.width, m_Face->glyph->bitmap.rows),
                glm::ivec2(m_Face->glyph->bitmap_left, m_Face->glyph->bitmap_top),
                static_cast<unsigned int>(m_Face->glyph->advance.x)
            };
            m_Characters[c] = character;
        }

        glBindTexture(GL_TEXTURE_2D, 0);
        m_FontLoaded = true;
        return true;
    }

    void TextRenderer::RenderText(const std::string& text, float x, float y, float scale, const glm::vec3& color) {
        if (!m_FontLoaded) {
            std::cerr << "ERROR::TEXTRENDERER: Font not loaded! Call LoadFont() first." << std::endl;
            return;
        }

        // Enable blending for text rendering
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glUseProgram(m_ShaderProgram);
        glUniform3f(glGetUniformLocation(m_ShaderProgram, "textColor"), color.x, color.y, color.z);
        glActiveTexture(GL_TEXTURE0);
        glBindVertexArray(m_VAO);

        int viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(viewport[2]), 0.0f, static_cast<float>(viewport[3]));
        glUniformMatrix4fv(glGetUniformLocation(m_ShaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        for (char c : text) {
            auto it = m_Characters.find(c);
            if (it == m_Characters.end()) continue;

            Character ch = it->second;

            float xpos = x + ch.bearing.x * scale;
            float ypos = y - (ch.size.y - ch.bearing.y) * scale;
            float w = ch.size.x * scale;
            float h = ch.size.y * scale;

            float vertices[6][4] = {
                { xpos,     ypos + h,   0.0f, 0.0f },
                { xpos,     ypos,       0.0f, 1.0f },
                { xpos + w, ypos,       1.0f, 1.0f },
                { xpos,     ypos + h,   0.0f, 0.0f },
                { xpos + w, ypos,       1.0f, 1.0f },
                { xpos + w, ypos + h,   1.0f, 0.0f }
            };

            glBindTexture(GL_TEXTURE_2D, ch.textureID);
            glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glDrawArrays(GL_TRIANGLES, 0, 6);

            x += (ch.advance >> 6) * scale;
        }

        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);

        glDisable(GL_BLEND);
    }

}