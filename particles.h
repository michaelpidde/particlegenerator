#pragma once

#include "particlegenerator.h"
#include "libs/SDL2-2.30.8/include/SDL.h"

void InitEmitter(ParticleEmitter &emitter, V2 clickPosition, EditorState &state);
void InitEmitter(ParticleEmitter &emitter, V2 clickPosition, EditorState &state, SDL_Texture *texture, i32 width,
                 i32 height);

void InitSpriteParticle(ParticleEmitter &emitter, SpriteParticle *particle, SDL_Texture *texture, i32 width, i32 height,
                        bool inUse);
void InitPixelParticle(ParticleEmitter &emitter, PixelParticle *particle, bool inUse);

void UpdateAndRenderPixelParticles(SDL_Renderer *renderer, ParticleEmitter &emitter, double delta);
void UpdateAndRenderSpriteParticles(SDL_Renderer *renderer, ParticleEmitter &emitter, double delta);
