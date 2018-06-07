#include <iostream>
#include <SDL.h>
#include <SDL_timer.h>
#include <SDL_image.h>

const int TILE_SIZE = 32;

const int SCREEN_WIDTH = TILE_SIZE*5;
const int SCREEN_HEIGHT = TILE_SIZE*5;

//tilemap indexes
#define    _X_ (0)
#define _HORZ_ (1)
#define _DIAG_ (2)
#define    _O_ (3)
#define _CENT_ (4)
#define _EDGE_ (5)
#define _CRNR_ (6)
#define _EMTY_ (7)

SDL_Rect sprites[8];

struct GameState {
    bool turn; //which player's turn?
    bool gameover; //is the game over?
    bool tie; //last player active is the winner, what if it's a tie?
    int board[9];
    int marks[9];
};

void logSDLError(std::ostream &os, const std::string &msg) {
    os << msg << " SDL Error: " << SDL_GetError() << std::endl;
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
    //draw base
    for (int y = 0; y < 5; y++) {
        for (int x = 0; x < 5; x++) {
            if ( x == 0 || x == 4 || y == 0 || y == 4 ) {//draw border                
                renderTexture(tilemap, ren, x*TILE_SIZE, y*TILE_SIZE, &sprites[_EMTY_]);
            } else if ( x == 1 ) {                
                if ( y == 1 ) //draw UL Corner
                    renderTextureEx(tilemap, ren, x*TILE_SIZE, y*TILE_SIZE, &sprites[_CRNR_]);
                if ( y == 2 ) //draw L  Edge    //flip horizontal
                    renderTextureEx(tilemap, ren, x*TILE_SIZE, y*TILE_SIZE, &sprites[_EDGE_], 0, NULL, SDL_FLIP_HORIZONTAL); 
                if ( y == 3 ) //draw LL Corner  //rotate 90 left
                    renderTextureEx(tilemap, ren, x*TILE_SIZE, y*TILE_SIZE, &sprites[_CRNR_], -90, NULL); 
            } else if ( x == 2 ) {
                if ( y == 1 ) //draw T  Edge    //rotate 90 left
                    renderTextureEx(tilemap, ren, x*TILE_SIZE, y*TILE_SIZE, &sprites[_EDGE_], -90, NULL); 
                if ( y == 2 ) //draw Center
                    renderTexture(tilemap, ren, x*TILE_SIZE, y*TILE_SIZE, &sprites[_CENT_]);
                if ( y == 3 ) //draw B  Edge    //rotate 90 right
                    renderTextureEx(tilemap, ren, x*TILE_SIZE, y*TILE_SIZE, &sprites[_EDGE_], 90, NULL);
            } else if ( x == 3 ) {
                if ( y == 1 ) //draw UR Corner  //flip horizontal
                    renderTextureEx(tilemap, ren, x*TILE_SIZE, y*TILE_SIZE, &sprites[_CRNR_], 0, NULL, SDL_FLIP_HORIZONTAL);
                if ( y == 2 ) //draw R  Edge
                    renderTextureEx(tilemap, ren, x*TILE_SIZE, y*TILE_SIZE, &sprites[_EDGE_]);
                if ( y == 3 ) //draw LR Corner  //rotate 180 left/right
                    renderTextureEx(tilemap, ren, x*TILE_SIZE, y*TILE_SIZE, &sprites[_CRNR_], 180, NULL);
            }
        }
    }

    //draw player marks
    for (int y = 0; y < 3; y++) {
        for (int x = 0; x < 3; x++) {
            switch (gamestate.board[3*y+x]) {
                case _X_:
                    renderTexture(tilemap, ren, x*TILE_SIZE+TILE_SIZE, y*TILE_SIZE+TILE_SIZE, &sprites[_X_]);
                    break;
                case _O_:
                    renderTexture(tilemap, ren, x*TILE_SIZE+TILE_SIZE, y*TILE_SIZE+TILE_SIZE, &sprites[_O_]);
                    break;
                case _EMTY_:
                default:
                    break;
            }
        }
    }
    
    if (gamestate.gameover and ! gamestate.tie) {
        //draw wining marks
        for (int y = 0; y < 3; y++) {
            for (int x = 0; x < 3; x++) {
                switch (gamestate.board[3*y+x]) {
                    case _HORZ_:
                        renderTexture(tilemap, ren, x*TILE_SIZE+TILE_SIZE, y*TILE_SIZE+TILE_SIZE, &sprites[_HORZ_]);
                        break;
                    case _DIAG_:
                        renderTexture(tilemap, ren, x*TILE_SIZE+TILE_SIZE, y*TILE_SIZE+TILE_SIZE, &sprites[_DIAG_]);
                        break;
                    case _EMTY_:
                    default:
                        break;
                }
            }
        }
    }
}

