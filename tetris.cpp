#include "tetris.h"

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
Texture tilemap;
GameState gs;
SDL_Rect sprites[18];
std::stringstream levelText;
std::stringstream scoreText;
std::stringstream linesText;
std::stringstream statusText;
Texture t_level;
Texture t_score;
Texture t_lines;
Texture t_status;

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
            window = SDL_CreateWindow("Tetris", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
            if (window == nullptr){
                logSDLError(std::cout, "SDL_CreateWindow");
                success = false;
            } else {
                renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
                if (renderer == nullptr){
                    logSDLError(std::cout, "SDL_CreateRenderer");
                    success = false;
                }
            }
        }
    }
    srand(rdtsc()); //seed random nicely

    return success;
}

bool load() {
    bool success = true;

    if (!tilemap.load_from_file("res/tetris.png")) {
        success = false;
    } else {
        // set standard alpha blending
        // tilemap.set_blend_mode(SDL_BLENDMODE_BLEND);
        // tilemap.set_alpha(255);
    }

    font = TTF_OpenFont("res/cc.ttf", 12);
    if (font == nullptr){
        logSDLError(std::cout, "TTF_OpenFont");
        success = false;
    }

    if (!t_level.load_from_rendered_text("_")) {
        success = false;
    }
    if (!t_score.load_from_rendered_text("_")) {
        success = false;
    }
    if (!t_lines.load_from_rendered_text("_")) {
        success = false;
    }
    if (!t_status.load_from_rendered_text("_")) {
        success = false;
    }
    
    sprites[PERIWINKLE].x = 0;
    sprites[PERIWINKLE].y = 0;
    sprites[PERIWINKLE].w = TILE_SIZE;
    sprites[PERIWINKLE].h = TILE_SIZE;
    sprites[NAVY].x = TILE_SIZE;
    sprites[NAVY].y = 0;
    sprites[NAVY].w = TILE_SIZE;
    sprites[NAVY].h = TILE_SIZE;
    sprites[ORANGE].x = TILE_SIZE*2;
    sprites[ORANGE].y = 0;
    sprites[ORANGE].w = TILE_SIZE;
    sprites[ORANGE].h = TILE_SIZE;
    sprites[YELLOW].x = TILE_SIZE*3;
    sprites[YELLOW].y = 0;
    sprites[YELLOW].w = TILE_SIZE;
    sprites[YELLOW].h = TILE_SIZE;
    sprites[GREEN].x = TILE_SIZE*4;
    sprites[GREEN].y = 0;
    sprites[GREEN].w = TILE_SIZE;
    sprites[GREEN].h = TILE_SIZE;
    sprites[PINK].x = TILE_SIZE*5;
    sprites[PINK].y = 0;
    sprites[PINK].w = TILE_SIZE;
    sprites[PINK].h = TILE_SIZE;
    sprites[RED].x = TILE_SIZE*6;
    sprites[RED].y = 0;
    sprites[RED].w = TILE_SIZE;
    sprites[RED].h = TILE_SIZE;
    sprites[PALETTE].x = TILE_SIZE*7;
    sprites[PALETTE].y = 0;
    sprites[PALETTE].w = TILE_SIZE;
    sprites[PALETTE].h = TILE_SIZE;
    sprites[UL_WALL].x = 0;
    sprites[UL_WALL].y = TILE_SIZE;
    sprites[UL_WALL].w = TILE_SIZE;
    sprites[UL_WALL].h = TILE_SIZE;
    sprites[TOP_WALL].x = TILE_SIZE;
    sprites[TOP_WALL].y = TILE_SIZE;
    sprites[TOP_WALL].w = TILE_SIZE;
    sprites[TOP_WALL].h = TILE_SIZE;
    sprites[UR_WALL].x = TILE_SIZE*2;
    sprites[UR_WALL].y = TILE_SIZE;
    sprites[UR_WALL].w = TILE_SIZE;
    sprites[UR_WALL].h = TILE_SIZE;
    sprites[LEFT_WALL].x = TILE_SIZE*3;
    sprites[LEFT_WALL].y = TILE_SIZE;
    sprites[LEFT_WALL].w = TILE_SIZE;
    sprites[LEFT_WALL].h = TILE_SIZE;
    sprites[EMPTY].x = TILE_SIZE*4;
    sprites[EMPTY].y = TILE_SIZE;
    sprites[EMPTY].w = TILE_SIZE;
    sprites[EMPTY].h = TILE_SIZE;
    sprites[RIGHT_WALL].x = TILE_SIZE*5;
    sprites[RIGHT_WALL].y = TILE_SIZE;
    sprites[RIGHT_WALL].w = TILE_SIZE;
    sprites[RIGHT_WALL].h = TILE_SIZE;
    sprites[LL_WALL].x = TILE_SIZE*6;
    sprites[LL_WALL].y = TILE_SIZE;
    sprites[LL_WALL].w = TILE_SIZE;
    sprites[LL_WALL].h = TILE_SIZE;
    sprites[BOTTOM_WALL].x = TILE_SIZE*7;
    sprites[BOTTOM_WALL].y = TILE_SIZE;
    sprites[BOTTOM_WALL].w = TILE_SIZE;
    sprites[BOTTOM_WALL].h = TILE_SIZE;
    sprites[LR_WALL].x = 0;
    sprites[LR_WALL].y = TILE_SIZE*2;
    sprites[LR_WALL].w = TILE_SIZE;
    sprites[LR_WALL].h = TILE_SIZE;
    sprites[BLANK].x = TILE_SIZE;
    sprites[BLANK].y = TILE_SIZE*2;
    sprites[BLANK].w = TILE_SIZE;
    sprites[BLANK].h = TILE_SIZE;

    return success;
}

