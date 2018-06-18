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

extern SDL_Renderer* renderer;
TTF_Font* font;

const int SCREEN_FPS = 60;
const int SCREEN_TICKS_PER_FRAME = 1000 / SCREEN_FPS;

const int BLOCK_SIZE = 32;
const int WELL_BLOCK_WIDTH = 12; //two columns wider than displayed field (for left hand collision detection)
const int WELL_BLOCK_HEIGHT = 22; // top two are "hidden"
const int SCREEN_BLOCK_WIDTH = 14;
const int SCREEN_BLOCK_HEIGHT = 25; // 

const int SCREEN_WIDTH = BLOCK_SIZE*SCREEN_BLOCK_WIDTH;
const int SCREEN_HEIGHT = BLOCK_SIZE*SCREEN_BLOCK_HEIGHT;

//tilemap indexes
const int CYAN   = 0; // I
const int BLUE   = 1; // J
const int ORANGE = 2; // L
const int YELLOW = 3; // O
const int GREEN  = 4; // S
const int PURPLE = 5; // T
const int RED    = 6; // Z
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
#define PAUSE   (1)
#define PLAY    (2)
#define OVER    (3)

enum Piece       { I, J, L, O, S, T, Z };
int max_ext[7] = { 4, 3, 3, 2, 3, 3, 3 };
enum Direction {LEFT, RIGHT, DOWN, HARD_DOWN, UP};


// collision masks
const unsigned NOHIT = 0b0000000111;
const unsigned WALLS = 0b0000000001;
const unsigned FLOOR = 0b0000000010;
const unsigned PIECE = 0b0000000100;
const unsigned PSELF = 0b0000001000;
const unsigned LWALL = 0b0000010000;
const unsigned RWALL = 0b0001000000;
const unsigned RPICE = 0b0010000000;
const unsigned LPICE = 0b0100000000;
const unsigned FPICE = 0b1000000000;


int block_step (int level) {
    float step_sec;
    step_sec = pow((0.8-((static_cast<float>(level)-1.0)*0.007)),(static_cast<float>(level)-1.0));
    int step_ms = static_cast<int>(step_sec*1000.0);
} 

int level_goal (int level) {
    int goal = 5*level;
    return goal;
}
/*
Goal of each level L is to earn 5*L points:
Line clear      WScore
Single          1
Double          3
Triple          5
Tetris          8
Tetris B2B      12
T-spin zero     1
T-spin single   3
T-spin double   7
T-spin triple   6
*/

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

/* VECTOR */
struct v2 {
    int x, y;
};

std::ostream &operator<<(std::ostream &os, v2 const &A) { 
    return os << "(" << A.x << "," << A.y << ")";
}

// convert pair of ints to v2
v2 V2(int A, int B) {
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

v2 operator*(int A, v2 B) {
    v2 r;
    r.x = A*B.x;
    r.y = A*B.y;
    return r;
};

v2 operator*(v2 A, int B) {
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

v2 operator-(v2 A, v2 B) {
    v2 r;
    r.x = A.x-B.x;
    r.y = A.y-B.y;
    return r;
};

/* TETRIS OBJECTS */
struct Block {
    int color;
};

struct Tetrimino {
    v2 ulpt;
    Block* blocks[4];
    v2 rotation[4];
};

struct GameState {
    int state;
    int level;
    int score;
    int lines;
    bool new_piece;
    Tetrimino piece;
    Block* blocks[(WELL_BLOCK_WIDTH)*WELL_BLOCK_HEIGHT];
    //adding a two space buffer to be null to help with detecting drops on edges
};

/*
Generates a sequence of all seven pieces permuted randomly, as if they were drawn from a bag. 
Deals all seven pieces before generating another bag.
*/
class Bag {
    public:
        Bag();
        ~Bag();
        void shuffle_bag();
        Piece draw_piece();
        Piece shake();
    private:
        std::vector<Piece> bag; 
};

Bag::Bag() {
    bag.reserve(7);
}

Bag::~Bag() {}

void Bag::shuffle_bag() {
    while(bag.size() < 7) {
        bag.push_back(shake());
    }
}

Piece Bag::draw_piece() {
    if (bag.empty()) {
        shuffle_bag();
    }
    Piece p = bag.back();
    bag.pop_back();
    return p; 
}

Piece Bag::shake() {
    Piece p;
    do {
        p = static_cast<Piece>(rand() % 7);
    } while (std::find(bag.begin(), bag.end(), p) != bag.end());
    return p;
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

/* TEXTURE CLASS */
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



