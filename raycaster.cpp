#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <cassert>
#include <cmath>

#include <SDL.h>

const uint32_t SCREEN_WIDTH = 1024;
const uint32_t SCREEN_HEIGHT = 512;

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
SDL_Texture *framebuffer_texture = NULL;

void logSDLError(std::ostream &os, const std::string &msg) {
    os << msg << " SDL Error: " << SDL_GetError() << std::endl;
}

bool init() {
    bool success = true;

    if (SDL_Init(SDL_INIT_VIDEO) != 0){
        logSDLError(std::cout, "SDL_Init");
        success = false;
    } else {
        window = SDL_CreateWindow("Leaper", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
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
    return success;
}

bool load() {
    bool success = true;
    return success;
}

bool close() {
    if (framebuffer_texture != nullptr) {SDL_DestroyTexture(framebuffer_texture); framebuffer_texture = NULL; }
    if (renderer != nullptr) { SDL_DestroyRenderer(renderer); renderer = NULL; }
    if (window != nullptr) { SDL_DestroyWindow(window); window = NULL; }
    SDL_Quit();
    return true;
}

// Initial code from @ssloy on github

uint32_t pack_color(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a=255) {
    return (a << 24u) + (b << 16u) + (g << 8u) + r;
}

void unpack_color(const uint32_t &color, uint8_t &r, uint8_t &g, uint8_t &b, uint8_t &a) {
    r = (color >>  0u) & 255u;
    g = (color >>  8u) & 255u;
    b = (color >> 16u) & 255u;
    a = (color >> 24u) & 255u;
}

void draw_rectangle(std::vector<uint32_t> &frame, const size_t i_w, const size_t i_h, const uint32_t color, const size_t x, const size_t y, const size_t w, const size_t h) {
    assert(frame.size() == i_w*i_h);
    for (size_t i = x; i < x + w; i++) {
        for (size_t j = y; j < y + h; j++) {
            assert(i<i_w && j<i_h);
            frame[i+j*i_w] = color;
        }
    }
}

void drop_ppm_image(const std::string &filename, const std::vector<uint32_t> &image, const size_t w, const size_t h) {
    assert(image.size() == w*h);
    std::ofstream ofs(filename, std::ios::binary);
    ofs << "P6" << std::endl << w << " " << h << std::endl << "255" << std::endl;
    for (size_t i = 0; i < h*w; ++i) {
        uint8_t r, g, b, a;
        unpack_color(image[i], r, g, b, a);
        ofs << static_cast<char>(r) << static_cast<char>(g) << static_cast<char>(b);
    }
    ofs.close();
}

const uint32_t white = pack_color(255, 255, 255);
const uint32_t black = pack_color(0, 0, 0);
const uint32_t gray = pack_color(160, 160, 160);
const uint32_t warm_gray = pack_color(162, 151, 163); //0
const uint32_t off_white = pack_color(249, 252, 241); //1
const uint32_t red = pack_color(190, 83, 85); //2
const uint32_t blue = pack_color(82, 107, 121);

uint32_t getMapColor(const char m) {
    switch (m) {
        case '0':
            return warm_gray;
        case '1':
            return off_white;
        case '2':
            return red;
        case '3':
            return blue;
        default:
            return white;
    }
}

int main(int argc, char **argv) {
    if (!init()) {
        std::cout << "Initialization Failed" << std::endl;
    } else {
        if (!load()) {
            std::cout << "Loading Failed" << std::endl;
        } else {
            bool quit = false;

            const size_t win_w = SCREEN_WIDTH; // image width
            const size_t win_h = SCREEN_HEIGHT; // image height

            std::vector<uint32_t> framebuffer(win_w*win_h, white); // the image itself, initialized to white

            const size_t map_w = 16;
            const size_t map_h = 16;
            const char map[] =  "0000011111110000"\
                                "2              0"\
                                "2      22222   0"\
                                "2     0        0"\
                                "0     0  2220000"\
                                "0     3        0"\
                                "0   20000      0"\
                                "0   0   22200  0"\
                                "0   0   0      0"\
                                "0   0   2  00000"\
                                "0       2      0"\
                                "1       2      0"\
                                "0       0      0"\
                                "0 0000000      0"\
                                "0              0"\
                                "0001111111100000";

            float player_x = 3.456;
            float player_y = 2.345;
            float player_a = 1.523;
            const float fov = M_PI / 3.0; //60 deg field of view (pi/3 rad)

            const size_t rect_w = win_w / (map_w*2);
            const size_t rect_h = win_h / map_h;

            uint32_t current_color = black;

            framebuffer_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);

            while (!quit) {
                SDL_Event event;
                while (SDL_PollEvent(&event) != 0) {
                    if (event.type == SDL_QUIT) {
                        quit = true;
                    }
                    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
                        quit = true;
                    }
                    if (event.type == SDL_KEYDOWN && event.key.repeat == 0) {
                        switch (event.key.keysym.sym) {
//                            case SDLK_1:
//                                fps_on = !fps_on;
//                                break;
                        }
                    }
                }

                // draw the map
                for (size_t j = 0; j < map_h; j++) {
                    for (size_t i = 0; i <map_w; i++) {
                        if (map[i+j*map_w] == ' ') continue;
                        size_t rect_x = i * rect_w;
                        size_t rect_y = j * rect_h;
                        current_color = getMapColor(map[i+j*map_w]);
                        draw_rectangle(framebuffer, win_w, win_h, current_color, rect_x, rect_y, rect_w, rect_h);
                    }
                }

                // draw player
//                const size_t player_w = 5;
//                const size_t player_h = 5;
//                draw_rectangle(framebuffer, win_w, win_h, black, player_x*rect_w, player_y*rect_h, player_w, player_h);
//
                // draw sight line
//                for (float c = 0; c < 20; c += .05) {
//                    float x = player_x + c * cos(player_a);
//                    float y = player_y + c * sin(player_a);
//                    if (map[int(x) + int(y) * map_w] != ' ') break;
//                    size_t px = x*rect_w;
//                    size_t py = y*rect_h;
//                    framebuffer[px + py*win_w] = white;
//                }

                // draw sight cone & projection view
                for (size_t i = 0; i < win_w/2; i++) { // sweep to have 1 ray for each column of the view image
                    float angle = player_a - fov / 2 + fov * i / float(win_w/2); // calculate the line of sweeping the fov cone by calculating the new angle in radians
                    for ( float c = 0; c < 20; c += .05) {
                        float cx = player_x + c * cos(angle);
                        float cy = player_y + c * sin(angle);
                        size_t px = cx*rect_w;
                        size_t py = cy*rect_h;
                        framebuffer[px + py*win_w] = gray; // draw the cone
                        if (map[int(cx) + int(cy) * map_w] != ' ') {
                            size_t column_height = win_h/(c*cos(angle-player_a)); // full height (win_h) * size of column (1/c) to get proportional size of column
                            draw_rectangle(framebuffer, win_w, win_h, getMapColor(map[int(cx) + int(cy) * map_w]), win_w/2+i, win_h/2-column_height/2, 1, column_height);
                            break;
                        }
                    }
                }

                SDL_UpdateTexture(framebuffer_texture, NULL, reinterpret_cast<void *>(framebuffer.data()), SCREEN_WIDTH*4);

                SDL_RenderClear(renderer);
                SDL_RenderCopy(renderer, framebuffer_texture, NULL, NULL);
                SDL_RenderPresent(renderer);
            }
//            drop_ppm_image("./out.ppm", framebuffer, win_w, win_h);
        }
    }
    close();
    return 0;
}
