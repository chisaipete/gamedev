#include "opengl.h"

SDL_Window* window = NULL;
SDL_GLContext context;

int main(int argc, char* args[]) {
    if (!init()) {
        std::cout << "Initialization Failed" << std::endl;
    } else {
        if (!load()) {
            std::cout << "Loading Failed" << std::endl;
        } else {
            bool quit = false;
            // bool fps_on = false;
            // int frame_count = 0;
            // float avgFPS = 0.0;

            // fps_timer.start();

            SDL_Event event;

            //Enable text input
            SDL_StartTextInput();

            while (!quit) {
                while (SDL_PollEvent(&event) != 0) {
                    if (event.type == SDL_QUIT || (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)) {
                            quit = true;
                    } else if (event.type == SDL_TEXTINPUT) {
                        int x = 0, y = 0;
                        SDL_GetMouseState( &x, &y );
                        handleKeys(event.text.text[0], x, y);
                        // if (event.type == SDL_KEYDOWN && event.key.repeat == 0) {
                        //     switch (event.key.keysym.sym) {
                        //         case SDLK_1:
                        //             fps_on = !fps_on;
                        //             break;
                        //     }
                        // }
                    }
                }

                render();

                // SDL_SetRenderDrawColor(renderer, BLACK.r, BLACK.g, BLACK.b, BLACK.a); //black
                // SDL_RenderClear(renderer);

                // image.render(0, 0, nullptr, 0, nullptr, SDL_FLIP_VERTICAL);
                
                // if (fps_on) {
                //     // calculate and render FPS
                //     avgFPS = frame_count / (fps_timer.get_ticks() / 1000.0);
                //     if (avgFPS > 2000000) {
                //         avgFPS = 0;
                //     }
                //     fpsText.str("");
                //     fpsText << (int)avgFPS << " fps";
                //     t_fps.load_from_rendered_text(fpsText.str(), RED);
                //     t_fps.render(SCREEN_WIDTH-t_fps.get_width(), 0);
                // }

                SDL_GL_SwapWindow(window);
                // frame_count++;
            }
            SDL_StopTextInput();
        }
    }
    close();
    return 0;
}