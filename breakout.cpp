#include <iostream>
#include <cstdlib>
#include <ctime>
#include <SDL.h>
#include <SDL_timer.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

unsigned long long rdtsc(){
    unsigned int lo,hi;
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((unsigned long long)hi << 32) | lo;
}

//TODO: add a scaling factor for EVERYTHING if we decide to change the size of things

const int TILE_SIZE = 32;
const int SCREEN_TILE_WIDTH = 20;
const int SCREEN_TILE_HEIGHT = 15;
const int SCREEN_WIDTH = TILE_SIZE*SCREEN_TILE_WIDTH; //640
const int SCREEN_HEIGHT = TILE_SIZE*SCREEN_TILE_HEIGHT; //480
const int BRICK_WIDTH = SCREEN_TILE_WIDTH-2;
const int BRICK_HEIGHT = SCREEN_TILE_HEIGHT;
const int BRICK_TILE_HEIGHT = 12;
const int MAX_BALL_VELOCITY = 1;
const int BALL_WIDTH = 4;
const int BALL_HEIGHT = 4;
const int PADDLE_WIDTH = 28; //varies if wider
const int PADDLE_HEIGHT = 7;

//LazyFoo27: should probably be making objects for things to encapsulate code better

//tilemap indexes
#define EMPTY              (-1)
#define RED_BRICK          ( 0)
#define GREEN_BRICK        ( 1)
#define BLUE_BRICK         ( 2)
#define VIOLET_BRICK       ( 3)
#define YELLOW_BRICK       ( 4)
#define BALL               ( 5)
#define LONG_PADDLE_LEFT   ( 6)
#define LONG_PADDLE_CENTER ( 7)
#define LONG_PADDLE_RIGHT  ( 8)
#define PADDLE             ( 9)
#define LEFT_ARENA_CORNER  (10)
#define RIGHT_ARENA_CORNER (11)
#define LEFT_ARENA_WALL    (12)
#define RIGHT_ARENA_WALL   (13)
#define FIRE_BALL          (14)

SDL_Rect sprites[15];

SDL_Color WHITE = { 255, 255, 255, 255 };

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
SDL_Texture* tilemap = NULL;

struct Ball {
    int ball_type;
    SDL_Point ball_pos;
    SDL_Point velocity;
    SDL_Rect collider;
};

struct GameState {
    bool gameover; //is the game over?
    bool paused;
    int score;
    int balls; //lives
    SDL_Point paddle_pos;
    int paddle_size;
    SDL_Rect paddle_collider;
    Ball active_balls[3];
    int bricks[BRICK_WIDTH*BRICK_HEIGHT];
};

GameState gamestate;

void logSDLError(std::ostream &os, const std::string &msg) {
    os << msg << " SDL Error: " << SDL_GetError() << std::endl;
}

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
            window = SDL_CreateWindow("Breakout", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
            if (window == nullptr){
                logSDLError(std::cout, "SDL_CreateWindow");
                success = false;
            } else {
                renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
                if (renderer == nullptr){
                    logSDLError(std::cout, "SDL_CreateRenderer");
                    success = false;
                } else {
                    //Initialize renderer color (also used for clearing)
                    // SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF); 
                    SDL_SetRenderDrawColor(renderer, 0x0, 0x0, 0x0, 0xFF); //black
                }
            }
        }
    }
    srand(rdtsc()); //seed random nicely

    return success;
}

