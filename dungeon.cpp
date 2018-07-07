#include "dungeon.h"

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
Timer frame_timer;
Timer movement_timer;
Timer generation_timer;
Timer fps_timer;
std::stringstream statusText;
std::stringstream fpsText;
Texture t_status;
Texture t_fps;
int generation_length;

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
    if (!t_fps.load_from_rendered_text("_", RED)) {
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
    return true;
}

struct Door {
    int face;
    //int width;
    //int offset;
};

class Room {
    friend class Level;
    public:
        Room(v2 origin);
        // ~Room();
        SDL_Rect get_collider();
        const int id;
        void render();
        void render_doors();
        void render_collider();
        void set_door(int last_direction);
    protected:
        static int cid;
        v2 position;
        SDL_Color color;
        SDL_Rect collider;
        std::vector<v2> boundary;
        std::vector<Door> doors;
};

int Room::cid = 0;

Room::Room(v2 origin) : id(cid++) {
    position = origin;
    color = WHITE;
    // TODO: randomize color, or make color representative of type
    boundary.clear();

    // TODO: boundary should be generated based on type of room, and should be a complete polygon

    // +--------+
    // |        |
    // |    +---+
    // |    |
    // +----+
    // boundary.push_back(V2(0,0));
    // boundary.push_back(V2(20,0));
    // boundary.push_back(V2(20,10));
    // boundary.push_back(V2(10,10));
    // boundary.push_back(V2(10,20));
    // boundary.push_back(V2(0,20));
    // boundary.push_back(V2(0,0));

    // Generation using simple max-min constraints for rectangular rooms
    int min_width = 20;
    int max_width = 40;
    int min_height = 20;
    int max_height = 40;
    int width = (rand() % max_width) + min_width;
    int height = (rand() % max_height) + min_height;

    // Generation using 20px rooms and 20px grid
    width = 20;
    height = 20;

    boundary.push_back(V2(0,0));
    boundary.push_back(V2(width,0));
    boundary.push_back(V2(width,height));
    boundary.push_back(V2(0,height));
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
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    v2 prev = V2(-999,-999);
    for (auto &pt : boundary) {
        if (prev.x != -999) {
            SDL_RenderDrawLine(renderer, position.x+prev.x, position.y+prev.y, position.x+pt.x, position.y+pt.y);
        }
        prev.x = pt.x;
        prev.y = pt.y;
    }
}

void Room::set_door(int last_direction) {
    int face;
    switch (last_direction) {
        case NORTH: face = SOUTH; break;
        case SOUTH: face = NORTH; break;
        case EAST:  face = WEST;  break;
        case WEST:  face = EAST;  break;
    }
    doors.push_back({face});
}

