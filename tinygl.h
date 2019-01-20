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
    bool load_from_pixels(GLuint* pixels, GLuint t_width, GLuint t_height);
    // bool load_from_rendered_text(std::string text, SDL_Color color = WHITE);
    void free();
    // void set_color(int r, int g, int b); //Uint8
    // void set_blend_mode(SDL_BlendMode blending);
    // void set_alpha(int a); //Uint8
    // void render(int x, int y, SDL_Rect* clip = NULL, double angle = 0.0, SDL_Point* center = nullptr, SDL_RendererFlip flip = SDL_FLIP_NONE);
    void render(GLfloat x, GLfloat y);
    GLuint get_texture_id();
    GLuint get_width();
    GLuint get_height();
protected:
    std::string file_path;
    GLuint texture_id;
    GLuint width;
    GLuint height;
};

Texture::Texture() {
    texture_id = 0;
    width = 0;
    height = 0;
}

Texture::~Texture() {
    free();
}

void Texture::free() {
    if (texture_id != 0) {
        glDeleteTextures(1, &texture_id);
        texture_id = 0;
    }
    width = 0;
    height = 0;
}

bool Texture::load_from_pixels(GLuint* pixels, GLuint t_width, GLuint t_height) {
    bool success = true;
    free();
    width = t_width;
    height = t_height;
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
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

bool Texture::load_from_file(std::string path) {
    bool success = true;
    free();
    file_path = path;
    // You should probably use CSurface::OnLoad ... ;)

    SDL_Surface* lsurface = IMG_Load(path.c_str());
    if (lsurface == nullptr){ //could also load to surface, then optimize with SDL_ConvertSurface?
        logSDLError(std::cout, "IMG_Load");
    } else {
        //Color key image
        // SDL_SetColorKey(lsurface, SDL_TRUE, SDL_MapRGB(lsurface->format, 0x00, 0xFF, 0xFF));
        //LazyFoo12, renderer color modulation (filter with color)
        //LazyFoo13, renderer alpha blending
        //Create texture from surface
        glGenTextures(1, &texture_id);
        glBindTexture(GL_TEXTURE_2D, texture_id);
        int Mode = GL_RGB;
        if (lsurface->format->BytesPerPixel == 4) {
            Mode = GL_RGBA;
        }
        width = lsurface->w;
        height = lsurface->h;
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, Mode, GL_UNSIGNED_BYTE, lsurface->pixels);
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

// void Texture::set_color(int r, int g, int b) {
//     SDL_SetTextureColorMod(texture, r, g, b);
// }

// void Texture::set_blend_mode(SDL_BlendMode blending) {
//     SDL_SetTextureBlendMode(texture, blending);
// }

// void Texture::set_alpha(int a) {
//     SDL_SetTextureAlphaMod(texture, a);
// }

void Texture::render(GLfloat x, GLfloat y) {
    if (texture_id != 0) {
        glLoadIdentity();
        glTranslatef(x, y, 0.f);
        glBindTexture(GL_TEXTURE_2D, texture_id);
        glBegin( GL_QUADS );
            glTexCoord2f(0.f, 0.f); glVertex2f(  0.f,    0.f);
            glTexCoord2f(1.f, 0.f); glVertex2f(width,    0.f);
            glTexCoord2f(1.f, 1.f); glVertex2f(width, height);
            glTexCoord2f(0.f, 1.f); glVertex2f(  0.f, height);
        glEnd();
        /*
            // For Ortho mode, of course
            int X = 0;
            int Y = 0;
            int Width = 100;
            int Height = 100;
             
            glBegin(GL_QUADS);
                glTexCoord2f(0, 0); glVertex3f(X, Y, 0);
                glTexCoord2f(1, 0); glVertex3f(X + Width, Y, 0);
                glTexCoord2f(1, 1); glVertex3f(X + Width, Y + Height, 0);
                glTexCoord2f(0, 1); glVertex3f(X, Y + Height, 0);
            glEnd();
        */
    }
}

GLuint Texture::get_texture_id() {
    return texture_id;
}

GLuint Texture::get_width() {
    return width;
}

GLuint Texture::get_height() {
    return height;
}


// class RawTexture: public Texture {
// public:
//     RawTexture();
//     ~RawTexture();
//     bool initialize(int w, int h);
//     bool lock_texture();
//     bool unlock_texture();
//     bool set(int x, int y, SDL_Color color);
//     void* get_pixels();
//     int get_pitch();
// protected:
//     void* pixels;
//     int pitch;
//     int format;
//     SDL_PixelFormat* mapping_format;
// };

// RawTexture::RawTexture() {
//     pixels = NULL;
//     pitch = 0;
// }

// RawTexture::~RawTexture() {
//     pixels = NULL;
//     pitch = 0;
//     format = 0;
//     SDL_FreeFormat(mapping_format);
// }

// bool RawTexture::initialize(int w, int h) {
//     free();
//     SDL_Texture* ntexture = NULL;
//     int bitdepth = 32;
//     SDL_Surface* bsurface = SDL_CreateRGBSurfaceWithFormat(0, w, h, bitdepth, SDL_PIXELFORMAT_RGBA32);
//     if (bsurface == nullptr) {
//         logSDLError(std::cout, "CreateRGBSurfaceWithFormat");
//     } else {
//         SDL_Surface* fsurface = SDL_ConvertSurfaceFormat(bsurface, SDL_GetWindowPixelFormat(window), 0);
//         if (fsurface == nullptr) {
//             logSDLError(std::cout, "ConvertSurfaceFormat");
//         } else {
//             format = SDL_GetWindowPixelFormat(window);
//             mapping_format = SDL_AllocFormat(format);
//             ntexture = SDL_CreateTexture(renderer, format, SDL_TEXTUREACCESS_STREAMING, fsurface->w, fsurface->h);
//             if (ntexture == nullptr) {
//                 logSDLError(std::cout, "CreateTexture");
//             } else {
//                 width = fsurface->w;
//                 height = fsurface->h;
//             }
//             SDL_FreeSurface(fsurface);
//         }
//         SDL_FreeSurface(bsurface);
//     }
//     texture = ntexture;
//     return texture != NULL;
// }

// bool RawTexture::lock_texture() {
//     bool success = true;
//     if (pixels != NULL) {
//         std::cout << "Texture is already locked!" << std::endl;
//         success = false;
//     } else {
//         if (SDL_LockTexture(texture, NULL, &pixels, &pitch) != 0) {
//             logSDLError(std::cout, "Unable to lock texture");
//             success = false;
//         }
//     }
//     return success;
// }

// bool RawTexture::unlock_texture() {
//     bool success = true;
//     if (pixels == NULL) {
//         std::cout << "Texture is not locked!" << std::endl;
//         success = false;
//     } else {
//         SDL_UnlockTexture(texture);
//         pixels = NULL;
//         pitch = 0;
//     }
//     return success;
// }

// bool RawTexture::set(int x, int y, SDL_Color color) {
//     if (x < 0 || y < 0 || x > width || y > height) {
//         return false;
//     }
//     int* _pixels = (int*) get_pixels();
//     // Setup the correct pixel bits to write to the texture
//     int pixel_color = SDL_MapRGBA(mapping_format, color.r, color.g, color.b, color.a);
//     // Set the pixel!
//     _pixels[x+y*width] = pixel_color;
//     return true;
// }

// void* RawTexture::get_pixels() {
//     return pixels;
// }

// int RawTexture::get_pitch() {
//     return pitch;
// }
