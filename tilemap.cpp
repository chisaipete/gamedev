#include "leaper.h"

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
Tilemap tilemap;
// Texture texture;

bool init() {
    bool success = true;

    if (SDL_Init(SDL_INIT_VIDEO) != 0){
        logSDLError(std::cout, "SDL_Init");
        success = false;
    } else {       
        if (TTF_Init() != 0){  //Future: do we need to call IMG_Init, it seems to work without it...is speed a factor? IMG_GetError might be needed
            logSDLError(std::cout, "TTF_Init");
            success = false;
        } else {
            window = SDL_CreateWindow("Tilemap Render Test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 8*32, 8*32, SDL_WINDOW_SHOWN);
            if (window == nullptr){
                logSDLError(std::cout, "SDL_CreateWindow");
                success = false;
            } else {
                renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
                if (renderer == nullptr){
                    logSDLError(std::cout, "SDL_CreateRenderer");
                    success = false;
                }
            }
        }
    }

    return success;
}

bool load() {
    bool success = true;

    if (!tilemap.load_from_file("res/testmap.json")) {
        success = false;
    }

    // if (!texture.load_from_file("res/pacman.png")) {
    //     success = false;
    // }

    return success;
}

bool close() {
    if (font != nullptr) { TTF_CloseFont(font);font = NULL; }
    if (renderer != nullptr) { SDL_DestroyRenderer(renderer); renderer = NULL; }
    if (window != nullptr) { SDL_DestroyWindow(window); window = NULL; }
    TTF_Quit();
    SDL_Quit();
    return true;
}

int main(int argc, char **argv) {
    if (!init()) {
        std::cout << "Initialization Failed" << std::endl;
    } else {
        if (!load()) {
            std::cout << "Loading Failed" << std::endl;
        } else {
            bool quit = false;

            while (!quit) {
                SDL_Event event;

                while (SDL_PollEvent(&event)) {
                    if (event.type == SDL_QUIT) {
                            quit = true;
                    }
                }

                SDL_SetRenderDrawColor(renderer, 0x0, 0x0, 0x0, 0xFF); //black
                SDL_RenderClear(renderer);
                // render
                // tilemap.debug();
                // texture.render(0,0);
                tilemap.render_layers();
                SDL_RenderPresent(renderer);
            }
        }
    }
    close();
    return 0;
}
