#include "leaper.h"

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
Texture level_map;
Texture actor_map;
SDL_Rect level_sprites[8];
SDL_Rect actor_sprites[7];
Timer frame_timer;
Timer movement_timer;
Timer fps_timer;
std::stringstream scoreText;
std::stringstream statusText;
std::stringstream fpsText;
Texture t_score;
Texture t_status;
Texture t_fps;

struct Block {
    int sprite;
    SDL_Rect collider;
};

struct GameState {
    int state;
    int score;
    int lives;
    int level;
    Block* blocks[LEVEL_TILE_WIDTH*LEVEL_TILE_HEIGHT];
};

GameState gs;

class Player {
    public:
        static const int width  = TILE_SIZE; // pixels
        static const int height = TILE_SIZE;
        static const int colw = TILE_SIZE/2;
        static const int colh = (TILE_SIZE/4)*3;
        static constexpr float max_speed = 160.0; //pixels / second
        static constexpr float acceleration = 320.0; //pixels / second^2
        static constexpr float gravity = 134.0; //pixels / second^2
        Player();
        void handle_event(SDL_Event &event);
        void move(float delta);
        void render();//v2 camera);
        v2 get_position();
        v2 get_velocity();
        int get_lives();
        int get_hp();
        // std::vector<Bullet> shots;
        SDL_Rect collider;
        // void fire();
    private:
        v2 position;
        v2 velocity;
        int hp;
        int lives;
        float walk_speed;
        bool up, down, left, right;
        SDL_RendererFlip flip;
        int frame;
        int walk_frame;
};

Player::Player() {
    position = {LEVEL_WIDTH/2, LEVEL_HEIGHT-(SCREEN_HEIGHT/2)};
    frame = STAND0;
    walk_frame = WALK0;
    hp = 10;
    collider = {position.x-width/2, position.y-height/2, colw, colh};
    lives = 3;
    walk_speed = 0.0;
    velocity = {0,0};
    up = false;
    down = false;
    left = false;
    right = false;
    flip = SDL_FLIP_NONE;
}

void Player::handle_event(SDL_Event &event) {
    if (event.type == SDL_KEYDOWN) { // && event.key.repeat == 0) {
        switch (event.key.keysym.sym) {
            case SDLK_UP:
                up = true;
                break;
            case SDLK_DOWN:
                down = true;
                break;
            case SDLK_LEFT:
                left = true;
                break;
            case SDLK_RIGHT:
                right = true;
                break;
            case SDLK_SPACE:
                // fire();
                // jump
                break;
            default:
                break;
        }
    } else if (event.type == SDL_KEYUP) { // && event.key.repeat == 0) {
        switch (event.key.keysym.sym) {
            case SDLK_UP:
                up = false;
                break;
            case SDLK_DOWN:
                down = false;
                break;
            case SDLK_LEFT:
                left = false;
                break;
            case SDLK_RIGHT:
                right = false;
                break;
            case SDLK_SPACE:
                break;
            default:
                break;
        }
    }
}

void Player::move(float delta) {
    v2 position_o = position;

    if (left) {
        // velocity.x -= acceleration * delta;
        velocity.x = -max_speed;
    }
    if (right) {
        // velocity.x += acceleration * delta;
        velocity.x = max_speed;
    }
    if (!left && !right) {
        velocity.x = 0.0;
    }
    if (velocity.x > max_speed) {
        velocity.x = max_speed;
    } else if (velocity.x < -max_speed) {
        velocity.x = -max_speed;
    }
    // //friction
    // velocity.x *= .99;
    // velocity.y *= .99;

    position.x += velocity.x * delta;
    position.y += velocity.y * delta;

    collider.x = position.x;
    collider.y = position.y;

    // TODO: check for collisions
    // look at all the tiles in the same rows and columns as the player, do x, then y
    // if there is nothing colliding below, apply gravity to y
    // if there is something colliding to the side, reduce speed to 0 on that direction
    unsigned collisions = 0b000;

    std::cout << collisions << " " << !(collisions & BOTTOM) << std::endl;

    if (!(collisions & BOTTOM)) {
        velocity.y += gravity * delta;
        if (velocity.y > max_speed) {
            velocity.y = max_speed;
        }
    }

    if (collider.x < 0) {
        position.x = 0;
        collider.x = position.x;
        velocity.x = 0.0;
    } else if (collider.x > SCREEN_WIDTH - colw) {
        position.x = SCREEN_WIDTH - colw;
        collider.x = position.x;
        velocity.x = 0.0;
    }
}

