#pragma once
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <AL/al.h>
#include <AL/alc.h>

namespace NihilEngine {

class AudioBuffer; // Forward declaration

class AudioSource {
public:
    AudioSource();
    ~AudioSource();

    void play();
    void pause();
    void stop();
    void setPosition(const glm::vec3& position);
    void setVelocity(const glm::vec3& velocity);
    void setVolume(float volume);
    void setPitch(float pitch);
    void setLooping(bool loop);
    void setBuffer(AudioBuffer* buffer);

    ALuint getSource() const { return source; }

private:
    ALuint source;
};

class AudioBuffer {
public:
    AudioBuffer();
    ~AudioBuffer();

    bool loadFromFile(const std::string& filename);
    bool isLoaded() const;
    ALuint getBuffer() const { return buffer; }

private:
    ALuint buffer;
    bool loaded;
};

class AudioSystem {
public:
    static AudioSystem& getInstance();

    bool initialize();
    void shutdown();

    void setListenerPosition(const glm::vec3& position);
    void setListenerOrientation(const glm::vec3& forward, const glm::vec3& up);
    void setListenerVelocity(const glm::vec3& velocity);

    AudioSource* createSource();
    AudioBuffer* createBuffer();

    void update();

private:
    AudioSystem();
    ~AudioSystem();

    ALCdevice* device;
    ALCcontext* context;

    std::vector<AudioSource*> sources;
    std::vector<AudioBuffer*> buffers;
};

} // namespace NihilEngine