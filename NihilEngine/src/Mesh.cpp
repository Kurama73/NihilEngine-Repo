#include <NihilEngine/Mesh.h>
#include <glad/glad.h>
#include <iostream>

namespace NihilEngine {

    // === CONSTRUCTEUR PAR DÉPLACEMENT ===
    Mesh::Mesh(Mesh&& other) noexcept
        : m_VAO(other.m_VAO), m_VBO(other.m_VBO), m_EBO(other.m_EBO), m_IndexCount(other.m_IndexCount) {
        // Transférer la propriété des buffers OpenGL
        other.m_VAO = 0;
        other.m_VBO = 0;
        other.m_EBO = 0;
        other.m_IndexCount = 0;
    }

    // === OPÉRATEUR = PAR DÉPLACEMENT ===
    Mesh& Mesh::operator=(Mesh&& other) noexcept {
        if (this != &other) {
            // Nettoyer les anciens buffers
            if (m_VAO != 0) {
                glDeleteVertexArrays(1, &m_VAO);
                glDeleteBuffers(1, &m_VBO);
                glDeleteBuffers(1, &m_EBO);
            }

            // Transférer
            m_VAO = other.m_VAO;
            m_VBO = other.m_VBO;
            m_EBO = other.m_EBO;
            m_IndexCount = other.m_IndexCount;

            // Invalider l'autre
            other.m_VAO = 0;
            other.m_VBO = 0;
            other.m_EBO = 0;
            other.m_IndexCount = 0;
        }
        return *this;
    }

    // === CONSTRUCTEUR STANDARD ===
    Mesh::Mesh(const std::vector<float>& vertices, const std::vector<unsigned int>& indices)
        : m_IndexCount(static_cast<int>(indices.size())) {
        glGenVertexArrays(1, &m_VAO);
        glGenBuffers(1, &m_VBO);
        glGenBuffers(1, &m_EBO);

        glBindVertexArray(m_VAO);

        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

        // Attribut position (location 0)
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // Attribut color (location 1)
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);

        // Debug (optionnel)
        std::cout << "Mesh created: VAO=" << m_VAO << ", VBO=" << m_VBO << ", EBO=" << m_EBO
                  << ", indices=" << m_IndexCount << std::endl;
    }

    // === DESTRUCTEUR ===
    Mesh::~Mesh() {
        if (m_VAO != 0) {
            std::cout << "Deleting Mesh: VAO=" << m_VAO << std::endl;
            glDeleteVertexArrays(1, &m_VAO);
            glDeleteBuffers(1, &m_VBO);
            glDeleteBuffers(1, &m_EBO);
        }
    }

    // === BIND / UNBIND / DRAW ===
    void Mesh::Bind() const {
        glBindVertexArray(m_VAO);
    }

    void Mesh::Unbind() const {
        glBindVertexArray(0);
    }

    void Mesh::Draw() const {
        glDrawElements(GL_TRIANGLES, m_IndexCount, GL_UNSIGNED_INT, 0);
    }

    // === CREATE CUBE ===
    Mesh Mesh::CreateCube(float size) {
        float half = size / 2.0f;
        std::vector<float> vertices = {
            // Front (red)
            -half, -half,  half,  1.0f, 0.0f, 0.0f,
             half, -half,  half,  1.0f, 0.0f, 0.0f,
             half,  half,  half,  1.0f, 0.0f, 0.0f,
            -half,  half,  half,  1.0f, 0.0f, 0.0f,
            // Back (green)
            -half, -half, -half,  0.0f, 1.0f, 0.0f,
            -half,  half, -half,  0.0f, 1.0f, 0.0f,
             half,  half, -half,  0.0f, 1.0f, 0.0f,
             half, -half, -half,  0.0f, 1.0f, 0.0f,
            // Left (blue)
            -half,  half,  half,  0.0f, 0.0f, 1.0f,
            -half,  half, -half,  0.0f, 0.0f, 1.0f,
            -half, -half, -half,  0.0f, 0.0f, 1.0f,
            -half, -half,  half,  0.0f, 0.0f, 1.0f,
            // Right (yellow)
             half,  half,  half,  1.0f, 1.0f, 0.0f,
             half, -half,  half,  1.0f, 1.0f, 0.0f,
             half, -half, -half,  1.0f, 1.0f, 0.0f,
             half,  half, -half,  1.0f, 1.0f, 0.0f,
            // Top (magenta)
            -half,  half, -half,  1.0f, 0.0f, 1.0f,
            -half,  half,  half,  1.0f, 0.0f, 1.0f,
             half,  half,  half,  1.0f, 0.0f, 1.0f,
             half,  half, -half,  1.0f, 0.0f, 1.0f,
            // Bottom (cyan)
            -half, -half, -half,  0.0f, 1.0f, 1.0f,
             half, -half, -half,  0.0f, 1.0f, 1.0f,
             half, -half,  half,  0.0f, 1.0f, 1.0f,
            -half, -half,  half,  0.0f, 1.0f, 1.0f
        };

        std::vector<unsigned int> indices = {
            0, 1, 2, 2, 3, 0,       // Front
            4, 5, 6, 6, 7, 4,       // Back
            8, 9, 10, 10, 11, 8,    // Left
            12, 13, 14, 14, 15, 12, // Right
            16, 17, 18, 18, 19, 16, // Top
            20, 21, 22, 22, 23, 20  // Bottom
        };

        return Mesh(vertices, indices);
    }

    // === CREATE TRIANGLE ===
    Mesh Mesh::CreateTriangle(float size) {
        float half = size / 2.0f;
        std::vector<float> vertices = {
            -half, -half, 0.0f,  1.0f, 0.0f, 0.0f,  // Red
             half, -half, 0.0f,  1.0f, 0.0f, 0.0f,  // Red
             0.0f,  half, 0.0f,  1.0f, 0.0f, 0.0f   // Red
        };

        std::vector<unsigned int> indices = { 0, 1, 2 };

        return Mesh(vertices, indices);
    }

}