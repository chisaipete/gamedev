#include "tetris.h"

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
Texture tilemap;
GameState gs;
SDL_Rect sprites[18];
Timer frame_timer;
Timer fps_timer;
std::stringstream levelText;
std::stringstream scoreText;
std::stringstream linesText;
std::stringstream statusText;
std::stringstream fpsText;
Texture t_level;
Texture t_score;
Texture t_lines;
Texture t_status;
Texture t_fps;
Bag seven_bag;

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
    if (!t_fps.load_from_rendered_text("_", RED_)) {
        success = false;
    }
    
    sprites[CYAN].x = 0;
    sprites[CYAN].y = 0;
    sprites[CYAN].w = BLOCK_SIZE;
    sprites[CYAN].h = BLOCK_SIZE;
    sprites[BLUE].x = BLOCK_SIZE;
    sprites[BLUE].y = 0;
    sprites[BLUE].w = BLOCK_SIZE;
    sprites[BLUE].h = BLOCK_SIZE;
    sprites[ORANGE].x = BLOCK_SIZE*2;
    sprites[ORANGE].y = 0;
    sprites[ORANGE].w = BLOCK_SIZE;
    sprites[ORANGE].h = BLOCK_SIZE;
    sprites[YELLOW].x = BLOCK_SIZE*3;
    sprites[YELLOW].y = 0;
    sprites[YELLOW].w = BLOCK_SIZE;
    sprites[YELLOW].h = BLOCK_SIZE;
    sprites[GREEN].x = BLOCK_SIZE*4;
    sprites[GREEN].y = 0;
    sprites[GREEN].w = BLOCK_SIZE;
    sprites[GREEN].h = BLOCK_SIZE;
    sprites[PURPLE].x = BLOCK_SIZE*5;
    sprites[PURPLE].y = 0;
    sprites[PURPLE].w = BLOCK_SIZE;
    sprites[PURPLE].h = BLOCK_SIZE;
    sprites[RED].x = BLOCK_SIZE*6;
    sprites[RED].y = 0;
    sprites[RED].w = BLOCK_SIZE;
    sprites[RED].h = BLOCK_SIZE;
    sprites[PALETTE].x = BLOCK_SIZE*7;
    sprites[PALETTE].y = 0;
    sprites[PALETTE].w = BLOCK_SIZE;
    sprites[PALETTE].h = BLOCK_SIZE;
    sprites[UL_WALL].x = 0;
    sprites[UL_WALL].y = BLOCK_SIZE;
    sprites[UL_WALL].w = BLOCK_SIZE;
    sprites[UL_WALL].h = BLOCK_SIZE;
    sprites[TOP_WALL].x = BLOCK_SIZE;
    sprites[TOP_WALL].y = BLOCK_SIZE;
    sprites[TOP_WALL].w = BLOCK_SIZE;
    sprites[TOP_WALL].h = BLOCK_SIZE;
    sprites[UR_WALL].x = BLOCK_SIZE*2;
    sprites[UR_WALL].y = BLOCK_SIZE;
    sprites[UR_WALL].w = BLOCK_SIZE;
    sprites[UR_WALL].h = BLOCK_SIZE;
    sprites[LEFT_WALL].x = BLOCK_SIZE*3;
    sprites[LEFT_WALL].y = BLOCK_SIZE;
    sprites[LEFT_WALL].w = BLOCK_SIZE;
    sprites[LEFT_WALL].h = BLOCK_SIZE;
    sprites[EMPTY].x = BLOCK_SIZE*4;
    sprites[EMPTY].y = BLOCK_SIZE;
    sprites[EMPTY].w = BLOCK_SIZE;
    sprites[EMPTY].h = BLOCK_SIZE;
    sprites[RIGHT_WALL].x = BLOCK_SIZE*5;
    sprites[RIGHT_WALL].y = BLOCK_SIZE;
    sprites[RIGHT_WALL].w = BLOCK_SIZE;
    sprites[RIGHT_WALL].h = BLOCK_SIZE;
    sprites[LL_WALL].x = BLOCK_SIZE*6;
    sprites[LL_WALL].y = BLOCK_SIZE;
    sprites[LL_WALL].w = BLOCK_SIZE;
    sprites[LL_WALL].h = BLOCK_SIZE;
    sprites[BOTTOM_WALL].x = BLOCK_SIZE*7;
    sprites[BOTTOM_WALL].y = BLOCK_SIZE;
    sprites[BOTTOM_WALL].w = BLOCK_SIZE;
    sprites[BOTTOM_WALL].h = BLOCK_SIZE;
    sprites[LR_WALL].x = 0;
    sprites[LR_WALL].y = BLOCK_SIZE*2;
    sprites[LR_WALL].w = BLOCK_SIZE;
    sprites[LR_WALL].h = BLOCK_SIZE;
    sprites[BLANK].x = BLOCK_SIZE;
    sprites[BLANK].y = BLOCK_SIZE*2;
    sprites[BLANK].w = BLOCK_SIZE;
    sprites[BLANK].h = BLOCK_SIZE;

    return success;
}

