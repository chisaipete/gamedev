#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <cassert>
#include <cmath>

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
    assert(frame.size() == i_w * i_h);
    for (size_t i = x; i < x + w; i++) {
        for (size_t j = y; j < y + h; j++) {
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

int main() {
    const size_t win_w = 512; // image width
    const size_t win_h = 512; // image height

    std::vector<uint32_t> framebuffer(win_w*win_h, 255); // the image itself, initialized to red

    for (size_t j = 0; j < win_h; j++) { // fill the screen with color gradients
        for (size_t i = 0; i < win_w; i++) {
            uint8_t r = float(255) * j / float(win_h); // varies between 0 and 255 as j sweeps the vertical
            uint8_t g = float(255) * i / float(win_w); // varies between 0 and 255 as i sweeps the horizontal
            uint8_t b = 0;
            framebuffer[i+j*win_w] = pack_color(r, g, b);
        }
    }

    const size_t map_w = 16;
    const size_t map_h = 16;

    const char map[] =  "1111111111111111"\
                        "1              1"\
                        "1      11111   1"\
                        "1     1        1"\
                        "1     1  1111111"\
                        "1     1        1"\
                        "1   11111      1"\
                        "1   1   11111  1"\
                        "1   1   1      1"\
                        "1   1   1  11111"\
                        "1       1      1"\
                        "1       1      1"\
                        "1       1      1"\
                        "1 1111111      1"\
                        "1              1"\
                        "1111111111111111";

    const size_t rect_w = win_w / map_w;
    const size_t rect_h = win_h / map_h;

    const uint32_t cyan = pack_color(0, 255, 255);

    for (size_t j = 0; j < map_h; j++) {
        for (size_t i = 0; i <map_w; i++) {
            if (map[i+j*map_w] != ' ') {
                size_t rect_x = i * rect_w;
                size_t rect_y = j * rect_h;
                draw_rectangle(framebuffer, win_w, win_h, cyan, rect_x, rect_y, rect_w, rect_h);
            }
        }
    }

    float player_x = 3.456;
    float player_y = 2.345;
    const size_t player_w = 5;
    const size_t player_h = 5;
    const uint32_t white = pack_color(255, 255, 255);

    draw_rectangle(framebuffer, win_w, win_h, white, player_x*rect_w, player_y*rect_h, player_w, player_h);

    float player_a = 1.523;

//    draw sight line
//    for (float c = 0; c < 20; c += .05) {
//        float x = player_x + c * cos(player_a);
//        float y = player_y + c * sin(player_a);
//        if (map[int(x) + int(y) * map_w] != ' ') break;
//        size_t px = x*rect_w;
//        size_t py = y*rect_h;
//        framebuffer[px + py*win_w] = white;
//    }

    const float fov = M_PI / 3.0; //60 deg field of view

//    draw sight cone
    for (size_t i = 0; i < win_w; i++) { // sweep to have 1 ray for each column of the view image
        float angle = player_a - fov / 2 + fov * i / float(win_w); // calculate the line of sweeping the fov cone by calculating the new angle in radians
        for ( float c = 0; c < 20; c += .05) {
            float cx = player_x + c * cos(angle);
            float cy = player_y + c * sin(angle);
            if (map[int(cx) + int(cy) * map_w] != ' ') break;
            size_t px = cx*rect_w;
            size_t py = cy*rect_h;
            framebuffer[px + py*win_w] = white;
        }
    }

    drop_ppm_image("./out.ppm", framebuffer, win_w, win_h);

    return 0;
}