int main(int argc, char **argv) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0){
        logSDLError(std::cout, "SDL_Init");
        return 1;
    }
    
    SDL_Window *window = SDL_CreateWindow("Tic-Tac-Toe", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
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

    std::string imagePath = "res/tictactoe.png";
    SDL_Texture *tilemap = IMG_LoadTexture(renderer, imagePath.c_str());
    if (tilemap == nullptr){
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        logSDLError(std::cout, "IMG_LoadTexture");
        SDL_Quit();
        return 1;
    }

    //setup tiles
    sprites[_X_].x = 0;
    sprites[_X_].y = 0;
    sprites[_X_].w = TILE_SIZE;
    sprites[_X_].h = TILE_SIZE;
    sprites[_HORZ_].x = TILE_SIZE*1;
    sprites[_HORZ_].y = 0;
    sprites[_HORZ_].w = TILE_SIZE;
    sprites[_HORZ_].h = TILE_SIZE;
    sprites[_DIAG_].x = TILE_SIZE*2;
    sprites[_DIAG_].y = 0;
    sprites[_DIAG_].w = TILE_SIZE;
    sprites[_DIAG_].h = TILE_SIZE;
    sprites[_O_].x = 0;
    sprites[_O_].y = TILE_SIZE;
    sprites[_O_].w = TILE_SIZE;
    sprites[_O_].h = TILE_SIZE;
    sprites[_CENT_].x = TILE_SIZE*1;
    sprites[_CENT_].y = TILE_SIZE;
    sprites[_CENT_].w = TILE_SIZE;
    sprites[_CENT_].h = TILE_SIZE;
    sprites[_EDGE_].x = TILE_SIZE*2;
    sprites[_EDGE_].y = TILE_SIZE;
    sprites[_EDGE_].w = TILE_SIZE;
    sprites[_EDGE_].h = TILE_SIZE;
    sprites[_CRNR_].x = 0;
    sprites[_CRNR_].y = TILE_SIZE*2;
    sprites[_CRNR_].w = TILE_SIZE;
    sprites[_CRNR_].h = TILE_SIZE;
    sprites[_EMTY_].x = TILE_SIZE*1;
    sprites[_EMTY_].y = TILE_SIZE*2;
    sprites[_EMTY_].w = TILE_SIZE;
    sprites[_EMTY_].h = TILE_SIZE;

    //setup gamestate {turn, gameover, tie, board, marks}
    GameState gamestate = {
        false, 
        false, 
        false,
        {_EMTY_, _EMTY_, _EMTY_,
         _EMTY_, _EMTY_, _EMTY_,
         _EMTY_, _EMTY_, _EMTY_},
        {_EMTY_, _EMTY_, _EMTY_,
         _EMTY_, _EMTY_, _EMTY_,
         _EMTY_, _EMTY_, _EMTY_},
    };

    //TODO: AI for player2
    //TODO: setup text
    //TODO: Menu Screen
    
    int quit = 0;

    while (!quit) {
        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    quit = 1;
                    break;

                // case SDL_KEYDOWN:
                //     switch (event.key.keysym.scancode) {
                //         case SDL_SCANCODE_W:
                //         case SDL_SCANCODE_UP:
                //             break;
                //         case SDL_SCANCODE_A:
                //         case SDL_SCANCODE_LEFT:
                //             break;
                //         case SDL_SCANCODE_S:
                //         case SDL_SCANCODE_DOWN:
                //             break;
                //         case SDL_SCANCODE_D:
                //         case SDL_SCANCODE_RIGHT:
                //             break;
                //     }
                //     break;

                // case SDL_KEYUP:
                //     switch (event.key.keysym.scancode) {
                //     case SDL_SCANCODE_W:
                //     case SDL_SCANCODE_UP:
                //         break;
                //     case SDL_SCANCODE_A:
                //     case SDL_SCANCODE_LEFT:
                //         break;
                //     case SDL_SCANCODE_S:
                //     case SDL_SCANCODE_DOWN:
                //         break;
                //     case SDL_SCANCODE_D:
                //     case SDL_SCANCODE_RIGHT:
                //         break;
                //     }
                //     break;
            }
        }

        //handle mouse events
        int mouse_x, mouse_y;
        int buttons = SDL_GetMouseState(&mouse_x, &mouse_y);
        int x = -1;
        int y = -1;

        if ( mouse_x >= TILE_SIZE && mouse_x < TILE_SIZE*2 ) {
            x = 0;
        } else if ( mouse_x >= TILE_SIZE*2 && mouse_x < TILE_SIZE*3 ) {
            x = 1;
        } else if ( mouse_x >= TILE_SIZE*3 && mouse_x < TILE_SIZE*4 ) {
            x = 2;
        }

        if (mouse_y >= TILE_SIZE && mouse_y < TILE_SIZE*2 ) {
            y = 0;
        } else if ( mouse_y >= TILE_SIZE*2 && mouse_y < TILE_SIZE*3 ) {
            y = 1;
        } else if (mouse_y >= TILE_SIZE*3 && mouse_y < TILE_SIZE*4 ) {
            y = 2;
        }

        if ( x > -1 && y > -1) {
            if (buttons & SDL_BUTTON(SDL_BUTTON_LEFT) && !gamestate.turn) {
                // add an X if the mouse is over a tile and left button is pressed
                gamestate.board[3*y+x] = _X_;
            } else if (buttons & SDL_BUTTON(SDL_BUTTON_RIGHT) && gamestate.turn) {
                // add an O if the mouse is over a tile and right button is pressed
                gamestate.board[3*y+x] = _O_;
            }
        }

        //clear backbuffer
        SDL_RenderClear(renderer);
        //write to backbuffer
        //remember furthest Z written first
        //write board, symbols, then winning marks
        renderBoard(renderer, tilemap, gamestate);
        //write text
        // SDL_RenderCopy(renderer, tilemap, NULL, NULL);
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
