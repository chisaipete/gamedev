#include <iostream>
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <algorithm>
#include <vector>
#include <unordered_map>
#include <SDL.h>
#include <SDL_timer.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
//Tiled Map support
#define CUTE_TILED_IMPLEMENTATION
#include "cute_tiled.h"
#include <libgen.h>

extern SDL_Window* window;
extern SDL_Renderer* renderer;
TTF_Font* font;

const float PI = 3.141592653589793238463;

const int SCREEN_FPS = 60;
const int SCREEN_TICKS_PER_FRAME = 1000 / SCREEN_FPS;
const int SCREEN_WIDTH = 480;
const int SCREEN_HEIGHT = 640;

const int TILE_SIZE = 32;

const int LEVEL_WIDTH = SCREEN_WIDTH;
// const int LEVEL_HEIGHT = (SCREEN_HEIGHT*10);
const int LEVEL_HEIGHT = TILE_SIZE*60;
const int LEVEL_TILE_WIDTH = 15;
// const int LEVEL_TILE_HEIGHT = LEVEL_HEIGHT/TILE_SIZE;
const int LEVEL_TILE_HEIGHT = 60;

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
        void define_sprites(int number, int offset, int t_width, int t_height);
    protected:
        friend class Tilemap;
        SDL_Texture* texture;
        std::string file_path;
        int width;
        int height;
        std::vector<SDL_Rect> sprites;
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

void Texture::define_sprites(int number, int offset, int t_width, int t_height) {
    // std::cout << "Define sprites: #" << number << " gid start: " << offset << std::endl;
    sprites.reserve(number);
    gid_offset = offset;
    tilewidth = t_width;
    tileheight = t_height;
    int gid = gid_offset;
    int x = 0;
    int y = 0;
    int t = 0;
    while (t < number) {
        SDL_Rect r;
        r.x = x;
        r.y = y;
        r.w = tilewidth;
        r.h = tileheight;
        sprites.push_back(r);
        // std::cout << "  tile: " << t << " " << r << std::endl;
        x += tilewidth;
        if (x >= width) {
            x = 0;
            y += tileheight;
        }
        ++t;
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


/* TILEMAP */
class Tilemap {
    public:
        Tilemap();
        ~Tilemap();
        void free();
        bool load_from_file(std::string path);
        void render_layers(SDL_Rect camera);
        void debug();
        int tile_gid(int position);
        SDL_Rect get_collider(int gid, int x, int y);
    protected:
        std::string map_path;
        cute_tiled_map_t* map;
        std::vector<Texture> textures;
        std::unordered_map<int,int> gid_to_texture;
        std::unordered_map<int,SDL_Rect> colliders;
};

Tilemap::Tilemap() {

}

Tilemap::~Tilemap() {
    free();
}

void Tilemap::free() {
    if (map != nullptr) {
        cute_tiled_free_map(map);
    }
}

bool Tilemap::load_from_file(std::string path) {
    map_path = path;
    bool success = true;
    free();
    // std::cout << "Loading: " << path << std::endl;
    map = cute_tiled_load_map_from_file(map_path.c_str(), NULL);
    if (map == nullptr) {
        success = false;
    } else {
        cute_tiled_tileset_t* ts = map->tilesets;
        int texture_count = 0;
        while (ts) {
            // char* fullpath = strcat(basename(path.c_str()), ts->image.ptr);
            // std::cout << "Loading Tileset: " << strcat(strcat(dirname(const_cast<char*>(path.c_str())),"/"), ts->image.ptr) << std::endl;
            textures.push_back(Texture());
            bool success = textures.back().load_from_file(strcat(strcat(dirname(const_cast<char*>(path.c_str())),"/"), ts->image.ptr));
            textures.back().define_sprites(ts->tilecount, ts->firstgid, ts->tilewidth, ts->tileheight);
            // std::cout << success << " " << texture_count << " " << textures.back().file_path << std::endl;
            for (int x = ts->firstgid; x < ts->firstgid+ts->tilecount; x++) {
                gid_to_texture[x] = texture_count;
            }
            texture_count++;
            cute_tiled_tile_descriptor_t* tile = ts->tiles;
            while (tile) {
                // tile->tile_index;
                cute_tiled_layer_t* objgrp = tile->objectgroup;
                while (objgrp) {
                    cute_tiled_object_t* obj = objgrp->objects;
                    while (obj) {
                        colliders[tile->tile_index + ts->firstgid] = SDL_Rect({static_cast<int>(obj->x), static_cast<int>(obj->y), obj->width, obj->height});
                        obj = obj->next;
                    }
                    objgrp = objgrp->next;
                }
                tile = tile->next;
            }
            ts = ts->next;
        }
        // std::cout << colliders.size() << std::endl;
    }
    // std::cout << "TILEMAP LAYER DATA:" << std::endl;
    // cute_tiled_layer_t* layer = map->layers;
    // while(layer) {
    //     for (int y = 0; y < map->height; y++) {
    //         for (int x = 0; x < map->width; x++) {
    //             std::cout << layer->data[map->width*y+x] << " ";
    //         }
    //         std::cout << std::endl;
    //     }
    //     layer = layer->next;
    //     std::cout << std::endl;
    // }
    return success;
}

void Tilemap::render_layers(SDL_Rect camera) {
    int y_tile_pos = camera.y/TILE_SIZE;
    std::cout << camera << " " << y_tile_pos << std::endl;
    // TODO: offset which tiles are drawn based on simple occlusion based on camera position

    cute_tiled_layer_t* layer = map->layers;
    // iterate over layers
    // selecting texture t by gid
    while (layer) {
        for (int y = y_tile_pos; y < map->height; y++) {
            for (int x = 0; x < map->width; x++) {
                int t_index = gid_to_texture[layer->data[map->width*y+x]];
                int s_index = layer->data[map->width*y+x] - textures[t_index].gid_offset;
                if (s_index >= 0) {
                    // std::cout << s_index << " ";
                    // std::cout << x*map->tilewidth << " " << y*map->tileheight << " " << textures[0].sprites[s_index] << std::endl;
                    textures[t_index].render(x*map->tilewidth, y*map->tileheight - (y_tile_pos*TILE_SIZE), &textures[t_index].sprites[s_index]);
                }
            }
            // std::cout << std::endl;
        }
        layer = layer->next;
    }
}

int Tilemap::tile_gid(int position) {
    cute_tiled_layer_t* layer = map->layers;
    int gid = 0;
    while (layer) {
        gid = layer->data[position];
        layer = layer->next;
    }
    return gid;
}

SDL_Rect Tilemap::get_collider(int gid, int x, int y) {
    auto search = colliders.find(gid);
    if(search != colliders.end()) {
        SDL_Rect c = colliders[gid];
        c.x += x;
        c.y += y;
        return c;
    } else {
        //HACK to return a collider that will never collide
        return {-x,-y,0,0};
    }      
}