void Player::render() {
    // TODO: change what frame is rendered depending on movement
    // actor_map.render(position.x-(width/2)-camera.x, position.y-(height/2)-camera.y, &actor_sprites[STAND0]);
    frame = STAND0;
    if (velocity.x < 0) {
        flip = SDL_FLIP_NONE;
        frame = (walk_frame/9)+2;
    }
    if (velocity.x > 0 ) {
        flip = SDL_FLIP_HORIZONTAL;
        frame = (walk_frame/9)+2;
    }
    if (velocity.y > 0 || velocity.y < 0) {
        frame = JUMP;
    }

    actor_map.render(position.x-(colw/2), position.y-(height/4), &actor_sprites[frame], 0.0, NULL, flip);

    walk_frame++;
    if (walk_frame/9 >= WALK_FRAMES) {
        walk_frame = WALK0;
    }
}

v2 Player::get_position() {
    return position;
}

v2 Player::get_velocity() {
    return velocity;
}

int Player::get_lives() {
    return lives;
}

int Player::get_hp() {
    return hp;
}

// void Player::fire() {
//     shots.push_back(Bullet(position, angle));
// }

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
            // window = SDL_CreateWindow("Leaper", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
            window = SDL_CreateWindow("Leaper", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, LEVEL_WIDTH, LEVEL_HEIGHT, SDL_WINDOW_SHOWN);
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
    int pixel_format = 0;

    if (!level_map.load_from_file("res/leaper_tiles.png")) {
        success = false;
    } else {
        // set standard alpha blending
        // level_map.set_blend_mode(SDL_BLENDMODE_BLEND);
        // level_map.set_alpha(255);
    }

    if (!actor_map.load_from_file("res/leaper_character.png")) {
        success = false;
    } else {
        // set standard alpha blending
        // actor_map.set_blend_mode(SDL_BLENDMODE_BLEND);
        // actor_map.set_alpha(255);
    }

    font = TTF_OpenFont("res/cc.ttf", 12);
    if (font == nullptr){
        logSDLError(std::cout, "TTF_OpenFont");
        success = false;
    }

    if (!t_score.load_from_rendered_text("_")) {
        success = false;
    }
    if (!t_status.load_from_rendered_text("_")) {
        success = false;
    }
    if (!t_fps.load_from_rendered_text("_", RED_)) {
        success = false;
    }
    
    // TODO: replace this with tilemap files and level map files (no more hardcodes)
    level_sprites[GRASS0].x = 0;
    level_sprites[GRASS0].y = 0;
    level_sprites[GRASS0].w = 32;
    level_sprites[GRASS0].h = 32;
    level_sprites[GRASS1].x = 32;
    level_sprites[GRASS1].y = 0;
    level_sprites[GRASS1].w = 32;
    level_sprites[GRASS1].h = 32;
    level_sprites[GRASS2].x = 64;
    level_sprites[GRASS2].y = 0;
    level_sprites[GRASS2].w = 32;
    level_sprites[GRASS2].h = 32;
    level_sprites[GROUND0].x = 0;
    level_sprites[GROUND0].y = 32;
    level_sprites[GROUND0].w = 32;
    level_sprites[GROUND0].h = 32;
    level_sprites[GROUND1].x = 32;
    level_sprites[GROUND1].y = 32;
    level_sprites[GROUND1].w = 32;
    level_sprites[GROUND1].h = 32;
    level_sprites[GROUND2].x = 64;
    level_sprites[GROUND2].y = 32;
    level_sprites[GROUND2].w = 32;
    level_sprites[GROUND2].h = 32;
    level_sprites[GRASS_LEDGE].x = 96;
    level_sprites[GRASS_LEDGE].y = 0;
    level_sprites[GRASS_LEDGE].w = 32;
    level_sprites[GRASS_LEDGE].h = 32;
    level_sprites[STONE_LEDGE].x = 128;
    level_sprites[STONE_LEDGE].y = 0;
    level_sprites[STONE_LEDGE].w = 32;
    level_sprites[STONE_LEDGE].h = 32;

    actor_sprites[STAND0].x = 0;
    actor_sprites[STAND0].y = 0;
    actor_sprites[STAND0].w = 32;
    actor_sprites[STAND0].h = 32;
    actor_sprites[STAND1].x = 32;
    actor_sprites[STAND1].y = 0;
    actor_sprites[STAND1].w = 32;
    actor_sprites[STAND1].h = 32;
    actor_sprites[WALK0].x = 64;
    actor_sprites[WALK0].y = 0;
    actor_sprites[WALK0].w = 32;
    actor_sprites[WALK0].h = 32;
    actor_sprites[WALK1].x = 96;
    actor_sprites[WALK1].y = 0;
    actor_sprites[WALK1].w = 32;
    actor_sprites[WALK1].h = 32;
    actor_sprites[WALK2].x = 128;
    actor_sprites[WALK2].y = 0;
    actor_sprites[WALK2].w = 32;
    actor_sprites[WALK2].h = 32;
    actor_sprites[JUMP].x = 160;
    actor_sprites[JUMP].y = 0;
    actor_sprites[JUMP].w = 32;
    actor_sprites[JUMP].h = 32;
    actor_sprites[HIT].x = 192;
    actor_sprites[HIT].y = 0;
    actor_sprites[HIT].w = 32;
    actor_sprites[HIT].h = 32;

    return success;
}