bool close() {
    tilemap.free();
    t_level.free();
    t_score.free();
    t_lines.free();
    t_status.free();
    if (font != nullptr) { TTF_CloseFont(font);font = NULL; }
    if (renderer != nullptr) { SDL_DestroyRenderer(renderer); renderer = NULL; }
    if (window != nullptr) { SDL_DestroyWindow(window); window = NULL; }
    TTF_Quit();
    SDL_Quit();
}

// class Tetrimino {

// }

// class Tile {

// }

void render_status() {
    // std::cout << "Level: " << gs.level << " " << gs.score << " Lines: " << gs.lines << std::endl;
    // level
    levelText.str("");
    levelText << "Level: " << gs.level;
    t_level.load_from_rendered_text(levelText.str());
    t_level.render(TILE_SIZE/2, SCREEN_HEIGHT-TILE_SIZE/2);
    // t_level.render(0, 0);

    // score
    scoreText.str("");
    scoreText << gs.score;
    t_score.load_from_rendered_text(scoreText.str());
    t_score.render((SCREEN_WIDTH/2)-(t_score.get_width()/2), SCREEN_HEIGHT-(TILE_SIZE/2));
    // t_score.render(0, 0);

    // lines
    linesText.str("");
    linesText << "Lines: " << gs.lines;
    t_lines.load_from_rendered_text(linesText.str());
    t_lines.render(SCREEN_WIDTH-(TILE_SIZE/2)-t_lines.get_width(), SCREEN_HEIGHT-(TILE_SIZE/2));
    // t_lines.render(0, 0);
}

void render_well() {
    for (int y = 0; y < SCREEN_TILE_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_TILE_WIDTH; x++) {
            if (y >= 2) {
                if (y == 2) {
                    if (x == 1) {
                        tilemap.render(x*TILE_SIZE, y*TILE_SIZE, &sprites[UL_WALL]);
                    } else if (x == SCREEN_TILE_WIDTH-2) {
                        tilemap.render(x*TILE_SIZE, y*TILE_SIZE, &sprites[UR_WALL]);
                    } else if (x > 1 && x < SCREEN_TILE_WIDTH-2) {
                        // tilemap.render(x*TILE_SIZE, y*TILE_SIZE, &sprites[TOP_WALL]);
                    }
                }
                if (y > 2 && y < SCREEN_TILE_HEIGHT-3 ) {
                    if (x == 1) {
                        tilemap.render(x*TILE_SIZE, y*TILE_SIZE, &sprites[LEFT_WALL]);
                    }
                    if (x == SCREEN_TILE_WIDTH-2) {
                        tilemap.render(x*TILE_SIZE, y*TILE_SIZE, &sprites[RIGHT_WALL]);
                    }
                }
                if (y == SCREEN_TILE_HEIGHT-3) {
                    if (x == 1) {
                        tilemap.render(x*TILE_SIZE, y*TILE_SIZE, &sprites[LL_WALL]);
                    } else if (x == SCREEN_TILE_WIDTH-2) {
                        tilemap.render(x*TILE_SIZE, y*TILE_SIZE, &sprites[LR_WALL]);
                    } else if (x > 1 && x < SCREEN_TILE_WIDTH-2) {
                        tilemap.render(x*TILE_SIZE, y*TILE_SIZE, &sprites[BOTTOM_WALL]);
                    }
                }
            }
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
            bool quit = false;
            int collision_debug = 0;
            int delta_ticks = 0;
            int frame_ticks = 0;

            //starting initialization
            gs.state = START;
            gs.level = 0;
            gs.score = 0;
            gs.lines = 0;

            while (!quit) {
                frame_ticks = SDL_GetTicks();
                SDL_Event event;

                while (SDL_PollEvent(&event)) {
                    if (event.type == SDL_QUIT) {
                            quit = true;
                    }
                    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE ) {
                        quit = true;
                    }
                }
                //Initialize renderer color (also used for clearing)
                // SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF); //white
                SDL_SetRenderDrawColor(renderer, 0x0, 0x0, 0x0, 0xFF); //black
                SDL_RenderClear(renderer);
                render_well();
                render_status();          
                // tilemap.set_color(r,g,b);
                // tilemap.set_alpha(a);
                // tilemap.render();
                // tilemap.render(SCREEN_WIDTH/2, SCREEN_HEIGHT/2, &sprites[RED]);
                // tilemap.render(x, y, &sprites[i], NULL, degrees, NULL, flipType);
                SDL_RenderPresent(renderer); 

                //already using delta for calculating position, now using to cap fps at 60
                delta_ticks = SDL_GetTicks() - frame_ticks;
                if (delta_ticks < SCREEN_TICKS_PER_FRAME) {
                    SDL_Delay(SCREEN_TICKS_PER_FRAME - delta_ticks);
                }
            }
        }
    }
    close();
    return 0;
}
