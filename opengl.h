#ifndef OPENGL_H
#define OPENGL_H

#include <GL/freeglut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdio.h>

#endif

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

bool initGL();
/*
Pre Condition:
- A valid OpenGL context
Post Condition:
- Initializes matrices and clear color
- Reports to console if there was an OpenGL error
- Returns false if there was an error in initalization
Side Effects:
- Projection matrix is set to identity matrix
- Modelview matrix is set to identity matrix
- Matrix mode is set to modelview
- Clear color is set to black
*/

void update();
/*
Pre Condition:
- None
Post Condition:
- Does per frame logic
Side Effects:
- None
*/

void render();
/*
Pre Condition:
- A valid OpenGL context
- Active modelview matrix
Post Condition:
- Renders the scene
Side Effects:
- Clears the color buffer
- Swaps the front/back buffer
- Sets matrix mode to modelview
- Translates modelview matrix to the center of the default screen
- Changes the current rendering color
*/

void runMainLoop(int val);
/*
Pre Condition:
- Initialized freeGLUT
Post Condition:
- Calls the main loop functions and sets itself to be called back in 1000 / SCREEN_FPS milliseconds
Side Effects:
- Sets glutTimerFunc
*/

void handleKeys(unsigned char key, int x, int y);
/*
Pre Condition:
- None
Post Condition:
- Toggles the color mode when the user presses q
- Cycles through different projection scales when the user presses e
Side Effects:
*/



/* ----------------------------------------------------- */

//The current color rendering mode
int gColorMode = COLOR_MODE_CYAN;
//The current viewport mode
int gViewportMode = VIEWPORT_MODE_FULL;
//Camera position
GLfloat gCameraX = 0.f, gCameraY = 0.f;

//The projection scale
GLfloat gProjectionScale = 1.f;

bool initGL() {
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
    //Save the default modelview matrix
    glPushMatrix();
    //Initialize clear color
    glClearColor(0.f, 0.f, 0.f, 1.f);
    //Check for errors
    GLenum error = glGetError();
    if (error != GL_NO_ERROR ) {
        printf("Error initializing OpenGL! %s\n", gluErrorString(error));
        return false;
    }
    return true;
}

void update() {

}

void render() {
    //Clear color buffer
    glClear(GL_COLOR_BUFFER_BIT);
    //Pop default matrix onto current matrix
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    //Save default matrix again
    glPushMatrix();
    //Move to center of the screen
    glTranslatef(SCREEN_WIDTH/2.f, SCREEN_HEIGHT/2.f, 0.f);
    //Red quad
    glBegin( GL_QUADS );
        glColor3f( 1.f, 0.f, 0.f );
        glVertex2f( -SCREEN_WIDTH / 4.f, -SCREEN_HEIGHT / 4.f );
        glVertex2f(  SCREEN_WIDTH / 4.f, -SCREEN_HEIGHT / 4.f );
        glVertex2f(  SCREEN_WIDTH / 4.f,  SCREEN_HEIGHT / 4.f );
        glVertex2f( -SCREEN_WIDTH / 4.f,  SCREEN_HEIGHT / 4.f );
    glEnd();

    //Move to the right of the screen
    glTranslatef( SCREEN_WIDTH, 0.f, 0.f );

    //Green quad
    glBegin( GL_QUADS );
        glColor3f( 0.f, 1.f, 0.f );
        glVertex2f( -SCREEN_WIDTH / 4.f, -SCREEN_HEIGHT / 4.f );
        glVertex2f(  SCREEN_WIDTH / 4.f, -SCREEN_HEIGHT / 4.f );
        glVertex2f(  SCREEN_WIDTH / 4.f,  SCREEN_HEIGHT / 4.f );
        glVertex2f( -SCREEN_WIDTH / 4.f,  SCREEN_HEIGHT / 4.f );
    glEnd();

    //Move to the lower right of the screen
    glTranslatef( 0.f, SCREEN_HEIGHT, 0.f );

    //Blue quad
    glBegin( GL_QUADS );
        glColor3f( 0.f, 0.f, 1.f );
        glVertex2f( -SCREEN_WIDTH / 4.f, -SCREEN_HEIGHT / 4.f );
        glVertex2f(  SCREEN_WIDTH / 4.f, -SCREEN_HEIGHT / 4.f );
        glVertex2f(  SCREEN_WIDTH / 4.f,  SCREEN_HEIGHT / 4.f );
        glVertex2f( -SCREEN_WIDTH / 4.f,  SCREEN_HEIGHT / 4.f );
    glEnd();

    //Move below the screen
    glTranslatef( -SCREEN_WIDTH, 0.f, 0.f );

    //Yellow quad
    glBegin( GL_QUADS );
        glColor3f( 1.f, 1.f, 0.f );
        glVertex2f( -SCREEN_WIDTH / 4.f, -SCREEN_HEIGHT / 4.f );
        glVertex2f(  SCREEN_WIDTH / 4.f, -SCREEN_HEIGHT / 4.f );
        glVertex2f(  SCREEN_WIDTH / 4.f,  SCREEN_HEIGHT / 4.f );
        glVertex2f( -SCREEN_WIDTH / 4.f,  SCREEN_HEIGHT / 4.f );
    glEnd();
    glutSwapBuffers();
}

void handleKeys(unsigned char key, int x, int y) {
    //If the user presses q
    if (key == 'w') {
        gCameraY -= 16.f;
    } else if (key == 's') {
        gCameraY += 16.f;
    } else if (key == 'a') {
        gCameraX -= 16.f;
    } else if (key == 'd') {
        gCameraX += 16.f;
    }
    //Take saved matrix off the stack and reset it
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glLoadIdentity();
    //Move camera to position
    glTranslatef(-gCameraX, -gCameraY, 0.f);
    //Save default matrix again with camera translation
    glPushMatrix();
}

void runMainLoop(int val) {
    //Frame logic
    update();
    render();
    //Run frame one more time
    glutTimerFunc(1000/SCREEN_FPS, runMainLoop, val);
}