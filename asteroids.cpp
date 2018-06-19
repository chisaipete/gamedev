#include "asteroids.h"

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
Texture spritemap;
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

class Ship {
public:
    static const int width  = 32; // pixels
    static const int height = 32;
    static const int rotation_speed = 90.0; //degrees / second
    Ship();
    void handle_event(SDL_Event &event);
    void move();
    void render();
private:
    v2 position;
    v2 velocity;
    double angle;
    double angular_velocity;
};

Ship::Ship() {
    position = {SCREEN_WIDTH/2,SCREEN_HEIGHT/2};
    velocity = {0,0};
    angle = 0.0; //clockwise, -> is 0, angle starts at 45 degrees offset from 0
    angular_velocity = 0.0;
}

void Ship::handle_event(SDL_Event &event) {
    if (event.type == SDL_KEYDOWN && event.key.repeat == 0) {
        switch (event.key.keysym.sym) {
            case SDLK_UP:
                angular_velocity -= rotation_speed;
                break;
            case SDLK_DOWN:
                angular_velocity += rotation_speed
                break;
            case SDLK_LEFT:
                break;
            case SDLK_RIGHT:
                break;
            default:
                break;
        }
    } else if (event.type == SDL_KEYUP && event.key.repeat == 0) {
        switch (event.key.keysym.sym) {
            case SDLK_UP:
                angular_velocity += rotation_speed;
                break;
            case SDLK_DOWN:
                angular_velocity -= rotation_speed;
                break;
            case SDLK_LEFT:
                break;
            case SDLK_RIGHT:
                break;
            default:
                break;
        }
    }
}

void Ship::move(float delta) { //actually moves the camera!
    // position += velocity;
    angle += speed * delta;
}

void Ship::render() {
    spritemap.render(position.x-(width/2), position.y-(height/2), &sprites[SHIP], angle);
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

    if (!spritemap.load_from_file("res/asteroids.png")) {
        success = false;
    } else {
        // set standard alpha blending
        // spritemap.set_blend_mode(SDL_BLENDMODE_BLEND);
        // spritemap.set_alpha(255);
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
    
    //TODO: set these up
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
            int frame_count = 0;
            int frame_ticks = 0;
            int delta_ticks = 0;
            float avgFPS = 0.0;

            //starting initialization
            gs.state = START;

            fps_timer.start();

            while (!quit) {
                frame_timer.start();
                SDL_Event event;

                Ship ship;

                while (SDL_PollEvent(&event)) {
                    if (event.type == SDL_QUIT) {
                            quit = true;
                    }
                    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE ) {
                        quit = true;
                    }
                    ship.handle_event(event);
                    if (event.type == SDL_KEYDOWN && event.key.repeat == 0) {
                        switch (event.key.keysym.sym) {
                            case SDLK_1:     
                                fps_on = !fps_on; 
                                break;
                            case SDLK_2:     
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
                }

                switch (gs.state) {
                    case START:
                        break;
                    case PAUSE:
                        // don't move stuff
                        break;
                    case PLAY:
                        // move stuff
                        break;
                    case OVER:
                        break;
                    default: 
                        break;
                }

                delta_ticks = movement_timer.get_ticks();
                float delta = delta_ticks / 1000.0;
                // move things with the delta
                ship.move(delta);
                // stop moving things with the delta
                movement_timer.start();

                SDL_SetRenderDrawColor(renderer, 0x0, 0x0, 0x0, 0xFF); //black
                SDL_RenderClear(renderer);
                // render things
                // render_scene();
                ship.render();
                render_status();
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