bool load() {
    bool success = true;

    std::string imagePath = "res/bricks.png";
    tilemap = IMG_LoadTexture(renderer, imagePath.c_str());
    if (tilemap == nullptr){ //could also load to surface, then optimize with SDL_ConvertSurface?
        logSDLError(std::cout, "IMG_LoadTexture");
        success = false;
    } else {
        //setup tiles
        sprites[RED_BRICK].x = 0;
        sprites[RED_BRICK].y = 0;
        sprites[RED_BRICK].w = TILE_SIZE;
        sprites[RED_BRICK].h = TILE_SIZE;
        sprites[GREEN_BRICK].x = TILE_SIZE*1;
        sprites[GREEN_BRICK].y = 0;
        sprites[GREEN_BRICK].w = TILE_SIZE;
        sprites[GREEN_BRICK].h = TILE_SIZE;
        sprites[BLUE_BRICK].x = TILE_SIZE*2;
        sprites[BLUE_BRICK].y = 0;
        sprites[BLUE_BRICK].w = TILE_SIZE;
        sprites[BLUE_BRICK].h = TILE_SIZE;
        sprites[VIOLET_BRICK].x = 0;
        sprites[VIOLET_BRICK].y = TILE_SIZE;
        sprites[VIOLET_BRICK].w = TILE_SIZE;
        sprites[VIOLET_BRICK].h = TILE_SIZE;
        sprites[YELLOW_BRICK].x = TILE_SIZE*1;
        sprites[YELLOW_BRICK].y = TILE_SIZE;
        sprites[YELLOW_BRICK].w = TILE_SIZE;
        sprites[YELLOW_BRICK].h = TILE_SIZE;
        sprites[BALL].x = TILE_SIZE*2;
        sprites[BALL].y = TILE_SIZE;
        sprites[BALL].w = TILE_SIZE;
        sprites[BALL].h = TILE_SIZE;
        sprites[LONG_PADDLE_LEFT].x = 0;
        sprites[LONG_PADDLE_LEFT].y = TILE_SIZE*2;
        sprites[LONG_PADDLE_LEFT].w = TILE_SIZE;
        sprites[LONG_PADDLE_LEFT].h = TILE_SIZE;
        sprites[LONG_PADDLE_CENTER].x = TILE_SIZE*1;
        sprites[LONG_PADDLE_CENTER].y = TILE_SIZE*2;
        sprites[LONG_PADDLE_CENTER].w = TILE_SIZE;
        sprites[LONG_PADDLE_CENTER].h = TILE_SIZE;
        sprites[LONG_PADDLE_RIGHT].x = TILE_SIZE*2;
        sprites[LONG_PADDLE_RIGHT].y = TILE_SIZE*2;
        sprites[LONG_PADDLE_RIGHT].w = TILE_SIZE;
        sprites[LONG_PADDLE_RIGHT].h = TILE_SIZE;
        sprites[PADDLE].x = 0;
        sprites[PADDLE].y = TILE_SIZE*3;
        sprites[PADDLE].w = TILE_SIZE;
        sprites[PADDLE].h = TILE_SIZE;
        sprites[LEFT_ARENA_CORNER].x = TILE_SIZE*1;
        sprites[LEFT_ARENA_CORNER].y = TILE_SIZE*3;
        sprites[LEFT_ARENA_CORNER].w = TILE_SIZE;
        sprites[LEFT_ARENA_CORNER].h = TILE_SIZE;
        sprites[RIGHT_ARENA_CORNER].x = TILE_SIZE*2;
        sprites[RIGHT_ARENA_CORNER].y = TILE_SIZE*3;
        sprites[RIGHT_ARENA_CORNER].w = TILE_SIZE;
        sprites[RIGHT_ARENA_CORNER].h = TILE_SIZE;
        sprites[LEFT_ARENA_WALL].x = 0;
        sprites[LEFT_ARENA_WALL].y = TILE_SIZE*4;
        sprites[LEFT_ARENA_WALL].w = TILE_SIZE;
        sprites[LEFT_ARENA_WALL].h = TILE_SIZE;
        sprites[RIGHT_ARENA_WALL].x = TILE_SIZE*1;
        sprites[RIGHT_ARENA_WALL].y = TILE_SIZE*4;
        sprites[RIGHT_ARENA_WALL].w = TILE_SIZE;
        sprites[RIGHT_ARENA_WALL].h = TILE_SIZE;
        sprites[FIRE_BALL].x = TILE_SIZE*2;
        sprites[FIRE_BALL].y = TILE_SIZE*4;
        sprites[FIRE_BALL].w = TILE_SIZE;
        sprites[FIRE_BALL].h = TILE_SIZE;
    }

    return success;
}

bool close() {
    if (tilemap != nullptr) { SDL_DestroyTexture(tilemap); tilemap = NULL; }
    if (renderer != nullptr) { SDL_DestroyRenderer(renderer); renderer = NULL; }
    if (window != nullptr) { SDL_DestroyWindow(window); window = NULL; }
    // IMG_Quit();
    TTF_Quit();
    SDL_Quit();
}

