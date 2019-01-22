#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <cstdlib>
// #include <ctime>
#include <cmath>
// #include <algorithm>
#include <vector>
#include <SDL.h>
#include <SDL_opengl.h>
#include <GL\GLU.h>
#include <SDL_timer.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

extern SDL_Window* window;
TTF_Font* font;

void logSDLError(std::ostream &os, const std::string &msg) {
    os << msg << " SDL Error: " << SDL_GetError() << std::endl;
}

void logGLError(std::ostream &os, const std::string &msg, GLenum error) {
    os << msg << " OpenGL Error: " << gluErrorString(error) << std::endl;
}

SDL_Color BLACK = {  0,  0,  0,255};
SDL_Color WHITE = {255,255,255,255};
SDL_Color GREEN = {  0,255,  0,255};
SDL_Color RED   = {255,  0,  0,255};

/* VECTORS */
template <class t> struct v2 {
    union {
        struct {t u, v;};
        struct {t x, y;};
        t raw[2];
    };
    v2() : u(0), v(0) {}
    v2(t _u, t _v) : u(_u), v(_v) {}
    inline v2<t> operator +(const v2<t> &V) const { return v2<t>(u+V.u, v+V.v); }
    inline v2<t> operator -(const v2<t> &V) const { return v2<t>(u-V.u, v-V.v); }
    inline v2<t> operator *(float F)        const { return v2<t>(u*F, v*F); }
    template <class> friend std::ostream& operator<<(std::ostream s, v2<t>& v);
};

template <class t> struct v3 {
    union {
        struct {t x, y, z;};
        t raw[3];
    };
    v3() : x(0), y(0), z(0) {}
    v3(t _x, t _y, t _z) : x(_x), y(_y), z(_z) {}
    inline v3<t> operator ^(const v3<t> &V) const { return v3<t>(y*V.z-z*V.y, z*V.x-x*V.z, x*V.y-y*V.x); }
    inline v3<t> operator +(const v3<t> &V) const { return v3<t>(x+V.x, y+V.y, z+V.z); }
    inline v3<t> operator -(const v3<t> &V) const { return v3<t>(x-V.x, y-V.y, z-V.z); }
    inline v3<t> operator *(float F)        const { return v3<t>(x*F, y*F, z*F); }
    inline t     operator *(const v3<t> &V) const { return x*V.x + y*V.y + z*V.z; }
    float norm () const { return std::sqrt(x*x+y*y+z*z); }
    v3<t> & normalize(t l=1) { *this = (*this)*(l/norm()); return *this; }
    template <class> friend std::ostream& operator<<(std::ostream& s, v3<t>& v);
};

typedef v2<float>   v2f;
typedef v2<int>     v2i;
typedef v3<float>   v3f;
typedef v3<int>     v3i;

template <class t> std::ostream& operator<<(std::ostream& s, v2<t>& v) {
    s << "(" << v.x << ", " << v.y << ")\n";
    return s;
}

template <class t> std::ostream& operator<<(std::ostream& s, v3<t>& v) {
    s << "(" << v.x << ", " << v.y << ", " << v.z << ")\n";
    return s;
}

/* OPENGL THINGS */

struct FRect {
    GLfloat x;
    GLfloat y;
    GLfloat w;
    GLfloat h;
};

/* OBJ FORMAT*/
class Model {
public:
    Model(const char *filename);
    ~Model();
    int num_vertexes();
    int num_faces();
    v3f vertex(int index);
    std::vector<int> face(int index);
private:
    std::vector<v3f> vertexes;
    std::vector<std::vector<int>> faces;
};

