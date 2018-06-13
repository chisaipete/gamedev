#include <iostream>
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <SDL.h>
#include <SDL_timer.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

extern SDL_Renderer* renderer;
TTF_Font* font;

const int SCREEN_FPS = 60;
const int SCREEN_TICKS_PER_FRAME = 1000 / SCREEN_FPS;

const int TILE_SIZE = 32;
const int WELL_TILE_WIDTH = 10;
const int WELL_TILE_HEIGHT = 22; // top two are "hidden"
const int SCREEN_TILE_WIDTH = 12;
const int SCREEN_TILE_HEIGHT = 25; // 

const int SCREEN_WIDTH = TILE_SIZE*SCREEN_TILE_WIDTH;
const int SCREEN_HEIGHT = TILE_SIZE*SCREEN_TILE_HEIGHT;

//tilemap indexes
#define PERIWINKLE  ( 0)
#define NAVY        ( 1)
#define ORANGE      ( 2)
#define YELLOW      ( 3)
#define GREEN       ( 4)
#define PINK        ( 5)
#define RED         ( 6)
#define PALETTE     ( 7)
#define UL_WALL     ( 8)
#define TOP_WALL    ( 9)
#define UR_WALL     (10)
#define LEFT_WALL   (11)
#define EMPTY       (12)
#define RIGHT_WALL  (13)
#define LL_WALL     (14)
#define BOTTOM_WALL (15)
#define LR_WALL     (16)
#define BLANK       (17)

//game state FSM
#define START   (0)
// #define PAUSE   (1)
// #define PLAY    (2)
// #define OVER    (3)

struct GameState {
    int state;
    int level;
    int score;
    int lines;
    // Paddle paddle;
    // Ball active_balls[3];
    // Brick bricks[BRICK_WIDTH*BRICK_HEIGHT];
    // Wall walls[SCREEN_TILE_WIDTH*SCREEN_TILE_HEIGHT];
};

SDL_Color WHITE = {255,255,255,255};

unsigned long long rdtsc(){
    unsigned int lo,hi;
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((unsigned long long)hi << 32) | lo;
}

void logSDLError(std::ostream &os, const std::string &msg) {
    os << msg << " SDL Error: " << SDL_GetError() << std::endl;
}

bool checkBoxCollision(SDL_Rect a, SDL_Rect b) {
    // assume collision [separating axis text]
    int leftA, leftB, rightA, rightB, topA, topB, bottomA, bottomB;
    //sides of a
    leftA = a.x;
    rightA = a.x + a.w;
    topA = a.y;
    bottomA = a.y + a.h;
    //sides of b
    leftB = b.x;
    rightB = b.x + b.w;
    topB = b.y;
    bottomB = b.y + b.h;
    //check for collisions!
    if (bottomA <= topB) return false;
    if (topA >= bottomB) return false;
    if (rightA <= leftB) return false;
    if (leftA >= rightB) return false;
    //none of the sides from a are outside b
    return true;
}

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
    private:
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

void Texture::free() {
    if (texture != NULL) {
        SDL_DestroyTexture(texture);
        texture = NULL;
        width = 0;
        height = 0;
    }
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

