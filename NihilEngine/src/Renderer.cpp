#include <NihilEngine/Renderer.h>
#include <NihilEngine/Camera.h>
#include <NihilEngine/Mesh.h>
#include <iostream>

namespace NihilEngine {

    const char* vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec3 aColor;
        out vec3 v_Color;
        uniform mat4 u_Model;
        uniform mat4 u_ViewProjection;
        void main() {
            gl_Position = u_ViewProjection * u_Model * vec4(aPos, 1.0);
            v_Color = aColor;
        }
    )";

    const char* fragmentShaderSource = R"(
        #version 330 core
        in vec3 v_Color;
        out vec4 FragColor;
        uniform vec3 u_ColorMultiplier;
        void main() {
            FragColor = vec4(v_Color * u_ColorMultiplier, 1.0);
        }
    )";

    Renderer::Renderer() {
        glEnable(GL_DEPTH_TEST);
        InitShaders();
        m_ColorMultiplierLoc = glGetUniformLocation(m_ShaderProgram, "u_ColorMultiplier");
        std::cout << "Renderer initialized" << std::endl;
    }

    Renderer::~Renderer() {
        glDeleteProgram(m_ShaderProgram);
    }

    void Renderer::InitShaders() {
        // Vertex Shader
        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
        glCompileShader(vertexShader);

        // Check compilation
        int success;
        char infoLog[512];
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
            std::cerr << "Vertex Shader Error: " << infoLog << std::endl;
        }

        // Fragment Shader
        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
        glCompileShader(fragmentShader);

        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
            std::cerr << "Fragment Shader Error: " << infoLog << std::endl;
        }

        // Shader Program
        m_ShaderProgram = glCreateProgram();
        glAttachShader(m_ShaderProgram, vertexShader);
        glAttachShader(m_ShaderProgram, fragmentShader);
        glLinkProgram(m_ShaderProgram);

        glGetProgramiv(m_ShaderProgram, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(m_ShaderProgram, 512, NULL, infoLog);
            std::cerr << "Shader Program Link Error: " << infoLog << std::endl;
        }

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }

    void Renderer::Clear() {
        glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void Renderer::DrawMesh(const Mesh& mesh, const Camera& camera, const glm::mat4& model, const glm::vec3& colorMultiplier) {
        glUseProgram(m_ShaderProgram);

        // Passer les matrices au shader
        glm::mat4 vp = camera.GetViewProjectionMatrix();
        GLuint vpLoc = glGetUniformLocation(m_ShaderProgram, "u_ViewProjection");
        glUniformMatrix4fv(vpLoc, 1, GL_FALSE, glm::value_ptr(vp));

        GLuint modelLoc = glGetUniformLocation(m_ShaderProgram, "u_Model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        glUniform3fv(m_ColorMultiplierLoc, 1, glm::value_ptr(colorMultiplier));

        mesh.Bind();
        mesh.Draw();
        mesh.Unbind();
    }
}