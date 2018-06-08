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
#define _VERT_ (8)
#define _DIAR_ (9)

SDL_Rect sprites[8];

SDL_Color WHITE = { 255, 255, 255, 255 };

struct GameState {
    bool turn; //which player's turn?
    bool gameover; //is the game over?
    bool tie; //last player active is the winner, what if it's a tie?
    int marked_spaces;
    int board[9];
    int marks[9];
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
    
    if (gamestate.gameover && !gamestate.tie) {
        //draw winning marks
        for (int y = 0; y < 3; y++) {
            for (int x = 0; x < 3; x++) {
                switch (gamestate.marks[3*y+x]) {
                    case _HORZ_:
                        renderTexture(tilemap, ren, x*TILE_SIZE+TILE_SIZE, y*TILE_SIZE+TILE_SIZE, &sprites[_HORZ_]);
                        break;
                    case _VERT_:
                        renderTextureEx(tilemap, ren, x*TILE_SIZE+TILE_SIZE, y*TILE_SIZE+TILE_SIZE, &sprites[_HORZ_], 90, NULL);
                        break;
                    case _DIAG_:
                        renderTexture(tilemap, ren, x*TILE_SIZE+TILE_SIZE, y*TILE_SIZE+TILE_SIZE, &sprites[_DIAG_]);
                        break;
                    case _DIAR_:
                        renderTextureEx(tilemap, ren, x*TILE_SIZE+TILE_SIZE, y*TILE_SIZE+TILE_SIZE, &sprites[_DIAG_], 90, NULL);
                        break;
                    case _EMTY_:
                    default:
                        break;
                }
            }
        }
    }
}

