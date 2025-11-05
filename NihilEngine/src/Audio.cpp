#include <NihilEngine/Audio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>

// Simple WAV loader (you might want to use a proper library like libsndfile)
struct WAVHeader {
    char riff[4];
    uint32_t fileSize;
    char wave[4];
    char fmt[4];
    uint32_t fmtSize;
    uint16_t audioFormat;
    uint16_t numChannels;
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t blockAlign;
    uint16_t bitsPerSample;
    char data[4];
    uint32_t dataSize;
};

namespace NihilEngine {

AudioSource::AudioSource() {
    alGenSources(1, &source);
    ALenum error = alGetError();
    if (error != AL_NO_ERROR || source == 0) {
        std::cerr << "Failed to generate audio source: " << alGetString(error) << std::endl;
        source = 0; // Invalid
    }
}

AudioSource::~AudioSource() {
    alDeleteSources(1, &source);
}

void AudioSource::play() {
    if (source != 0) {
        alSourcePlay(source);
    }
}

void AudioSource::pause() {
    if (source != 0) {
        alSourcePause(source);
    }
}

void AudioSource::stop() {
    if (source != 0) {
        alSourceStop(source);
    }
}

void AudioSource::setPosition(const glm::vec3& position) {
    if (source != 0) {
        alSource3f(source, AL_POSITION, position.x, position.y, position.z);
    }
}

void AudioSource::setVelocity(const glm::vec3& velocity) {
    if (source != 0) {
        alSource3f(source, AL_VELOCITY, velocity.x, velocity.y, velocity.z);
    }
}

void AudioSource::setVolume(float volume) {
    if (source != 0) {
        alSourcef(source, AL_GAIN, volume);
    }
}

void AudioSource::setPitch(float pitch) {
    if (source != 0) {
        alSourcef(source, AL_PITCH, pitch);
    }
}

void AudioSource::setLooping(bool loop) {
    if (source != 0) {
        alSourcei(source, AL_LOOPING, loop ? AL_TRUE : AL_FALSE);
    }
}

void AudioSource::setBuffer(AudioBuffer* buffer) {
    if (source != 0) {
        if (buffer) {
            alSourcei(source, AL_BUFFER, buffer->getBuffer());
        } else {
            alSourcei(source, AL_BUFFER, 0);
        }
    }
}

AudioBuffer::AudioBuffer() : loaded(false) {
    alGenBuffers(1, &buffer);
}

AudioBuffer::~AudioBuffer() {
    alDeleteBuffers(1, &buffer);
}

bool AudioBuffer::loadFromFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open audio file: " << filename << std::endl;
        return false;
    }

    WAVHeader header;
    file.read(reinterpret_cast<char*>(&header), sizeof(WAVHeader));

    if (std::strncmp(header.riff, "RIFF", 4) != 0 || std::strncmp(header.wave, "WAVE", 4) != 0) {
        std::cerr << "Invalid WAV file: " << filename << std::endl;
        return false;
    }

    std::vector<char> data(header.dataSize);
    file.read(data.data(), header.dataSize);

    ALenum format = (header.numChannels == 1) ?
        (header.bitsPerSample == 8 ? AL_FORMAT_MONO8 : AL_FORMAT_MONO16) :
        (header.bitsPerSample == 8 ? AL_FORMAT_STEREO8 : AL_FORMAT_STEREO16);

    alBufferData(buffer, format, data.data(), header.dataSize, header.sampleRate);

    loaded = true;
    return true;
}

bool AudioBuffer::isLoaded() const {
    return loaded;
}

AudioSystem::AudioSystem() : device(nullptr), context(nullptr) {}

AudioSystem::~AudioSystem() {
    shutdown();
}

AudioSystem& AudioSystem::getInstance() {
    static AudioSystem instance;
    return instance;
}

bool AudioSystem::initialize() {
    std::cout << "[DEBUG] Opening audio device..." << std::endl;
    device = alcOpenDevice(nullptr);
    if (!device) {
        std::cerr << "Failed to open audio device" << std::endl;
        return false;
    }
    std::cout << "[DEBUG] Audio device opened" << std::endl;

    std::cout << "[DEBUG] Creating audio context..." << std::endl;
    context = alcCreateContext(device, nullptr);
    if (!context) {
        std::cerr << "Failed to create audio context" << std::endl;
        alcCloseDevice(device);
        device = nullptr;
        return false;
    }
    std::cout << "[DEBUG] Audio context created" << std::endl;

    std::cout << "[DEBUG] Making context current..." << std::endl;
    if (!alcMakeContextCurrent(context)) {
        std::cerr << "Failed to make audio context current" << std::endl;
        alcDestroyContext(context);
        context = nullptr;
        alcCloseDevice(device);
        device = nullptr;
        return false;
    }
    std::cout << "[DEBUG] Context made current" << std::endl;

    ALCenum alcError = alcGetError(device);
    if (alcError != ALC_NO_ERROR) {
        std::cerr << "ALC error after init: " << alcGetString(device, alcError) << std::endl;
        return false;
    }

    std::cout << "[OK] Audio system initialized" << std::endl;
    return true;
}

void AudioSystem::shutdown() {
    for (auto source : sources) {
        delete source;
    }
    sources.clear();

    for (auto buffer : buffers) {
        delete buffer;
    }
    buffers.clear();

    alcMakeContextCurrent(nullptr);
    if (context) {
        alcDestroyContext(context);
        context = nullptr;
    }
    if (device) {
        alcCloseDevice(device);
        device = nullptr;
    }
}

void AudioSystem::setListenerPosition(const glm::vec3& position) {
    alListener3f(AL_POSITION, position.x, position.y, position.z);
}

void AudioSystem::setListenerOrientation(const glm::vec3& forward, const glm::vec3& up) {
    float orientation[] = { forward.x, forward.y, forward.z, up.x, up.y, up.z };
    alListenerfv(AL_ORIENTATION, orientation);
}

void AudioSystem::setListenerVelocity(const glm::vec3& velocity) {
    alListener3f(AL_VELOCITY, velocity.x, velocity.y, velocity.z);
}

AudioSource* AudioSystem::createSource() {
    AudioSource* source = new AudioSource();
    sources.push_back(source);
    return source;
}

AudioBuffer* AudioSystem::createBuffer() {
    AudioBuffer* buffer = new AudioBuffer();
    buffers.push_back(buffer);
    return buffer;
}

void AudioSystem::update() {
    // Update audio system if needed
}

} // namespace NihilEngine