#pragma once

#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>

namespace NihilEngine {
    enum class VertexAttribute {
        Position,
        Normal,
        TexCoord,
        Color
    };

    class Mesh {
    public:
        Mesh(const std::vector<float>& vertices, const std::vector<unsigned int>& indices, const std::vector<VertexAttribute>& attributes);
        ~Mesh();

        Mesh(Mesh&& other) noexcept;
        Mesh& operator=(Mesh&& other) noexcept;

        void Bind() const;
        void Unbind() const;
        void Draw() const;

        static Mesh CreateCube(float size = 1.0f);
        static Mesh CreateTriangle(float size = 1.0f);
        static Mesh CreateQuad(float size = 1.0f);

    private:
        GLuint m_VAO, m_VBO, m_EBO;
        int m_IndexCount;
        void SetupAttributes(const std::vector<VertexAttribute>& attributes);
    };
}