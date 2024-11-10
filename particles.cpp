#include "particlegenerator.h"
#include "particles.h"
#include "libs/SDL/include/SDL.h"

void InitSpriteParticle(ParticleEmitter &emitter, SpriteParticle *particle, SDL_Texture *texture, i32 width, i32 height,
                        bool inUse) {
    emitter.particleType = ParticleType::Sprite;

    // Emit entire particle collection at once
    if(emitter.emitRate == 0.0f || inUse) {
        particle->inUse = true;
    } else {
        particle->inUse = false;
    }
    particle->velocity = emitter.particleVelocity;
    particle->elapsed = 0.0f;
    particle->rotationRate = emitter.particleRotationRate;
    particle->rotationMagnitude = emitter.particleRotationMagnitude;
    particle->rotationCounter = 0.0f;
    particle->angle = 0.0f;

    particle->texture = texture;
    particle->textureAlpha = 255;
    particle->width = width;
    particle->height = height;

    particle->rotationCenter.x = (i32)(particle->width / 2);
    particle->rotationCenter.y = (i32)(particle->height / 2);

    particle->direction = {
        .x = RandomInteger(-10, 10),
        .y = RandomInteger(-10, 10)
    };
    particle->position = {
        .x = RandomInteger(emitter.position.x, emitter.position.x + emitter.width),
        .y = RandomInteger(emitter.position.y, emitter.position.y + emitter.height)
    };
}

void InitPixelParticle(ParticleEmitter &emitter, PixelParticle *particle, bool inUse) {
    emitter.particleType = ParticleType::Pixel;

    // Emit entire particle collection at once
    if(emitter.emitRate == 0.0f || inUse) {
        particle->inUse = true;
    } else {
        particle->inUse = false;
    }
    particle->velocity = emitter.particleVelocity;
    particle->elapsed = 0;

    switch((PixelColorScheme)emitter.pixelColorScheme) {
        case PixelColorScheme::White:
            particle->color.r = 255;
            particle->color.g = 255;
            particle->color.b = 255;
            particle->color.a = 255;
            break;
        case PixelColorScheme::America:
            switch(RandomInteger(0, 2)) {
                case 0:
                    particle->color.r = 255;
                    particle->color.g = 0;
                    particle->color.b = 0;
                    break;
                case 1:
                    particle->color.r = 0;
                    particle->color.g = 0;
                    particle->color.b = 255;
                    break;
                case 2:
                    particle->color.r = 255;
                    particle->color.g = 255;
                    particle->color.b = 255;
                    break;
            }
            particle->color.a = 255;
            break;
    }

    particle->direction = {
        .x = RandomInteger(-10, 10),
        .y = RandomInteger(-10, 10)
    };
    particle->position = {
        .x = RandomInteger(emitter.position.x, emitter.position.x + emitter.width),
        .y = RandomInteger(emitter.position.y, emitter.position.y + emitter.height)
    };
}

void BaseEmitter(ParticleEmitter &emitter, V2 clickPosition, EditorState &state) {
    emitter = {};
    emitter.width = state.emitterWidth;
    emitter.height = state.emitterHeight;

    emitter.position = {
        .x = clickPosition.x - ((i32) emitter.width / 2),
        .y = clickPosition.y - ((i32) emitter.height / 2)
    };

    emitter.maxParticles = state.maxParticles;
    emitter.particleType = state.particleType;
    emitter.particleDuration = state.particleDuration;
    emitter.particleVelocity = state.particleVelocity;
    emitter.particleFade = state.particleFade;
    emitter.showBounds = state.showEmitterBounds;
    emitter.emitRate = state.emitRate;
    emitter.emitDuration = state.emitDuration;
    emitter.emitMagnitude = state.emitMagnitude;
    emitter.particleRotationRate = state.particleRotationRate;
    emitter.particleRotationMagnitude = state.particleRotationMagnitude;
    emitter.clearScreen = state.clearScreen;
    emitter.pixelColorScheme = state.pixelColorScheme;
}

void InitEmitter(ParticleEmitter &emitter, V2 clickPosition, EditorState &state) {
    BaseEmitter(emitter, clickPosition, state);

    // TODO: Arena allocate if this is brought into game code
    emitter.particles.pixels = (PixelParticle *) malloc(sizeof(PixelParticle) * emitter.maxParticles);
    for(u32 i = 0; i < emitter.maxParticles; ++i) {
        PixelParticle *particle = &emitter.particles.pixels[i];
        InitPixelParticle(emitter, particle, false);
    }
    emitter.initialized = true;
}

void InitEmitter(ParticleEmitter &emitter, V2 clickPosition, EditorState &state, SDL_Texture *texture, i32 width,
                 i32 height) {
    BaseEmitter(emitter, clickPosition, state);

    emitter.particles.sprites = (SpriteParticle *) malloc(sizeof(SpriteParticle) * emitter.maxParticles);
    for(u32 i = 0; i < emitter.maxParticles; ++i) {
        SpriteParticle *particle = &emitter.particles.sprites[i];
        InitSpriteParticle(emitter, particle, texture, width, height, false);
    }
    emitter.initialized = true;
}

