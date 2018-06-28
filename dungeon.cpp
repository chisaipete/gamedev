#include "dungeon.h"

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
Timer frame_timer;
Timer movement_timer;
Timer fps_timer;
std::stringstream statusText;
std::stringstream fpsText;
Texture t_status;
Texture t_fps;

struct UI_State {
    int state;
};

UI_State ui;

bool init() {
    bool success = true;

    if (SDL_Init(SDL_INIT_VIDEO) != 0){
        logSDLError(std::cout, "SDL_Init");
        success = false;
    } else {       
        if (TTF_Init() != 0){
            logSDLError(std::cout, "TTF_Init");
            success = false;
        } else {
            window = SDL_CreateWindow("Dungeon", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
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

    font = TTF_OpenFont("res/cc.ttf", 12);
    if (font == nullptr){
        logSDLError(std::cout, "TTF_OpenFont");
        success = false;
    }
    if (!t_status.load_from_rendered_text("_")) {
        success = false;
    }
    if (!t_fps.load_from_rendered_text("_", RED_)) {
        success = false;
    }
    return success;
}

void delete_level() {
}

bool close() {
    delete_level();
    t_status.free();
    t_fps.free();
    if (font != nullptr) { TTF_CloseFont(font); font = NULL; }
    if (renderer != nullptr) { SDL_DestroyRenderer(renderer); renderer = NULL; }
    if (window != nullptr) { SDL_DestroyWindow(window); window = NULL; }
    TTF_Quit();
    SDL_Quit();
}

class Room {
    friend class Level;
    public:
        Room(v2 origin);
        // ~Room();
        SDL_Rect get_collider();
        const int id;
        void render();
        void render_collider();
    private:
        static int cid;
        v2 position;
        SDL_Color color;
        SDL_Rect collider;
        std::vector<v2> boundary;
};

int Room::cid = 0;

Room::Room(v2 origin) : id(cid++) {
    position = origin;
    color = WHITE;
    boundary.clear();
    // TODO: boundary should be generated based on type of room, and should be a complete polygon
    // +--------+
    // |        |
    // |    +---+
    // |    |
    // +----+
    // TODO: boundary may also have max size constraints for generation
    boundary.push_back(V2(0,0));
    boundary.push_back(V2(20,0));
    boundary.push_back(V2(20,10));
    boundary.push_back(V2(10,10));
    boundary.push_back(V2(10,20));
    boundary.push_back(V2(0,20));
    boundary.push_back(V2(0,0));

    int x_min = SCREEN_WIDTH;
    int x_max = 0;
    int y_min = SCREEN_HEIGHT;
    int y_max = 0;

    for (auto &pt : boundary) {
        if (pt.x < x_min) {
            x_min = pt.x;
        }
        if (pt.x > x_max) {
            x_max = pt.x;
        }
        if (pt.y < y_min) {
            y_min = pt.y;
        }
        if (pt.y > y_max) {
            y_max = pt.y;
        }
    }
    collider = {x_min, y_min, x_max-x_min, y_max-y_min};
}

void Room::render() {
    v2 prev = V2(-999,-999);
    for (auto &pt : boundary) {
        if (prev.x != -999) {
            SDL_RenderDrawLine(renderer, position.x+prev.x, position.y+prev.y, position.x+pt.x, position.y+pt.y);
        }
        prev.x = pt.x;
        prev.y = pt.y;
    }
}

SDL_Rect Room::get_collider() {
    SDL_Rect collider_box = {position.x+collider.x, position.y+collider.y, collider.w, collider.h};
    return collider_box;
}

void Room::render_collider() {
    SDL_Rect collider_box = get_collider();
    SDL_RenderDrawRect(renderer, &collider_box);
}

class Level {
    friend class Room;
    public:
        Level();
        // ~Level();
        void handle_event(SDL_Event &event);
        void spawn_room();
        std::vector<Room> get_rooms();
    private:
        std::vector<Room> rooms;
        v2 origin;
};

Level::Level() {
    rooms.clear();
    origin = V2(SCREEN_WIDTH/2,SCREEN_HEIGHT/2);
}

void Level::spawn_room() {
    rooms.push_back(Room(origin));

    //check collisions with all older rooms, and adjust by surface annealing
    if (rooms.size() > 1) { 
        bool bad_location = true;
        while (bad_location) {
            auto rm = rooms.rbegin();
            ++rm; //skip last room (one just added)
            for (auto end=rooms.rend(); rm != end; ++rm) {
                // std::cout << rooms.back().get_collider() << "[" << rooms.back().id << "] vs [" << rm->id << "]" << rm->get_collider() << std::endl;
                // TODO: check for new room offset so we can have an irregular grid and irregular room sizes
                if(check_collision(rooms.back().get_collider(), rm->get_collider())) {
                    bad_location = true;
                    break;
                } else {
                    bad_location = false;
                }
            }
            if (bad_location) {
                //TODO: check for going out of a max boundary?
                // if we collide, choose random N S E W direction and push out the new room by a grid amount
                int direction = rand() % 4;
                switch (direction) {
                    case NORTH:
                        rooms.back().position.y -= 20;
                        break;
                    case SOUTH:
                        rooms.back().position.y += 20;
                        break;
                    case EAST:
                        rooms.back().position.x += 20;
                        break;
                    case WEST:
                        rooms.back().position.x -= 20;
                        break;
                }
            }
        }
    }

}

void Level::handle_event(SDL_Event &event) {
    if (event.type == SDL_KEYDOWN) { // && event.key.repeat == 0) {
        switch (event.key.keysym.sym) {
            case SDLK_a:
                spawn_room();
                break;
            case SDLK_UP:
            case SDLK_DOWN:
            case SDLK_LEFT:
            case SDLK_RIGHT:
            case SDLK_SPACE:
            default:
                break;
        }
    } else if (event.type == SDL_KEYUP) { // && event.key.repeat == 0) {
        switch (event.key.keysym.sym) {
            case SDLK_a:
                break;
            case SDLK_UP:
            case SDLK_DOWN:
            case SDLK_LEFT:
            case SDLK_RIGHT:
            case SDLK_SPACE:
            default:
                break;
        }
    }
}

std::vector<Room> Level::get_rooms() {
    return rooms;
}

void render_status(Level level) {
    statusText.str("");
    switch (ui.state) {
        case INFO:
            statusText << "D U N G E O N";
            break;
        case ACTIVE:
        default:
            statusText << "Rooms: " << level.get_rooms().size();
            break;
    }
    t_status.load_from_rendered_text(statusText.str());
    t_status.render(5, SCREEN_HEIGHT-t_status.get_height());
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

            //starting initialization
            ui.state = INFO;
            Level level = Level();
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
                                switch (ui.state){
                                    case INFO:
                                        ui.state = ACTIVE;
                                        break;
                                    case ACTIVE:
                                        ui.state = INFO;
                                        break;
                                    default:
                                        break;
                                }
                                break;
                        }
                    }
                    level.handle_event(event);
                }

                delta_ticks = movement_timer.get_ticks();
                float delta = delta_ticks / 1000.0;
                // move things with the delta
                // std::cout << "Motion START" << std::endl;
                switch (ui.state){
                    case INFO:
                    case ACTIVE:
                    default:
                        break;
                }
                // stop moving things with the delta
                movement_timer.start();
                // std::cout << "Motion END" << std::endl;

                // std::cout << "Render START" << std::endl;
                SDL_SetRenderDrawColor(renderer, 0x0, 0x0, 0x0, 0xFF); //black
                SDL_RenderClear(renderer);
                // render things
                SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF); //white
                for (auto &room : level.get_rooms()) {
                    room.render();
                }
                SDL_SetRenderDrawColor(renderer, 0x0, 0x0, 0x0, 0xFF); //black
                render_status(level);

                if (collision_debug) {
                    SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 0xFF); //red
                    for (auto &room : level.get_rooms()) {
                        room.render_collider();
                    }
                    SDL_SetRenderDrawColor(renderer, 0x0, 0x0, 0x0, 0xFF); //black
                }
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
                // stop rendering things
                SDL_RenderPresent(renderer);
                frame_count++;
                // std::cout << "Render END" << std::endl;

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


