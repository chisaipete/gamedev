#include "tetris.h"

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
Texture tilemap;
GameState gs;
SDL_Rect sprites[18];
Timer frame_timer;
Timer fps_timer;
Timer piece_timer;
int level_tick;
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

v2 get_deepest_position();
unsigned check_collisions(v2 new_position, v2* new_rotation);


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
    // level
    levelText.str("");
    levelText << "Level: " << gs.level;
    t_level.load_from_rendered_text(levelText.str());
    t_level.render(BLOCK_SIZE/2, SCREEN_HEIGHT-BLOCK_SIZE/2);

    // score
    scoreText.str("");
    scoreText << gs.score;
    t_score.load_from_rendered_text(scoreText.str());
    t_score.render((SCREEN_WIDTH/2)-(t_score.get_width()/2), SCREEN_HEIGHT-(BLOCK_SIZE/2));

    // lines
    linesText.str("");
    linesText << "Lines: " << gs.lvl_lines << " / " << gs.goal << "  (" << gs.lines << ")";
    t_lines.load_from_rendered_text(linesText.str());
    t_lines.render(SCREEN_WIDTH-(BLOCK_SIZE/2)-t_lines.get_width(), SCREEN_HEIGHT-(BLOCK_SIZE/2));

    statusText.str("");
    switch (gs.state) {
        case START:
            statusText << "TETRIS";
            break;
        case PAUSE:
            statusText << "PAUSED";
            break;
        case PLAY:
            statusText << " ";
            break;
        case OVER:
            statusText << "G A M E  O V E R";
            break;
    }
    t_status.load_from_rendered_text(statusText.str());
    t_status.render((SCREEN_WIDTH/2)-(t_status.get_width()/2), SCREEN_HEIGHT/2);
}

