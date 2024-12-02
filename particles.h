#pragma once

#include "api\particlegenerator.h"

#include "libs/SDL/include/SDL.h"
// cpp_common
#include "types.h"

void InitSpriteParticle(ParticleEmitter &emitter, SpriteParticle *particle, bool inUse);
void InitPixelParticle(ParticleEmitter &emitter, PixelParticle *particle, bool inUse);
