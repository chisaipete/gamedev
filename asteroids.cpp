#include "asteroids.h"

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
Texture spritemap;
RawTexture starfield;
SDL_Rect sprites[19];
Timer frame_timer;
Timer movement_timer;
Timer fps_timer;
std::stringstream scoreText;
std::stringstream statusText;
std::stringstream fpsText;
Texture t_score;
Texture t_status;
Texture t_fps;

struct GameState {
    int state;
    int score;
    int ships;
};

GameState gs;

bool check_box_collision(SDL_Rect a, SDL_Rect b) {
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

class Asteroid {
    public:
        Asteroid(v2 spawn_position);
        // ~Asteroid();
        void move(float delta);
        void render(v2 camera);
        void set_hp(int size);
        SDL_Rect collider;
    private:
        int width;
        int height;
        v2 position;
        int speed;
        double angle;
        double angular_velocity;
        int size;
        int hp;
};

Asteroid::Asteroid(v2 spawn_position) {
    size = AST_M0;
    width = sprites[size].w;
    height = sprites[size].h;
    int x_sign = (rand() % 2 ? -1 : 1);
    int y_sign = (rand() % 2 ? -1 : 1);
    position = {
        spawn_position.x + ((rand() % 100)*x_sign + 50*x_sign),
        spawn_position.y + ((rand() % 100)*y_sign + 50*y_sign) 
    };
    speed = rand() % 20; // pixels/second
    angle = rand() % 360;
    angular_velocity = rand() % 90; // degrees /second
    collider = {position.x-width/2, position.y-height/2, width, height};
}

void Asteroid::set_hp(int size) {
    switch (size) {
        case AST_T0:
        case AST_T1:
        case AST_T2:
        case AST_T3:
            hp = 1;
            break;
        case AST_S0:
        case AST_S1:
        case AST_S2:
        case AST_S3:
            hp = 2;
            break;
        case AST_M0:
        case AST_M1:
        case AST_M2:
        case AST_M3:
            hp = 3;
            break;
        case AST_L0:
            hp = 4;
            break;
    }
}

void Asteroid::move(float delta) {
    position.x += speed * delta;
    position.y += speed * delta;
    angle += angular_velocity * delta;
    collider.x = position.x-width/2;
    collider.y = position.y-height/2;
}

void Asteroid::render(v2 camera) {
    spritemap.render(position.x-(width/2)-camera.x, position.y-(height/2)-camera.y, &sprites[size], angle);
}

class Bullet {
    public:
        static const int width = 5;
        static const int height = 5;
        static const int speed = 640.0; //pixels / second
        Bullet(v2 ship_position, double ship_angle);
        // void handle_event(SDL_Event &event);
        bool move(float delta, std::vector<Asteroid> &asteroids);
        void render(v2 camera);
        float get_time_to_death();
        SDL_Rect collider;
    private:
        v2 position;
        double angle;
        float time_to_death;
};

Bullet::Bullet(v2 ship_position, double ship_angle) {
    position = {ship_position.x, ship_position.y};
    angle = ship_angle;
    time_to_death = 1.0;
    collider = {position.x-width/2, position.y-height/2, width, height};
}

bool Bullet::move(float delta, std::vector<Asteroid> &asteroids) {
    position.x += cos((angle+45.0)*PI/180.0) * speed * delta;
    position.y += sin((angle+45.0)*PI/180.0) * speed * delta;
    time_to_death -= delta;
    collider.x = position.x-width/2;
    collider.y = position.y-height/2;

    // check for collision with asteroids
    std::vector<Asteroid>::iterator astr;
    for (astr = asteroids.begin(); astr != asteroids.end();) {
        if(check_box_collision(collider, astr->collider)) {
            time_to_death = 0.0;
            astr = asteroids.erase(astr);
        } else {
            ++astr;
        }
    }
}

void Bullet::render(v2 camera) {
    spritemap.render(position.x-(width/2)-camera.x, position.y-(height/2)-camera.y, &sprites[BULLET], angle);
}

float Bullet::get_time_to_death() {
    return time_to_death;
}

class Ship {
    public:
        static const int width  = 32; // pixels
        static const int height = 32;
        static const int thrust_width  = 28; // pixels
        static const int thrust_height = 28;
        static const int thrust_speed = 320.0; //pixels / second
        static const int rotation_speed = 360.0; //degrees / second
        Ship();
        void handle_event(SDL_Event &event);
        void move(float delta, std::vector<Asteroid> &asteroids);
        void render(v2 camera);
        v2 get_position();
        std::vector<Bullet> shots; 
        SDL_Rect collider;
        void fire();
    private:
        v2 position;
        double angle;
        double angular_velocity;
        int frame;
        bool thruster;
        int hp;
};

Ship::Ship() {
    position = {LEVEL_WIDTH/2,LEVEL_HEIGHT/2};
    angle = 0.0; //clockwise, -> is 0, angle starts at 45 degrees offset from 0
    angular_velocity = 0.0;
    frame = 0;
    thruster = false;
    hp = 1;
    collider = {position.x-width/2, position.y-height/2, width, height};
}

void Ship::handle_event(SDL_Event &event) {
    if (event.type == SDL_KEYDOWN) { // && event.key.repeat == 0) {
        switch (event.key.keysym.sym) {
            case SDLK_UP:
                angular_velocity = -rotation_speed;
                break;
            case SDLK_DOWN:
                angular_velocity = rotation_speed;
                break;
            case SDLK_LEFT:
                thruster = true;
                break;
            case SDLK_RIGHT:
                break;
            case SDLK_SPACE:
                fire();
                break;
            default:
                break;
        }
    } else if (event.type == SDL_KEYUP) { // && event.key.repeat == 0) {
        switch (event.key.keysym.sym) {
            case SDLK_UP:
                angular_velocity = 0.0;
                break;
            case SDLK_DOWN:
                angular_velocity = 0.0;
                break;
            case SDLK_LEFT:
                thruster = false;
                break;
            case SDLK_RIGHT:
                break;
            case SDLK_SPACE:
                break;
            default:
                break;
        }
    }
}

void Ship::move(float delta, std::vector<Asteroid> &asteroids) {
    v2 position_o = position;

    // position += velocity;
    angle += angular_velocity * delta;

    if (thruster) { //clockwise, -> is 0, angle starts at 45 degrees offset from 0
        position.x += cos((angle+45.0)*PI/180.0) * thrust_speed * delta;
        position.y += sin((angle+45.0)*PI/180.0) * thrust_speed * delta;
    }

    collider.x = position.x-width/2;
    collider.y = position.y-height/2;

    // check for collision with asteroids
    std::vector<Asteroid>::iterator astr;
    for (astr = asteroids.begin(); astr != asteroids.end();) {
        if(check_box_collision(collider, astr->collider)) {
            position = position_o;
            collider.x = position.x;
            collider.y = position.y;
            thruster = false;
        }
        ++astr;
    }

    // move bullets
    // check expiration
    std::vector<Bullet>::iterator itr;
    for (itr = shots.begin(); itr != shots.end();) {
        itr->move(delta, asteroids);
        if (itr->get_time_to_death() <= 0) {
            itr = shots.erase(itr);
        } else {
            ++itr;
        }
    }
}

void Ship::render(v2 camera) {
    spritemap.render(position.x-(width/2)-camera.x, position.y-(height/2)-camera.y, &sprites[SHIP], angle);

    if (thruster) {
        //animate thruster
        SDL_Point rot_pos = {thrust_width+2,thrust_height+2};
        spritemap.render(position.x-(thrust_width+2)-camera.x, position.y-(thrust_height+2)-camera.y, &sprites[frame / 4], angle, &rot_pos);
        frame++;
        if (frame / 4 >= ANIMATION_FRAMES) {  // loop animation
            frame = THRUST0;
        }
    }
    // render bullets
    for ( auto &i : shots) {
        i.render(camera);
    }
}

v2 Ship::get_position() {
    return position;
}

void Ship::fire() {
    shots.push_back(Bullet(position, angle));
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
            window = SDL_CreateWindow("Asteroids", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
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

    if (!spritemap.load_from_file("res/asteroids.png")) {
        success = false;
    } else {
        // set standard alpha blending
        // spritemap.set_blend_mode(SDL_BLENDMODE_BLEND);
        // spritemap.set_alpha(255);
    }

    if (!starfield.initialize(LEVEL_WIDTH, LEVEL_HEIGHT)) {
        success = false;
    } else {
        // generate starfield
        if (!starfield.lock_texture()) {
            std::cout << "Unable to lock starfield texture" << std::endl;
        } else {
            int format = SDL_GetWindowPixelFormat(window);
            SDL_PixelFormat* mapping_format = SDL_AllocFormat(format);
            int* pixels = static_cast<int*>(starfield.get_pixels());
            int pixel_count = (starfield.get_pitch() / 4) * starfield.get_height();
            int black = SDL_MapRGBA(mapping_format, 0x00, 0x00, 0x00, 0xFF);
            int white = SDL_MapRGBA(mapping_format, 0xFF, 0xFF, 0xFF, 0xFF);
            int star;
            // fill starfield with color
            for (int i = 0; i < pixel_count; i++) {
                star = static_cast<int>(rand() % 1000);
                if (star > 998) {
                    pixels[i] = white;
                } else {
                    pixels[i] = black;
                }
            }
            starfield.unlock_texture();
            SDL_FreeFormat(mapping_format);
        }
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
    
    sprites[THRUST0].x = 32;
    sprites[THRUST0].y = 0;
    sprites[THRUST0].w = 28;
    sprites[THRUST0].h = 28;
    sprites[THRUST1].x = 60;
    sprites[THRUST1].y = 0;
    sprites[THRUST1].w = 28;
    sprites[THRUST1].h = 28;
    sprites[THRUST2].x = 88;
    sprites[THRUST2].y = 0;
    sprites[THRUST2].w = 28;
    sprites[THRUST2].h = 28;
    sprites[THRUST3].x = 116;
    sprites[THRUST3].y = 0;
    sprites[THRUST3].w = 28;
    sprites[THRUST3].h = 28;
    sprites[BULLET].x = 144;
    sprites[BULLET].y = 0;
    sprites[BULLET].w = 5;
    sprites[BULLET].h = 5;
    sprites[SHIP].x = 0;
    sprites[SHIP].y = 0;
    sprites[SHIP].w = 32;
    sprites[SHIP].h = 32;
    sprites[AST_T0].x = 0;
    sprites[AST_T0].y = 64;
    sprites[AST_T0].w = 16;
    sprites[AST_T0].h = 16;
    sprites[AST_T1].x = 16;
    sprites[AST_T1].y = 64;
    sprites[AST_T1].w = 16;
    sprites[AST_T1].h = 16;
    sprites[AST_T2].x = 32;
    sprites[AST_T2].y = 64;
    sprites[AST_T2].w = 16;
    sprites[AST_T2].h = 16;
    sprites[AST_T3].x = 48;
    sprites[AST_T3].y = 64;
    sprites[AST_T3].w = 16;
    sprites[AST_T3].h = 16;
    sprites[AST_S0].x = 0;
    sprites[AST_S0].y = 32;
    sprites[AST_S0].w = 32;
    sprites[AST_S0].h = 32;
    sprites[AST_S1].x = 32;
    sprites[AST_S1].y = 32;
    sprites[AST_S1].w = 32;
    sprites[AST_S1].h = 32;
    sprites[AST_S2].x = 64;
    sprites[AST_S2].y = 32;
    sprites[AST_S2].w = 32;
    sprites[AST_S2].h = 32;
    sprites[AST_S3].x = 64;
    sprites[AST_S3].y = 64;
    sprites[AST_S3].w = 32;
    sprites[AST_S3].h = 32;
    sprites[AST_M0].x = 0;
    sprites[AST_M0].y = 96;
    sprites[AST_M0].w = 64;
    sprites[AST_M0].h = 64;
    sprites[AST_M1].x = 64;
    sprites[AST_M1].y = 96;
    sprites[AST_M1].w = 64;
    sprites[AST_M1].h = 64;
    sprites[AST_M2].x = 128;
    sprites[AST_M2].y = 96;
    sprites[AST_M2].w = 64;
    sprites[AST_M2].h = 64;
    sprites[AST_M3].x = 96;
    sprites[AST_M3].y = 32;
    sprites[AST_M3].w = 64;
    sprites[AST_M3].h = 64;
    sprites[AST_L0].x = 192;
    sprites[AST_L0].y = 32;
    sprites[AST_L0].w = 128;
    sprites[AST_L0].h = 128;

    return success;
}

bool close() {
    spritemap.free();
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
    scoreText << gs.score;
    t_score.load_from_rendered_text(scoreText.str());
    t_score.render((SCREEN_WIDTH/2)-(t_score.get_width()/2), SCREEN_HEIGHT-(t_score.get_height()));

    statusText.str("");
    switch (gs.state) {
        case START:
            statusText << "ASTEROIDS";
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

void render_scene(bool debug = false) {
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

            Ship ship;
            v2 ship_position;
            ship_position = ship.get_position();
            SDL_Rect camera = {ship_position.x - (SCREEN_WIDTH / 2), ship_position.y - (SCREEN_HEIGHT / 2), SCREEN_WIDTH, SCREEN_HEIGHT};
            std::vector<Asteroid> asteroids;

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
                                asteroids.push_back(Asteroid(ship.get_position()));
                                break;
                            case SDLK_KP_ENTER:
                            case SDLK_RETURN:
                                switch (gs.state) {
                                    case START:
                                        gs.state = PLAY;
                                        break;
                                    case PAUSE:
                                        gs.state = PLAY;
                                        break;
                                    case PLAY: 
                                        gs.state = PAUSE; 
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
                    ship.handle_event(event);
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
                        ship.move(delta, asteroids);  //and bullets
                        ship_position = ship.get_position();
                        // move asteroids
                        for ( auto &a : asteroids) {
                            a.move(delta);
                        }
                        camera.x = ship_position.x - (SCREEN_WIDTH / 2);
                        camera.y = ship_position.y - (SCREEN_HEIGHT / 2);
                        // keep the camera in level bounds
                        if(camera.x < 0) { 
                            camera.x = 0;
                        }
                        if(camera.y < 0) {
                            camera.y = 0;
                        }
                        if(camera.x > LEVEL_WIDTH - camera.w) {
                            camera.x = LEVEL_WIDTH - camera.w;
                        }
                        if(camera.y > LEVEL_HEIGHT - camera.h) {
                            camera.y = LEVEL_HEIGHT - camera.h;
                        }
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
                // render_scene();
                starfield.render(0, 0, &camera);
                if (gs.state == PLAY) {
                    ship.render(V2(camera.x, camera.y)); // ship and bullets rendered
                    // render asteroids
                    for ( auto &a : asteroids) {
                        a.render(V2(camera.x, camera.y));
                    }
                }
                render_status();
                if (collision_debug) {
                    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF); //white
                    //ship collider
                    SDL_Rect scol = ship.collider;
                    scol.x -= camera.x;
                    scol.y -= camera.y;
                    SDL_RenderDrawRect(renderer, &scol);
                    //bullet colliders
                    for ( auto &b : ship.shots) {
                        SDL_Rect bcol = b.collider;
                        bcol.x -= camera.x;
                        bcol.y -= camera.y;
                        SDL_RenderDrawRect(renderer, &bcol);
                    }
                    //asteroid colliders
                    for ( auto &a : asteroids) {
                        SDL_Rect acol = a.collider;
                        acol.x -= camera.x;
                        acol.y -= camera.y;
                        SDL_RenderDrawRect(renderer, &acol);
                    }
                    SDL_SetRenderDrawColor(renderer, 0x0, 0x0, 0x0, 0xFF); //black
                }
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