void renderStatus(SDL_Renderer *ren, GameState &gamestate) {
    if (!gamestate.gameover) {
        if (gamestate.turn) {
            SDL_Texture* image = loadText_to_Texture("It's my turn!", ren);
            int iW, iH;
            SDL_QueryTexture(image, NULL, NULL, &iW, &iH);
            renderTexture(image, ren, SCREEN_WIDTH/2 - iW/2, SCREEN_HEIGHT-TILE_SIZE/2);
        } else {
            SDL_Texture* image = loadText_to_Texture("It's the human's turn.", ren);
            int iW, iH;
            SDL_QueryTexture(image, NULL, NULL, &iW, &iH);
            renderTexture(image, ren, SCREEN_WIDTH/2 - iW/2, SCREEN_HEIGHT-TILE_SIZE/2);
        }
    } else {
        if (gamestate.tie) {
            SDL_Texture* image = loadText_to_Texture("A tie.", ren);
            int iW, iH;
            SDL_QueryTexture(image, NULL, NULL, &iW, &iH);
            renderTexture(image, ren, SCREEN_WIDTH/2 - iW/2, SCREEN_HEIGHT-TILE_SIZE/2);
        } else if (gamestate.turn) {            
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

    //setup gamestate {turn, gameover, tie, marked_spaces, board, marks}
    GameState gamestate = {
        false, 
        false, 
        false,
        0,
        {_EMTY_, _EMTY_, _EMTY_,
         _EMTY_, _EMTY_, _EMTY_,
         _EMTY_, _EMTY_, _EMTY_},
        {_EMTY_, _EMTY_, _EMTY_,
         _EMTY_, _EMTY_, _EMTY_,
         _EMTY_, _EMTY_, _EMTY_},
    };
    
    int quit = 0;
    int turnTime = 0;

    while (!quit) {
        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    quit = 1;
                    break;
            /*
                case SDL_KEYDOWN:
                    switch (event.key.keysym.scancode) {
                        case SDL_SCANCODE_W:
                        case SDL_SCANCODE_UP:
                            break;
                        case SDL_SCANCODE_A:
                        case SDL_SCANCODE_LEFT:
                            break;
                        case SDL_SCANCODE_S:
                        case SDL_SCANCODE_DOWN:
                            break;
                        case SDL_SCANCODE_D:
                        case SDL_SCANCODE_RIGHT:
                            break;
                    }
                    break;

                case SDL_KEYUP:
                    switch (event.key.keysym.scancode) {
                    case SDL_SCANCODE_W:
                    case SDL_SCANCODE_UP:
                        break;
                    case SDL_SCANCODE_A:
                    case SDL_SCANCODE_LEFT:
                        break;
                    case SDL_SCANCODE_S:
                    case SDL_SCANCODE_DOWN:
                        break;
                    case SDL_SCANCODE_D:
                    case SDL_SCANCODE_RIGHT:
                        break;
                    }
                    break;
            */
            }
        }

        if (!gamestate.gameover) {
            if (!gamestate.turn) {
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
                    if (buttons & SDL_BUTTON(SDL_BUTTON_LEFT)) {
                        // add an X if the mouse is over a tile and left button is pressed
                        int move = 3*y+x;
                        if (gamestate.board[move] == _EMTY_) {
                            gamestate.board[move] = _X_;
                            gamestate.marked_spaces += 1;
                            gamestate.turn = !gamestate.turn;
                            turnTime = SDL_GetTicks();
                        }
                    } 
                /*
                    else if (buttons & SDL_BUTTON(SDL_BUTTON_RIGHT)) {
                        // add an O if the mouse is over a tile and right button is pressed
                        gamestate.board[3*y+x] = _O_;
                    }
                */
                }
            } else {
                // handle mouse "jitter"
                int delta = SDL_GetTicks() - turnTime;
                if (delta > 300) {
                    //Player 2 (AI) Turn
                    int move = -1;
                    //random move on open spot
                    do {
                        move = rand() % 9;
                    } while (gamestate.board[move] != _EMTY_);
                    gamestate.board[move] = _O_;
                    gamestate.marked_spaces += 1;
                    gamestate.turn = !gamestate.turn;
                    //TODO: killer AI, never lose
                }
            }

            /*
            0, 1, 2
            3, 4, 5
            6, 7, 8
            */

            //check for win condition
            //if there is three in a row of a single mark
            //that mark's player is the winner (currently has the turn)
            //mark the winning triple
            if (gamestate.board[0] != _EMTY_ && gamestate.board[0] == gamestate.board[1] && gamestate.board[0] == gamestate.board[2]) {
                // top row 
                gamestate.gameover = true;
                gamestate.marks[0] = _HORZ_;
                gamestate.marks[1] = _HORZ_;
                gamestate.marks[2] = _HORZ_;
            } else if (gamestate.board[3] != _EMTY_ && gamestate.board[3] == gamestate.board[4] && gamestate.board[3] == gamestate.board[5]) {
                // middle row
                gamestate.gameover = true;
                gamestate.marks[3] = _HORZ_;
                gamestate.marks[4] = _HORZ_;
                gamestate.marks[5] = _HORZ_;            
            } else if (gamestate.board[6] != _EMTY_ && gamestate.board[6] == gamestate.board[7] && gamestate.board[6] == gamestate.board[8]) {
                // bottom row
                gamestate.gameover = true;
                gamestate.marks[6] = _HORZ_;
                gamestate.marks[7] = _HORZ_;
                gamestate.marks[8] = _HORZ_;            
            } else if (gamestate.board[0] != _EMTY_ && gamestate.board[0] == gamestate.board[3] && gamestate.board[0] == gamestate.board[6]) {
                // left column
                gamestate.gameover = true;
                gamestate.marks[0] = _VERT_;
                gamestate.marks[3] = _VERT_;
                gamestate.marks[6] = _VERT_;
            } else if (gamestate.board[1] != _EMTY_ && gamestate.board[1] == gamestate.board[4] && gamestate.board[1] == gamestate.board[7]) {
                // center column
                gamestate.gameover = true;
                gamestate.marks[1] = _VERT_;
                gamestate.marks[4] = _VERT_;
                gamestate.marks[7] = _VERT_;
            } else if (gamestate.board[2] != _EMTY_ && gamestate.board[2] == gamestate.board[5] && gamestate.board[2] == gamestate.board[8]) {
                // right column
                gamestate.gameover = true;
                gamestate.marks[2] = _VERT_;
                gamestate.marks[5] = _VERT_;
                gamestate.marks[8] = _VERT_;
            } else if (gamestate.board[0] != _EMTY_ && gamestate.board[0] == gamestate.board[4] && gamestate.board[0] == gamestate.board[8]) {
                // ul to lr diag
                gamestate.gameover = true;
                gamestate.marks[0] = _DIAR_;
                gamestate.marks[4] = _DIAR_;
                gamestate.marks[8] = _DIAR_;
            } else if (gamestate.board[6] != _EMTY_ && gamestate.board[6] == gamestate.board[4] && gamestate.board[6] == gamestate.board[2]) {
                // ll to ur diag
                gamestate.gameover = true;
                gamestate.marks[6] = _DIAG_;
                gamestate.marks[4] = _DIAG_;
                gamestate.marks[2] = _DIAG_;
            } else if (gamestate.marked_spaces == 9) {
                //else if all the spaces are full
                //it's a tie
                gamestate.tie = true;
                gamestate.gameover = true;
            }

            if (gamestate.gameover) {
                //flip the turn indicator back to the winner
                gamestate.turn = !gamestate.turn;
            }
        }

        // std::cout << "over: " << gamestate.gameover << " tie: " << gamestate.tie << " turn: " << gamestate.turn << std::endl;

        //clear backbuffer
        SDL_RenderClear(renderer);
        //write to backbuffer
        //remember furthest Z written first
        //write board, symbols, then winning marks
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