void Room::render_doors() {
    // SDL_SetRenderDrawColor(renderer, 0x0, 0xFF, 0x0, 0xFF); //green
    SDL_SetRenderDrawColor(renderer, 0x0, 0x0, 0x0, 0xFF); //black
    int x, y, w, h;
    for (auto &dr : doors) {
        switch (dr.face) {
            case NORTH:
                w = 10;
                h = 5;
                x = 5;
                y = -2;
                break;
            case SOUTH:
                w = 10;
                h = 5;
                x = 5;
                y = collider.h-2;
                break;
            case EAST:
                w = 5;
                h = 10;
                x = collider.w-2;
                y = 5;
                break;
            case WEST:
                w = 5;
                h = 10;
                x = -2;
                y = 5;
                break;
        }
        SDL_Rect door = {position.x+x, position.y+y, w, h};
        SDL_RenderFillRect(renderer, &door);
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

class Cell : public Room {
    friend class Level;
    friend class Field;
    public:
        Cell(v2 origin, int _size);
        void set_next_state(bool _state);
        void commit_next_state();
        void toggle_state();
        void render();
    protected:
        bool state;
        bool next_state;
        int size;
};

Cell::Cell(v2 origin, int _size) : Room(origin) {
    position = origin;
    state = false;
    next_state = false;
    color = BLACK;
    size = _size;
    boundary.clear();
    boundary.push_back(V2(0,0));
    boundary.push_back(V2(size,0));
    boundary.push_back(V2(size,size));
    boundary.push_back(V2(0,size));
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

void Cell::set_next_state(bool _state) {
    next_state = _state;
}

void Cell::commit_next_state() {
    state = next_state;
    // next_state = state;
    if (state) {
        color = WHITE;
    } else {
        color = BLACK;
    }
}

void Cell::toggle_state() {
    state = !state;
    // next_state = state;
    if (state) {
        color = WHITE;
    } else {
        color = BLACK;
    }
}

void Cell::render() {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_Rect cell_box = {position.x+collider.x, position.y+collider.y, size, size};
    SDL_RenderFillRect(renderer, &cell_box);
}

class Level {
    friend class Room;
    friend class Cell;
    public:
        Level();
        // ~Level();
        void handle_event(SDL_Event &event);
        void spawn_room();
        std::vector<Room> get_rooms();
    protected:
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
        int direction = -1;
        while (bad_location) {
            auto rm = rooms.rbegin();
            ++rm; //skip last room (one just added)
            SDL_Rect hit_room;
            for (auto end=rooms.rend(); rm != end; ++rm) {
                // std::cout << rooms.back().get_collider() << "[" << rooms.back().id << "] vs [" << rm->id << "]" << rm->get_collider() << std::endl;
                // TODO: check for new room offset so we can have an irregular grid and irregular room sizes
                if(check_collision(rooms.back().get_collider(), rm->get_collider())) {
                    hit_room = rm->get_collider();
                    bad_location = true;
                    break;
                } else {
                    bad_location = false;
                }
            }
            if (bad_location) {
                // std::cout << hit_room << std::endl;
                //TODO: check for going out of a max boundary?
                // if we collide, choose random N S E W direction and push out the new room by a grid amount
                direction = rand() % 4;
                switch (direction) {
                    case NORTH:
                        rooms.back().position.y -= hit_room.h;
                        break;
                    case SOUTH:
                        rooms.back().position.y += hit_room.h;
                        break;
                    case EAST:
                        rooms.back().position.x += hit_room.w;
                        break;
                    case WEST:
                        rooms.back().position.x -= hit_room.w;
                        break;
                }
            }
        }
        if (direction != -1) {
            rooms.back().set_door(direction);
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

class Field : public Level {
    friend class Cell;
    friend class Room;
    public:
        Field(int _size);
        void handle_event(SDL_Event &event);
        void step_time();
        std::vector<Cell> get_rooms();
        int get_live_neighbors(v2 location);
        void print_state();
        void print_next_state();
    protected:
        std::vector<Cell> cells;
        int size;
};

Field::Field(int _size = 20) {
    size = _size;
    int cell_size = 20;
    origin = V2(SCREEN_WIDTH/2, SCREEN_HEIGHT/2);
    origin.x = origin.x - (size/2)*cell_size;
    origin.y = origin.y - (size/2)*cell_size;
    cells.clear();
    cells.reserve(size*size);
    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            cells.push_back(Cell(V2(origin.x + (x*cell_size), origin.y + (y*cell_size)), cell_size));
        }
    }
}

//TODO: pattern loader: https://en.wikipedia.org/wiki/File:Game_of_life_glider_gun.svg
//TODO: auto tick generations (how fast can we go!?)

void Field::handle_event(SDL_Event &event) {
    if (event.type == SDL_KEYDOWN) { // && event.key.repeat == 0) {
        switch (event.key.keysym.sym) {
            case SDLK_t:
                step_time();
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
    } else if (event.type == SDL_MOUSEBUTTONDOWN) { // || event.type == SDL_MOUSEBUTTONUP) {
        //Get mouse position
        int x, y;
        SDL_GetMouseState( &x, &y );
        for (auto &cell : cells) {
            if (x >= cell.position.x && x < cell.position.x + cell.size && y >= cell.position.y && y < cell.position.y + cell.size) {
                cell.toggle_state();
                break;
            }
        }
    }
}

int Field::get_live_neighbors(v2 loc) {
    int alive = 0;
    //not allowing wrap at boundaries
    //sum states of all eight neighbors
    // std::cout << "checking: " << loc << " id:" << cells[size*loc.y+loc.x].id << std::endl;
    for (int y = loc.y - 1; y < loc.y + 2; y++) {
        for (int x = loc.x -1; x < loc.x + 2; x++) {
            try {
                if (cells.at(size*y+x).state) {
                    if ((V2(x,y) == loc)) {
                        // std::cout << "  i" << cells[size*y+x].id << "->" << V2(x,y) << 'S';
                        throw std::out_of_range("force bad cell");
                    } else if (loc.x == size-1 && x > loc.x) {
                        throw std::out_of_range("force bad cell");
                    } else if (loc.x == 0 && x < loc.x) {
                        throw std::out_of_range("force bad cell");
                    } else {
                        ++alive;
                        // std::cout << "  i" << cells[size*y+x].id << "->" << V2(x,y) << 'A';
                    }
                } else {
                    if ((V2(x,y) == loc)) {
                        // std::cout << "  i" << cells[size*y+x].id << "->" << V2(x,y) << 'S';
                        throw std::out_of_range("force bad cell");
                    } else if (loc.x == size-1 && x > loc.x) {
                        throw std::out_of_range("force bad cell");
                    } else if (loc.x == 0 && x < loc.x) {
                        throw std::out_of_range("force bad cell");
                    } else {
                        // std::cout << "  i" << cells[size*y+x].id << "->" << V2(x,y) << 'D';
                    }
                }
            }
            catch (const std::out_of_range &error) {
                // std::cout << V2(x,y) << " isn't a valid cell!" << std::endl;
                // std::cout << "  i" << "X" << "->" << V2(x,y) << 'X';
            }
            // std::cout << std::endl;
        }
    }
    // std::cout << loc << " n_alive:" << alive << std::endl;
    return alive;
}

void Field::step_time() {
    // Here is where we program automata
    // std::cout << "Current State" << std::endl;
    // print_state();
    // std::cout << "Current Next State" << std::endl;
    // print_next_state();
    // std::cout << "Alive Neighbor Counts" << std::endl;
    Cell* c;
    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            c = &cells[size*y+x];
            int alive_neigh = get_live_neighbors(V2(x,y));
            // std::cout << alive_neigh;
            if (c->state) { //alive
                if (alive_neigh < 2) {
                    c->set_next_state(DIE); //underpopulation
                } else if (alive_neigh > 3) {
                    c->set_next_state(DIE); //overpopulation
                } else { //2 or 3 neighbors
                    c->set_next_state(LIVE); //live to next generation
                }
            } else { //dead
                if (alive_neigh == 3) {
                    c->set_next_state(LIVE); //reproduce
                } else {
                    c->set_next_state(DIE); //rot
                }
            }
        }
        // std::cout << std::endl;
    }
    // std::cout << "Current State" << std::endl;
    // print_state();
    // std::cout << "Updated Next State" << std::endl;
    // print_next_state();
    // Commit state simultaneously
    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            cells[size*y+x].commit_next_state();
        }
    }
    // std::cout << "New State" << std::endl;
    // print_state();
    // std::cout << "New Next State" << std::endl;
    // print_next_state();
    // std::cout << "----------------------" << std::endl;
}