Model::Model(const char *filename) : vertexes(), faces() {
    std::ifstream in;
    in.open (filename, std::ifstream::in);
    if (in.fail()) return;
    std::string line;
    while (!in.eof()) {
        std::getline(in, line);
        std::istringstream iss(line.c_str());
        char trash;
        if (!line.compare(0, 2, "v ")) {
            iss >> trash;
            v3f v;
            for (int i=0; i<3; i++) iss >> v.raw[i];
            vertexes.push_back(v);
        } else if (!line.compare(0, 2, "f ")) {
            std::vector<int> f;
            int itrash, idx;
            iss >> trash;
            while (iss >> idx >> trash >> itrash >> trash >> itrash) {
                idx--;
                f.push_back(idx);
            }
            faces.push_back(f);
        }
    }
    std::cerr << "# v# " << vertexes.size() << " f# " << faces.size() << std::endl;
}

Model::~Model() {}

int Model::num_vertexes() {
    return (int) vertexes.size();
}

int Model::num_faces() {
    return (int) faces.size();
}

std::vector<int> Model::face(int index) {
    return faces[index];
}

v3f Model::vertex(int index) {
    return vertexes[index];
}

/* TIMER CLASS */
class Timer {
public:
    Timer();
    ~Timer();
    void start();
    void stop();
    void pause();
    void unpause();
    int get_ticks();
    bool is_started();
    bool is_paused();
private:
    int start_ticks;
    int paused_ticks;
    bool paused;
    bool started;
};

Timer::Timer() {
    start_ticks = 0;
    paused_ticks = 0;
    paused = false;
    started = false;
}

Timer::~Timer() {}

void Timer::start() {
    started = true;
    paused = false;
    start_ticks = SDL_GetTicks();
    paused_ticks = 0;
}

void Timer::stop() {
    started = false;
    paused = false;
    start_ticks = 0;
    paused_ticks = 0;
}

void Timer::pause() {
    if (started && !paused) {
        paused = true;
        paused_ticks = SDL_GetTicks() - start_ticks;
        start_ticks = 0;
    }
}

void Timer::unpause() {
    if (started && paused) {
        paused = false;
        start_ticks = SDL_GetTicks() - paused_ticks;
        paused_ticks = 0;
    }
}

int Timer::get_ticks() {
    int time = 0; //stopped time
    if (started) {
        if (paused) {
            time = paused_ticks; //time when paused
        } else {
            time = SDL_GetTicks() - start_ticks;  //delta from start to now
        }
    }
    return time;
}

bool Timer::is_started() {
    return started;
}

bool Timer::is_paused() {
    return paused && started;
}

/* TEXTURE CLASSES */
class Texture {
public:
    Texture();
    ~Texture();
    bool load_from_file(std::string path);
    bool load_from_file_b(std::string path);
    bool load_from_pixels(GLuint* pixels, GLuint i_width, GLuint i_height, GLuint t_width, GLuint t_height, int mode);
    // bool load_from_rendered_text(std::string text, SDL_Color color = WHITE);
    void free();
    // void set_color(int r, int g, int b); //Uint8
    // void set_blend_mode(SDL_BlendMode blending);
    // void set_alpha(int a); //Uint8
    // void render(int x, int y, SDL_Rect* clip = NULL, double angle = 0.0, SDL_Point* center = nullptr, SDL_RendererFlip flip = SDL_FLIP_NONE);
    void render(GLfloat x, GLfloat y, FRect* clip = NULL);
    GLuint get_texture_id();
    GLuint get_width();
    GLuint get_height();
    GLuint get_image_width();
    GLuint get_image_height();
protected:
    std::string file_path;
    GLuint texture_id;
    GLuint texture_width;
    GLuint texture_height;
    GLuint image_width;
    GLuint image_height;
    GLuint power_of_two(GLuint number);
};

Texture::Texture() {
    texture_id = 0;
    texture_width = 0;
    texture_height = 0;
    image_width = 0;
    image_height = 0;
}

Texture::~Texture() {
    free();
}

void Texture::free() {
    std::cout << "Freeing texture" << std::endl;
    if (texture_id != 0) {
        std::cout << "Deleting texture id: " << texture_id << std::endl;
        glDeleteTextures(1, &texture_id);
        texture_id = 0;
    }
    texture_width = 0;
    texture_height = 0;
    image_width = 0;
    image_height = 0;
}

