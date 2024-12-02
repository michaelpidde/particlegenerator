#pragma once

#include "libs\SDL\include\SDL.h"
// cpp_common
#include "types.h"

#ifdef API_EXPORT
    #define _API __declspec(dllexport)
#else
    #define _API __declspec(dllimport)
#endif

struct SpriteAsset {
    i32 width = 0;
    i32 height = 0;
    SDL_Texture *texture;
    char name[255];
};

struct EmitterConfiguration {
    i32 maxParticles = 5000;
    i32 emitterWidth = 32;
    i32 emitterHeight = 32;
    double emitDuration = 1.0f;
    double emitRate = 0.05f;
    bool showEmitterBounds = false;
    double particleDuration = 1.5f;
    double particleFade = 1.015f;
    double particleVelocity = 1.0f;
    i32 emitMagnitude = 25;
    i32 particleType = 0;
    double particleRotationRate = 0.0f;
    i32 particleRotationMagnitude = 0;
    bool clearScreen = true;
    i32 pixelColorScheme = 0;
    i32 spriteCount = 0;
    i32 selectedSprite = 0;
    SpriteAsset *sprites;
};

struct PixelParticle {
    V2 position = {};
    RGBA color = {};
    double elapsed = 0.0f;
    double velocity = 1.0f;
    double moveDelta = 0.0f;
    V2 direction = {};
    bool inUse = false;
};

struct SpriteParticle {
    V2 position = {};
    i32 width = 32;
    i32 height = 32;
    SDL_Texture *texture;
    u8 textureAlpha = 255;
    double elapsed = 0.0f;
    double velocity = 1.0f;
    double angle = 0.0f;
    double rotationRate = 0.0f;
    i32 rotationMagnitude = 0;
    double rotationCounter = 0.0f;
    SDL_Point rotationCenter = {};
    double moveDelta = 0.0f;
    V2 direction = {};
    bool inUse = false;
};

enum PixelColorScheme {
    White, America
};

struct ParticleEmitter {
    bool initialized = false;
    u32 maxParticles = 5000;
    u32 width = 32;
    u32 height = 32;
    V2 position = {};
    double particleDuration = 1.5f;
    double particleVelocity = 1.0f;
    double particleFade = 1.015f;
    double particleRotationRate = 0.0f;
    i32 particleRotationMagnitude = 0;
    double emitDuration = 1.0f;
    double emitDurationCounter = 0.0f;
    double emitRate = 0.05f;
    double emitRateCounter = -1;
    i32 emitMagnitude = 25;
    bool showBounds = false;
    i32 particleType = 0;
    union {
        PixelParticle *pixels;
        SpriteParticle *sprites;
    } particles;
    i32 pixelColorScheme = 0;
    i32 selectedSprite = 0;
    bool clearScreen = true;
};

enum ParticleType {
    Pixel, Sprite
};

_API void LoadParticleSprites(SDL_Renderer *renderer, EmitterConfiguration *config, const char *folder);
_API void InitEmitter(ParticleEmitter &emitter, V2 origin, EmitterConfiguration &config, ParticleType type);
_API void UpdateAndRenderParticles(SDL_Renderer *renderer, ParticleEmitter &emitter, double delta);