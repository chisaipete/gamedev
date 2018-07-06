#include <iostream>
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <algorithm>
#include <vector>
#include <SDL.h>
#include <SDL_timer.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
//Tiled Map support
#define CUTE_TILED_IMPLEMENTATION
#include "cute_tiled.h"

extern SDL_Window* window;
extern SDL_Renderer* renderer;
TTF_Font* font;

const float PI = 3.141592653589793238463;

const int SCREEN_FPS = 60;
const int SCREEN_TICKS_PER_FRAME = 1000 / SCREEN_FPS;
const int SCREEN_WIDTH = 480;
const int SCREEN_HEIGHT = 640;

const int LEVEL_WIDTH = SCREEN_WIDTH;
// const int LEVEL_HEIGHT = (SCREEN_HEIGHT*10);
const int LEVEL_HEIGHT = (SCREEN_HEIGHT);
const int TILE_SIZE = 32;
const int LEVEL_TILE_WIDTH = LEVEL_WIDTH/TILE_SIZE;
const int LEVEL_TILE_HEIGHT = LEVEL_HEIGHT/TILE_SIZE;

const int WALK_FRAMES = 3;
//tilemap indexes
const int EMPTY  = -1;
const int STAND0 =  0;
const int STAND1 =  1;
const int WALK0  =  2;
const int WALK1  =  3;
const int WALK2  =  4;
const int JUMP   =  5;
const int HIT    =  6;

const int GRASS0  =  0;
const int GRASS1  =  1;
const int GRASS2  =  2;
const int GROUND0 =  3;
const int GROUND1 =  4;
const int GROUND2 =  5;
const int GRASS_LEDGE  =  6;
const int STONE_LEDGE  =  7;

//game state FSM
#define START   (0)
#define PAUSE   (1)
#define PLAY    (2)
#define OVER    (3)

SDL_Color WHITE = {255,255,255,255};
SDL_Color RED_ = {255,0,0,255};

unsigned long long rdtsc(){
    unsigned int lo,hi;
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((unsigned long long)hi << 32) | lo;
}

void logSDLError(std::ostream &os, const std::string &msg) {
    os << msg << " SDL Error: " << SDL_GetError() << std::endl;
}

std::ostream &operator<<(std::ostream &os, SDL_Rect const &R) { 
    return os << "(x:"<< R.x << " y:" << R.y << " w:" << R.w << " h:" << R.h << ")";
}

/* VECTOR & CIRCLE */

struct Circle {
    int x, y;
    int r;
};

void DrawCircle(SDL_Renderer *Renderer, Circle &c)
{  //midpoint algorithm
   int x = c.r - 1;
   int y = 0;
   int tx = 1;
   int ty = 1;
   int err = tx - (c.r << 1);    // shifting bits left by 1 effectively
                                 // doubles the value. == tx - diameter
   while (x >= y) 
   {
      //  Each of the following renders an octant of the circle
      SDL_RenderDrawPoint(Renderer, c.x + x, c.y - y);
      SDL_RenderDrawPoint(Renderer, c.x + x, c.y + y);
      SDL_RenderDrawPoint(Renderer, c.x - x, c.y - y);
      SDL_RenderDrawPoint(Renderer, c.x - x, c.y + y);
      SDL_RenderDrawPoint(Renderer, c.x + y, c.y - x);
      SDL_RenderDrawPoint(Renderer, c.x + y, c.y + x);
      SDL_RenderDrawPoint(Renderer, c.x - y, c.y - x);
      SDL_RenderDrawPoint(Renderer, c.x - y, c.y + x);

      if (err <= 0)
      {
         y++;
         err += ty;
         ty += 2;
      }
      else if (err > 0)
      {
         x--;
         tx += 2;
         err += tx - (c.r << 1);
      }
   }
}

struct v2 {
    float x, y;
};

std::ostream &operator<<(std::ostream &os, v2 const &A) { 
    return os << "(" << A.x << "," << A.y << ")";
}

// convert pair of ints to v2
v2 V2(float A, float B) {
    v2 r;
    r.x = A;
    r.y = B;
    return r;
};