std::vector<Cell> Field::get_rooms() {
    return cells;
}

void Field::print_state() {
    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            std::cout << cells[size*y+x].state ? "1" : "0";
        }
        std::cout << std::endl;
    }
}

void Field::print_next_state() {
    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            std::cout << cells[size*y+x].next_state ? "1" : "0";
        }
        std::cout << std::endl;
    }
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
            int cur_ticks = 0;
            float avgFPS = 0.0;

            //starting initialization
            ui.state = INFO;
            generation_length = 300;
            // Level level = Level();
            Field level = Field(37);
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
                            case SDLK_COMMA:
                            case SDLK_LESS:
                                generation_length -= 10;
                                break;
                            case SDLK_PERIOD:
                            case SDLK_GREATER:
                                generation_length += 10;
                                break;
                            case SDLK_KP_ENTER:
                            case SDLK_RETURN:
                                switch (ui.state){
                                    case INFO:
                                        ui.state = ACTIVE;
                                        generation_timer.start();
                                        break;
                                    case ACTIVE:
                                        ui.state = PAUSE;
                                        generation_timer.pause();
                                        break;
                                    case PAUSE:
                                        ui.state = ACTIVE;
                                        generation_timer.unpause();
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
                    case PAUSE:
                        break;
                    case ACTIVE:
                        cur_ticks = generation_timer.get_ticks();
                        if (cur_ticks >= generation_length) {
                            level.step_time();
                            generation_timer.start();
                        }
                        break;
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

                for (auto &room : level.get_rooms()) {
                    room.render();
                    room.render_doors();
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
                    t_fps.load_from_rendered_text(fpsText.str(), RED);
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


