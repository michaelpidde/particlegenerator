#include <cassert>
#include <ctime>
#include <iostream>
#include <stdint.h>
#include <windows.h>

#define SDL_MAIN_HANDLED

#include "libs/SDL2-2.30.8/include/SDL.h"
#include "libs/SDL2_image-2.8.2/include/SDL_image.h"
#include "libs/imgui/backends/imgui_impl_sdl2.h"
#include "libs/imgui/backends/imgui_impl_sdlrenderer2.h"

#define IMGUI_DEFINE_MATH_OPERATORS

#include "libs/imgui/imgui.h"

#define u64 uint64_t
#define u32 uint32_t
#define u16 uint16_t
#define u8 uint8_t
#define i64 int64_t
#define i32 int32_t
#define i16 int16_t
#define i8 int8_t

LARGE_INTEGER GetWallClock() {
    LARGE_INTEGER result;
    QueryPerformanceCounter(&result);
    return result;
}

static u64 frequency;

double GetElapsedTime(LARGE_INTEGER start, LARGE_INTEGER end) {
    return (double) (end.QuadPart - start.QuadPart) / frequency;
}

i32 RandomInteger(i32 min, i32 max) {
    return rand() % (max - min + 1) + min;
}

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
    u32 maxParticles = 100;
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
    i32 emitMagnitude = 1;
    bool showBounds = false;
    i32 particleType = 0;
    union {
        PixelParticle *pixels;
        SpriteParticle *sprites;
    } particles;
    bool clearScreen = true;
};

struct EditorState {
    i32 maxParticles = 1;
    i32 emitterWidth = 32;
    i32 emitterHeight = 32;
    double emitDuration = 1.0f;
    double emitRate = 0.05f;
    bool showEmitterBounds = false;
    double particleDuration = 1.5f;
    double particleFade = 1.015f;
    double particleVelocity = 1.0f;
    i32 emitMagnitude = 1;
    i32 particleType = 0;
    double particleRotationRate = 0.0f;
    i32 particleRotationMagnitude = 0;
    bool clearScreen = true;
};

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

    switch(RandomInteger(0, 2)) {
        case 0:
            particle->color.r = 255;
            particle->color.g = 150;
            particle->color.b = 150;
            break;
        case 1:
            particle->color.r = 150;
            particle->color.g = 150;
            particle->color.b = 255;
            break;
        case 2:
            particle->color.r = 255;
            particle->color.g = 255;
            particle->color.b = 255;
            break;
    }
    particle->color.a = 255;

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
        printf("Deallocate Pixels\n");
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
        printf("Deallocate Sprites\n");
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

int main() {
    srand((u32) time(NULL));

    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }

    SDL_WindowFlags window_flags = (SDL_WindowFlags) (SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window *window = SDL_CreateWindow("Dear ImGui SDL2+SDL_Renderer example", SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
    if(window == nullptr) {
        printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
        return -1;
    }
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_TARGETTEXTURE);
    if(renderer == nullptr) {
        SDL_Log(SDL_GetError());
        return -1;
    }

    // TODO: This is just spammed in here. Obvious make this more dynamic to load different images.
    SDL_Surface *loadingSurface = IMG_Load("sprite\\woman_front2_larger.png");
    assert(loadingSurface != NULL);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, loadingSurface);
    assert(texture != NULL);
    SDL_FreeSurface(loadingSurface);
    i32 textureWidth = 0;
    i32 textureHeight = 0;
    SDL_QueryTexture(texture, NULL, NULL, &textureWidth, &textureHeight);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;

    // Setup Dear ImGui style
    ImGui::StyleColorsClassic();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);

    // Editor state
    ParticleEmitter emitter = {};
    EditorState state = {};
    V2 mousePosition = {};

    /*
     * TODO: The window could be resized at which point we would need to generate a new texture of that size.
     * That can be handled in an event handler. Copy the contents of this texture into a new one if
     * we need to not "clear the screen" between frames.
     */
    SDL_Texture *particleTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, (i32)io.DisplaySize.x, (i32)io.DisplaySize.y);
    SDL_SetTextureBlendMode(particleTexture, SDL_BLENDMODE_BLEND);

#ifdef FIX_FRAMERATE
    // Set Windows thread resolution so we can maybe sleep in loop
    u32 desiredResolution = 1;
    const bool fixFramerate = (timeBeginPeriod(desiredResolution) == TIMERR_NOERROR);
    const u8 targetFramerate = 60;
    const double targetMsForFrame = (1.0 / targetFramerate) * 1000;