void delete_blocks() {
    for (int x = 0; x < WELL_BLOCK_WIDTH; x++) {
        for (int y = 0; y < WELL_BLOCK_HEIGHT; y++) {
            if (gs.blocks[WELL_BLOCK_WIDTH*y+x] != NULL) {
                delete gs.blocks[WELL_BLOCK_WIDTH*y+x];
                gs.blocks[WELL_BLOCK_WIDTH*y+x] = NULL;
            }
        }
    }
}

bool close() {
    delete &seven_bag;
    // delete &frame_timer;
    delete_blocks();
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

void render_status() {
    // std::cout << "Level: " << gs.level << " " << gs.score << " Lines: " << gs.lines << std::endl;
    // level
    levelText.str("");
    levelText << "Level: " << gs.level;
    t_level.load_from_rendered_text(levelText.str());
    t_level.render(BLOCK_SIZE/2, SCREEN_HEIGHT-BLOCK_SIZE/2);
    // t_level.render(0, 0);

    // score
    scoreText.str("");
    scoreText << gs.score;
    t_score.load_from_rendered_text(scoreText.str());
    t_score.render((SCREEN_WIDTH/2)-(t_score.get_width()/2), SCREEN_HEIGHT-(BLOCK_SIZE/2));
    // t_score.render(0, 0);

    // lines
    linesText.str("");
    linesText << "Lines: " << gs.lines;
    t_lines.load_from_rendered_text(linesText.str());
    t_lines.render(SCREEN_WIDTH-(BLOCK_SIZE/2)-t_lines.get_width(), SCREEN_HEIGHT-(BLOCK_SIZE/2));
    // t_lines.render(0, 0);
}

void render_well(bool debug = false) {
    //blocks
    for (int x = 0; x < WELL_BLOCK_WIDTH; x++) {
        for (int y = 0; y < WELL_BLOCK_HEIGHT; y++) {
            if (gs.blocks[WELL_BLOCK_WIDTH*y+x] != NULL) {
                tilemap.render((x+2)*BLOCK_SIZE, (y+1)*BLOCK_SIZE, &sprites[gs.blocks[WELL_BLOCK_WIDTH*y+x]->color]);

            } //TODO: make sure render offsets match the blocks array
            if (debug) {
                SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF); //white
                SDL_Rect quad = {(x+2)*BLOCK_SIZE, (y+1)*BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE};
                SDL_RenderDrawRect(renderer, &quad);
                SDL_SetRenderDrawColor(renderer, 0x0, 0x0, 0x0, 0xFF); //black    
            }
        }
    }
    //well
    for (int y = 0; y < SCREEN_BLOCK_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_BLOCK_WIDTH; x++) {
            if (y >= 2) {
                if (y == 2) {
                    if (x == 1) {
                        tilemap.render(x*BLOCK_SIZE, y*BLOCK_SIZE, &sprites[UL_WALL]);
                    } else if (x == SCREEN_BLOCK_WIDTH-2) {
                        tilemap.render(x*BLOCK_SIZE, y*BLOCK_SIZE, &sprites[UR_WALL]);
                    } else if (x > 1 && x < SCREEN_BLOCK_WIDTH-2) {
                        // tilemap.render(x*BLOCK_SIZE, y*BLOCK_SIZE, &sprites[TOP_WALL]);
                    }
                }
                if (y > 2 && y < SCREEN_BLOCK_HEIGHT-2 ) {
                    if (x == 1) {
                        tilemap.render(x*BLOCK_SIZE, y*BLOCK_SIZE, &sprites[LEFT_WALL]);
                    }
                    if (x == SCREEN_BLOCK_WIDTH-2) {
                        tilemap.render(x*BLOCK_SIZE, y*BLOCK_SIZE, &sprites[RIGHT_WALL]);
                    }
                }
                if (y == SCREEN_BLOCK_HEIGHT-2) {
                    if (x == 1) {
                        tilemap.render(x*BLOCK_SIZE, y*BLOCK_SIZE, &sprites[LL_WALL]);
                    } else if (x == SCREEN_BLOCK_WIDTH-2) {
                        tilemap.render(x*BLOCK_SIZE, y*BLOCK_SIZE, &sprites[LR_WALL]);
                    } else if (x > 1 && x < SCREEN_BLOCK_WIDTH-2) {
                        tilemap.render(x*BLOCK_SIZE, y*BLOCK_SIZE, &sprites[BOTTOM_WALL]);
                    }
                }
            }
        }
    }
}

