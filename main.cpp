#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string.h>
#include <windows.h>

// This is dumb. Define this prior to ANYTHING that includes SDL.h
#define SDL_MAIN_HANDLED

#include "api\particlegenerator.h"
#include "particles.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "libs/imgui/backends/imgui_impl_sdl2.h"
#include "libs/imgui/backends/imgui_impl_sdlrenderer2.h"
#include "libs/imgui/imgui.h"

LARGE_INTEGER GetWallClock() {
    LARGE_INTEGER result;
    QueryPerformanceCounter(&result);
    return result;
}

static u64 frequency;

double GetElapsedTime(LARGE_INTEGER start, LARGE_INTEGER end) {
    return (double) (end.QuadPart - start.QuadPart) / frequency;
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
    EmitterConfiguration config = {};
    V2 mousePosition = {};

    LoadParticleSprites(renderer, &config, "sprite");

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
    const u16 refreshMs = 500;
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
                        switch((ParticleType) config.particleType) {
                            case ParticleType::Pixel:
                                InitEmitter(emitter, mousePosition, config, ParticleType::Pixel);
                                break;
                            case ParticleType::Sprite:
                                InitEmitter(emitter, mousePosition, config, ParticleType::Sprite);
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
        
        UpdateAndRenderParticles(renderer, emitter, msPerFrame);

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
        ImGui::RadioButton("Pixel", &config.particleType, 0);
        ImGui::RadioButton("Sprite", &config.particleType, 1);

        ImGui::NewLine();
        ImGui::SeparatorText("Emitter");

        ImGui::SliderInt("Max Particles", &config.maxParticles, 1, 5000);
        ImGui::SliderInt("Emitter Width", &config.emitterWidth, 1, 250);
        ImGui::SliderInt("Emitter Height", &config.emitterHeight, 1, 250);
        ImGui::InputDouble("Emit Rate", &config.emitRate, 0.001f, 0.1f);
        ImGui::SliderInt("Emit Magnitude", &config.emitMagnitude, 1, 100);
        ImGui::Text("(0 Rate will emit all particles at once)");
        ImGui::InputDouble("Emit Duration", &config.emitDuration, 1.0f, 1.0f);
        ImGui::Checkbox("Show Bounds", &config.showEmitterBounds);
        ImGui::Checkbox("Clear Screen Between Frames", &config.clearScreen);

        ImGui::NewLine();
        ImGui::SeparatorText("Particle");

        ImGui::InputDouble("Particle Duration", &config.particleDuration, 0.1f, 0.1f);
        ImGui::InputDouble("Particle Fade", &config.particleFade, 1.0f, 0.5f);
        ImGui::InputDouble("Particle Velocity", &config.particleVelocity, 1.0f, 1.0f);
        if((ParticleType) config.particleType == ParticleType::Pixel) {
            ImGui::Combo("Color Style", &config.pixelColorScheme, "White\0America\0\0");
        }
        if((ParticleType) config.particleType == ParticleType::Sprite) {
            ImGui::InputDouble("Particle Rotation", &config.particleRotationRate, 0.1f, 1.0f);
            ImGui::SliderInt("Rotation Magnitude", &config.particleRotationMagnitude, 0, 25);
            if(config.spriteCount > 0) {
                if(ImGui::BeginCombo("Sprite Image", config.sprites[config.selectedSprite].name)) {
                    for(i32 i = 0; i < config.spriteCount; ++i) {
                        const bool selected = (config.selectedSprite == i);
                        ImGui::PushID(i);
                        if(ImGui::Selectable(config.sprites[i].name, selected)) {
                            config.selectedSprite = i;
                        }
                        if(selected) {
                            ImGui::SetItemDefaultFocus();
                        }
                        ImGui::PopID();
                    }
                    ImGui::EndCombo();
                }
            } else {
                // Something to add a folder?
            }
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
