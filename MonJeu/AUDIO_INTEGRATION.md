# Audio Integration Guide for Player Class

## Overview
The Player class now includes audio capabilities using NihilEngine's AudioSystem. This allows for spatial 3D audio effects that enhance gameplay immersion.

## Audio Files Required
You'll need to add the following audio files to your `MonJeu/assets/sounds/` directory:

- `footstep.wav` - Sound played when player walks
- `jump.wav` - Sound played when player jumps
- `land.wav` - Sound played when player lands after falling

## File Format Requirements
- **Format**: WAV files (uncompressed)
- **Sample Rate**: 44100 Hz recommended
- **Channels**: Mono or Stereo
- **Bit Depth**: 16-bit recommended

## Implementation Details

### Audio Components Added
```cpp
// Audio source for spatial positioning
NihilEngine::AudioSource* m_AudioSource = nullptr;

// Audio buffers for different sound effects
NihilEngine::AudioBuffer* m_FootstepBuffer = nullptr;
NihilEngine::AudioBuffer* m_JumpBuffer = nullptr;
NihilEngine::AudioBuffer* m_LandBuffer = nullptr;
```

### Audio Methods
- `PlayFootstepSound()` - Plays walking sound effects
- `PlayJumpSound()` - Plays jump sound
- `PlayLandSound()` - Plays landing sound after falls
- `UpdateAudioPosition()` - Updates audio source position to match player

### Audio Triggers
- **Footsteps**: Play every 0.5 seconds when moving on ground
- **Jump**: Plays when spacebar is pressed while on ground
- **Land**: Plays when hitting ground after falling (only if falling fast enough)

## Usage Example
```cpp
// In your game initialization
Player* player = new Player();

// Audio is automatically initialized in Player constructor
// Make sure AudioSystem is initialized in main.cpp first

// Audio will automatically play during gameplay:
// - Footsteps when walking
// - Jump sound when jumping
// - Land sound when hitting ground
```

## Next Steps
1. Add the required audio files to `MonJeu/assets/sounds/`
2. Uncomment the LoadFromFile calls in Player constructor
3. Test the audio integration
4. Adjust volume levels if needed

## Troubleshooting
- If no sounds play, check that AudioSystem is initialized in main.cpp
- Verify audio files exist and are in the correct format
- Check console output for any audio loading errors