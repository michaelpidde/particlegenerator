#include <ctime>
#include <iostream>
#include <stdint.h>
#include <windows.h>

#define SDL_MAIN_HANDLED

#include "libs/SDL2-2.30.8/include/SDL.h"
#include "libs/imgui/imgui.h"
#include "libs/imgui/backends/imgui_impl_sdl2.h"
#include "libs/imgui/backends/imgui_impl_sdlrenderer2.h"

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

enum struct Direction : u8 {
    North = 0,
    Northeast = 1,
    East = 2,
    Southeast = 3,
    South = 4,
    Southwest = 5,
    West = 6,
    Northwest = 7
};

struct PixelParticle {
    V2 position = {};
    RGBA color = {};
    double elapsed = 0.0f;
    double velocity = 2.0f;
    double moveDelta = 0.0f;
    Direction direction = Direction::North;
    bool done;
};

struct ParticleEmitter {
    bool initialized = false;
    u32 maxParticles = 100;
    u32 width = 32;
    u32 height = 32;
    V2 position = {};
    double particleDuration = 1.5f;
    double fade = 1.015f;
    //float emitDuration = 1.0f;
    PixelParticle *particles;
};

void InitParticleEmitter(ParticleEmitter &emitter, V2 clickPosition, u32 maxParticles = 100) {
    emitter.position = {
        .x = clickPosition.x - ((i32)emitter.width / 2),
        .y = clickPosition.y - ((i32)emitter.height / 2)
    };
    emitter.maxParticles = maxParticles;
    // TODO: Arena allocate if this is brought into game code
    emitter.particles = (PixelParticle *) malloc(sizeof(PixelParticle) * emitter.maxParticles);
    for(u32 i = 0; i < emitter.maxParticles; ++i) {
        PixelParticle particle = {};
        particle.position = {
            .x = emitter.position.x - ((i32)emitter.width / 2),
            .y = emitter.position.y - ((i32)emitter.height / 2)
        };
        particle.color = {
            .r = 255,
            .g = 255,
            .b = 255,
            .a = 255,
        };
        particle.direction = (Direction)RandomInteger(0, 7);
        particle.position = {
            .x = RandomInteger(emitter.position.x, emitter.position.x + emitter.width),
            .y = RandomInteger(emitter.position.y, emitter.position.y + emitter.height)
        };
        emitter.particles[i] = particle;
    }
    emitter.initialized = true;
}

void UpdateAndRenderParticles(SDL_Renderer *renderer, ParticleEmitter *emitter, double delta) {
    if(!emitter->initialized) {
        return;
    }

    bool allParticlesDone = true;
    for(u32 i = 0; i < emitter->maxParticles; ++i) {
        PixelParticle *particle = &emitter->particles[i];

        if(particle->done) {
            continue;
        }

        // If we get here, we're not done with all particles
        allParticlesDone = false;

        // Conversion of milliseconds into seconds
        particle->elapsed += delta * 0.001f;
        particle->moveDelta += delta * 0.001f;

        if(particle->elapsed >= emitter->particleDuration) {
            particle->done = true;
            continue;
        }

        switch(particle->direction) {
            case Direction::North:
                particle->position.y -= (i32)(1 * particle->velocity);
                break;
            case Direction::Northeast:
                particle->position.y -= (i32)(1 * particle->velocity);
                particle->position.x += (i32)(1 * particle->velocity);
                break;
            case Direction::East:
                particle->position.x += (i32)(1 * particle->velocity);
                break;
            case Direction::Southeast:
                particle->position.y += (i32)(1 * particle->velocity);
                particle->position.x += (i32)(1 * particle->velocity);
                break;
            case Direction::South:
                particle->position.y += (i32)(1 * particle->velocity);
                break;
            case Direction::Southwest:
                particle->position.y += (i32)(1 * particle->velocity);
                particle->position.x -= (i32)(1 * particle->velocity);
                break;
            case Direction::West:
                particle->position.x -= (i32)(1 * particle->velocity);
                break;
            case Direction::Northwest:
                particle->position.y -= (i32)(1 * particle->velocity);
                particle->position.x -= (i32)(1 * particle->velocity);
                break;
        }

        particle->color.a = (u8)(particle->color.a / emitter->fade);

        SDL_SetRenderDrawColor(renderer, particle->color.r, particle->color.g, particle->color.b, particle->color.a);
        SDL_RenderDrawPoint(renderer, particle->position.x, particle->position.y);
    }

    if(allParticlesDone) {
        emitter->initialized = false;
        return;
    }

    // TODO: This is only for testing to see the emitter position and size
    SDL_Rect rect = {};
    rect.h = emitter->height;
    rect.w = emitter->width;
    rect.x = emitter->position.x;
    rect.y = emitter->position.y;
    SDL_SetRenderDrawColor(renderer, 100, 200, 255, 255);
    SDL_RenderDrawRect(renderer, &rect);
}

int main() {
    srand((u32)time(NULL));

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
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
    if(renderer == nullptr) {
        SDL_Log("Error creating SDL_Renderer!");
        return -1;
    }
    if(SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND) == 0) {
        printf(SDL_GetError());
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);

    // Editor state
    float someValue = 0.0f;
    ParticleEmitter emitter = {};
    V2 mousePosition = {};

    // Set Windows thread resolution so we can maybe sleep in loop
    u32 desiredResolution = 1;
    const bool fixFramerate = (timeBeginPeriod(desiredResolution) == TIMERR_NOERROR);
    const u8 targetFramerate = 60;
    const double targetMsForFrame = (1.0 / targetFramerate) * 1000;
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
            if(event.type == SDL_MOUSEBUTTONDOWN) {
                SDL_GetMouseState(&mousePosition.x, &mousePosition.y);
                if(!emitter.initialized) {
                    InitParticleEmitter(emitter, mousePosition, 100);
                    printf("Particle emitter created\n");
                }
            }
        }
        if(SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED) {
            SDL_Delay(10);
            continue;
        }

        // Start the Dear ImGui frame
        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        // Set up window
        ImGui::Begin("Particle Settings");
        ImGui::SliderFloat("Some Value", &someValue, 0.0f, 1.0f);
        // This was useful to compare my loop's timing to what IMGUI is doing
        //ImGui::Text("IMGUI averages %.3f ms/f (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        ImGui::Text("Loop averages %.02f ms/f (%.02f FPS)", msPerFrame, fps);
        ImGui::End();

        // Rendering
        ImGui::Render();
        SDL_RenderSetScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderClear(renderer);
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);

        UpdateAndRenderParticles(renderer, &emitter, msPerFrame);

        SDL_RenderPresent(renderer);

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

        endCount = GetWallClock();
        endCpuCycles = __rdtsc();
        elapsedCount = endCount.QuadPart - lastCount.QuadPart;
        elapsedCpuCycles = endCpuCycles - lastCpuCycles;
        msPerFrame = ((1000.0f * (double) elapsedCount) / (double) frequency);
        refreshCounter += (u16)msPerFrame;
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
