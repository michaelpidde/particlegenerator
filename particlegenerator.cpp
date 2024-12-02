#include <cassert>
#include <filesystem>
#include <iterator>

#include "api\particlegenerator.h"
#include "libs/SDL_image/include/SDL_image.h"

/*
 * TODO: Pass in memory block for config.sprites
 */
void LoadParticleSprites(SDL_Renderer *renderer, EmitterConfiguration *config, const char *folder) {
    try {
        const auto path = std::filesystem::path(folder);
        const auto dirItr = std::filesystem::directory_iterator(path);
        for(const auto &entry : dirItr) ++config->spriteCount;
        config->sprites = (SpriteAsset *) malloc(sizeof(SpriteAsset) * config->spriteCount);
        SpriteAsset *currentSprite = config->sprites;

        const auto dirItr2 = std::filesystem::directory_iterator(path);
        for(const auto &entry : dirItr2) {
            SDL_Surface *loadingSurface = IMG_Load(entry.path().string().c_str());
            assert(loadingSurface != NULL);
            currentSprite->texture = SDL_CreateTextureFromSurface(renderer, loadingSurface);
            assert(currentSprite->texture != NULL);
            SDL_FreeSurface(loadingSurface);
            SDL_QueryTexture(currentSprite->texture, NULL, NULL, &currentSprite->width, &currentSprite->height);
            strcpy_s(currentSprite->name, 255, entry.path().string().c_str());
            ++currentSprite;
        }
    } catch (std::exception &e) {
        printf("Error loading particle sprite folder (%s): %s\n", folder, e.what());
    }
}