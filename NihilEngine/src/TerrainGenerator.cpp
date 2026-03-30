#include <NihilEngine/TerrainGenerator.h>
#include <algorithm>
#include <chrono>
#include <iostream>

namespace {
const char* kTerrainComputeShaderSource = R"(
#version 430 core
layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

layout(std430, binding = 0) buffer HeightBuffer {
    float heights[];
};

uniform int u_StartX;
uniform int u_StartZ;
uniform int u_Width;
uniform int u_Height;
uniform float u_Scale;
uniform float u_BaseHeight;
uniform float u_Amplitude;
uniform float u_Frequency;
uniform float u_Persistence;
uniform int u_Octaves;
uniform int u_Seed;

float hash12(vec2 p) {
    vec3 p3 = fract(vec3(p.xyx) * 0.1031);
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.x + p3.y) * p3.z);
}

float valueNoise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);

    float a = hash12(i + vec2(0.0, 0.0));
    float b = hash12(i + vec2(1.0, 0.0));
    float c = hash12(i + vec2(0.0, 1.0));
    float d = hash12(i + vec2(1.0, 1.0));

    vec2 u = f * f * (3.0 - 2.0 * f);
    return mix(mix(a, b, u.x), mix(c, d, u.x), u.y) * 2.0 - 1.0;
}

float fractalNoise(vec2 p, int octaves, float persistence, float frequency, float seedOffset) {
    float total = 0.0;
    float amp = 1.0;
    float maxAmp = 0.0;
    float freq = max(frequency, 0.00001);
    vec2 seedVec = vec2(seedOffset * 0.173, seedOffset * 0.271);

    for (int i = 0; i < octaves; ++i) {
        total += valueNoise(p * freq + seedVec) * amp;
        maxAmp += amp;
        amp *= persistence;
        freq *= 2.0;
    }

    if (maxAmp <= 0.0) {
        return 0.0;
    }
    return total / maxAmp;
}

float ridgedNoise(vec2 p, int octaves, float persistence, float frequency, float seedOffset) {
    float n = fractalNoise(p, octaves, persistence, frequency, seedOffset + 7.0);
    return 1.0 - abs(n);
}

void main() {
    uint gx = gl_GlobalInvocationID.x;
    uint gz = gl_GlobalInvocationID.y;

    if (gx >= uint(u_Width) || gz >= uint(u_Height)) {
        return;
    }

    int x = u_StartX + int(gx);
    int z = u_StartZ + int(gz);
    vec2 p = vec2(float(x) * u_Scale, float(z) * u_Scale);
    float seedOffset = float(u_Seed) * 0.01;

    float base = fractalNoise(p, u_Octaves, u_Persistence, u_Frequency, seedOffset);
    float ridge = ridgedNoise(p, max(1, u_Octaves / 2), u_Persistence * 0.8, u_Frequency * 2.0, seedOffset);
    float combined = base * 0.7 + ridge * 0.3;

    uint idx = gz * uint(u_Width) + gx;
    heights[idx] = u_BaseHeight + combined * u_Amplitude;
}
)";
}

namespace NihilEngine {

TerrainGenerator::TerrainGenerator(unsigned int seed)
    : noise(seed), m_Seed(seed), baseHeight(0.0f), amplitude(10.0f), frequency(0.01f), octaves(4), persistence(0.5f) {}

TerrainGenerator::~TerrainGenerator() {
    if (m_HeightSsbo != 0) {
        glDeleteBuffers(1, &m_HeightSsbo);
    }
    if (m_ComputeProgram != 0) {
        glDeleteProgram(m_ComputeProgram);
    }
}

float TerrainGenerator::getHeight(float x, float z) const {
    // Génère du bruit fractal pour le terrain de base
    float baseNoise = noise.fractal(x, z, octaves, persistence, frequency);

    // Ajoute du bruit ridged pour les montagnes
    float mountainNoise = noise.ridged(x, z, octaves / 2, persistence * 0.8f, frequency * 2.0f);

    // Combine les bruits
    float combinedNoise = baseNoise * 0.7f + mountainNoise * 0.3f;

    // Applique l'amplitude et la hauteur de base
    return baseHeight + combinedNoise * amplitude;
}

std::vector<std::vector<float>> TerrainGenerator::generateHeightMap(int width, int height, float scale) const {
    return generateHeightMapRegion(0, 0, width, height, scale);
}

std::vector<std::vector<float>> TerrainGenerator::generateHeightMapRegion(
    int startX,
    int startZ,
    int width,
    int height,
    float scale) const {
    const auto begin = std::chrono::high_resolution_clock::now();
    std::vector<std::vector<float>> heightMap(height, std::vector<float>(width));

    std::vector<float> flatHeights;
    if (m_UseGpuPreferred && GenerateHeightMapRegionGpu(startX, startZ, width, height, scale, flatHeights)) {
        m_LastGenerationUsedGpu = true;
        for (int z = 0; z < height; ++z) {
            for (int x = 0; x < width; ++x) {
                heightMap[z][x] = flatHeights[z * width + x];
            }
        }
        const auto end = std::chrono::high_resolution_clock::now();
        m_LastGenerationMs = std::chrono::duration<double, std::milli>(end - begin).count();
        return heightMap;
    }

    m_LastGenerationUsedGpu = false;
    for (int z = 0; z < height; ++z) {
        for (int x = 0; x < width; ++x) {
            float worldX = static_cast<float>(startX + x) * scale;
            float worldZ = static_cast<float>(startZ + z) * scale;
            heightMap[z][x] = getHeight(worldX, worldZ);
        }
    }

    const auto end = std::chrono::high_resolution_clock::now();
    m_LastGenerationMs = std::chrono::duration<double, std::milli>(end - begin).count();

    return heightMap;
}

bool TerrainGenerator::IsGpuComputeAvailable() const {
    return InitGpuCompute();
}

bool TerrainGenerator::InitGpuCompute() const {
    if (m_GpuInitAttempted) {
        return m_GpuAvailable;
    }
    m_GpuInitAttempted = true;

    GLint major = 0;
    GLint minor = 0;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);
    if (major < 4 || (major == 4 && minor < 3)) {
        m_GpuAvailable = false;
        return false;
    }