void move_piece(Direction d) {
    //check for collisions with other pieces or the bottom
    //if it hits the bottom, release to board
    v2 new_ulpt;
    v2 poso;
    v2 posn[4];
    v2 tmp;
    switch (d) {
        case LEFT: new_ulpt = gs.piece.ulpt + V2(-1,0); break;
        case RIGHT: new_ulpt = gs.piece.ulpt + V2(1,0); break;
        case DOWN: new_ulpt = gs.piece.ulpt + V2(0,1); break;
        case HARD_DOWN: 
        default:
            break;
    }
    //check collisions at new position
    //walls
    //check for hitting bottom
    for (int i = 0; i < 4; i++) {
        tmp = new_ulpt + gs.piece.rotation[i];
        // sides
        if (tmp.x < 0 || tmp.x >= WELL_BLOCK_WIDTH) {
            return;
        }
        // bottom
        if (tmp.y > 21) {
            return; // TODO: handle releasing to board
            //release to board and spawn new piece
        }
    }

    //commit new position
    for (int i = 0; i < 4; i++) {
        poso = gs.piece.ulpt + gs.piece.rotation[i];
        posn[i] = new_ulpt + gs.piece.rotation[i];
        gs.blocks[WELL_BLOCK_WIDTH*(poso.y)+(poso.x)] = NULL;
    }
    for (int i = 0; i < 4; i++) {
        gs.blocks[WELL_BLOCK_WIDTH*(posn[i].y)+(posn[i].x)] = gs.piece.blocks[i];
    }
    gs.piece.ulpt = new_ulpt;


}

bool release_piece() {
    for (int i = 0; i < 4; i++) {
        gs.piece.blocks[i] = NULL;
    }
}

