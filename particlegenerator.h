#pragma once

#include <windows.h>
#include <stdint.h>
#include "libs/SDL2-2.30.8/include/SDL.h"

#define u64 uint64_t
#define u32 uint32_t
#define u16 uint16_t
#define u8 uint8_t
#define i64 int64_t
#define i32 int32_t
#define i16 int16_t
#define i8 int8_t

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
    bool clearScreen = true;
};

struct EditorState {
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
};

inline i32 RandomInteger(i32 min, i32 max) {
    return rand() % (max - min + 1) + min;
}