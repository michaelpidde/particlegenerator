#pragma once

#include "particlegenerator.h"
#include "libs/SDL2-2.30.8/include/SDL.h"

struct V2 {
    i32 x;
    i32 y;
};

struct RGBA {
    u8 r;
    u8 g;
    u8 b;
    u8 a;
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

enum ParticleType {
    Pixel, Sprite
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
    bool clearScreen = true;
};

void InitEmitter(ParticleEmitter &emitter, V2 clickPosition, EditorState &state);
void InitEmitter(ParticleEmitter &emitter, V2 clickPosition, EditorState &state, SDL_Texture *texture, i32 width,
                 i32 height);

void InitSpriteParticle(ParticleEmitter &emitter, SpriteParticle *particle, SDL_Texture *texture, i32 width, i32 height,
                        bool inUse);
void InitPixelParticle(ParticleEmitter &emitter, PixelParticle *particle, bool inUse);

void UpdateAndRenderPixelParticles(SDL_Renderer *renderer, ParticleEmitter &emitter, double delta);
void UpdateAndRenderSpriteParticles(SDL_Renderer *renderer, ParticleEmitter &emitter, double delta);