void rotate_piece(bool reverse = false) {
    int xn, yn, me;
    v2 poso;
    v2 posn[4];

    me = max_ext[gs.piece.blocks[0]->color];
    //reverse rotates to the left, otherwise right
    if (!reverse) {
        for (int i = 0; i < 4; i++) {
            poso = gs.piece.ulpt + gs.piece.rotation[i];

            xn = 1 - (gs.piece.rotation[i].y - (me - 2));
            yn = gs.piece.rotation[i].x;

            gs.piece.rotation[i] = {xn, yn};
            posn[i] = gs.piece.ulpt + gs.piece.rotation[i];

            gs.blocks[WELL_BLOCK_WIDTH*(poso.y)+(poso.x)] = NULL;
        }
        for (int i = 0; i < 4; i++) {
            gs.blocks[WELL_BLOCK_WIDTH*(posn[i].y)+(posn[i].x)] = gs.piece.blocks[i];
        }
        //TODO: check collisions/kicks
        //if collide, rotate back
    } else {
        for (int i = 0; i < 4; i++){
            poso = gs.piece.ulpt + gs.piece.rotation[i];

            xn = gs.piece.rotation[i].y;
            yn = 1 - (gs.piece.rotation[i].x - (me - 2));

            gs.piece.rotation[i] = {xn, yn};
            posn[i] = gs.piece.ulpt + gs.piece.rotation[i];

            gs.blocks[WELL_BLOCK_WIDTH*(poso.y)+(poso.x)] = NULL;
        }
        for (int i = 0; i < 4; i++) {
            gs.blocks[WELL_BLOCK_WIDTH*(posn[i].y)+(posn[i].x)] = gs.piece.blocks[i];
        }
        //check collisions/kicks
        //if collide, rotate back
    }
}