//LazyFoo 7
// SDL_Texture* loadTexture(std::string path) {
//     //final texture
//     SDL_Texture* newTexture = NULL;

//     //Load image from path
//     SDL_Surface* loadedSurface = 
// }

//LazyFoo 10 - color keying, define texture wrapper class

SDL_Texture* loadText_to_Texture(const std::string &message, SDL_Renderer* ren, const std::string &fontFile = "res/cc.ttf", SDL_Color color = WHITE, int fontSize = 12) {
    //Open the font
    TTF_Font *font = TTF_OpenFont(fontFile.c_str(), fontSize);
    if (font == nullptr){
        logSDLError(std::cout, "TTF_OpenFont");
        return nullptr;
    }
    //We need to first render to a surface as that's what TTF_RenderText returns, then
    //load that surface into a texture
    SDL_Surface *surf = TTF_RenderText_Blended(font, message.c_str(), color);
    if (surf == nullptr){
        TTF_CloseFont(font);
        logSDLError(std::cout, "TTF_RenderText");
        return nullptr;
    }
    SDL_Texture *texture = SDL_CreateTextureFromSurface(ren, surf);
    if (texture == nullptr){
        logSDLError(std::cout, "CreateTexture");
    }
    //Clean up the surface and font
    SDL_FreeSurface(surf);
    TTF_CloseFont(font);
    return texture;
}

void renderTexture(SDL_Texture* tex, SDL_Renderer *ren, SDL_Rect dst, SDL_Rect* clip = nullptr) {
    SDL_RenderCopy(ren, tex, clip, &dst);
}

void renderTextureEx(SDL_Texture* tex, SDL_Renderer *ren, SDL_Rect dst, SDL_Rect* clip = nullptr, double angle = 0, SDL_Point* center = nullptr, SDL_RendererFlip flip = SDL_FLIP_NONE) {
    SDL_RenderCopyEx(ren, tex, clip, &dst, angle, center, flip);
}

void renderTexture(SDL_Texture* tex, SDL_Renderer* ren, int x, int y, SDL_Rect* clip = nullptr) {
    SDL_Rect dst;
    dst.x = x;
    dst.y = y;
    if (clip != nullptr) {
        dst.w = clip->w;
        dst.h = clip->h;
    } else {
        SDL_QueryTexture(tex, NULL, NULL, &dst.w, &dst.h);
    }
    renderTexture(tex, ren, dst, clip);
}

//support flipping & rotating sprites
void renderTextureEx(SDL_Texture* tex, SDL_Renderer* ren, int x, int y, SDL_Rect* clip = nullptr, double angle = 0, SDL_Point* center = nullptr, SDL_RendererFlip flip = SDL_FLIP_NONE) {
    SDL_Rect dst;
    dst.x = x;
    dst.y = y;
    if (clip != nullptr) {
        dst.w = clip->w;
        dst.h = clip->h;
    } else {
        SDL_QueryTexture(tex, NULL, NULL, &dst.w, &dst.h);
    }
    if (center == nullptr) {
        SDL_Point center;
        center.x = clip->w / 2;
        center.y = clip->h / 2;
    }
    renderTextureEx(tex, ren, dst, clip, angle, center, flip);
}

