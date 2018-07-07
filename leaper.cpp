#include "leaper.h"

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
Texture level_map;
Texture actor_map;
Timer frame_timer;
Timer movement_timer;
Timer fps_timer;
std::stringstream scoreText;
std::stringstream statusText;
std::stringstream fpsText;
Texture t_score;
Texture t_status;
Texture t_fps;
cute_tiled_map_t* tiled_map;
cute_tiled_tileset_t* level_tiles;
cute_tiled_tileset_t* actor_tiles;
std::vector<SDL_Rect> level_sprites;
int level_sprites_gid_offset;
SDL_Rect actor_sprites[7];
int actor_sprites_gid_offset;

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

struct EditorState {
    int active_sprite;
    bool level_edit;
};

EditorState es;

class Player {
    public:
        static const int width  = TILE_SIZE; // pixels
        static const int height = TILE_SIZE;
        static const int colw = TILE_SIZE/2;
        static const int colh = (TILE_SIZE/4)*3;
        static constexpr float max_speed = 200.0; //pixels / second
        static constexpr float jump_speed = 200.0; //pixels / second
        // static constexpr float acceleration = 320.0; //pixels / second^2
        static constexpr float gravity = 800.0; //pixels / second^2 (134)
        static constexpr float max_fall_speed = gravity; //pixels / second

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
        bool jump, in_jump;
        SDL_RendererFlip flip;
        int frame;
        int walk_frame;
};

Player::Player() {
    position = {LEVEL_WIDTH/2, LEVEL_HEIGHT-(SCREEN_HEIGHT/2)};
    frame = STAND0;
    walk_frame = WALK0;
    hp = 10;
    collider = {static_cast<int>(position.x)-width/2, static_cast<int>(position.y)-height/2, colw, colh};
    lives = 3;
    walk_speed = 0.0;
    velocity = {0,0};
    up = false;
    down = false;
    left = false;
    right = false;
    jump = false;
    in_jump = false;
    flip = SDL_FLIP_NONE;
}