bool spawn_piece() { //https://xkcd.com/888/
    bool success = true;
    // select piece using 7-in-a-Bag random generator
    // attempt to place piece at proper spawn point in well: if it collides, game over
    //TODO: correct collision detection
    Piece p = seven_bag.draw_piece();
    v2 pos;
    switch (p) {
        case I:
            gs.piece.ulpt = {4,0};
            // TODO: checking collision for all blocks
            if (gs.blocks[WELL_BLOCK_WIDTH*(gs.piece.ulpt.y+1)+(gs.piece.ulpt.x+1)] == NULL) {
                // allocating block
                for (int i = 0; i < 4; i++) {
                    gs.piece.blocks[i] = new Block;
                    gs.piece.blocks[i]->color = I;
                    switch (i) {
                        case 0:
                            gs.piece.rotation[i] = {0,1};
                            pos = gs.piece.ulpt + gs.piece.rotation[i];
                            gs.blocks[WELL_BLOCK_WIDTH*(pos.y)+(pos.x)] = gs.piece.blocks[i];
                            break;
                        case 1:
                            gs.piece.rotation[i] = {1,1};
                            pos = gs.piece.ulpt + gs.piece.rotation[i];
                            gs.blocks[WELL_BLOCK_WIDTH*(pos.y)+(pos.x)] = gs.piece.blocks[i];
                            break;
                        case 2:
                            gs.piece.rotation[i] = {2,1};
                            pos = gs.piece.ulpt + gs.piece.rotation[i];
                            gs.blocks[WELL_BLOCK_WIDTH*(pos.y)+(pos.x)] = gs.piece.blocks[i];
                            break;
                        case 3:
                            gs.piece.rotation[i] = {3,1};
                            pos = gs.piece.ulpt + gs.piece.rotation[i];
                            gs.blocks[WELL_BLOCK_WIDTH*(pos.y)+(pos.x)] = gs.piece.blocks[i];
                            break;
                    }
                }
            } else {
                success = false;
            }
            break;
        case J:
           gs.piece.ulpt = {4,0};
            // checking collision
            if (gs.blocks[WELL_BLOCK_WIDTH*(gs.piece.ulpt.y+1)+(gs.piece.ulpt.x+1)] == NULL) {
                // allocating block
                for (int i = 0; i < 4; i++) {
                    gs.piece.blocks[i] = new Block;
                    gs.piece.blocks[i]->color = J;
                    switch (i) {
                        case 0:
                            gs.piece.rotation[i] = {0,0};
                            pos = gs.piece.ulpt + gs.piece.rotation[i];
                            gs.blocks[WELL_BLOCK_WIDTH*(pos.y)+(pos.x)] = gs.piece.blocks[i];
                            break;
                        case 1:
                            gs.piece.rotation[i] = {0,1};
                            pos = gs.piece.ulpt + gs.piece.rotation[i];
                            gs.blocks[WELL_BLOCK_WIDTH*(pos.y)+(pos.x)] = gs.piece.blocks[i];
                            break;
                        case 2:
                            gs.piece.rotation[i] = {1,1};
                            pos = gs.piece.ulpt + gs.piece.rotation[i];
                            gs.blocks[WELL_BLOCK_WIDTH*(pos.y)+(pos.x)] = gs.piece.blocks[i];
                            break;
                        case 3:
                            gs.piece.rotation[i] = {2,1};
                            pos = gs.piece.ulpt + gs.piece.rotation[i];
                            gs.blocks[WELL_BLOCK_WIDTH*(pos.y)+(pos.x)] = gs.piece.blocks[i];
                            break;
                    }
                }
            } else {
                success = false;
            }
            break;
        case L:
           gs.piece.ulpt = {4,0};
            // checking collision
            if (gs.blocks[WELL_BLOCK_WIDTH*(gs.piece.ulpt.y+1)+(gs.piece.ulpt.x+1)] == NULL) {
                // allocating block
                for (int i = 0; i < 4; i++) {
                    gs.piece.blocks[i] = new Block;
                    gs.piece.blocks[i]->color = L;
                    switch (i) {
                        case 0:
                            gs.piece.rotation[i] = {0,1};
                            pos = gs.piece.ulpt + gs.piece.rotation[i];
                            gs.blocks[WELL_BLOCK_WIDTH*(pos.y)+(pos.x)] = gs.piece.blocks[i];
                            break;
                        case 1:
                            gs.piece.rotation[i] = {1,1};
                            pos = gs.piece.ulpt + gs.piece.rotation[i];
                            gs.blocks[WELL_BLOCK_WIDTH*(pos.y)+(pos.x)] = gs.piece.blocks[i];
                            break;
                        case 2:
                            gs.piece.rotation[i] = {2,1};
                            pos = gs.piece.ulpt + gs.piece.rotation[i];
                            gs.blocks[WELL_BLOCK_WIDTH*(pos.y)+(pos.x)] = gs.piece.blocks[i];
                            break;
                        case 3:
                            gs.piece.rotation[i] = {2,0};
                            pos = gs.piece.ulpt + gs.piece.rotation[i];
                            gs.blocks[WELL_BLOCK_WIDTH*(pos.y)+(pos.x)] = gs.piece.blocks[i];
                            break;
                    }
                }
            } else {
                success = false;
            }
            break;
        case O:
           gs.piece.ulpt = {5,0};
            // checking collision
            if (gs.blocks[WELL_BLOCK_WIDTH*(gs.piece.ulpt.y+1)+(gs.piece.ulpt.x+1)] == NULL) {
                // allocating block
                for (int i = 0; i < 4; i++) {
                    gs.piece.blocks[i] = new Block;
                    gs.piece.blocks[i]->color = O;
                    switch (i) {
                        case 0:
                            gs.piece.rotation[i] = {0,0};
                            pos = gs.piece.ulpt + gs.piece.rotation[i];
                            gs.blocks[WELL_BLOCK_WIDTH*(pos.y)+(pos.x)] = gs.piece.blocks[i];
                            break;
                        case 1:
                            gs.piece.rotation[i] = {1,0};
                            pos = gs.piece.ulpt + gs.piece.rotation[i];
                            gs.blocks[WELL_BLOCK_WIDTH*(pos.y)+(pos.x)] = gs.piece.blocks[i];
                            break;
                        case 2:
                            gs.piece.rotation[i] = {0,1};
                            pos = gs.piece.ulpt + gs.piece.rotation[i];
                            gs.blocks[WELL_BLOCK_WIDTH*(pos.y)+(pos.x)] = gs.piece.blocks[i];
                            break;
                        case 3:
                            gs.piece.rotation[i] = {1,1};
                            pos = gs.piece.ulpt + gs.piece.rotation[i];
                            gs.blocks[WELL_BLOCK_WIDTH*(pos.y)+(pos.x)] = gs.piece.blocks[i];
                            break;
                    }
                }
            } else {
                success = false;
            }
            break;
        case S:
           gs.piece.ulpt = {4,0};
            // checking collision
            if (gs.blocks[WELL_BLOCK_WIDTH*(gs.piece.ulpt.y+1)+(gs.piece.ulpt.x+1)] == NULL) {
                // allocating block
                for (int i = 0; i < 4; i++) {
                    gs.piece.blocks[i] = new Block;
                    gs.piece.blocks[i]->color = S;
                    switch (i) {
                        case 0:
                            gs.piece.rotation[i] = {0,1};
                            pos = gs.piece.ulpt + gs.piece.rotation[i];
                            gs.blocks[WELL_BLOCK_WIDTH*(pos.y)+(pos.x)] = gs.piece.blocks[i];
                            break;
                        case 1:
                            gs.piece.rotation[i] = {1,0};
                            pos = gs.piece.ulpt + gs.piece.rotation[i];
                            gs.blocks[WELL_BLOCK_WIDTH*(pos.y)+(pos.x)] = gs.piece.blocks[i];
                            break;
                        case 2:
                            gs.piece.rotation[i] = {1,1};
                            pos = gs.piece.ulpt + gs.piece.rotation[i];
                            gs.blocks[WELL_BLOCK_WIDTH*(pos.y)+(pos.x)] = gs.piece.blocks[i];
                            break;
                        case 3:
                            gs.piece.rotation[i] = {2,0};
                            pos = gs.piece.ulpt + gs.piece.rotation[i];
                            gs.blocks[WELL_BLOCK_WIDTH*(pos.y)+(pos.x)] = gs.piece.blocks[i];
                            break;
                    }
                }
            } else {
                success = false;
            }
            break;
        case T:
           gs.piece.ulpt = {4,0};
            // checking collision
            if (gs.blocks[WELL_BLOCK_WIDTH*(gs.piece.ulpt.y+1)+(gs.piece.ulpt.x+1)] == NULL) {
                // allocating block
                for (int i = 0; i < 4; i++) {
                    gs.piece.blocks[i] = new Block;
                    gs.piece.blocks[i]->color = T;
                    switch (i) {
                        case 0:
                            gs.piece.rotation[i] = {1,0};
                            pos = gs.piece.ulpt + gs.piece.rotation[i];
                            gs.blocks[WELL_BLOCK_WIDTH*(pos.y)+(pos.x)] = gs.piece.blocks[i];
                            break;
                        case 1:
                            gs.piece.rotation[i] = {0,1};
                            pos = gs.piece.ulpt + gs.piece.rotation[i];
                            gs.blocks[WELL_BLOCK_WIDTH*(pos.y)+(pos.x)] = gs.piece.blocks[i];
                            break;
                        case 2:
                            gs.piece.rotation[i] = {1,1};
                            pos = gs.piece.ulpt + gs.piece.rotation[i];
                            gs.blocks[WELL_BLOCK_WIDTH*(pos.y)+(pos.x)] = gs.piece.blocks[i];
                            break;
                        case 3:
                            gs.piece.rotation[i] = {2,1};
                            pos = gs.piece.ulpt + gs.piece.rotation[i];
                            gs.blocks[WELL_BLOCK_WIDTH*(pos.y)+(pos.x)] = gs.piece.blocks[i];
                            break;
                    }
                }
            } else {
                success = false;
            }
            break;
        case Z:
           gs.piece.ulpt = {4,0};
            // checking collision
            if (gs.blocks[WELL_BLOCK_WIDTH*(gs.piece.ulpt.y+1)+(gs.piece.ulpt.x+1)] == NULL) {
                // allocating block
                for (int i = 0; i < 4; i++) {
                    gs.piece.blocks[i] = new Block;
                    gs.piece.blocks[i]->color = Z;
                    switch (i) {
                        case 0:
                            gs.piece.rotation[i] = {0,0};
                            pos = gs.piece.ulpt + gs.piece.rotation[i];
                            gs.blocks[WELL_BLOCK_WIDTH*(pos.y)+(pos.x)] = gs.piece.blocks[i];
                            break;
                        case 1:
                            gs.piece.rotation[i] = {1,0};
                            pos = gs.piece.ulpt + gs.piece.rotation[i];
                            gs.blocks[WELL_BLOCK_WIDTH*(pos.y)+(pos.x)] = gs.piece.blocks[i];
                            break;
                        case 2:
                            gs.piece.rotation[i] = {1,1};
                            pos = gs.piece.ulpt + gs.piece.rotation[i];
                            gs.blocks[WELL_BLOCK_WIDTH*(pos.y)+(pos.x)] = gs.piece.blocks[i];
                            break;
                        case 3:
                            gs.piece.rotation[i] = {2,1};
                            pos = gs.piece.ulpt + gs.piece.rotation[i];
                            gs.blocks[WELL_BLOCK_WIDTH*(pos.y)+(pos.x)] = gs.piece.blocks[i];
                            break;
                    }
                }
            } else {
                success = false;
            }
            break;
    }
    return success;
}

