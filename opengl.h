#ifndef OPENGL_H
#define OPENGL_H

#include "tinygl.h"
#include <SDL.h>
#include <SDL_opengl.h>
#include <GL\GLU.h>
#include <stdio.h>
#include <string>

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

//The current color rendering mode
int gColorMode = COLOR_MODE_CYAN;
//The current viewport mode
int gViewportMode = VIEWPORT_MODE_FULL;
//Camera position
GLfloat gCameraX = 0.f, gCameraY = 0.f;

//The projection scale
GLfloat gProjectionScale = 1.f;

Texture gCheckerBoardTexture;
Texture gLoadedTexture;
Texture gLoadedTextureIrregular;
Texture gArrowTexture;
FRect gArrowClips[4];
Texture gCircleTexture;

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

    // Checkerboard pixels
    const int CHECKERBOARD_WIDTH = 128;
    const int CHECKERBOARD_HEIGHT = 128;
    const int CHECKERBOARD_PIXEL_COUNT = CHECKERBOARD_WIDTH * CHECKERBOARD_HEIGHT;
    GLuint checkerBoard[CHECKERBOARD_PIXEL_COUNT];
    for (int i = 0; i < CHECKERBOARD_PIXEL_COUNT; ++i) {
        GLubyte* colors = (GLubyte*)&checkerBoard[i];
        if (i/128 & 16 ^ i % 128 & 16) {
            colors[0] = 0xFF;
            colors[1] = 0xFF;
            colors[2] = 0xFF;
            colors[3] = 0xFF;
        } else {
            colors[0] = 0xFF;
            colors[1] = 0x00;
            colors[2] = 0x00;
            colors[3] = 0xFF;
        }
    }
    if (!gCheckerBoardTexture.load_texture_from_pixels(checkerBoard, CHECKERBOARD_WIDTH, CHECKERBOARD_HEIGHT, CHECKERBOARD_WIDTH, CHECKERBOARD_HEIGHT)) {
        printf("Unable to load checkerboard texture!\n");
        success = false;
    }
    if (!gLoadedTextureIrregular.load_texture_from_file("res/opengl.png")) {
        printf("Unable to load texture from file!\n");
        success = false;
    }
    if (!gArrowTexture.load_texture_from_file("res/arrows.png")) {
        printf("Unable to load texture from file!\n");
        success = false;
    } else {
        gArrowClips[0].x =   0.f;
        gArrowClips[0].y =   0.f;
        gArrowClips[0].w = 128.f;
        gArrowClips[0].h = 128.f;
        gArrowClips[1].x = 128.f;
        gArrowClips[1].y =   0.f;
        gArrowClips[1].w = 128.f;
        gArrowClips[1].h = 128.f;
        gArrowClips[2].x =   0.f;
        gArrowClips[2].y = 128.f;
        gArrowClips[2].w = 128.f;
        gArrowClips[2].h = 128.f;
        gArrowClips[3].x = 128.f;
        gArrowClips[3].y = 128.f;
        gArrowClips[3].w = 128.f;
        gArrowClips[3].h = 128.f;
    }
    if (!gLoadedTexture.load_texture_from_file("res/texture.png")) {
        printf("Unable to load texture from file!\n");
        success = false;
    }
    if(!gCircleTexture.load_texture_from_file_with_colorkey("res/circle.png", 000, 255, 255)) {
        printf("Unable to load texture from file!\n");
        success = false;
    }
    // gCircleTexture.lock();
    // //Calculate target color
    // GLuint targetColor;
    // GLubyte* colors = (GLubyte*)&targetColor;
    // colors[0] = 000;
    // colors[1] = 255;
    // colors[2] = 255;
    // colors[3] = 255;
    // GLuint* pixels = gCircleTexture.get_pixel_data();
    // GLuint pixel_count = gCircleTexture.get_width() * gCircleTexture.get_height();
    // for (int i = 0; i < pixel_count; i++) {
    //     if (pixels[i] == targetColor) {
    //         pixels[i] = 0;
    //     }
    // }
    // for (int y = 0; y < gCircleTexture.get_height(); y++) {
    //     for (int x = 0; x < gCircleTexture.get_width(); x++) {
    //         if (y % 10 != x % 10) {
    //             gCircleTexture.set_pixel(x, y, 0);
    //         }
    //     }
    // }
    // gCircleTexture.unlock();

    return success;
}

bool close() {
    gLoadedTexture.free();
    gLoadedTextureIrregular.free();
    gArrowTexture.free();
    gCircleTexture.free();
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

    //Calculate centered offsets &
    //Render texture
    gLoadedTextureIrregular.render((SCREEN_WIDTH-gLoadedTextureIrregular.get_image_width())/2.f, (SCREEN_HEIGHT-gLoadedTextureIrregular.get_image_height())/2.f);
    gLoadedTexture.render((SCREEN_WIDTH-gLoadedTexture.get_image_width())/2.f, (SCREEN_HEIGHT-gLoadedTexture.get_image_height())/2.f);
    gCheckerBoardTexture.render((SCREEN_WIDTH-gCheckerBoardTexture.get_image_width())/2.f, (SCREEN_HEIGHT-gCheckerBoardTexture.get_image_height())/2.f);
    gArrowTexture.render(0.f, 0.f, &gArrowClips[0]);
    gArrowTexture.render(SCREEN_WIDTH - gArrowClips[1].w, 0.f, &gArrowClips[1]);
    gArrowTexture.render(0.f, SCREEN_HEIGHT - gArrowClips[2].h, &gArrowClips[2]);
    gArrowTexture.render(SCREEN_WIDTH - gArrowClips[3].w, SCREEN_HEIGHT - gArrowClips[3].h, &gArrowClips[3]);
    glColor4f(1.f, 1.f, 1.f, 0.5f);
    gCircleTexture.render((SCREEN_WIDTH-gCircleTexture.get_image_width())/2.f, (SCREEN_HEIGHT-gCircleTexture.get_image_height())/2.f);
    glColor4f(1.f, 1.f, 1.f, 1.f);
}

void handleKeys(unsigned char key, int x, int y) {
    //If the user presses q
    // if (key == 'w') {
    //     gCameraY -= 16.f;
    // } else if (key == 's') {
    //     gCameraY += 16.f;
    // } else if (key == 'a') {
    //     gCameraX -= 16.f;
    // } else if (key == 'd') {
    //     gCameraX += 16.f;
    // }
    // //Take saved matrix off the stack and reset it
    // glMatrixMode(GL_MODELVIEW);
    // glPopMatrix();
    // glLoadIdentity();
    // //Move camera to position
    // glTranslatef(-gCameraX, -gCameraY, 0.f);
    // //Save default matrix again with camera translation
    // glPushMatrix();
}
