#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace NihilEngine {
    class Camera;
    class Mesh;

    class Renderer {
    public:
        Renderer();
        ~Renderer();

        void Clear();
        void DrawMesh(const Mesh& mesh, const Camera& camera, const glm::mat4& model);
        void DrawCrosshair(int windowWidth, int windowHeight);

        // --- NOUVEAU: Dessine une ligne 3D (pour le laser) ---
        void DrawLine3D(const glm::vec3& start, const glm::vec3& end, const Camera& camera, const glm::vec3& color = glm::vec3(1.0f, 0.0f, 0.0f));

    private:
        GLuint m_ShaderProgram;
        void InitShaders();

        GLuint m_CrosshairShaderProgram;
        GLuint m_CrosshairVAO, m_CrosshairVBO;
        void InitCrosshair();

        // --- NOUVEAU: Pour les lignes 3D ---
        GLuint m_LineShaderProgram;
        GLuint m_LineVAO, m_LineVBO;
        void InitLineShader();
    };
}