#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <cassert>
#include <cmath>

#include <SDL.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

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

bool load_texture(const std::string filename, std::vector<uint32_t> &texture, size_t &text_size, size_t &text_cnt) {
    int nchannels = -1, w, h;
    unsigned char *pixmap = stbi_load(filename.c_str(), &w, &h, &nchannels, 0);
    if (!pixmap) {
        std::cerr << "Error: can not load the textures" << std::endl;
        return false;
    }

    if (4!=nchannels) {
        std::cerr << "Error: the texture must be a 32 bit image" << std::endl;
        stbi_image_free(pixmap);
        return false;
    }

    text_cnt = w/h;
    text_size = w/text_cnt;
    if (w!=h*int(text_cnt)) {
        std::cerr << "Error: the texture file must contain N square textures packed horizontally" << std::endl;
        stbi_image_free(pixmap);
        return false;
    }

    texture = std::vector<uint32_t>(w*h);
    for (int j=0; j<h; j++) {
        for (int i=0; i<w; i++) {
            uint8_t r = pixmap[(i+j*w)*4+0];
            uint8_t g = pixmap[(i+j*w)*4+1];
            uint8_t b = pixmap[(i+j*w)*4+2];
            uint8_t a = pixmap[(i+j*w)*4+3];
            texture[i+j*w] = pack_color(r, g, b, a);
        }
    }
    stbi_image_free(pixmap);
    return true;
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

const uint32_t white = pack_color(255, 255, 255);
const uint32_t black = pack_color(0, 0, 0);
const uint32_t gray = pack_color(160, 160, 160);
const uint32_t red = pack_color(190, 83, 85); //2

struct Sprite {
    float x, y;
    size_t texture_id;
};

void drawSprites(const size_t win_w, const size_t win_h, std::vector<uint32_t> &framebuffer, const std::vector<Sprite> &sprites, const size_t map_w, const size_t map_h) {
    for (size_t i=0; i < sprites.size(); i++) {
        draw_rectangle(framebuffer, win_w, win_h, red, (sprites[i].x * map_w)-3, (sprites[i].y * map_h)-3, 6, 6);
    }
}

void drawMap(const size_t win_w, const size_t win_h, std::vector<uint32_t> &framebuffer, const std::vector<uint32_t> &wall_textures, size_t wall_texture_size, const size_t map_w, const size_t map_h, const char *map, const size_t rect_w, const size_t rect_h) {// draw the map
    for (size_t j = 0; j < map_h; j++) {
        for (size_t i = 0; i <map_w; i++) {
            if (map[i+j*map_w] == ' ') continue;
            size_t rect_x = i * rect_w;
            size_t rect_y = j * rect_h;
//            size_t current_color = getMapColor(map[i+j*map_w]);
//            draw_rectangle(framebuffer, win_w, win_h, current_color, rect_x, rect_y, rect_w, rect_h);
            size_t texture_id = int(map[i+j*map_w] - '0');
            draw_rectangle(framebuffer, win_w, win_h, wall_textures[texture_id*wall_texture_size], rect_x, rect_y, rect_w, rect_h);
        }
    }
}

std::vector<uint32_t> getTextureColumn(const std::vector<uint32_t> &wall_textures, size_t wall_texture_size, size_t wall_texture_count, size_t texture_id, int texture_x_coord, size_t column_height) {
    const size_t texture_w = wall_texture_size*wall_texture_count;
    const size_t texture_h = wall_texture_size;
    std::vector<uint32_t> column(column_height);
    for (size_t y = 0; y < column_height; y++) {
        size_t px = texture_id * wall_texture_size + texture_x_coord;
        size_t py = (y * wall_texture_size) / column_height;
        column[y] = wall_textures[px + py * texture_w];
    }
    return column;
}

void drawConeAndProjection(const size_t win_w, const size_t win_h, std::vector<uint32_t> &framebuffer, const std::vector<uint32_t> &wall_textures, size_t wall_texture_size, size_t wall_texture_count, const size_t map_w, const char *map, float player_x, float player_y, float player_a, const float fov, const size_t rect_w, const size_t rect_h) {
    for (size_t i = 0; i < win_w / 2; i++) { // sweep to have 1 ray for each column of the view image
        float angle = player_a - fov / 2 + fov * i / float(win_w/2); // calculate the line of sweeping the fov cone by calculating the new angle in radians
        for ( float c = 0; c < 20; c += .05) {
            float cx = player_x + c * cos(angle);
            float cy = player_y + c * sin(angle);
            int px = cx*rect_w;
            int py = cy*rect_h;
            framebuffer[px + py*win_w] = gray; // draw the cone
            if (map[int(cx) + int(cy) * map_w] != ' ') {
                size_t texture_id = int(map[int(cx) + int(cy) * map_w] - '0');
                size_t column_height = win_h/(c*cos(angle-player_a)); // full height (win_h) * size of column (1/c) to get proportional size of column

                float hit_x = cx - floor(cx + .5);  // these are the fractional parts of where we hit the wall
                float hit_y = cy - floor(cy + .5);  // they dictate how far away from the intersection of the map gridlines we are, and therefore how far into the texture
                int texture_coord_x = hit_x * wall_texture_size;
                if (std::abs(hit_y) > std::abs(hit_x)) { // are we a "vertical" or "horizontal" wall wrt the map tile...i.e. we may be sliding along the edge and need to ID the correct texture to use
                    texture_coord_x = hit_y * wall_texture_size;
                }
                if (texture_coord_x < 0) texture_coord_x += wall_texture_size; //this can go negative...fix it

                std::vector<uint32_t> column = getTextureColumn(wall_textures, wall_texture_size, wall_texture_count, texture_id, texture_coord_x, column_height);

                px = win_w/2+i;
                for (size_t j=0; j < column_height; j++) {
                    py = j + win_h/2-column_height/2;
                    if (py < 0 || py >= (int) win_h) continue;
                    framebuffer[px + py * win_w] = column[j];
                }
                break;
            }
        }
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
            bool rotate = true;

            const size_t win_w = SCREEN_WIDTH; // image width
            const size_t win_h = SCREEN_HEIGHT; // image height


            const size_t map_w = 16;
            const size_t map_h = 16;
            const char map[] =  "0002222222220000"\
                                "1              0"\
                                "1      11111   0"\
                                "1     0        0"\
                                "0     0  1110000"\
                                "0     3        0"\
                                "0   10000      0"\
                                "0   3   11100  0"\
                                "5   4   0      0"\
                                "5   4   1  00000"\
                                "0       1      0"\
                                "2       1      0"\
                                "0       0      0"\
                                "0 0000000      0"\
                                "0              0"\
                                "0002222222200000";

            float player_x = 3.456;
            float player_y = 2.345;
            float player_a = 1.523;
            const float fov = M_PI / 3.0; //60 deg field of view (pi/3 rad)
            const float fov_degree = 2*M_PI / 360.0;

            std::vector<Sprite> sprites{ {1.834, 8.765, 0}, {5.323, 5.365, 1}, {4.123, 10.265, 1} };

            const size_t rect_w = win_w / (map_w*2);
            const size_t rect_h = win_h / map_h;

            std::vector<uint32_t> wall_textures;
            size_t wall_texture_size;
            size_t wall_texture_count;

            if (!load_texture("./res/walltextures.png", wall_textures, wall_texture_size, wall_texture_count)) {
                std::cerr << "Failed to load wall textures" << std::endl;
            }

            std::vector<uint32_t> framebuffer(win_w*win_h, white); // the image itself, initialized to white
            framebuffer_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);

            int frame_delay = 5;

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
                            case SDLK_1:
                                rotate = !rotate;
                                break;
                        }
                    }
                }

                if (rotate) {
                    if (frame_delay == 0) {
                        frame_delay = 5;
                        player_a += fov_degree;
                    } else {
                        frame_delay -= 1;
                    }
                }

                framebuffer = std::vector<uint32_t>(win_w*win_h, white); //clear screen

                drawMap(win_w, win_h, framebuffer, wall_textures, wall_texture_size, map_w, map_h, map, rect_w, rect_h);
                drawConeAndProjection(win_w, win_h, framebuffer, wall_textures, wall_texture_size, wall_texture_count, map_w, map, player_x, player_y, player_a, fov, rect_w, rect_h);

                drawSprites(win_w, win_h, framebuffer, sprites, rect_h, rect_w);

                SDL_UpdateTexture(framebuffer_texture, NULL, reinterpret_cast<void *>(framebuffer.data()), SCREEN_WIDTH*4);

                SDL_RenderClear(renderer);
                SDL_RenderCopy(renderer, framebuffer_texture, NULL, NULL);
                SDL_RenderPresent(renderer);
            }
        }
    }
    close();
    return 0;
}