#endif
    // These control how often we should refresh the FPS display
    const u16 refreshMs = 100;
    u16 refreshCounter = 0;

    LARGE_INTEGER performanceFrequencyQuery;
    QueryPerformanceFrequency(&performanceFrequencyQuery);
    frequency = performanceFrequencyQuery.QuadPart;

    LARGE_INTEGER lastCount = GetWallClock();
    u64 lastCpuCycles = __rdtsc();

    LARGE_INTEGER endCount;
    u64 endCpuCycles = 0;
    u64 elapsedCount = 0;
    u64 elapsedCpuCycles = 0;
    double msPerFrame = 0;
    double fps = 0;

    // Main loop
    bool done = false;
    while(!done) {
        SDL_Event event;
        while(SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if(event.type == SDL_QUIT) {
                done = true;
            }
            if(event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE &&
               event.window.windowID == SDL_GetWindowID(window)) {
                done = true;
            }
            if(!io.WantCaptureMouse) {
                if(event.type == SDL_MOUSEBUTTONDOWN) {
                    SDL_GetMouseState(&mousePosition.x, &mousePosition.y);
                    if(!emitter.initialized) {
                        switch((ParticleType) state.particleType) {
                            case ParticleType::Pixel:
                                InitEmitter(emitter, mousePosition, state);
                                printf("Pixel emitter created\n");
                                break;
                            case ParticleType::Sprite:
                                InitEmitter(emitter, mousePosition, state, texture, textureWidth, textureHeight);
                                printf("Sprite emitter created\n");
                                break;
                            default:
                                printf("Can't create emitter type %d\n", state.particleType);
                                break;
                        }

                    }
                }
            }
        }
        if(SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED) {
            SDL_Delay(10);
            continue;
        }

        if(SDL_SetRenderTarget(renderer, particleTexture) != 0) {
            SDL_Log("Cannot render to target: %s", SDL_GetError());
        }
        if(emitter.clearScreen) {
            // Clear texture per frame
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
            SDL_RenderClear(renderer);
        }
        switch((ParticleType) emitter.particleType) {
            case ParticleType::Pixel:
                UpdateAndRenderPixelParticles(renderer, emitter, msPerFrame);
                break;
            case ParticleType::Sprite:
                UpdateAndRenderSpriteParticles(renderer, emitter, msPerFrame);
                break;
        }
        SDL_SetRenderTarget(renderer, NULL);
        SDL_Rect srcPos;
        srcPos.x = 0;
        srcPos.y = 0;
        srcPos.w = (i32)io.DisplaySize.x;
        srcPos.h = (i32)io.DisplaySize.y;
        SDL_RenderCopy(renderer, particleTexture, &srcPos, &srcPos);

        // Start the Dear ImGui frame
        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        // Set up window
        ImGui::SetNextWindowBgAlpha(255);
        ImGui::SetNextWindowPos(ImVec2(2, 2));
        ImGui::SetNextWindowSize(ImVec2(400, 0));
        bool open = true;
        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar |
                                       ImGuiWindowFlags_NoMove |
                                       ImGuiWindowFlags_NoResize |
                                       ImGuiWindowFlags_AlwaysAutoResize;
        ImGui::Begin("Settings", &open, windowFlags);
        ImGui::RadioButton("Pixel", &state.particleType, 0);
        ImGui::RadioButton("Sprite", &state.particleType, 1);

        ImGui::NewLine();
        ImGui::SeparatorText("Emitter");

        ImGui::SliderInt("Max Particles", &state.maxParticles, 1, 5000);
        ImGui::SliderInt("Emitter Width", &state.emitterWidth, 1, 250);
        ImGui::SliderInt("Emitter Height", &state.emitterHeight, 1, 250);
        ImGui::InputDouble("Emit Rate", &state.emitRate, 0.001f, 0.1f);
        ImGui::SliderInt("Emit Magnitude", &state.emitMagnitude, 1, 100);
        ImGui::Text("(0 Rate will emit all particles at once)");
        ImGui::InputDouble("Emit Duration", &state.emitDuration, 1.0f, 1.0f);
        ImGui::Checkbox("Show Bounds", &state.showEmitterBounds);
        ImGui::Checkbox("Clear Screen Between Frames", &state.clearScreen);

        ImGui::NewLine();
        ImGui::SeparatorText("Particle");

        ImGui::InputDouble("Particle Duration", &state.particleDuration, 0.1f, 0.1f);
        ImGui::InputDouble("Particle Fade", &state.particleFade, 1.0f, 0.5f);
        ImGui::InputDouble("Particle Velocity", &state.particleVelocity, 1.0f, 1.0f);
        if((ParticleType) state.particleType == ParticleType::Sprite) {
            ImGui::InputDouble("Particle Rotation", &state.particleRotationRate, 0.1f, 1.0f);
            ImGui::SliderInt("Rotation Magnitude", &state.particleRotationMagnitude, 0, 25);
        }

        ImGui::NewLine();
        ImGui::SeparatorText("Stats");
        ImGui::Text("Loop averages %.02f ms/f (%.02f FPS)", msPerFrame, fps);
        ImGui::End();

//        ImGui::ShowDemoWindow();

        // Rendering
        ImGui::Render();
        SDL_RenderSetScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);

        SDL_RenderPresent(renderer);

#ifdef FIX_FRAMERATE
        LARGE_INTEGER workCount = GetWallClock();
        double msElapsedForFrame = GetElapsedTime(lastCount, workCount);
        if(msElapsedForFrame < targetMsForFrame) {
            if(fixFramerate) {
                u8 sleepMs = (u8)(targetMsForFrame - msElapsedForFrame);
                if(sleepMs > 0) {
                    Sleep(sleepMs);
                }
            }

            while(msElapsedForFrame < targetMsForFrame) {
                msElapsedForFrame += GetElapsedTime(lastCount, GetWallClock());
            }
        }
#endif

        endCount = GetWallClock();
        endCpuCycles = __rdtsc();
        elapsedCount = endCount.QuadPart - lastCount.QuadPart;
        elapsedCpuCycles = endCpuCycles - lastCpuCycles;
        msPerFrame = ((1000.0f * (double) elapsedCount) / (double) frequency);
        refreshCounter += (u16) msPerFrame;
        if(refreshCounter >= refreshMs) {
            fps = ((double) frequency / (double) elapsedCount);
            refreshCounter = 0;
        }
        lastCount = endCount;
        lastCpuCycles = endCpuCycles;
    }

    // Cleanup
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
