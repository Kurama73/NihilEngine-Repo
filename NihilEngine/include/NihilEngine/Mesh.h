#pragma once

#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>

namespace NihilEngine {
    class Mesh {
    public:
        Mesh(const std::vector<float>& vertices, const std::vector<unsigned int>& indices);
        ~Mesh();

        // Interdire la copie
        Mesh(const Mesh&) = delete;
        Mesh& operator=(const Mesh&) = delete;

        // Autoriser le déplacement
        Mesh(Mesh&& other) noexcept;
        Mesh& operator=(Mesh&& other) noexcept;

        void Bind() const;
        void Unbind() const;
        void Draw() const;

        static Mesh CreateCube(float size = 1.0f);
        static Mesh CreateTriangle(float size = 1.0f);

    private:
        GLuint m_VAO, m_VBO, m_EBO;
        int m_IndexCount;
    };
}