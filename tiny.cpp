#include "tiny.h"

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

RawTexture image;

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
            window = SDL_CreateWindow("Tiny Renderer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
            // window = SDL_CreateWindow("Leaper", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, LEVEL_WIDTH, LEVEL_HEIGHT, SDL_WINDOW_SHOWN);
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
    if (!image.initialize(SCREEN_WIDTH, SCREEN_HEIGHT)) {
        success = false;
    }
    return success;
}

bool close() {
    image.free();
    if (font != nullptr) { TTF_CloseFont(font); font = NULL; }
    if (renderer != nullptr) { SDL_DestroyRenderer(renderer); renderer = NULL; }
    if (window != nullptr) { SDL_DestroyWindow(window); window = NULL; }
    TTF_Quit();
    SDL_Quit();
    return true;
}

void line(int x0, int y0, int x1, int y1, RawTexture &i, SDL_Color c) {
    // FIRST ATTEMPT
    // for (float t = 0; t < 1; t += .01) {
    //     int x = x0*(1-t) + x1*t;
    //     int y = y0*(1-t) + y1*t;
    //     i.set(x, y, c);
    // }

    // SECOND ATTEMPT
    // for (int x = x0; x <= x1; x++) {
    //     float t = (x-x0) / (float)(x1-x0);
    //     int y = y0*(1-t) + y1*t;
    //     i.set(x, y, c);
    // }

    // THIRD ATTEMPT
    bool steep = false;
    if (std::abs(x0-x1) < std::abs(y0-y1)) { //if the line is steep, transpose
        std::swap(x0, y0);
        std::swap(x1, y1);
        steep = true;
    }
    if (x0>x1) { //make it left to right
        std::swap(x0, x1);
        std::swap(y0, y1);
    }
    for (int x = x0; x <= x1; x++) {
        float t = (x-x0) / (float)(x1-x0);
        int y = y0*(1-t) + y1*t;
        if (steep) {
            i.set(y, x, c); //if transposed, de-transpose
        } else {
            i.set(x, y, c);
        }
    }
}

void render() {
    image.lock_texture();
    // image.set(52, 41, RED);
    line(13, 20, 80, 40, image, WHITE);
    line(20, 13, 40, 80, image, RED);
    line(80, 40, 13, 20, image, RED);
    image.unlock_texture();
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

                while (SDL_PollEvent(&event) != 0) {
                    if (event.type == SDL_QUIT) {
                            quit = true;
                    }
                    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE ) {
                        quit = true;
                    }
                }

                render();

                SDL_SetRenderDrawColor(renderer, 0x0, 0x0, 0x0, 0xFF); //black
                SDL_RenderClear(renderer);

                image.render(0, 0, nullptr, 0, nullptr, SDL_FLIP_VERTICAL);

                SDL_RenderPresent(renderer);
            }
        }
    }
    close();
    return 0;
}