bool operator==(v2 A, v2 B) {
    if (A.x == B.x && A.y == B.y)
        return true;
    return false;
};

v2 operator-(v2 A) {
    v2 r;
    r.x = -A.x;
    r.y = -A.y;
    return r;
};

v2 operator*(float A, v2 B) {
    v2 r;
    r.x = A*B.x;
    r.y = A*B.y;
    return r;
};

v2 operator*(v2 A, float B) {
    v2 r;
    r.x = B*A.x;
    r.y = B*A.y;
    return r;
};

v2 operator*(v2 A, v2 B) {
    v2 r;
    r.x = A.x*B.x;
    r.y = A.y*B.y;
    return r;
};

v2 operator+(v2 A, v2 B) {
    v2 r;
    r.x = A.x+B.x;
    r.y = A.y+B.y;
    return r;
};

v2 operator+=(v2 A, v2 B) {
    v2 r;
    r.x = A.x+B.x;
    r.y = A.y+B.y;
    return r;
};

v2 operator-(v2 A, v2 B) {
    v2 r;
    r.x = A.x-B.x;
    r.y = A.y-B.y;
    return r;
};

/* CAMERA CLASS*/
// class Camera {
//     public:
//         Camera();
//         ~Camera();
//     private:
//         v2 position;
//         SDL_Rect* rect;
// }

float dist_sqr(Circle A, Circle B) {
    int dx = B.x - A.x;
    int dy = B.y - A.y;
    return dx*dx + dy*dy;
}

float dist_sqr(v2 A, v2 B) {
    int dx = B.x - A.x;
    int dy = B.y - A.y;
    return dx*dx + dy*dy;
}

float dist_sqr(Circle A, v2 B) {
    int dx = B.x - A.x;
    int dy = B.y - A.y;
    return dx*dx + dy*dy;
}

v2 rect_center(SDL_Rect a) {
    float x = a.x + (a.w/2.0);
    float y = a.y + (a.h/2.0);
    return V2(x,y);
}

v2 screen_to_tile(v2 position) {
    return V2(int(position.x/TILE_SIZE), int(position.y/TILE_SIZE));
}

/* COLLISION DETECTION */
const unsigned NOHIT =  0b000001111;
const unsigned HORIZ =  0b000000011;
const unsigned LEFT =   0b000000001;
const unsigned RIGHT =  0b000000010;
const unsigned VERT =   0b000001100;
const unsigned BOTTOM = 0b000000100;
const unsigned TOP =    0b000001000;

unsigned check_collision(SDL_Rect a, SDL_Rect b) {
    unsigned collision_mask = 0b0000;

    v2 ca = rect_center(a);
    v2 cb = rect_center(b);
    float w = 0.5 * (a.w + b.w);
    float h = 0.5 * (a.h + b.h);
    float dx = ca.x - cb.x;
    float dy = ca.y - cb.y;

    //check for collisions!
    if (abs(dx) <= w && abs(dy) <= h) {
        //collision
        float wy = w * dy;
        float hx = h * dx;

        if (wy > hx) {
            if (wy > -hx) {
                // top
                collision_mask |= TOP;
            } else {
                // left
                collision_mask |= LEFT;
            }
        }  else {
            if (wy > -hx) {
                // right
                collision_mask |= RIGHT;
            } else {
                // bottom
                collision_mask |= BOTTOM;
            }
        }
    }
    
    return collision_mask;
}

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
        SDL_Texture* texture;
        int width;
        int height;
};

Texture::Texture() {
    texture = NULL;
    width = 0;
    height = 0;
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
        void* get_pixels();
        int get_pitch();
    protected:
        void* pixels;
        int pitch;
};

RawTexture::RawTexture() {
    pixels = NULL;
    pitch = 0;
}

RawTexture::~RawTexture() {
    pixels = NULL;
    pitch = 0;
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
            ntexture = SDL_CreateTexture(renderer, SDL_GetWindowPixelFormat(window), SDL_TEXTUREACCESS_STREAMING, fsurface->w, fsurface->h);
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

void* RawTexture::get_pixels() {
    return pixels;
}

int RawTexture::get_pitch() {
    return pitch;
}
