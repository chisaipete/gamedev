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

const int TILE_SIZE = 32;

const int SCREEN_WIDTH = TILE_SIZE*5;
const int SCREEN_HEIGHT = TILE_SIZE*4;

//tilemap indexes
#define _PAPR_ (0)
#define _ROCK_ (1)
#define _SCIS_ (2)
#define _SELC_ (3)
#define _CHEK_ (4)
#define _CROS_ (5)
#define _EMTY_ (6)

SDL_Rect sprites[6];

SDL_Color WHITE = { 255, 255, 255, 255 };

struct GameState {
    bool gameover; //is the game over?
    bool tie; //last player active is the winner, what if it's a tie?
    int winner; //declared winner
    int played_rounds;
    int rounds[6];
    int marks[6];
    int score[2];
};

void logSDLError(std::ostream &os, const std::string &msg) {
    os << msg << " SDL Error: " << SDL_GetError() << std::endl;
}

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

void renderBoard(SDL_Renderer *ren, SDL_Texture *tilemap, GameState &gamestate) {
    //draw player marks
    for (int y = 0; y < 2; y++) {
        for (int x = 0; x < 3; x++) {
            switch (gamestate.rounds[3*y+x]) {
                case _ROCK_:
                    renderTexture(tilemap, ren, x*TILE_SIZE+TILE_SIZE, y*TILE_SIZE+TILE_SIZE, &sprites[_ROCK_]);
                    break;
                case _PAPR_:
                    renderTexture(tilemap, ren, x*TILE_SIZE+TILE_SIZE, y*TILE_SIZE+TILE_SIZE, &sprites[_PAPR_]);
                    break;
                case _SCIS_:
                    renderTexture(tilemap, ren, x*TILE_SIZE+TILE_SIZE, y*TILE_SIZE+TILE_SIZE, &sprites[_SCIS_]);
                    break;
                case _EMTY_:
                default:
                    break;
            }
            switch (gamestate.marks[3*y+x]) {
                case _CHEK_:
                    renderTexture(tilemap, ren, x*TILE_SIZE+TILE_SIZE, y*TILE_SIZE+TILE_SIZE, &sprites[_CHEK_]);
                    break;
                case _CROS_:
                    renderTexture(tilemap, ren, x*TILE_SIZE+TILE_SIZE, y*TILE_SIZE+TILE_SIZE, &sprites[_CROS_]);
                    break;
                case _EMTY_:
                default:
                    break;
            }
        }
    }
    
    if (!gamestate.gameover) {
        //draw selector on current round
        switch (gamestate.played_rounds) {
            case 0:
                renderTexture(tilemap, ren, TILE_SIZE,   TILE_SIZE*2, &sprites[_SELC_]);
                break;
            case 1:
                renderTexture(tilemap, ren, TILE_SIZE*2, TILE_SIZE*2, &sprites[_SELC_]);
                break;
            case 2:
                renderTexture(tilemap, ren, TILE_SIZE*3, TILE_SIZE*2, &sprites[_SELC_]);
            default:
                break;
        }
    }
}

void renderStatus(SDL_Renderer *ren, GameState &gamestate) {
    if (!gamestate.gameover) {
        SDL_Texture* image = loadText_to_Texture("I'm not looking...", ren);
        int iW, iH;
        SDL_QueryTexture(image, NULL, NULL, &iW, &iH);
        renderTexture(image, ren, SCREEN_WIDTH/2 - iW/2, SCREEN_HEIGHT-TILE_SIZE);
        image = loadText_to_Texture("Pick your weapon!", ren);
        SDL_QueryTexture(image, NULL, NULL, &iW, &iH);
        renderTexture(image, ren, SCREEN_WIDTH/2 - iW/2, SCREEN_HEIGHT-TILE_SIZE/2);
    } else {
        if (gamestate.tie) {
            SDL_Texture* image = loadText_to_Texture("A tie.", ren);
            int iW, iH;
            SDL_QueryTexture(image, NULL, NULL, &iW, &iH);
            renderTexture(image, ren, SCREEN_WIDTH/2 - iW/2, SCREEN_HEIGHT-TILE_SIZE/2);
        } else if (gamestate.winner) {            
            SDL_Texture* image = loadText_to_Texture("I won!", ren);
            int iW, iH;
            SDL_QueryTexture(image, NULL, NULL, &iW, &iH);
            renderTexture(image, ren, SCREEN_WIDTH/2 - iW/2, SCREEN_HEIGHT-TILE_SIZE/2);
        } else {
            SDL_Texture* image = loadText_to_Texture("The human won.", ren);
            int iW, iH;
            SDL_QueryTexture(image, NULL, NULL, &iW, &iH);
            renderTexture(image, ren, SCREEN_WIDTH/2 - iW/2, SCREEN_HEIGHT-TILE_SIZE/2);
        }
    }
}

