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