void UpdateAndRenderPixelParticles(SDL_Renderer *renderer, ParticleEmitter &emitter, double delta) {
    if(!emitter.initialized) {
        return;
    }

    if(emitter.emitDurationCounter <= emitter.emitDuration) {
        if(emitter.emitRateCounter >= emitter.emitRate || emitter.emitRateCounter == -1) {
            i32 magnitudeCount = 0;
            for(u32 i = 0; i < emitter.maxParticles; ++i) {
                PixelParticle *particle = &emitter.particles.pixels[i];
                if(!particle->inUse) {
                    InitPixelParticle(emitter, particle, true);
                    if(emitter.emitMagnitude > 1 && magnitudeCount < emitter.emitMagnitude) {
                        ++magnitudeCount;
                    } else {
                        break;
                    }
                }
            }

            emitter.emitRateCounter = 0;
        } else {
            emitter.emitRateCounter += delta * 0.001f;
        }

        emitter.emitDurationCounter += delta * 0.001f;
    }

    bool allParticlesDone = true;
    for(u32 i = 0; i < emitter.maxParticles; ++i) {
        PixelParticle *particle = &emitter.particles.pixels[i];

        if(!particle->inUse) {
            continue;
        }

        // If we get here, we're not done with all particles
        allParticlesDone = false;

        // Conversion of milliseconds into seconds
        particle->elapsed += delta * 0.001f;
        particle->moveDelta += delta;

        if(particle->elapsed >= emitter.particleDuration) {
            particle->inUse = false;
            continue;
        }

        if(particle->moveDelta >= particle->velocity) {
            particle->position.x += (i32)(particle->direction.x * particle->velocity);
            particle->position.y += (i32)(particle->direction.y * particle->velocity);
            particle->moveDelta = 0;
        }

        // TODO: This probably needs to be wrapped in a counter
        particle->color.a = (u8)(particle->color.a / emitter.particleFade);

        SDL_SetRenderDrawColor(renderer, particle->color.r, particle->color.g, particle->color.b, particle->color.a);
        SDL_RenderDrawPoint(renderer, particle->position.x, particle->position.y);
    }

    if(allParticlesDone) {
//        printf("Deallocate Pixels\n");
        emitter.initialized = false;
        free(emitter.particles.pixels);
        return;
    }

    if(emitter.showBounds) {
        SDL_Rect rect = {};
        rect.h = emitter.height;
        rect.w = emitter.width;
        rect.x = emitter.position.x;
        rect.y = emitter.position.y;
        SDL_SetRenderDrawColor(renderer, 100, 200, 255, 255);
        SDL_RenderDrawRect(renderer, &rect);
    }
}

void UpdateAndRenderSpriteParticles(SDL_Renderer *renderer, ParticleEmitter &emitter, double delta) {
    if(!emitter.initialized) {
        return;
    }

    if(emitter.emitDurationCounter <= emitter.emitDuration) {
        if(emitter.emitRateCounter >= emitter.emitRate || emitter.emitRateCounter == -1) {
            i32 magnitudeCount = 0;
            for(u32 i = 0; i < emitter.maxParticles; ++i) {
                SpriteParticle *particle = &emitter.particles.sprites[i];
                if(!particle->inUse) {
                    InitSpriteParticle(emitter, particle, particle->texture, particle->width, particle->height, true);
                    if(emitter.emitMagnitude > 1 && magnitudeCount < emitter.emitMagnitude) {
                        ++magnitudeCount;
                    } else {
                        break;
                    }
                }
            }

            emitter.emitRateCounter = 0;
        } else {
            emitter.emitRateCounter += delta * 0.001f;
        }

        emitter.emitDurationCounter += delta * 0.001f;
    }

    bool allParticlesDone = true;
    for(u32 i = 0; i < emitter.maxParticles; ++i) {
        SpriteParticle *particle = &emitter.particles.sprites[i];

        if(!particle->inUse) {
            continue;
        }

        // If we get here, we're not done with all particles
        allParticlesDone = false;

        particle->elapsed += delta * 0.001f;
        particle->moveDelta += delta;
        particle->rotationCounter += delta * 0.001f;

        if(particle->elapsed >= emitter.particleDuration) {
            particle->inUse = false;
            continue;
        }

        if(particle->rotationCounter >= particle->rotationRate) {
            particle->angle += 1 * particle->rotationMagnitude;
            particle->rotationCounter = 0;
        }

        if(particle->moveDelta >= particle->velocity) {
            particle->position.x += (i32)(particle->direction.x * particle->velocity);
            particle->position.y += (i32)(particle->direction.y * particle->velocity);
            particle->moveDelta = 0;
        }

        particle->textureAlpha = (u8)(particle->textureAlpha / emitter.particleFade);

        SDL_SetTextureAlphaMod(particle->texture, particle->textureAlpha);

        SDL_Rect srcRect;
        srcRect.x = 0;
        srcRect.y = 0;
        srcRect.w = particle->width;
        srcRect.h = particle->height;

        SDL_Rect destRect;
        destRect.x = particle->position.x;
        destRect.y = particle->position.y;
        destRect.w = particle->width;
        destRect.h = particle->height;

        SDL_RenderCopyEx(renderer, particle->texture, &srcRect, &destRect, particle->angle, &particle->rotationCenter,
                         SDL_FLIP_NONE);
    }

    if(allParticlesDone) {
//        printf("Deallocate Sprites\n");
        emitter.initialized = false;
        free(emitter.particles.sprites);
        return;
    }

    if(emitter.showBounds) {
        SDL_Rect rect = {};
        rect.h = emitter.height;
        rect.w = emitter.width;
        rect.x = emitter.position.x;
        rect.y = emitter.position.y;
        SDL_SetRenderDrawColor(renderer, 100, 200, 255, 255);
        SDL_RenderDrawRect(renderer, &rect);
    }
}
