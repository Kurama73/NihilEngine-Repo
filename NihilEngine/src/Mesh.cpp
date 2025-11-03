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

    // === CONSTRUCTEUR STANDARD (MODIFIÉ) ===
    Mesh::Mesh(const std::vector<float>& vertices, const std::vector<unsigned int>& indices)
        : m_IndexCount(static_cast<int>(indices.size())) {

        // Si on nous donne un mesh vide (ex: chunk sans blocs), ne rien faire
        if (vertices.empty() || indices.empty()) {
            m_VAO = 0;
            m_VBO = 0;
            m_EBO = 0;
            m_IndexCount = 0;
            return;
        }

        glGenVertexArrays(1, &m_VAO);
        glGenBuffers(1, &m_VBO);
        glGenBuffers(1, &m_EBO);

        glBindVertexArray(m_VAO);

        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

        // --- NOUVELLE STRUCTURE DU VERTEX ---
        // 3 (Pos) + 3 (Normal) + 2 (UV) = 8 floats
        int stride = 8 * sizeof(float);

        // Attribut position (location 0)
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
        glEnableVertexAttribArray(0);

        // Attribut normale (location 1)
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        // Attribut UV (location 2)
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);

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

    // Les fonctions CreateCube et CreateTriangle ont été supprimées
    // car elles généraient l'ancien format de vertex (6-float).
}