int main(int argc, char **argv) {
    srand(rdtsc()); //seed random nicely
    if (SDL_Init(SDL_INIT_VIDEO) != 0){
        logSDLError(std::cout, "SDL_Init");
        return 1;
    }

    if (TTF_Init() != 0){
        logSDLError(std::cout, "TTF_Init");
        SDL_Quit();
        return 1;
    }
    
    SDL_Window *window = SDL_CreateWindow("Rock, Paper, Scissors", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == nullptr){
        logSDLError(std::cout, "SDL_CreateWindow");
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == nullptr){
        SDL_DestroyWindow(window);
        logSDLError(std::cout, "SDL_CreateRenderer");
        SDL_Quit();
        return 1;
    }

    std::string imagePath = "res/rps.png";
    SDL_Texture *tilemap = IMG_LoadTexture(renderer, imagePath.c_str());
    if (tilemap == nullptr){
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        logSDLError(std::cout, "IMG_LoadTexture");
        SDL_Quit();
        return 1;
    }

    //setup tiles
    sprites[_PAPR_].x = 0;
    sprites[_PAPR_].y = 0;
    sprites[_PAPR_].w = TILE_SIZE;
    sprites[_PAPR_].h = TILE_SIZE;
    sprites[_ROCK_].x = TILE_SIZE*1;
    sprites[_ROCK_].y = 0;
    sprites[_ROCK_].w = TILE_SIZE;
    sprites[_ROCK_].h = TILE_SIZE;
    sprites[_SCIS_].x = TILE_SIZE*2;
    sprites[_SCIS_].y = 0;
    sprites[_SCIS_].w = TILE_SIZE;
    sprites[_SCIS_].h = TILE_SIZE;
    sprites[_SELC_].x = 0;
    sprites[_SELC_].y = TILE_SIZE;
    sprites[_SELC_].w = TILE_SIZE;
    sprites[_SELC_].h = TILE_SIZE;
    sprites[_CHEK_].x = TILE_SIZE*1;
    sprites[_CHEK_].y = TILE_SIZE;
    sprites[_CHEK_].w = TILE_SIZE;
    sprites[_CHEK_].h = TILE_SIZE;
    sprites[_CROS_].x = TILE_SIZE*2;
    sprites[_CROS_].y = TILE_SIZE;
    sprites[_CROS_].w = TILE_SIZE;
    sprites[_CROS_].h = TILE_SIZE;

    //setup gamestate {turn, gameover, tie, played_rounds, rounds, marks}
    GameState gamestate = {
        false, 
        false, 
        -1,
        0,
        {_EMTY_, _EMTY_, _EMTY_,
         _ROCK_, _EMTY_, _EMTY_},
        {_EMTY_, _EMTY_, _EMTY_,
         _EMTY_, _EMTY_, _EMTY_},
        {0,0},
    };
    
    int quit = 0;
    int turnTime = 0;
    int total_rounds = 3;

    while (!quit) {
        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    quit = 1;
                    break;
            }
        }
      
        if (!gamestate.gameover) {
            int delta = SDL_GetTicks() - turnTime;
            if (delta > 200) {
                const unsigned char* keyboard = SDL_GetKeyboardState(NULL);
                if (keyboard[SDL_SCANCODE_RETURN] || keyboard[SDL_SCANCODE_KP_ENTER]) {
                    // make CPU move
                    gamestate.rounds[gamestate.played_rounds] = rand() % 3;
                    // mark round
                    if (gamestate.rounds[gamestate.played_rounds] == _ROCK_) {
                        if (gamestate.rounds[gamestate.played_rounds+total_rounds] == _ROCK_) {
                            gamestate.marks[gamestate.played_rounds] = _CROS_;
                            gamestate.marks[gamestate.played_rounds+total_rounds] = _CROS_;
                        } else if (gamestate.rounds[gamestate.played_rounds+total_rounds] == _PAPR_) {
                            gamestate.marks[gamestate.played_rounds] = _CROS_;
                            gamestate.marks[gamestate.played_rounds+total_rounds] = _CHEK_;
                            gamestate.score[0] += 1;
                        } else if (gamestate.rounds[gamestate.played_rounds+total_rounds] == _SCIS_) {
                            gamestate.marks[gamestate.played_rounds] = _CHEK_;
                            gamestate.marks[gamestate.played_rounds+total_rounds] = _CROS_;
                            gamestate.score[1] += 1;
                        }
                    } else if (gamestate.rounds[gamestate.played_rounds] == _PAPR_) {
                        if (gamestate.rounds[gamestate.played_rounds+total_rounds] == _ROCK_) {
                            gamestate.marks[gamestate.played_rounds] = _CHEK_;
                            gamestate.marks[gamestate.played_rounds+total_rounds] = _CROS_;
                            gamestate.score[1] += 1;
                        } else if (gamestate.rounds[gamestate.played_rounds+total_rounds] == _PAPR_) {
                            gamestate.marks[gamestate.played_rounds] = _CROS_;
                            gamestate.marks[gamestate.played_rounds+total_rounds] = _CROS_;
                        } else if (gamestate.rounds[gamestate.played_rounds+total_rounds] == _SCIS_) {
                            gamestate.marks[gamestate.played_rounds] = _CROS_;
                            gamestate.marks[gamestate.played_rounds+total_rounds] = _CHEK_;
                            gamestate.score[0] += 1;
                        }
                    } else if (gamestate.rounds[gamestate.played_rounds] == _SCIS_) {
                        if (gamestate.rounds[gamestate.played_rounds+total_rounds] == _ROCK_) {
                            gamestate.marks[gamestate.played_rounds] = _CROS_;
                            gamestate.marks[gamestate.played_rounds+total_rounds] = _CHEK_;
                            gamestate.score[0] += 1;
                        } else if (gamestate.rounds[gamestate.played_rounds+total_rounds] == _PAPR_) {
                            gamestate.marks[gamestate.played_rounds] = _CHEK_;
                            gamestate.marks[gamestate.played_rounds+total_rounds] = _CROS_;
                            gamestate.score[1] += 1;
                        } else if (gamestate.rounds[gamestate.played_rounds+total_rounds] == _SCIS_) {
                            gamestate.marks[gamestate.played_rounds] = _CROS_;
                            gamestate.marks[gamestate.played_rounds+total_rounds] = _CROS_;                            
                        }
                    }

                    // record player move
                    gamestate.played_rounds++;
                    // check for win condition
                    if (gamestate.score[0] > 1 || gamestate.score[1] > 1 || gamestate.played_rounds == total_rounds) {
                        if (gamestate.score[0] > gamestate.score[1]) {
                            gamestate.winner = 0;
                        } else if (gamestate.score[0] < gamestate.score[1]) {
                            gamestate.winner = 1;
                        } else {
                            gamestate.tie = true;
                        }
                        gamestate.gameover = true;
                    } else {
                        gamestate.rounds[gamestate.played_rounds+total_rounds] = _ROCK_;
                    }
                    turnTime = SDL_GetTicks();
                } else if (keyboard[SDL_SCANCODE_LEFT] || keyboard[SDL_SCANCODE_UP]) {
                    // change weapon left
                    gamestate.rounds[gamestate.played_rounds+total_rounds] -= _ROCK_;
                    if (gamestate.rounds[gamestate.played_rounds+total_rounds] < _PAPR_) 
                        gamestate.rounds[gamestate.played_rounds+total_rounds] = _SCIS_;
                    turnTime = SDL_GetTicks();
                } else if (keyboard[SDL_SCANCODE_RIGHT] || keyboard[SDL_SCANCODE_DOWN]) {
                    // change weapon right
                    gamestate.rounds[gamestate.played_rounds+total_rounds] += _ROCK_;
                    if (gamestate.rounds[gamestate.played_rounds+total_rounds] > _SCIS_)
                        gamestate.rounds[gamestate.played_rounds+total_rounds] = _PAPR_;
                    turnTime = SDL_GetTicks();
                }
            }
        }

        //clear backbuffer
        SDL_RenderClear(renderer);
        //write to backbuffer
        //remember furthest Z written first
        //symbols, then winning marks, then selector
        renderBoard(renderer, tilemap, gamestate);
        //write text     
        renderStatus(renderer, gamestate);
        //swap buffers
        SDL_RenderPresent(renderer);
        SDL_Delay(1000/60); //wait for 60ms
    }

    SDL_DestroyTexture(tilemap);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}