void renderArena(SDL_Renderer *ren, SDL_Texture *tm, GameState &gamestate) {
    //write background, then arena, then bricks, then paddle and then ball
    //could optimize this by drawing the arena once, then storing it in a layer to blit
    //draw arena 
    for (int y = 0; y < SCREEN_TILE_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_TILE_WIDTH; x++) {
            if (x == 0) {
                if (y > 0 ) {
                    renderTexture(tm, ren, x*TILE_SIZE, y*TILE_SIZE, &sprites[LEFT_ARENA_WALL]);
                } else {
                    renderTexture(tm, ren, x*TILE_SIZE, y*TILE_SIZE, &sprites[LEFT_ARENA_CORNER]);
                }
            } else if (x == SCREEN_TILE_WIDTH-1) {
                if (y > 0 ) {
                    renderTexture(tm, ren, x*TILE_SIZE, y*TILE_SIZE, &sprites[RIGHT_ARENA_WALL]);
                } else if (y == 0){
                    renderTexture(tm, ren, x*TILE_SIZE, y*TILE_SIZE, &sprites[RIGHT_ARENA_CORNER]);
                }
            }
            if (y == 0) {
                if (x > 0 && x < SCREEN_TILE_WIDTH-1) {
                    renderTextureEx(tm, ren, x*TILE_SIZE, y*TILE_SIZE, &sprites[LEFT_ARENA_WALL], 90, NULL);
                }
            }
        }
    }
    //draw bricks (12px tall, full tile is 32px, 10px buffer around each, 32px wide)
    for (int y = 0; y < BRICK_HEIGHT; y++) {
        for (int x = 0; x < BRICK_WIDTH; x++) {
            renderTexture(tm, ren, (x+1)*TILE_SIZE, y*BRICK_TILE_HEIGHT, &sprites[gamestate.bricks[BRICK_WIDTH*y+x]]);
        }
    }

    //TODO: optimize away some of these constants into #defines
    //draw paddle
    switch (gamestate.paddle_size) {
        case 1:
            renderTexture(tm, ren, gamestate.paddle_pos.x-((TILE_SIZE/2)-(PADDLE_WIDTH/2)), gamestate.paddle_pos.y-((TILE_SIZE/2)-(PADDLE_HEIGHT/2)), &sprites[PADDLE]);
            break;
        default:
            break;
    }
    
    //draw ball
    for (int i = 0; i < 3; i++) {  //up to 3 balls
        switch (gamestate.active_balls[i].ball_type) {
            case FIRE_BALL:
                renderTexture(tm, ren, gamestate.active_balls[i].ball_pos.x-((TILE_SIZE/2)-(BALL_WIDTH/2)), gamestate.active_balls[i].ball_pos.y-((TILE_SIZE/2)-(BALL_HEIGHT/2)), &sprites[FIRE_BALL]);
                break;
            case BALL:
                renderTexture(tm, ren, gamestate.active_balls[i].ball_pos.x-((TILE_SIZE/2)-(BALL_WIDTH/2)), gamestate.active_balls[i].ball_pos.y-((TILE_SIZE/2)-(BALL_HEIGHT/2)), &sprites[BALL]);
                break;
            default:
                //EMPTY
                break;
        }
    }
}

// void renderStatus(SDL_Renderer *ren, GameState &gamestate) {
//     if (!gamestate.gameover) {
//         if (gamestate.turn) {
//             SDL_Texture* image = loadText_to_Texture("It's my turn!", ren);
//             int iW, iH;
//             SDL_QueryTexture(image, NULL, NULL, &iW, &iH);
//             renderTexture(image, ren, SCREEN_WIDTH/2 - iW/2, SCREEN_HEIGHT-TILE_SIZE/2);
//         } else {
//             SDL_Texture* image = loadText_to_Texture("It's the human's turn.", ren);
//             int iW, iH;
//             SDL_QueryTexture(image, NULL, NULL, &iW, &iH);
//             renderTexture(image, ren, SCREEN_WIDTH/2 - iW/2, SCREEN_HEIGHT-TILE_SIZE/2);
//         }
//     } else {
//         if (gamestate.tie) {
//             SDL_Texture* image = loadText_to_Texture("A tie.", ren);
//             int iW, iH;
//             SDL_QueryTexture(image, NULL, NULL, &iW, &iH);
//             renderTexture(image, ren, SCREEN_WIDTH/2 - iW/2, SCREEN_HEIGHT-TILE_SIZE/2);
//         } else if (gamestate.turn) {            
//             SDL_Texture* image = loadText_to_Texture("I won!", ren);
//             int iW, iH;
//             SDL_QueryTexture(image, NULL, NULL, &iW, &iH);
//             renderTexture(image, ren, SCREEN_WIDTH/2 - iW/2, SCREEN_HEIGHT-TILE_SIZE/2);
//         } else {
//             SDL_Texture* image = loadText_to_Texture("The human won.", ren);
//             int iW, iH;
//             SDL_QueryTexture(image, NULL, NULL, &iW, &iH);
//             renderTexture(image, ren, SCREEN_WIDTH/2 - iW/2, SCREEN_HEIGHT-TILE_SIZE/2);
//         }
//     }
// }

