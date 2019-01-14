#include <iostream>
// #include <sstream>
#include <cstdlib>
// #include <ctime>
// #include <cmath>
// #include <algorithm>
// #include <vector>
#include <SDL.h>
#include <SDL_timer.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

extern SDL_Window* window;
extern SDL_Renderer* renderer;
TTF_Font* font;

const int SCREEN_WIDTH = 100;
const int SCREEN_HEIGHT = 100;

void logSDLError(std::ostream &os, const std::string &msg) {
    os << msg << " SDL Error: " << SDL_GetError() << std::endl;
}

SDL_Color WHITE = {255,255,255,255};
SDL_Color RED = {255,0,0,255};

/* TIMER CLASS */
class Timer {
    public:
        Timer();
        ~Timer();
        void start();
        void stop();
        void pause();
        void unpause();
        int get_ticks();
        bool is_started();
        bool is_paused();
    private:
        int start_ticks;
        int paused_ticks;
        bool paused;
        bool started;
};

Timer::Timer() {
    start_ticks = 0;
    paused_ticks = 0;
    paused = false;
    started = false;
}

Timer::~Timer() {}

void Timer::start() {
    started = true;
    paused = false;
    start_ticks = SDL_GetTicks();
    paused_ticks = 0;
}

void Timer::stop() {
    started = false;
    paused = false;
    start_ticks = 0;
    paused_ticks = 0;
}

void Timer::pause() {
    if (started && !paused) {
        paused = true;
        paused_ticks = SDL_GetTicks() - start_ticks;
        start_ticks = 0;
    }
}

void Timer::unpause() {
    if (started && paused) {
        paused = false;
        start_ticks = SDL_GetTicks() - paused_ticks;
        paused_ticks = 0;
    }
}

int Timer::get_ticks() {
    int time = 0; //stopped time
    if (started) {
        if (paused) {
            time = paused_ticks; //time when paused
        } else {
            time = SDL_GetTicks() - start_ticks;  //delta from start to now
        }
    }
    return time;
}

bool Timer::is_started() {
    return started;
}

bool Timer::is_paused() {
    return paused && started;
}

/* TEXTURE CLASSES */
class Texture {
    public:
        Texture();
        ~Texture();
        bool load_from_file(std::string path);
        bool load_from_rendered_text(std::string text, SDL_Color color = WHITE);
        void free();
        void set_color(int r, int g, int b); //Uint8
        void set_blend_mode(SDL_BlendMode blending);
        void set_alpha(int a); //Uint8
        void render(int x, int y, SDL_Rect* clip = NULL, double angle = 0.0, SDL_Point* center = nullptr, SDL_RendererFlip flip = SDL_FLIP_NONE);
        int get_width();
        int get_height();
    protected:
        friend class Tilemap;
        SDL_Texture* texture;
        std::string file_path;
        int width;
        int height;
        int gid_offset;
        int tilewidth;
        int tileheight;
};

Texture::Texture() {
    texture = NULL;
    width = 0;
    height = 0;
    tilewidth = 0;
    tileheight = 0;
    gid_offset = 0;
}

Texture::~Texture() {
    free();
}

void Texture::free() {
    if (texture != NULL) {
        SDL_DestroyTexture(texture);
        texture = NULL;
        width = 0;
        height = 0;
    }
}

bool Texture::load_from_file(std::string path) {
    free();
    file_path = path;
    SDL_Texture* ntexture = NULL;
    SDL_Surface* lsurface = IMG_Load(path.c_str());
    if (lsurface == nullptr){ //could also load to surface, then optimize with SDL_ConvertSurface?
        logSDLError(std::cout, "IMG_Load");
    } else {
        //Color key image
        // SDL_SetColorKey(lsurface, SDL_TRUE, SDL_MapRGB(lsurface->format, 0x00, 0xFF, 0xFF));
        //LazyFoo12, renderer color modulation (filter with color)
        //LazyFoo13, renderer alpha blending
        //Create texture from surface
        ntexture = SDL_CreateTextureFromSurface(renderer, lsurface);
        if (ntexture == nullptr) {
            logSDLError(std::cout, "CreateTexture");
        } else {
            width = lsurface->w;
            height = lsurface->h;
        }
        SDL_FreeSurface(lsurface);
    }
    texture = ntexture;
    return texture != NULL;
}