GLuint Texture::power_of_two(GLuint number) {
    // what black magic is this?
    if (number != 0) {
        number--;
        number |= (number >>1);
        number |= (number >>2);
        number |= (number >>4);
        number |= (number >>8);
        number |= (number >>16);
        number++;
    }
    return number;
}

bool Texture::load_from_pixels(GLuint* pixels, GLuint i_width, GLuint i_height, GLuint t_width, GLuint t_height, int mode=GL_RGBA) {
    std::cout << "Loading from pixel payload" << std::endl;
    bool success = true;
    free();
    image_width = i_width;
    image_height = i_height;
    texture_width = t_width;
    texture_height = t_height;

    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);

    std::cout << image_width << "x" << image_height << " " << texture_width << "x" << texture_height << std::endl;
    std::cout << texture_id << std::endl;
    std::cout << pixels << std::endl;

    glTexImage2D(GL_TEXTURE_2D, 0, mode, texture_width, texture_height, 0, mode, GL_UNSIGNED_BYTE, pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) { 
        logGLError(std::cout, "Error loading texture from pixels!", error); 
        success = false;
    }
    return success;
}

bool Texture::load_from_file_b(std::string path) {
    std::cout << "Loading from: " << path << std::endl;
    bool success = true;
    free();
    file_path = path;
    SDL_Surface* lsurface = IMG_Load(path.c_str());
    if (lsurface == nullptr){
        logSDLError(std::cout, "IMG_Load");
    } else {
        //Create texture from surface
        glGenTextures(1, &texture_id);
        glBindTexture(GL_TEXTURE_2D, texture_id);

        int Mode = GL_RGB;
        if (lsurface->format->BytesPerPixel == 4) {
            Mode = GL_RGBA;
        }
        image_width = lsurface->w;
        image_height = lsurface->h;
        texture_width = lsurface->w;
        texture_height = lsurface->h;

        glTexImage2D(GL_TEXTURE_2D, 0, Mode, image_width, image_height, 0, Mode, GL_UNSIGNED_BYTE, lsurface->pixels);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);
        GLenum error = glGetError();
        if (error != GL_NO_ERROR) { 
            logGLError(std::cout, "Error loading texture from pixels!", error); 
            success = false;
        }
    }
    SDL_FreeSurface(lsurface);
    return success;
}