    GLuint shader = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(shader, 1, &kTerrainComputeShaderSource, nullptr);
    glCompileShader(shader);

    GLint compileOk = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileOk);
    if (compileOk != GL_TRUE) {
        char log[1024] = {0};
        glGetShaderInfoLog(shader, static_cast<GLsizei>(sizeof(log)), nullptr, log);
        std::cerr << "[TerrainGenerator] Compute shader compilation failed: " << log << std::endl;
        glDeleteShader(shader);
        m_GpuAvailable = false;
        return false;
    }

    m_ComputeProgram = glCreateProgram();
    glAttachShader(m_ComputeProgram, shader);
    glLinkProgram(m_ComputeProgram);
    glDeleteShader(shader);

    GLint linkOk = GL_FALSE;
    glGetProgramiv(m_ComputeProgram, GL_LINK_STATUS, &linkOk);
    if (linkOk != GL_TRUE) {
        char log[1024] = {0};
        glGetProgramInfoLog(m_ComputeProgram, static_cast<GLsizei>(sizeof(log)), nullptr, log);
        std::cerr << "[TerrainGenerator] Compute program link failed: " << log << std::endl;
        glDeleteProgram(m_ComputeProgram);
        m_ComputeProgram = 0;
        m_GpuAvailable = false;
        return false;
    }

    glGenBuffers(1, &m_HeightSsbo);
    m_GpuAvailable = (m_HeightSsbo != 0);

    if (m_GpuAvailable) {
        std::cout << "[TerrainGenerator] GPU compute path enabled for heightmap generation." << std::endl;
    }

    return m_GpuAvailable;
}

bool TerrainGenerator::GenerateHeightMapRegionGpu(
    int startX,
    int startZ,
    int width,
    int height,
    float scale,
    std::vector<float>& outHeights) const {
    if (width <= 0 || height <= 0) {
        return false;
    }

    if (!InitGpuCompute()) {
        return false;
    }

    const size_t elementCount = static_cast<size_t>(width) * static_cast<size_t>(height);
    const size_t byteSize = elementCount * sizeof(float);
    outHeights.resize(elementCount);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_HeightSsbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, byteSize, nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_HeightSsbo);

    glUseProgram(m_ComputeProgram);
    glUniform1i(glGetUniformLocation(m_ComputeProgram, "u_StartX"), startX);
    glUniform1i(glGetUniformLocation(m_ComputeProgram, "u_StartZ"), startZ);
    glUniform1i(glGetUniformLocation(m_ComputeProgram, "u_Width"), width);
    glUniform1i(glGetUniformLocation(m_ComputeProgram, "u_Height"), height);
    glUniform1f(glGetUniformLocation(m_ComputeProgram, "u_Scale"), scale);
    glUniform1f(glGetUniformLocation(m_ComputeProgram, "u_BaseHeight"), baseHeight);
    glUniform1f(glGetUniformLocation(m_ComputeProgram, "u_Amplitude"), amplitude);
    glUniform1f(glGetUniformLocation(m_ComputeProgram, "u_Frequency"), frequency);
    glUniform1f(glGetUniformLocation(m_ComputeProgram, "u_Persistence"), persistence);
    glUniform1i(glGetUniformLocation(m_ComputeProgram, "u_Octaves"), octaves);
    glUniform1i(glGetUniformLocation(m_ComputeProgram, "u_Seed"), static_cast<int>(m_Seed));

    const GLuint groupX = static_cast<GLuint>((width + 15) / 16);
    const GLuint groupY = static_cast<GLuint>((height + 15) / 16);
    glDispatchCompute(groupX, groupY, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT);

    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, byteSize, outHeights.data());
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    return true;
}

}