bool Texture::load_from_rendered_text(std::string text, SDL_Color color) {
    free();
    SDL_Surface* surf = TTF_RenderText_Blended(font, text.c_str(), color);
    if (surf == nullptr){
        logSDLError(std::cout, "TTF_RenderText");
    } else {
        texture = SDL_CreateTextureFromSurface(renderer, surf);
        if (texture == nullptr){
            logSDLError(std::cout, "CreateTexture");
        } else {
            width = surf->w;
            height = surf->h;
        }
        SDL_FreeSurface(surf);
    }
    return texture != NULL;
}

void Texture::set_color(int r, int g, int b) {
    SDL_SetTextureColorMod(texture, r, g, b);
}

void Texture::set_blend_mode(SDL_BlendMode blending) {
    SDL_SetTextureBlendMode(texture, blending);
}

void Texture::set_alpha(int a) {
    SDL_SetTextureAlphaMod(texture, a);
}

void Texture::render(int x, int y, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip) {
    SDL_Rect ren_quad = {x, y, width, height};
    if (clip != NULL) {
        ren_quad.w = clip->w;
        ren_quad.h = clip->h;
    }
    SDL_RenderCopyEx(renderer, texture, clip, &ren_quad, angle, center, flip);
}

int Texture::get_width() {
    return width;
}

int Texture::get_height() {
    return height;
}


class RawTexture: public Texture {
    public:
        RawTexture();
        ~RawTexture();
        bool initialize(int w, int h);
        bool lock_texture();
        bool unlock_texture();
        bool set(int x, int y, SDL_Color color);
        void* get_pixels();
        int get_pitch();
    protected:
        void* pixels;
        int pitch;
        int format;
        SDL_PixelFormat* mapping_format;
};

RawTexture::RawTexture() {
    pixels = NULL;
    pitch = 0;
}

RawTexture::~RawTexture() {
    pixels = NULL;
    pitch = 0;
    format = 0;
    SDL_FreeFormat(mapping_format);
}

bool RawTexture::initialize(int w, int h) {
    free();
    SDL_Texture* ntexture = NULL;
    int bitdepth = 32;
    SDL_Surface* bsurface = SDL_CreateRGBSurfaceWithFormat(0, w, h, bitdepth, SDL_PIXELFORMAT_RGBA32);
    if (bsurface == nullptr) {
        logSDLError(std::cout, "CreateRGBSurfaceWithFormat");
    } else {
        SDL_Surface* fsurface = SDL_ConvertSurfaceFormat(bsurface, SDL_GetWindowPixelFormat(window), 0);
        if (fsurface == nullptr) {
            logSDLError(std::cout, "ConvertSurfaceFormat");
        } else {
            format = SDL_GetWindowPixelFormat(window);
            mapping_format = SDL_AllocFormat(format);
            ntexture = SDL_CreateTexture(renderer, format, SDL_TEXTUREACCESS_STREAMING, fsurface->w, fsurface->h);
            if (ntexture == nullptr) {
                logSDLError(std::cout, "CreateTexture");
            } else {
                width = fsurface->w;
                height = fsurface->h;
            }
            SDL_FreeSurface(fsurface);
        }
        SDL_FreeSurface(bsurface);
    }
    texture = ntexture;
    return texture != NULL;
}

bool RawTexture::lock_texture() {
    bool success = true;
    if (pixels != NULL) {
        std::cout << "Texture is already locked!" << std::endl;
        success = false;
    } else {
        if (SDL_LockTexture(texture, NULL, &pixels, &pitch) != 0) {
            logSDLError(std::cout, "Unable to lock texture");
            success = false;
        }
    }
    return success;
}

bool RawTexture::unlock_texture() {
    bool success = true;
    if (pixels == NULL) {
        std::cout << "Texture is not locked!" << std::endl;
        success = false;
    } else {
        SDL_UnlockTexture(texture);
        pixels = NULL;
        pitch = 0;
    }
    return success;
}

bool RawTexture::set(int x, int y, SDL_Color color) {
    if (x < 0 || y < 0 || x > width || y > height) {
        return false;
    }
    int* _pixels = (int*) get_pixels();
    // Setup the correct pixel bits to write to the texture
    int pixel_color = SDL_MapRGBA(mapping_format, color.r, color.g, color.b, color.a);
    // Set the pixel!
    _pixels[x+y*width] = pixel_color;
    return true;
}

void* RawTexture::get_pixels() {
    return pixels;
}

int RawTexture::get_pitch() {
    return pitch;
}
