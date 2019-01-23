#ifndef OPENGL_H
#define OPENGL_H

#include "tinygl.h"
// #include <SDL.h>
// #include <SDL_opengl.h>
// #include <GL\GLU.h>
// #include <string>

#endif

extern SDL_GLContext context;

//Screen Constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const int SCREEN_FPS = 60;

//Color Modes
const int COLOR_MODE_CYAN = 0;
const int COLOR_MODE_MULTI = 1;

//Viewport mode
enum ViewPortMode {
    VIEWPORT_MODE_FULL,
    VIEWPORT_MODE_HALF_CENTER,
    VIEWPORT_MODE_HALF_TOP,
    VIEWPORT_MODE_QUAD,
    VIEWPORT_MODE_RADAR
};

bool init();

bool initGL();

bool load();

void update();

void render();

bool close();

void runMainLoop(int val);

void handleKeys(unsigned char key, int x, int y);

/* ----------------------------------------------------- */

v2f gQuadVertices[4];
GLuint gIndices[4];
GLuint gVertexBuffer = 0;
GLuint gIndexBuffer = 0;

bool init() {
    bool success = true;

    if (SDL_Init(SDL_INIT_VIDEO) != 0){
        logSDLError(std::cout, "SDL_Init");
        success = false;
    } else {     
        //User OpenGL 3.1
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);  
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

        if (TTF_Init() != 0){  //Future: do we need to call IMG_Init, it seems to work without it...is speed a factor? IMG_GetError might be needed
            logSDLError(std::cout, "TTF_Init");
            success = false;
        } else {
            window = SDL_CreateWindow("OpenGL", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
            if (window == nullptr){
                logSDLError(std::cout, "SDL_CreateWindow");
                success = false;
            } else {
                context = SDL_GL_CreateContext(window);
                if (context == nullptr) {
                    logSDLError(std::cout, "OpenGL Context Creation");
                    success = false;
                } else {
                    //Use VSync
                    if (SDL_GL_SetSwapInterval(1) < 0) {
                        logSDLError(std::cout, "Warning: Unable to set VSync!");
                    }
                    //Initialize OpenGL
                    if (!initGL()) {
                        printf("Unable to initialize OpenGL!\n");
                        success = false;
                    }
                }
            }
        }
    }
    return success;
}

bool initGL() {
    bool success = true;
    GLenum glewError = glewInit();
    if (glewError != GLEW_OK) {
        printf("Error initializing GLEW! %s\n", glewGetErrorString(glewError));
        return false;
    }
    if (!GLEW_VERSION_2_1) {
        printf("OpenGL 2.1 not supported!\n");
        return false;
    }

    GLenum error = GL_NO_ERROR;

    //Set the viewport
    glViewport(0.f, 0.f, SCREEN_WIDTH, SCREEN_HEIGHT);

    //Initialize Projection Matrix
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0, 1.0, -1.0);
    /*
        Notice how we multiplied the orthographic matrix against the projection matrix. 
        This is what the projection matrix is for, to control how we view our geometry. 
        If we wanted 3D perspective, we'd multiply the projection matrix against the 
        perspective matrix (done with either gluPerspective() or glFrustum()).
    */
    //Initialize Modelview Matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    //Initialize clear color
    glClearColor(0.f, 0.f, 0.f, 1.f);
    //Enable texturing
    glEnable(GL_TEXTURE_2D);
    //Set blending
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST); //useful in 3d, not 2d
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //Check for errors
    error = glGetError();
    if (error != GL_NO_ERROR ) {
        printf("Error initializing OpenGL! %s\n", gluErrorString(error));
        success = false;
    }
    return success;
}

bool load() {
    bool success = true;
    font = TTF_OpenFont("res/cc.ttf", 12);
    if (font == nullptr){
        logSDLError(std::cout, "TTF_OpenFont");
        success = false;
    }
    // if (!t_fps.load_from_rendered_text("_", RED)) {
    //     success = false;
    // }

    // model = new Model("res/african_head.obj");

    //Set quad verticies
    gQuadVertices[0].x = SCREEN_WIDTH * 1.f / 4.f;
    gQuadVertices[0].y = SCREEN_HEIGHT * 1.f / 4.f;

    gQuadVertices[1].x = SCREEN_WIDTH * 3.f / 4.f;
    gQuadVertices[1].y = SCREEN_HEIGHT * 1.f / 4.f;

    gQuadVertices[2].x = SCREEN_WIDTH * 3.f / 4.f;
    gQuadVertices[2].y = SCREEN_HEIGHT * 3.f / 4.f;

    gQuadVertices[3].x = SCREEN_WIDTH * 1.f / 4.f;
    gQuadVertices[3].y = SCREEN_HEIGHT * 3.f / 4.f;

    //Set rendering indices
    gIndices[0] = 0;
    gIndices[1] = 1;
    gIndices[2] = 2;
    gIndices[3] = 3;

    //Create VBO
    glGenBuffers(1, &gVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, gVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, 4*sizeof(v2f), gQuadVertices, GL_STATIC_DRAW);

    //Create IBO
    glGenBuffers(1, &gIndexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIndexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4*sizeof(GLuint), gIndices, GL_STATIC_DRAW);

    return success;
}

bool close() {
    // t_fps.free();
    // image.free();
    // if (model != nullptr) { delete model; }
    if (font != nullptr) { TTF_CloseFont(font); font = NULL; }
    if (window != nullptr) { SDL_DestroyWindow(window); window = NULL; }
    TTF_Quit();
    SDL_Quit();
    return true;
}

void update() {
}

void render() {
    //Clear color buffer
    glClear(GL_COLOR_BUFFER_BIT);

    //Enable vertex arrays
    glEnableClientState(GL_VERTEX_ARRAY);
        //Set vertex data 
        glBindBuffer(GL_ARRAY_BUFFER, gVertexBuffer);
        glVertexPointer(2, GL_FLOAT, 0, NULL);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIndexBuffer);
        glDrawElements(GL_QUADS, 4, GL_UNSIGNED_INT, NULL);
    //Disable vertex arrays
    glDisableClientState(GL_VERTEX_ARRAY);
}

void handleKeys(unsigned char key, int x, int y) {
}