int main(int argc, char **argv) {
    if (!init()) {
        std::cout << "Initialization Failed" << std::endl;
    } else {
        if (!load()) {
            std::cout << "Loading Failed" << std::endl;
        } else {
            bool quit = false;
            bool position_debug = false;
            bool collision_debug = false;
            int frame_count = 0;
            int delta_ticks = 0;
            float avgFPS = 0.0;

            //starting initialization
            gs.state = START;
            gs.level = 0;
            gs.score = 0;
            gs.lines = 0;
            for (int x = 0; x < WELL_BLOCK_WIDTH; x++) {
                for (int y = 0; y < WELL_BLOCK_HEIGHT; y++) {
                    gs.blocks[WELL_BLOCK_WIDTH*y+x] = NULL;
                }
            }
            spawn_piece();

            fps_timer.start();

            while (!quit) {
                frame_timer.start();
                SDL_Event event;

                while (SDL_PollEvent(&event)) {
                    if (event.type == SDL_QUIT) {
                            quit = true;
                    }
                    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE ) {
                        quit = true;
                    }
                    if (event.type == SDL_KEYDOWN && event.key.repeat == 0) {
                        switch (event.key.keysym.sym) {
                            case SDLK_1:     
                                position_debug = !position_debug; 
                                break;
                            case SDLK_2:     
                                collision_debug = !collision_debug; 
                                break; 
                            case SDLK_9:
                                move_piece(DOWN);
                                break;
                            case SDLK_0:
                                release_piece();
                                delete_blocks(); // be careful with this, can easily cause a seg fault
                                spawn_piece();
                                break;
                            case SDLK_w:
                            case SDLK_UP:
                                rotate_piece();
                                break;
                            case SDLK_s:
                            case SDLK_DOWN:                                
                                rotate_piece(true);
                                break;
                            case SDLK_a:
                            case SDLK_LEFT:
                                move_piece(LEFT);
                                break;
                            case SDLK_d:
                            case SDLK_RIGHT:
                                move_piece(RIGHT);
                                break;
                            case SDLK_SPACE:
                                // move_piece(HARD_DOWN);
                                break;
                        }
                    }
                }

                // calculate and render FPS
                avgFPS = frame_count / (fps_timer.get_ticks() / 1000.0);
                if (avgFPS > 2000000) {
                    avgFPS = 0;
                }
                fpsText.str("");
                fpsText << "fps: " << avgFPS;
                t_fps.load_from_rendered_text(fpsText.str(), RED_);
                // Initialize renderer color (also used for clearing)
                // SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF); //white
                SDL_SetRenderDrawColor(renderer, 0x0, 0x0, 0x0, 0xFF); //black
                SDL_RenderClear(renderer);
                render_well(position_debug);
                render_status();
                t_fps.render(SCREEN_WIDTH-t_fps.get_width(), 0);
                SDL_RenderPresent(renderer);
                frame_count++;

                // already using delta for calculating position, now using to cap fps at 60
                delta_ticks = frame_timer.get_ticks();
                if (delta_ticks < SCREEN_TICKS_PER_FRAME) {
                    SDL_Delay(SCREEN_TICKS_PER_FRAME - delta_ticks);
                }
            }
        }
    }
    close();
    return 0;
}
