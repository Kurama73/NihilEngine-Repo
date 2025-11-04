// Mesh.cpp
#include <NihilEngine/Mesh.h>
#include <glad/glad.h>
#include <iostream>

namespace NihilEngine {

// ============================================================
// MESH IMPLEMENTATION
// ============================================================

Mesh::Mesh(const std::vector<float>& vertices,
           const std::vector<unsigned int>& indices,
           const std::vector<VertexAttribute>& attributes)
    : m_IndexCount(static_cast<int>(indices.size())) {

    if (vertices.empty() || indices.empty()) {
        m_VAO = m_VBO = m_EBO = 0;
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

    SetupAttributes(attributes);

    glBindVertexArray(0);
}

void Mesh::SetupAttributes(const std::vector<VertexAttribute>& attributes) {
    int stride = 0;
    for (auto attr : attributes) {
        switch (attr) {
            case VertexAttribute::Position: stride += 3; break;
            case VertexAttribute::Normal:   stride += 3; break;
            case VertexAttribute::TexCoord: stride += 2; break;
            case VertexAttribute::Color:    stride += 4; break;
        }
    }
    stride *= sizeof(float);

    int offset = 0;
    int location = 0;
    for (auto attr : attributes) {
        int size = 0;
        switch (attr) {
            case VertexAttribute::Position: size = 3; break;
            case VertexAttribute::Normal:   size = 3; break;
            case VertexAttribute::TexCoord: size = 2; break;
            case VertexAttribute::Color:    size = 4; break;
        }
        glVertexAttribPointer(location, size, GL_FLOAT, GL_FALSE, stride, (void*)(offset * sizeof(float)));
        glEnableVertexAttribArray(location);
        offset += size;
        location++;
    }
}

Mesh::~Mesh() {
    if (m_VAO) {
        glDeleteVertexArrays(1, &m_VAO);
        glDeleteBuffers(1, &m_VBO);
        glDeleteBuffers(1, &m_EBO);
    }
}

Mesh::Mesh(Mesh&& other) noexcept
    : m_VAO(other.m_VAO), m_VBO(other.m_VBO), m_EBO(other.m_EBO), m_IndexCount(other.m_IndexCount) {
    other.m_VAO = other.m_VBO = other.m_EBO = 0;
    other.m_IndexCount = 0;
}

Mesh& Mesh::operator=(Mesh&& other) noexcept {
    if (this != &other) {
        if (m_VAO) {
            glDeleteVertexArrays(1, &m_VAO);
            glDeleteBuffers(1, &m_VBO);
            glDeleteBuffers(1, &m_EBO);
        }
        m_VAO = other.m_VAO;
        m_VBO = other.m_VBO;
        m_EBO = other.m_EBO;
        m_IndexCount = other.m_IndexCount;

        other.m_VAO = other.m_VBO = other.m_EBO = 0;
        other.m_IndexCount = 0;
    }
    return *this;
}

void Mesh::Bind() const   { glBindVertexArray(m_VAO); }
void Mesh::Unbind() const { glBindVertexArray(0); }
void Mesh::Draw() const   { glDrawElements(GL_TRIANGLES, m_IndexCount, GL_UNSIGNED_INT, 0); }

// ============================================================
// FACTORY METHODS
// ============================================================

Mesh Mesh::CreateCube(float size) {
    float s = size * 0.5f;
    std::vector<float> vertices = {
        // Pos          Normal        UV
        -s, -s, -s,   0, 0, -1,     0, 0,
         s, -s, -s,   0, 0, -1,     1, 0,
         s,  s, -s,   0, 0, -1,     1, 1,
        -s,  s, -s,   0, 0, -1,     0, 1,

        -s, -s,  s,   0, 0,  1,     0, 0,
         s, -s,  s,   0, 0,  1,     1, 0,
         s,  s,  s,   0, 0,  1,     1, 1,
        -s,  s,  s,   0, 0,  1,     0, 1,

        -s, -s, -s,  -1, 0,  0,     0, 0,
        -s,  s, -s,  -1, 0,  0,     1, 0,
        -s,  s,  s,  -1, 0,  0,     1, 1,
        -s, -s,  s,  -1, 0,  0,     0, 1,

         s, -s, -s,   1, 0,  0,     0, 0,
         s,  s, -s,   1, 0,  0,     1, 0,
         s,  s,  s,   1, 0,  0,     1, 1,
         s, -s,  s,   1, 0,  0,     0, 1,

        -s,  s, -s,   0, 1,  0,     0, 0,
         s,  s, -s,   0, 1,  0,     1, 0,
         s,  s,  s,   0, 1,  0,     1, 1,
        -s,  s,  s,   0, 1,  0,     0, 1,

        -s, -s, -s,   0,-1,  0,     0, 0,
         s, -s, -s,   0,-1,  0,     1, 0,
         s, -s,  s,   0,-1,  0,     1, 1,
        -s, -s,  s,   0,-1,  0,     0, 1
    };

    std::vector<unsigned int> indices = {
        0, 1, 2,  2, 3, 0,
        4, 5, 6,  6, 7, 4,
        8, 9,10, 10,11, 8,
        12,13,14, 14,15,12,
        16,17,18, 18,19,16,
        20,21,22, 22,23,20
    };

    std::vector<VertexAttribute> attrs = {
        VertexAttribute::Position,
        VertexAttribute::Normal,
        VertexAttribute::TexCoord
    };

    return Mesh(vertices, indices, attrs);
}

Mesh Mesh::CreateQuad(float size) {
    float s = size * 0.5f;
    std::vector<float> vertices = {
        -s, -s, 0.0f,  0, 0, 1,  0, 0,
         s, -s, 0.0f,  0, 0, 1,  1, 0,
         s,  s, 0.0f,  0, 0, 1,  1, 1,
        -s,  s, 0.0f,  0, 0, 1,  0, 1
    };
    std::vector<unsigned int> indices = { 0, 1, 2,  2, 3, 0 };
    std::vector<VertexAttribute> attrs = { VertexAttribute::Position, VertexAttribute::Normal, VertexAttribute::TexCoord };
    return Mesh(vertices, indices, attrs);
}

} // namespace NihilEngine