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
        void DrawMesh(const Mesh& mesh, const Camera& camera, const glm::mat4& model = glm::mat4(1.0f), const glm::vec3& colorMultiplier = glm::vec3(1.0f));
        void SwapBuffers(); // Peut-être pas nécessaire si Window gère

    private:
        GLuint m_ShaderProgram;
        GLuint m_ColorMultiplierLoc;

        void InitShaders();
    };
}