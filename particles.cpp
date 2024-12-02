#include "api\particlegenerator.h"
#include "particles.h"
#include "libs/SDL/include/SDL.h"
// cpp_common
#include "methods.h"

void InitSpriteParticle(ParticleEmitter &emitter, SpriteParticle *particle, SDL_Texture *texture, i32 width, i32 height, bool inUse) {
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
    particle->width = width;
    particle->height = height;

    particle->textureAlpha = 255;

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

void BaseEmitter(ParticleEmitter &emitter, V2 origin, EmitterConfiguration &config) {
    emitter = {};
    emitter.width = config.emitterWidth;
    emitter.height = config.emitterHeight;

    emitter.position = {
        .x = origin.x - ((i32) emitter.width / 2),
        .y = origin.y - ((i32) emitter.height / 2)
    };

    emitter.maxParticles = config.maxParticles;
    emitter.particleType = config.particleType;
    emitter.particleDuration = config.particleDuration;
    emitter.particleVelocity = config.particleVelocity;
    emitter.particleFade = config.particleFade;
    emitter.showBounds = config.showEmitterBounds;
    emitter.emitRate = config.emitRate;
    emitter.emitDuration = config.emitDuration;
    emitter.emitMagnitude = config.emitMagnitude;
    emitter.particleRotationRate = config.particleRotationRate;
    emitter.particleRotationMagnitude = config.particleRotationMagnitude;
    emitter.clearScreen = config.clearScreen;
    emitter.pixelColorScheme = config.pixelColorScheme;
    emitter.selectedSprite = config.selectedSprite;
}

void InitEmitter(ParticleEmitter &emitter, V2 origin, EmitterConfiguration &config, ParticleType type) {
    BaseEmitter(emitter, origin, config);

    // TODO: Arena allocate if this is brought into game code
    switch(type) {
        case ParticleType::Pixel:
            emitter.particles.pixels = (PixelParticle *) malloc(sizeof(PixelParticle) * emitter.maxParticles);
            for(u32 i = 0; i < emitter.maxParticles; ++i) {
                PixelParticle *particle = &emitter.particles.pixels[i];
                InitPixelParticle(emitter, particle, false);
            }
            break;
        case ParticleType::Sprite:
            emitter.particles.sprites = (SpriteParticle *) malloc(sizeof(SpriteParticle) * emitter.maxParticles);
            SDL_Texture *texture = config.sprites[config.selectedSprite].texture;
            i32 width = config.sprites[config.selectedSprite].width;
            i32 height = config.sprites[config.selectedSprite].height;
            for(u32 i = 0; i < emitter.maxParticles; ++i) {
                SpriteParticle *particle = &emitter.particles.sprites[i];
                InitSpriteParticle(emitter, particle, texture, width, height, false);
            }
            break;
    }

    emitter.initialized = true;
}

void UpdateAndRenderParticles(SDL_Renderer *renderer, ParticleEmitter &emitter, double delta) {
    if(!emitter.initialized) {
        return;
    }

    if(emitter.emitDurationCounter <= emitter.emitDuration) {
        if(emitter.emitRateCounter >= emitter.emitRate || emitter.emitRateCounter == -1) {
            i32 magnitudeCount = 0;
            PixelParticle *pixel;
            SpriteParticle *sprite;

            bool emitMoreParticles = false;
            u32 i = 0;
            if(i < emitter.maxParticles) {
                emitMoreParticles = true;
            }
            while(emitMoreParticles) {
                switch(emitter.particleType) {
                    case ParticleType::Pixel:
                        pixel = &emitter.particles.pixels[i];
                        if(!pixel->inUse) {
                            InitPixelParticle(emitter, pixel, true);
                            if(emitter.emitMagnitude > 1 && magnitudeCount < emitter.emitMagnitude) {
                                ++magnitudeCount;
                            } else {
                                emitMoreParticles = false;
                            }
                        }
                    break;
                    case ParticleType::Sprite:
                        sprite = &emitter.particles.sprites[i];
                        if(!sprite->inUse) {
                            InitSpriteParticle(emitter, sprite, sprite->texture, sprite->width, sprite->height, true);
                            if(emitter.emitMagnitude > 1 && magnitudeCount < emitter.emitMagnitude) {
                                ++magnitudeCount;
                            } else {
                                emitMoreParticles = false;
                            }
                        }
                    break;
                }

                ++i;
            }


            emitter.emitRateCounter = 0;
        } else {
            emitter.emitRateCounter += delta * 0.001f;
        }

        emitter.emitDurationCounter += delta * 0.001f;
    }

    bool allParticlesDone = true;

    /*
     * Iterate individual particles to move and render them
     */
    switch(emitter.particleType) {


        case ParticleType::Pixel:
            for(u32 i = 0; i < emitter.maxParticles; ++i) {
                PixelParticle *pixel = &emitter.particles.pixels[i];

                if(!pixel->inUse) {
                    continue;
                }

                // If we get here, we're not done with all particles
                allParticlesDone = false;

                // Conversion of milliseconds into seconds
                pixel->elapsed += delta * 0.001f;
                pixel->moveDelta += delta;

                if(pixel->elapsed >= emitter.particleDuration) {
                    pixel->inUse = false;
                    continue;
                }

                if(pixel->moveDelta >= pixel->velocity) {
                    pixel->position.x += (i32)(pixel->direction.x * pixel->velocity);
                    pixel->position.y += (i32)(pixel->direction.y * pixel->velocity);
                    pixel->moveDelta = 0;
                }

                // TODO: This probably needs to be wrapped in a counter
                pixel->color.a = (u8)(pixel->color.a / emitter.particleFade);

                SDL_SetRenderDrawColor(renderer, pixel->color.r, pixel->color.g, pixel->color.b, pixel->color.a);
                SDL_RenderDrawPoint(renderer, pixel->position.x, pixel->position.y);
            }
        break;


        case ParticleType::Sprite:
            for(u32 i = 0; i < emitter.maxParticles; ++i) {
                SpriteParticle *sprite = &emitter.particles.sprites[i];

                if(!sprite->inUse) {
                    continue;
                }

                // If we get here, we're not done with all particles
                allParticlesDone = false;

                sprite->elapsed += delta * 0.001f;
                sprite->moveDelta += delta;
                sprite->rotationCounter += delta * 0.001f;

                if(sprite->elapsed >= emitter.particleDuration) {
                    sprite->inUse = false;
                    continue;
                }

                if(sprite->rotationCounter >= sprite->rotationRate) {
                    sprite->angle += 1 * sprite->rotationMagnitude;
                    sprite->rotationCounter = 0;
                }

                if(sprite->moveDelta >= sprite->velocity) {
                    sprite->position.x += (i32)(sprite->direction.x * sprite->velocity);
                    sprite->position.y += (i32)(sprite->direction.y * sprite->velocity);
                    sprite->moveDelta = 0;
                }

                sprite->textureAlpha = (u8)(sprite->textureAlpha / emitter.particleFade);

                SDL_SetTextureAlphaMod(sprite->texture, sprite->textureAlpha);

                SDL_Rect srcRect;
                srcRect.x = 0;
                srcRect.y = 0;
                srcRect.w = sprite->width;
                srcRect.h = sprite->height;

                SDL_Rect destRect;
                destRect.x = sprite->position.x;
                destRect.y = sprite->position.y;
                destRect.w = sprite->width;
                destRect.h = sprite->height;

                SDL_RenderCopyEx(renderer, sprite->texture, &srcRect, &destRect, sprite->angle, &sprite->rotationCenter,
                                SDL_FLIP_NONE);
            }
        break;


    }

    if(allParticlesDone) {
        emitter.initialized = false;
        switch(emitter.particleType) {
            case ParticleType::Pixel:
                free(emitter.particles.pixels);
            break;
            case ParticleType::Sprite:
                free(emitter.particles.sprites);
            break;
        }
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