bool Texture::load_from_file(std::string path) {
    std::cout << "Loading from: " << path << std::endl;
    bool success = true;
    free();
    file_path = path;
    SDL_Surface* lsurface = IMG_Load(path.c_str());
    if (lsurface == nullptr){
        logSDLError(std::cout, "IMG_Load");
    } else {
        int mode = GL_RGB;
        if (lsurface->format->BytesPerPixel == 4) {
            mode = GL_RGBA;
        }
        if (mode == GL_RGB) {
            std::cout << "GL_RGB pixel mode detected" << std::endl;
        } else {
            std::cout << "GL_RGBA pixel mode detected" << std::endl;
        }

        image_width = lsurface->w;
        image_height = lsurface->h;
        // Calculate texture dimensions needed
        texture_width = power_of_two(image_width);
        texture_height = power_of_two(image_height);

        std::cout << image_width << "x" << image_height << " -> " << texture_width << "x" << texture_height << std::endl;

        if (image_width != texture_width || image_height != texture_height) {
            std::cout << "NOT Power of 2 texture dimensions, loading from new surface" << std::endl;
            // create new surface at desired size
            int bitdepth = 32;
            SDL_Surface* nsurface = SDL_CreateRGBSurfaceWithFormat(0, texture_width, texture_height, bitdepth, SDL_PIXELFORMAT_RGBA32);
            if (nsurface == nullptr) {
                logSDLError(std::cout, "SDL_CreateRGBSurfaceWithFormat");
            } else {
                // // paint with fill color: Not sure if needed?
                if (nsurface->format->BytesPerPixel == 4) {
                    mode = GL_RGBA;
                }
                // if (mode == GL_RGB) {
                //     std::cout << "GL_RGB pixel mode detected" << std::endl;
                //     SDL_FillRect(nsurface, NULL, SDL_MapRGB(nsurface->format, 255, 255, 0));
                // } else {
                //     std::cout << "GL_RGBA pixel mode detected" << std::endl;
                //     SDL_FillRect(nsurface, NULL, SDL_MapRGBA(nsurface->format, 255, 255, 0, 255));
                // }
                // copy first image into new image, at upper leftW
                int blit = SDL_BlitSurface(lsurface, NULL, nsurface, NULL);
                if (blit != 0) {
                    logSDLError(std::cout, "Surface load from file blit");
                }
                std::cout << nsurface->pixels << std::endl;
                success = load_from_pixels((GLuint*)nsurface->pixels, image_width, image_height, texture_width, texture_height, mode);
            }
            SDL_FreeSurface(nsurface);
        } else {
            std::cout << "Power of 2 texture dimensions, loading from surface" << std::endl;
            std::cout << lsurface->pixels << std::endl;
            success = load_from_pixels((GLuint*)lsurface->pixels, image_width, image_height, texture_width, texture_height, mode);
        }
    }
    SDL_FreeSurface(lsurface);
    return success;
}

// bool Texture::load_from_rendered_text(std::string text, SDL_Color color) {
//     free();
//     SDL_Surface* surf = TTF_RenderText_Blended(font, text.c_str(), color);
//     if (surf == nullptr){
//         logSDLError(std::cout, "TTF_RenderText");
//     } else {
//         texture = SDL_CreateTextureFromSurface(renderer, surf);
//         if (texture == nullptr){
//             logSDLError(std::cout, "CreateTexture");
//         } else {
//             width = surf->w;
//             height = surf->h;
//         }
//         SDL_FreeSurface(surf);
//     }
//     return texture != NULL;
// }

void Texture::render(GLfloat x, GLfloat y, FRect* clip) {
    if (texture_id != 0) {
        glLoadIdentity();
        //Texture coordinates
        GLfloat t_top = 0.f;
        GLfloat t_bottom = (GLfloat)image_height / (GLfloat)texture_height;
        GLfloat t_left = 0.f;
        GLfloat t_right = (GLfloat)image_width / (GLfloat)texture_width;
        //Vertex coordinates
        GLfloat q_width = image_width;
        GLfloat q_height = image_height;
        //Handle texture clipping
        if (clip != NULL) {
            t_left = clip->x / texture_width;
            t_right = (clip->x + clip->w) / texture_width;
            t_top = clip->y / texture_height;
            t_bottom = (clip->y + clip->h) / texture_height;
            q_width = clip->w;
            q_height = clip->h;
        }
        //Move to rendering point
        glTranslatef(x, y, 0.f);
        //Set texture ID
        glBindTexture(GL_TEXTURE_2D, texture_id);
        //Render textured quad
        glBegin( GL_QUADS );
            glTexCoord2f( t_left,    t_top); glVertex2f(    0.f,      0.f);
            glTexCoord2f(t_right,    t_top); glVertex2f(q_width,      0.f);
            glTexCoord2f(t_right, t_bottom); glVertex2f(q_width, q_height);
            glTexCoord2f( t_left, t_bottom); glVertex2f(    0.f, q_height);
        glEnd();
    }
}

GLuint Texture::get_texture_id() {
    return texture_id;
}

GLuint Texture::get_width() {
    return texture_width;
}

GLuint Texture::get_height() {
    return texture_height;
}

GLuint Texture::get_image_width() {
    return image_width;
}

GLuint Texture::get_image_height() {
    return image_height;
}