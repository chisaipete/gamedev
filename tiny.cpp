#include "tiny.h"

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

Timer fps_timer;
std::stringstream fpsText;
Texture t_fps;

RawTexture image;
Model* model = NULL;

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 800;

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
    font = TTF_OpenFont("res/cc.ttf", 12);
    if (font == nullptr){
        logSDLError(std::cout, "TTF_OpenFont");
        success = false;
    }

    model = new Model("res/african_head.obj");

    if (!t_fps.load_from_rendered_text("_", RED)) {
        success = false;
    }
    if (!image.initialize(SCREEN_WIDTH, SCREEN_HEIGHT)) {
        success = false;
    }
    return success;
}

bool close() {
    t_fps.free();
    image.free();
    delete model;
    if (font != nullptr) { TTF_CloseFont(font); font = NULL; }
    if (renderer != nullptr) { SDL_DestroyRenderer(renderer); renderer = NULL; }
    if (window != nullptr) { SDL_DestroyWindow(window); window = NULL; }
    TTF_Quit();
    SDL_Quit();
    return true;
}

void line(int x0, int y0, int x1, int y1, RawTexture &i, SDL_Color c) {
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
    int dx = x1-x0;
    int dy = y1-y0;
    int derror2 = std::abs(dy)*2;
    int error2 = 0;
    int y = y0;
    for (int x = x0; x <= x1; x++) {
        if (steep) {
            i.set(y, x, c); //if transposed, de-transpose
        } else {
            i.set(x, y, c);
        }
        error2 += derror2;
        if (error2 > dx) {
            y += (y1 > y0 ? 1 : -1);
            error2 -= dx*2;
        }
    }
}

void render() {
    image.lock_texture();
    // image.set(52, 41, RED);

    // line(13, 20, 80, 40, image, WHITE);
    // line(20, 13, 40, 80, image, RED);
    // line(80, 40, 13, 20, image, RED);

    for (int i = 0; i < model->num_faces(); i++) {
        std::vector<int> face = model->face(i);
        for (int j = 0; j < 3; j++) {
            v3f v0 = model->vertex(face[j]);
            v3f v1 = model->vertex(face[(j+1)%3]);
            int x0 = (v0.x+1.)*SCREEN_WIDTH/2.0;
            int y0 = (v0.y+1.)*SCREEN_HEIGHT/2.0;
            int x1 = (v1.x+1.)*SCREEN_WIDTH/2.0;
            int y1 = (v1.y+1.)*SCREEN_HEIGHT/2.0;
            line(x0, y0, x1, y1, image, WHITE);
        }
    }

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
            bool fps_on = false;
            int frame_count = 0;
            float avgFPS = 0.0;

            fps_timer.start();

            while (!quit) {
                SDL_Event event;

                while (SDL_PollEvent(&event) != 0) {
                    if (event.type == SDL_QUIT) {
                            quit = true;
                    }
                    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE ) {
                        quit = true;
                    }
                    if (event.type == SDL_KEYDOWN && event.key.repeat == 0) {
                        switch (event.key.keysym.sym) {
                            case SDLK_1:
                                fps_on = !fps_on;
                                break;
                        }
                    }
                }

                render();

                SDL_SetRenderDrawColor(renderer, 0x0, 0x0, 0x0, 0xFF); //black
                SDL_RenderClear(renderer);

                image.render(0, 0, nullptr, 0, nullptr, SDL_FLIP_VERTICAL);
                
                if (fps_on) {
                    // calculate and render FPS
                    avgFPS = frame_count / (fps_timer.get_ticks() / 1000.0);
                    if (avgFPS > 2000000) {
                        avgFPS = 0;
                    }
                    fpsText.str("");
                    fpsText << (int)avgFPS << " fps";
                    t_fps.load_from_rendered_text(fpsText.str(), RED);
                    t_fps.render(SCREEN_WIDTH-t_fps.get_width(), 0);
                }

                SDL_RenderPresent(renderer);
                frame_count++;
            }
        }
    }
    close();
    return 0;
}