void render_well(bool debug = false) {
    if (gs.state == PLAY || gs.state == OVER) {
        if (gs.piece.blocks[0] != NULL) {
            // ghost
            v2 ulpt = get_deepest_position() + V2(0,1);
            tilemap.set_alpha(128);
            v2 pos;
            for (int i = 0; i < 4; i++) {
                pos = ulpt + gs.piece.rotation[i];
                tilemap.render((pos.x)*BLOCK_SIZE, (pos.y)*BLOCK_SIZE, &sprites[gs.piece.blocks[0]->color]);
            }
            tilemap.set_alpha(255);
            // piece 
            if (gs.piece.blocks[0] != NULL) {
                for (int i = 0; i < 4; i++) {
                    pos = gs.piece.ulpt + V2(0,1) + gs.piece.rotation[i];
                    tilemap.render((pos.x)*BLOCK_SIZE, (pos.y)*BLOCK_SIZE, &sprites[gs.piece.blocks[0]->color]);
                }
            }
        }
        //blocks
        for (int x = 0; x < WELL_BLOCK_WIDTH; x++) {
            for (int y = 0; y < WELL_BLOCK_HEIGHT; y++) {
                if (gs.blocks[WELL_BLOCK_WIDTH*y+x] != NULL) {
                    tilemap.render((x)*BLOCK_SIZE, (y+1)*BLOCK_SIZE, &sprites[gs.blocks[WELL_BLOCK_WIDTH*y+x]->color]);

                }
                if (debug) {
                    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF); //white
                    SDL_Rect quad = {(x)*BLOCK_SIZE, (y+1)*BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE};
                    SDL_RenderDrawRect(renderer, &quad);
                    SDL_SetRenderDrawColor(renderer, 0x0, 0x0, 0x0, 0xFF); //black    
                }
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

bool release_piece() {
    v2 pos;
    for (int i = 0; i < 4; i++) {
        pos = gs.piece.ulpt + gs.piece.rotation[i];
        gs.blocks[WELL_BLOCK_WIDTH*(pos.y)+(pos.x)] = gs.piece.blocks[i];      
        gs.piece.blocks[i] = NULL;
    }
}

void delete_piece() {
    for (int i = 0; i < 4; i++) {
        delete gs.piece.blocks[i];
        gs.piece.blocks[i] = NULL;
    }
}

unsigned check_collisions(v2 new_position, v2* new_rotation) {
    if (gs.piece.blocks[0] == NULL) {
        return 32;
    }
    // checks for all kinds of collisions for a given piece, requires a new position ulpt
    unsigned collision_mask = 0b000;
    // bool success = true;
    bool self = false;
    v2 temp = {0,0};
    v2 old = {0,0};
    //iterate through present position of piece
    //check against all possible collisions, if we collide, return false
    for (int i = 0; i < 4; i++) {
        temp = new_position + new_rotation[i];
    //walls
        if (temp.x < 2 || temp.x >= WELL_BLOCK_WIDTH) {
            collision_mask |= WALLS;
            if (temp.x < 2) {
                collision_mask |= LWALL;
            } else if (temp.x >= WELL_BLOCK_WIDTH) {
                collision_mask |= RWALL;
            }
        }
    //floor
        if (temp.y > 21) {
            collision_mask |= FLOOR;
        }
    //stationary pieces
        //if we hit a piece            
        if (temp.x > 0) {
            if (gs.blocks[WELL_BLOCK_WIDTH*(temp.y)+(temp.x)] != NULL) {
                //if it's not one of the other tiles' old positions
                self = false;
                for(int i = 0; i < 4; i++) {
                    old = gs.piece.ulpt + gs.piece.rotation[i];
                    if (old == temp) {
                        self = true;
                    }
                }
                if (!self) {
                    collision_mask |= PIECE;
                    if (old.x < temp.x) {
                        collision_mask |= RPICE;
                    }
                    if (old.x > temp.x) {
                        collision_mask |= LPICE;
                    }
                    if (old.y > temp.y) {
                        collision_mask |= FPICE;
                    }
                } else {
                    collision_mask |= PSELF;
                }
            }
        }
    }
    return collision_mask;
}

v2 get_deepest_position() {
    // based on present position, check for the lowest point the current piece can go, and return a new ulpt
    // this originally went outside the bounds of the board, so I widened the board with 2 unused columns
    // to get around the issue of uninitialized area on the edge of the array for our rotation scheme 
    v2 lowest_ulpt = gs.piece.ulpt;
    unsigned collisions = check_collisions(lowest_ulpt, gs.piece.rotation);
    while (!(collisions & NOHIT)) {
        lowest_ulpt = lowest_ulpt + V2(0,1);
        collisions = check_collisions(lowest_ulpt, gs.piece.rotation);
    }
    return lowest_ulpt + V2(0, -1);
}

bool spawn_piece() { //https://xkcd.com/888/
    bool success = true;
    // select piece using 7-in-a-Bag random generator
    // attempt to place piece at proper spawn point in well: if it collides, game over
    Piece p = seven_bag.draw_piece();
    unsigned collisions = 0b000;
    v2 pos[4];
    switch (p) {
        case I:
            gs.piece.ulpt = {5,0};
            for (int i = 0; i < 4; i++) {
                gs.piece.blocks[i] = new Block;
                gs.piece.blocks[i]->color = I;
                switch (i) {
                    case 0: gs.piece.rotation[i] = {0,1}; break;
                    case 1: gs.piece.rotation[i] = {1,1}; break;
                    case 2: gs.piece.rotation[i] = {2,1}; break;
                    case 3: gs.piece.rotation[i] = {3,1}; break;
                }
            }
            break;
        case J:
            gs.piece.ulpt = {5,0};
            for (int i = 0; i < 4; i++) {
                gs.piece.blocks[i] = new Block;
                gs.piece.blocks[i]->color = J;
                switch (i) {
                    case 0: gs.piece.rotation[i] = {0,0}; break;
                    case 1: gs.piece.rotation[i] = {0,1}; break;
                    case 2: gs.piece.rotation[i] = {1,1}; break;
                    case 3: gs.piece.rotation[i] = {2,1}; break;
                }
            }
            break;
        case L:
            gs.piece.ulpt = {5,0};
            for (int i = 0; i < 4; i++) {
                gs.piece.blocks[i] = new Block;
                gs.piece.blocks[i]->color = L;
                switch (i) {
                    case 0: gs.piece.rotation[i] = {0,1}; break;
                    case 1: gs.piece.rotation[i] = {1,1}; break;
                    case 2: gs.piece.rotation[i] = {2,1}; break;
                    case 3: gs.piece.rotation[i] = {2,0}; break;
                }
            }
            break;
        case O:
            gs.piece.ulpt = {6,0};
            for (int i = 0; i < 4; i++) {
                gs.piece.blocks[i] = new Block;
                gs.piece.blocks[i]->color = O;
                switch (i) {
                    case 0: gs.piece.rotation[i] = {0,0}; break;
                    case 1: gs.piece.rotation[i] = {1,0}; break;
                    case 2: gs.piece.rotation[i] = {0,1}; break;
                    case 3: gs.piece.rotation[i] = {1,1}; break;
                }
            }
            break;
        case S:
            gs.piece.ulpt = {5,0};
            for (int i = 0; i < 4; i++) {
                gs.piece.blocks[i] = new Block;
                gs.piece.blocks[i]->color = S;
                switch (i) {
                    case 0: gs.piece.rotation[i] = {0,1}; break;
                    case 1: gs.piece.rotation[i] = {1,0}; break;
                    case 2: gs.piece.rotation[i] = {1,1}; break;
                    case 3: gs.piece.rotation[i] = {2,0}; break;
                }
            }
            break;
        case T:
            gs.piece.ulpt = {5,0};
            for (int i = 0; i < 4; i++) {
                gs.piece.blocks[i] = new Block;
                gs.piece.blocks[i]->color = T;
                switch (i) {
                    case 0: gs.piece.rotation[i] = {1,0}; break;
                    case 1: gs.piece.rotation[i] = {0,1}; break;
                    case 2: gs.piece.rotation[i] = {1,1}; break;
                    case 3: gs.piece.rotation[i] = {2,1}; break;
                }
            }
            break;
        case Z:
           gs.piece.ulpt = {5,0};
            for (int i = 0; i < 4; i++) {
                gs.piece.blocks[i] = new Block;
                gs.piece.blocks[i]->color = Z;
                switch (i) {
                    case 0: gs.piece.rotation[i] = {0,0}; break;
                    case 1: gs.piece.rotation[i] = {1,0}; break;
                    case 2: gs.piece.rotation[i] = {1,1}; break;
                    case 3: gs.piece.rotation[i] = {2,1}; break;
                }
            }
            break;
    }
    // check for collisions with existing blocks, if so, destroy block and return false, indicating game over
    collisions = check_collisions(gs.piece.ulpt, gs.piece.rotation);
    if (collisions & PSELF) {
        gs.piece.ulpt = {0,0};
        delete_piece();
        success = false;
    }
    return success;
}

void move_piece(Direction d) {
    if (gs.piece.blocks[0] == NULL) {
        return;
    }
    // calculate new position
    v2 new_ulpt;
    v2 poso;
    v2 posn[4];
    v2 tmp;
    switch (d) {
        case LEFT:
            new_ulpt = gs.piece.ulpt + V2(-1,0); break;
        case RIGHT:
            new_ulpt = gs.piece.ulpt + V2(1,0); break;
        case DOWN:
            new_ulpt = gs.piece.ulpt + V2(0,1); break;
        case HARD_DOWN:
            new_ulpt = get_deepest_position(); break;
        case UP:
            new_ulpt = gs.piece.ulpt + V2(0,-1); break;
        default:
            break;
    }
    // check collisions at new position
    // using a collision bitmask to handle multiple types of collisions
    unsigned collisions = check_collisions(new_ulpt, gs.piece.rotation);

    // commit new position
    if (!(collisions & NOHIT)) {
       gs.piece.ulpt = new_ulpt;
    }
    // create a new piece if we're set
    if (collisions & FLOOR || collisions & PIECE) {
        if (d == DOWN) {
            gs.new_piece = true;
        }
    }
}

void rotate_piece(bool reverse = false, int kicks = 0) {
    if (gs.piece.blocks[0] == NULL) {
        return;
    }
    // calculate new position
    int xn, yn, me;
    v2 poso;
    v2 posn[4];
    v2 new_rotation[4];

    me = max_ext[gs.piece.blocks[0]->color];
    //reverse rotates to the left, otherwise right
    if (!reverse) {
        for (int i = 0; i < 4; i++) {
            xn = 1 - (gs.piece.rotation[i].y - (me - 2));
            yn = gs.piece.rotation[i].x;
            new_rotation[i] = {xn, yn};
        }
    } else {
        for (int i = 0; i < 4; i++){
            xn = gs.piece.rotation[i].y;
            yn = 1 - (gs.piece.rotation[i].x - (me - 2));
            new_rotation[i] = {xn, yn};
        }
    }
    // check collisions at new position
    unsigned collisions = check_collisions(gs.piece.ulpt, new_rotation);

    // commit new position
    if (!(collisions & NOHIT)) {
        for (int i = 0; i < 4; i++) {
            gs.piece.rotation[i] = new_rotation[i];
        }
    } else {
        // wall/floor kicks
        if (kicks < 2) {
            if (collisions & LWALL || (collisions & LPICE && !(collisions && RPICE))) { // TODO: check for piece kicks
                std::cout << "left kick" << std::endl;
                move_piece(RIGHT);
                rotate_piece(reverse, ++kicks);
            } else if (collisions & RWALL || (collisions & RPICE && !(collisions && LPICE))) {
                std::cout << "right kick" << std::endl;
                move_piece(LEFT);
                rotate_piece(reverse, ++kicks);
            } else if (collisions & FLOOR || collisions & FPICE) {
                std::cout << "floor kick" << std::endl;
                move_piece(UP);
                rotate_piece(reverse, ++kicks);
            }
        }
    }
}

void check_lines() {
    bool full;
    bool empty;
    bool piece;
    int lines = 0;
    // iterate line by line (top to bottom)
    for (int y = 0; y < WELL_BLOCK_HEIGHT; y++) {
        // looking for lines where it's completely full with blocks
        full = true; //TODO: need to reject the active piece
        for (int x = 2; x < WELL_BLOCK_WIDTH; x++) {
            if (gs.blocks[WELL_BLOCK_WIDTH*y+x] == NULL ) {
                full = false;
                break;
            }
        }
        if (full) {
            lines++;
            //delete all blocks in line
            for (int x = 2; x < WELL_BLOCK_WIDTH; x++) {
                delete gs.blocks[WELL_BLOCK_WIDTH*y+x];
                gs.blocks[WELL_BLOCK_WIDTH*y+x] = NULL;
            }
            //shift all blocks in line down
            for (int j = y-1; j > 0; j--) {
                empty = true;
                for (int x = 2; x < WELL_BLOCK_WIDTH; x++) {
                    if (gs.blocks[WELL_BLOCK_WIDTH*j+x] != NULL ) {
                        empty = false;
                        break;
                    }
                }
                if (!empty) {
                    int k = j + 1;
                    for (int x = 2; x < WELL_BLOCK_WIDTH; x++) {
                        gs.blocks[WELL_BLOCK_WIDTH*k+x] = gs.blocks[WELL_BLOCK_WIDTH*j+x];
                        gs.blocks[WELL_BLOCK_WIDTH*j+x] = NULL;
                    }
                }
            }
        }
    }
    //adjust score
    switch (lines) {
        case 4:
            gs.lines += 4;
            gs.score += 8;
            gs.lvl_lines += 4;  
            break;
        case 3:
            gs.lines += 3;
            gs.score += 5;
            gs.lvl_lines += 3;
            break;
        case 2:
            gs.lines += 2;
            gs.score += 3;
            gs.lvl_lines += 2;
            break;
        case 1:
            gs.lines += 1;
            gs.score += 1;
            gs.lvl_lines += 1;
            break;
        default:
            break;
    }
}

void clear_board() {
    gs.piece.ulpt = {0,0};
    for (int i = 0; i < 4; i++) {
        gs.piece.blocks[i] = NULL;
        gs.piece.rotation[i] = {0,0};
    }
    for (int x = 0; x < WELL_BLOCK_WIDTH; x++) {
        for (int y = 0; y < WELL_BLOCK_HEIGHT; y++) {
            gs.blocks[WELL_BLOCK_WIDTH*y+x] = NULL;
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
            bool fps_on = false;
            bool position_debug = false;
            bool spawn_success = true;
            bool next_level = false;
            int frame_count = 0;
            int delta_ticks = 0;
            int cur_ticks = 0;
            float avgFPS = 0.0;

            //starting initialization
            gs.state = START;

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
                                fps_on = !fps_on; 
                                break;
                            case SDLK_8:
                                switch (gs.state) {
                                    case OVER:
                                    case PAUSE:
                                    case START:
                                        gs.state = PLAY;
                                        level_tick = block_step(gs.level);
                                        piece_timer.start();
                                        break;
                                    case PLAY:
                                        gs.state = START;
                                        break;
                                }
                                break; 
                            case SDLK_9:
                                move_piece(DOWN);
                                break;
                            case SDLK_0:
                                release_piece();
                                delete_blocks(); // be careful with this, can easily cause a seg fault
                                spawn_success = spawn_piece();
                                break;
                            case SDLK_COMMA:
                            case SDLK_LESS:
                                gs.level -= 1;
                                if (gs.level < 1) gs.level = 1;
                                level_tick = block_step(gs.level);
                                break;
                            case SDLK_PERIOD:
                            case SDLK_GREATER:
                                gs.level += 1;
                                if (gs.level > 15) gs.level = 15;
                                level_tick = block_step(gs.level);
                                break;
                            case SDLK_w:
                            case SDLK_UP:
                                rotate_piece(false);
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
                                move_piece(HARD_DOWN);
                                break;
                            case SDLK_KP_ENTER:
                            case SDLK_RETURN:
                                switch (gs.state) {
                                    case START:
                                        gs.state = PLAY;
                                        gs.new_piece = true;
                                        level_tick = block_step(++gs.level);
                                        gs.goal = level_goal(gs.level);
                                        piece_timer.start();
                                        break;
                                    case PAUSE:
                                        gs.state = PLAY;
                                        piece_timer.unpause();
                                        break;
                                    case PLAY: 
                                        gs.state = PAUSE; 
                                        piece_timer.pause();
                                        break;
                                    case OVER: 
                                        gs.state = START; 
                                        break;
                                    default:
                                        break;
                                }
                                break;
                        }
                    }
                }

                switch (gs.state) {
                    case START:
                        gs.level = 0;
                        gs.lvl_lines = 0;
                        gs.score = 0;
                        gs.lines = 0;
                        gs.new_piece = false;
                        clear_board();
                        break;
                    case PAUSE:
                        break;
                    case PLAY:
                        cur_ticks = piece_timer.get_ticks();
                        if (cur_ticks >= level_tick) {
                            if (gs.new_piece) {
                                release_piece();
                                spawn_success = spawn_piece();
                                gs.new_piece = !gs.new_piece;
                            } else {
                                move_piece(DOWN);
                            }
                            piece_timer.start();
                            check_lines();
                        }
                        if (!spawn_success) {
                            gs.state = OVER;
                            piece_timer.stop();
                        }
                        if (gs.lvl_lines >= gs.goal) {
                            level_tick = block_step(++gs.level);
                            gs.goal = level_goal(gs.level);
                            gs.lvl_lines = 0;
                        }
                        break;
                    case OVER:
                    default: break;
                }

                // Initialize renderer color (also used for clearing)
                // SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF); //white
                SDL_SetRenderDrawColor(renderer, 0x0, 0x0, 0x0, 0xFF); //black
                SDL_RenderClear(renderer);
                render_well(position_debug);
                render_status();
                if (fps_on) {
                    // calculate and render FPS
                    avgFPS = frame_count / (fps_timer.get_ticks() / 1000.0);
                    if (avgFPS > 2000000) {
                        avgFPS = 0;
                    }
                    fpsText.str("");
                    fpsText << avgFPS << "fps";
                    t_fps.load_from_rendered_text(fpsText.str(), RED_);
                    t_fps.render(SCREEN_WIDTH-t_fps.get_width(), 0);
                }
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