void Player::handle_event(SDL_Event &event) {
    // FIXME: theses feel sloppy with respect to simultaneous presses
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
                if (!in_jump) {
                    jump = true;
                    in_jump = true;
                }
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
                jump = false;
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
    if (jump && !in_jump) {
        velocity.y = -jump_speed;
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

    int x_min, x_max, y_min, y_max;
    v2 tile_position = screen_to_tile(V2(collider.x,collider.y));
    x_min = tile_position.x - 1;
    if (x_min < 0) x_min = 0;
    x_max = tile_position.x + 1;
    if (x_max >= LEVEL_TILE_WIDTH) x_max = LEVEL_TILE_WIDTH -1;
    y_min = tile_position.y - 1;
    if (y_min < 0) y_min = 0;
    y_max = tile_position.y + 1;
    if (y_max >= LEVEL_TILE_HEIGHT) y_max = LEVEL_TILE_HEIGHT -1;


    unsigned collisions = 0b000;

    v2 top_of_ground = {0,0};
    for (int y = y_min; y <= y_max; y++) {
        for (int x = x_min; x <= x_max; x++) {
            if (gs.blocks[LEVEL_TILE_WIDTH*y+x] != NULL && gs.blocks[LEVEL_TILE_WIDTH*y+x]->sprite != EMPTY) {
                collisions |= check_collision(collider, gs.blocks[LEVEL_TILE_WIDTH*y+x]->collider);
                if (collisions & BOTTOM) {
                    top_of_ground.x = position.x;
                    top_of_ground.y = gs.blocks[LEVEL_TILE_WIDTH*y+x]->collider.y - collider.h;
                    break;
                }
            }
        }
    }

    // std::cout << screen_to_tile(position) << " " << collisions;
    // if (collisions & TOP) std::cout << " top";
    // if (collisions & BOTTOM) std::cout << " bottom";
    // if (collisions & LEFT) std::cout << " left";
    // if (collisions & RIGHT) std::cout << " right";
    // std::cout << std::endl;
    if (collisions & BOTTOM) {
        // set position to be just touching, not fall a delta distance again
        position = top_of_ground;
        velocity.y = 0.0;
        in_jump = false;
    } else {
        velocity.y += gravity * delta;
        if (velocity.y > max_fall_speed) {
            velocity.y = max_fall_speed;
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

    tiled_map = cute_tiled_load_map_from_file("res/leaper.json", NULL);
    if (tiled_map == nullptr) {
        success = false;
    } else {
        cute_tiled_tileset_t* ts = tiled_map->tilesets;
        while (ts) {
            if (strcmp(ts->image.ptr,"leaper_tiles.png") == 0) {
                std::cout << ts->tilecount << std::endl;
                level_sprites.reserve(ts->tilecount);
                level_sprites_gid_offset = ts->firstgid;
                int gid = ts->firstgid;
                int x = 0;
                int y = 0;
                int t = 0;
                while (t < ts->tilecount) {
                    std::cout << t;
                    SDL_Rect r;
                    r.x = x;
                    r.y = y;
                    r.w = ts->tilewidth;
                    r.h = ts->tileheight;
                    level_sprites.push_back(r);
                    x += ts->tilewidth;
                    if (x >= ts->imagewidth) {
                        x = 0;
                        y += ts->tileheight;
                    }
                    ++t;
                }
            } else if (strcmp(ts->image.ptr,"leaper_character.png") == 0) {
                actor_tiles = ts;
            }
            ts = ts->next;
        }
        std::cout << level_sprites.size() << std::endl;
        for (auto &t: level_sprites) {
            std::cout << t << std::endl;
        }

        // TODO: replace this with tilemap files and level map files (no more hardcodes)
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
    }

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
    // tmx_map_free(map);
    //TODO: free tilesets, cute_tiled* things
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
    return true;
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
    cute_tiled_layer_t* layer = tiled_map->layers;
    // iterate over layers
    while (layer) {
        for (int y = 0; y < tiled_map->height; y++) {
            for (int x = 0; x < tiled_map->width; x++) {
                level_map.render(
                    x*tiled_map->tilewidth, 
                    y*tiled_map->tileheight, 
                    &level_sprites[layer->data[tiled_map->width*y+x] - level_sprites_gid_offset]
                    );
            }
        }
        layer = layer->next;
    }

    if (es.level_edit) {
        if (es.active_sprite != EMPTY) {
            level_map.render(0,0, &level_sprites[es.active_sprite]);
        }
    }
}

//TODO: make this function load the level sprites properly
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

void cycle_edit_sprite() {
    es.active_sprite += 1;
    if (es.active_sprite >= level_sprites.size()) {
        es.active_sprite = 0;
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
            es.level_edit = false;
            es.active_sprite = GRASS0;

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
                            case SDLK_3:
                                es.level_edit = !es.level_edit;
                                break; 
                            case SDLK_4:
                                cycle_edit_sprite();
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
                    } else if (event.type == SDL_MOUSEBUTTONDOWN && es.level_edit) { // || event.type == SDL_MOUSEBUTTONUP) {
                        //Get mouse position
                        int x, y;
                        SDL_GetMouseState( &x, &y );
                        v2 target_tile = screen_to_tile(V2(x,y));
                        int index = LEVEL_TILE_WIDTH*static_cast<int>(target_tile.y)+static_cast<int>(target_tile.x);
                        if (gs.blocks[index] == NULL) {
                            gs.blocks[index] = new Block();
                        }
                        gs.blocks[index]->sprite = es.active_sprite;
                        switch (es.active_sprite) {
                            case EMPTY:
                            case GRASS0:
                            case GRASS1:
                            case GRASS2:
                                gs.blocks[index]->collider = {0,0,0,0};
                                break;
                            case GROUND0:
                            case GROUND1:
                            case GROUND2:
                                gs.blocks[index]->collider = {static_cast<int>(target_tile.x)*TILE_SIZE, static_cast<int>(target_tile.y)*TILE_SIZE, TILE_SIZE, TILE_SIZE};
                                break;
                            case GRASS_LEDGE:
                            case STONE_LEDGE:
                                gs.blocks[index]->collider = {static_cast<int>(target_tile.x)*TILE_SIZE, static_cast<int>(target_tile.y)*TILE_SIZE, TILE_SIZE, TILE_SIZE/3};
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