void loadLevel(GameState &gamestate, int LevelNum = 0) {
    switch (LevelNum) {
        case 1:
            //simple rainbow
            for (int y = 0; y < BRICK_HEIGHT; y++) {
                for (int x = 0; x < BRICK_WIDTH; x++) {
                    switch (y % 5) {
                        case RED_BRICK:
                            gamestate.bricks[BRICK_WIDTH*y+x] = RED_BRICK;
                            break;
                        case GREEN_BRICK:
                            gamestate.bricks[BRICK_WIDTH*y+x] = GREEN_BRICK;
                            break;
                        case BLUE_BRICK:
                            gamestate.bricks[BRICK_WIDTH*y+x] = BLUE_BRICK;
                            break;
                        case VIOLET_BRICK:
                            gamestate.bricks[BRICK_WIDTH*y+x] = VIOLET_BRICK;
                            break;
                        case YELLOW_BRICK:
                            gamestate.bricks[BRICK_WIDTH*y+x] = YELLOW_BRICK;
                            break;
                    }
                    
                }
            }
            break;
        default:
            //populate empty bricks
            for (int y = 0; y < BRICK_HEIGHT; y++) {
                for (int x = 0; x < BRICK_WIDTH; x++) {
                    gamestate.bricks[BRICK_WIDTH*y+x] = EMPTY;
                }
            }
            break;
    }
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

bool checkPixCollision() {

}

void moveBalls(GameState &gamestate) {
    for (int i = 0; i < 3; i++) {  //up to 3 balls
        switch (gamestate.active_balls[i].ball_type) {
            case FIRE_BALL:
            case BALL:
                gamestate.active_balls[i].ball_pos.x += gamestate.active_balls[i].velocity.x;
                gamestate.active_balls[i].collider.x = gamestate.active_balls[i].ball_pos.x;
                //check for collision on every wall, brick and paddle
                //wall check
                //brick check
                //paddle check
                switch (gamestate.paddle_size) {
                    case 1:
                        if (checkBoxCollision(gamestate.active_balls[i].collider, gamestate.paddle_collider)) {
                            gamestate.active_balls[i].ball_pos.x -= gamestate.active_balls[i].velocity.x;
                            gamestate.active_balls[i].collider.x = gamestate.active_balls[i].ball_pos.x;
                        }
                        break;
                    default:
                        break;
                }
                //ball check **BONUS**
                gamestate.active_balls[i].ball_pos.y += gamestate.active_balls[i].velocity.y;
                gamestate.active_balls[i].collider.y = gamestate.active_balls[i].ball_pos.y;
                //check for collision on every wall, brick and paddle
                //wall check
                //brick check
                //paddle check
                switch (gamestate.paddle_size) {
                    case 1:
                        if (checkBoxCollision(gamestate.active_balls[i].collider, gamestate.paddle_collider)) {
                            gamestate.active_balls[i].ball_pos.y -= gamestate.active_balls[i].velocity.y;
                            gamestate.active_balls[i].collider.y = gamestate.active_balls[i].ball_pos.y;
                        }
                        break;
                    default:
                        break;
                }
                //ball check **BONUS**
                break;
            default:
                break;
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
            int quit = 0;
            //setup gamestate and first level loadout (maybe spit into two structs??)
            gamestate.gameover = false;
            gamestate.paused = false;
            gamestate.score = 0;
            gamestate.balls = 3;   //TODO: check that this collider is aligned on the paddle (perhaps render collider guides)
            gamestate.paddle_pos = {(SCREEN_WIDTH/2)-(PADDLE_WIDTH/2), SCREEN_HEIGHT-(TILE_SIZE-(TILE_SIZE/2))-(PADDLE_HEIGHT/2)};
            gamestate.paddle_size = 1;  //TODO: fix rendering offset as well
            gamestate.paddle_collider = {gamestate.paddle_pos.x,gamestate.paddle_pos.y,PADDLE_WIDTH,PADDLE_HEIGHT};
            gamestate.active_balls[0].ball_type = BALL;
            gamestate.active_balls[0].ball_pos = {(SCREEN_WIDTH/2)-(BALL_WIDTH/2), SCREEN_HEIGHT-(TILE_SIZE+BRICK_TILE_HEIGHT)-(BALL_HEIGHT/2)};
            gamestate.active_balls[0].velocity = {0,0};
            gamestate.active_balls[0].collider = {gamestate.active_balls[0].ball_pos.x,gamestate.active_balls[0].ball_pos.y,BALL_WIDTH,BALL_HEIGHT};
            gamestate.active_balls[1].ball_type = EMPTY;
            gamestate.active_balls[1].ball_pos = {0,0};
            gamestate.active_balls[1].velocity = {0,0};
            gamestate.active_balls[1].collider = {gamestate.active_balls[1].ball_pos.x,gamestate.active_balls[1].ball_pos.y,BALL_WIDTH,BALL_HEIGHT};
            gamestate.active_balls[2].ball_type = EMPTY;
            gamestate.active_balls[2].ball_pos = {0,0};
            gamestate.active_balls[2].velocity = {0,0};
            gamestate.active_balls[2].collider = {gamestate.active_balls[2].ball_pos.x,gamestate.active_balls[2].ball_pos.y,BALL_WIDTH,BALL_HEIGHT};
            loadLevel(gamestate, 1);

            //initialize wall colliders!

            while (!quit) {
                SDL_Event event;

                while (SDL_PollEvent(&event)) {
                    switch (event.type) {
                        case SDL_QUIT:
                            quit = 1;
                            break;
                    }
                    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE ) {
                        quit = 1;
                    }
                    if (event.type == SDL_KEYDOWN && event.key.repeat == 0) {
                        switch(event.key.keysym.sym) {
                            case SDLK_UP:    gamestate.active_balls[0].velocity.y -= MAX_BALL_VELOCITY; break;
                            case SDLK_DOWN:  gamestate.active_balls[0].velocity.y += MAX_BALL_VELOCITY; break;
                            case SDLK_LEFT:  gamestate.active_balls[0].velocity.x -= MAX_BALL_VELOCITY; break;
                            case SDLK_RIGHT: gamestate.active_balls[0].velocity.x += MAX_BALL_VELOCITY; break;
                        }
                    } else if(event.type == SDL_KEYUP && event.key.repeat == 0) {
                        //Adjust the velocity
                        switch(event.key.keysym.sym) {
                            case SDLK_UP:    gamestate.active_balls[0].velocity.y += MAX_BALL_VELOCITY; break;
                            case SDLK_DOWN:  gamestate.active_balls[0].velocity.y -= MAX_BALL_VELOCITY; break;
                            case SDLK_LEFT:  gamestate.active_balls[0].velocity.x += MAX_BALL_VELOCITY; break;
                            case SDLK_RIGHT: gamestate.active_balls[0].velocity.x -= MAX_BALL_VELOCITY; break;
                        }
                    }
                }

                if (!gamestate.gameover) {
                    moveBalls(gamestate);
                }

                //clear backbuffer
                SDL_RenderClear(renderer);
                //write to backbuffer
                //remember furthest Z written first
                //LazyFoo 8, could use geometry (rect, lines, points) in SDL renderer instead!
                //LazyFoo 9, viewports! (minimaps)
                //LazyFoo12, renderer color modulation (filter with color)
                //LazyFoo13, renderer alpha blending
                //write background, then arena, then bricks, then paddle and then ball
                renderArena(renderer, tilemap, gamestate);
                //write text        
                // renderStatus(renderer, gamestate);

                //GUIDELINES
                //white
                SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF); 
                SDL_RenderDrawLine(renderer, SCREEN_WIDTH/2, SCREEN_HEIGHT, SCREEN_WIDTH/2, 0);
                SDL_RenderDrawLine(renderer, 0, SCREEN_HEIGHT-(TILE_SIZE-(TILE_SIZE/2)), SCREEN_WIDTH, SCREEN_HEIGHT-(TILE_SIZE-(TILE_SIZE/2)));
                SDL_RenderDrawLine(renderer, 0, SCREEN_HEIGHT-(TILE_SIZE+BRICK_TILE_HEIGHT), SCREEN_WIDTH, SCREEN_HEIGHT-(TILE_SIZE+BRICK_TILE_HEIGHT));
                //back to black
                SDL_SetRenderDrawColor(renderer, 0x0, 0x0, 0x0, 0xFF);
                //END GUIDELINES

                //swap buffers
                SDL_RenderPresent(renderer);
                SDL_Delay(1000/60); //wait for 60ms
            }
        }
    }
    close();
    return 0;
}