void delete_level() {
    for (int x = 0; x < LEVEL_TILE_WIDTH; x++) {
        for (int y = 0; y < LEVEL_TILE_HEIGHT; y++) {
            if (gs.blocks[LEVEL_TILE_WIDTH*y+x] != NULL) {
                delete gs.blocks[LEVEL_TILE_WIDTH*y+x];
                gs.blocks[LEVEL_TILE_WIDTH*y+x] = NULL;
            }
        }
    }
}

bool close() {
    delete_level();
    level_map.free();
    actor_map.free();
    t_score.free();
    t_status.free();
    t_fps.free();
    if (font != nullptr) { TTF_CloseFont(font); font = NULL; }
    if (renderer != nullptr) { SDL_DestroyRenderer(renderer); renderer = NULL; }
    if (window != nullptr) { SDL_DestroyWindow(window); window = NULL; }
    TTF_Quit();
    SDL_Quit();
}

void render_status() {
    // score
    scoreText.str("");
    scoreText << " ";
    t_score.load_from_rendered_text(scoreText.str());
    t_score.render((SCREEN_WIDTH/2)-(t_score.get_width()/2), SCREEN_HEIGHT-(t_score.get_height()));

    statusText.str("");
    switch (gs.state) {
        case START:
            statusText << "LEAPER";
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

void render_scene() {//v2 camera = V2(0,0)) {
    // TODO: render blocks from bottom to top, clipping through the camera
    for (int y = 0; y < LEVEL_TILE_HEIGHT; y++) {
        for (int x = 0; x < LEVEL_TILE_WIDTH; x++) {
            if (gs.blocks[LEVEL_TILE_WIDTH*y+x] != NULL && gs.blocks[LEVEL_TILE_WIDTH*y+x]->sprite != EMPTY) {
                // level_map.render((x*TILE_SIZE)-camera.x, (y*TILE_SIZE)-camera.y, &level_sprites[gs.blocks[LEVEL_TILE_WIDTH*y+x]->sprite]);
                level_map.render((x*TILE_SIZE), (y*TILE_SIZE), &level_sprites[gs.blocks[LEVEL_TILE_WIDTH*y+x]->sprite]);
            }
        }
    }
}

void load_level(int num = -1) {
    int level;
    if (num == -1) {
        level = gs.level;
    } else {
        level = num;
    }
    int y;
    //blocks
    switch (level) {
        case 1:
            y = LEVEL_TILE_HEIGHT - 2;
            for (int x = 0; x < LEVEL_TILE_WIDTH; x++) {
                int bnum = (x % 3);
                gs.blocks[LEVEL_TILE_WIDTH*y+x] = new Block();
                gs.blocks[LEVEL_TILE_WIDTH*y+x]->sprite = bnum;
                gs.blocks[LEVEL_TILE_WIDTH*y+x]->collider = {0,0,0,0};
            }
            y++;
            for (int x = 0; x < LEVEL_TILE_WIDTH; x++) {
                int bnum = (x % 3) + 3;
                gs.blocks[LEVEL_TILE_WIDTH*y+x] = new Block();
                gs.blocks[LEVEL_TILE_WIDTH*y+x]->sprite = bnum;
                gs.blocks[LEVEL_TILE_WIDTH*y+x]->collider = {x*TILE_SIZE, y*TILE_SIZE, TILE_SIZE, TILE_SIZE};
            }
            break;
        default:
            //populate empty blocks
            break;
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
            bool collision_debug = false;
            int frame_count = 0;
            int frame_ticks = 0;
            int delta_ticks = 0;
            float avgFPS = 0.0;

            Player player;
            v2 player_position;
            // SDL_Rect camera = {0, LEVEL_HEIGHT-(SCREEN_HEIGHT/2), SCREEN_WIDTH, SCREEN_HEIGHT};

            //starting initialization
            gs.state = START;

            fps_timer.start();

            while (!quit) {
                frame_timer.start();
                SDL_Event event;

                while (SDL_PollEvent(&event) != 0) {
                    if (event.type == SDL_QUIT) {
                            quit = true;
                    }
                    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE ) {
                        quit = true;
                    }
                    if (event.type == SDL_KEYDOWN && event.key.repeat == 0) {
                        switch (event.key.keysym.sym) {
                            case SDLK_1:     
                                fps_on = !fps_on; 
                                break;
                            case SDLK_2:
                                collision_debug = !collision_debug;
                                break;
                            case SDLK_8:
                                break; 
                            case SDLK_9:
                                break;
                            case SDLK_0:
                                break;
                            case SDLK_KP_ENTER:
                            case SDLK_RETURN:
                                switch (gs.state) {
                                    case START:
                                        gs.state = PLAY;
                                        gs.level = 1;
                                        load_level();
                                        break;
                                    case PAUSE:
                                        gs.state = PLAY;
                                        break;
                                    case PLAY: 
                                        gs.state = PAUSE; 
                                        break;
                                    case OVER:
                                        player = Player();
                                        player_position = player.get_position();
                                        // camera.x = player_position.x - (SCREEN_WIDTH / 2);
                                        // camera.y = player_position.y - (SCREEN_HEIGHT / 2);
                                        gs.state = START;
                                        break;
                                    default:
                                        break;
                                }
                                break;
                        }
                    }
                    player.handle_event(event);
                }


                delta_ticks = movement_timer.get_ticks();
                float delta = delta_ticks / 1000.0;

                switch (gs.state) {
                    case START:
                        break;
                    case PAUSE:
                        // don't move stuff
                        break;
                    case PLAY:
                        // move things with the delta
                        player.move(delta);
                        // player_position = player.get_position();
                        // camera.y = player_position.y - (SCREEN_HEIGHT / 2);
                        // keep the camera in level bounds
                        // if(camera.x < 0) { 
                        //     camera.x = 0;
                        // }
                        // if(camera.y < 0) {
                        //     camera.y = 0;
                        // }
                        // if(camera.x > LEVEL_WIDTH - camera.w) {
                        //     camera.x = LEVEL_WIDTH - camera.w;
                        // }
                        // if(camera.y > LEVEL_HEIGHT - camera.h) {
                        //     camera.y = LEVEL_HEIGHT - camera.h;
                        // }
                        // stop moving things with the delta
                        break;
                    case OVER:
                        break;
                    default: 
                        break;
                }

                movement_timer.start();

                SDL_SetRenderDrawColor(renderer, 0x0, 0x0, 0x0, 0xFF); //black
                SDL_RenderClear(renderer);
                // render things
                render_scene();//V2(camera.x, camera.y));
                if (gs.state == PLAY) {
                    player.render();//V2(camera.x, camera.y)); // player and bullets rendered
                }
                render_status();
                if (collision_debug) {
                    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF); //white
                    //player collider
                    SDL_Rect col = player.collider;
                    // col.x -= camera.x;
                    // col.y -= camera.y;
                    SDL_RenderDrawRect(renderer, &col);

                    for (int y = 0; y < LEVEL_TILE_HEIGHT; y++) {
                        for (int x = 0; x < LEVEL_TILE_WIDTH; x++) {
                            if (gs.blocks[LEVEL_TILE_WIDTH*y+x] != NULL && gs.blocks[LEVEL_TILE_WIDTH*y+x]->sprite != EMPTY) {
                                col = gs.blocks[LEVEL_TILE_WIDTH*y+x]->collider;
                                // col.x -= camera.x;
                                // col.y -= camera.y;
                                SDL_RenderDrawRect(renderer, &col);
                            }
                        }
                    }

                    SDL_SetRenderDrawColor(renderer, 0x0, 0x0, 0x0, 0xFF); //black
                }

                SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF); //white

                SDL_SetRenderDrawColor(renderer, 0x0, 0x0, 0x0, 0xFF); //black
                
                // stop rendering things
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
                frame_ticks = frame_timer.get_ticks();
                if (frame_ticks < SCREEN_TICKS_PER_FRAME) {
                    SDL_Delay(SCREEN_TICKS_PER_FRAME - frame_ticks);
                }
            }
        }
    }
    close();
    return 0;
}
