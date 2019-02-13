#include <iostream>
#include <sstream>
#include <stdio.h>
#include <string>
#include <fstream>
#include <cstdlib>
#include <cmath>
#include <vector>
#include <SDL.h>
#include <glew.h>
#include <SDL_opengl.h>
#include <GL\GLU.h>
#include <SDL_timer.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
// #include <ctime>
// #include <algorithm>

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
template <class T> struct v2 {
    union {
        struct {T u, v;};
        struct {T x, y;};
        struct {T s, t;};
        T raw[2];
    };
    v2() : u(0), v(0) {}
    v2(T _u, T _v) : u(_u), v(_v) {}
    inline v2<T> operator +(const v2<T> &V) const { return v2<T>(u+V.u, v+V.v); }
    inline v2<T> operator -(const v2<T> &V) const { return v2<T>(u-V.u, v-V.v); }
    inline v2<T> operator *(float F)        const { return v2<T>(u*F, v*F); }
    template <class> friend std::ostream& operator<<(std::ostream s, v2<T>& v);
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

typedef v2<GLfloat>   v2f;
typedef v2<GLfloat>   t2f;
// typedef v2<float>   v2f;
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

struct VertexData2D {
    v2f position;
    t2f texture_coordinate;
};

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
GLenum DEFAULT_TEXTURE_WRAP = GL_REPEAT;

class Texture {
public:
    Texture();
    virtual ~Texture();
    bool load_texture_from_file(std::string path);
    bool load_texture_from_file_with_colorkey(std::string path, GLubyte r, GLubyte g, GLubyte b, GLubyte a=000);
    bool load_pixels_from_file(std::string path);
    bool load_texture_from_pixels(GLuint* pixels, GLuint i_width, GLuint i_height, GLuint t_width, GLuint t_height);
    bool load_texture_from_pixels();
    // bool load_from_rendered_text(std::string text, SDL_Color color = WHITE);
    virtual void free_texture();
    void render(GLfloat x, GLfloat y, FRect* clip = nullptr);
    GLuint get_texture_id();
    GLuint get_width();
    GLuint get_height();
    GLuint get_image_width();
    GLuint get_image_height();
    bool lock();
    bool unlock();
    GLuint* get_pixel_data();
    GLuint get_pixel(GLuint x, GLuint y);
    void set_pixel(GLuint x, GLuint y, GLuint pixel);
protected:
    GLuint power_of_two(GLuint number);
    GLenum detect_format(SDL_Surface* surface);
    void init_VBO();
    void free_VBO();
    std::string file_path;
    GLuint texture_id;
    GLuint* raw_pixels;
    GLuint texture_width;
    GLuint texture_height;
    GLuint image_width;
    GLuint image_height;
    GLuint VBO_id;
    GLuint IBO_id;
    GLenum pixel_mode;
};

Texture::Texture() {
    texture_id = 0;
    raw_pixels = nullptr;
    image_width = 0;
    image_height = 0;
    texture_width = 0;
    texture_height = 0;
    VBO_id = 0;
    IBO_id = 0;
    pixel_mode = 0;
}

Texture::~Texture() {
    free_texture();
    free_VBO();
}

void Texture::free_texture() {
    if (texture_id != 0) {
        glDeleteTextures(1, &texture_id);
        texture_id = 0;
    }
    if (raw_pixels != nullptr) {
        delete[] raw_pixels;
        raw_pixels = nullptr;
        pixel_mode = 0;
    }
    image_width = 0;
    image_height = 0;
    texture_width = 0;
    texture_height = 0;
}

bool Texture::lock() {
    //If we have member pixels, it means we've already locked the texture
    if (raw_pixels == nullptr && texture_id != 0) {
        //Alloc memory for texture data
        GLuint size = texture_width * texture_height;
        raw_pixels = new GLuint[size];
        //Set current texture
        glBindTexture(GL_TEXTURE_2D, texture_id);
        //Get pixels
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, raw_pixels);
        //Unbind texture
        glBindTexture(GL_TEXTURE_2D, 0);
        return true;
    }
    return false;
}

bool Texture::unlock() {
    //If texture is locked, and it exists
    if (raw_pixels != nullptr && texture_id != 0) {
        //Set current texture
        glBindTexture(GL_TEXTURE_2D, texture_id);
        //Update texture
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texture_width, texture_height, GL_RGBA, GL_UNSIGNED_BYTE, raw_pixels);
        //Delete pixels
        delete[] raw_pixels;
        raw_pixels = nullptr;
        //Unbind texture
        glBindTexture(GL_TEXTURE_2D, 0);
        return true;
    }
    return false;
}

void Texture::init_VBO() {
    //texture loaded and VBO doesn't exist
    if (texture_id != 0 && VBO_id == 0) {
        //Vertex data
        VertexData2D vertex_data [4];
        GLuint index_data[4];
        //set rendering indicies
        index_data[0] = 0;
        index_data[1] = 1;
        index_data[2] = 2;
        index_data[3] = 3;
        //Create VBO
        glGenBuffers(1, &VBO_id);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_id);
        glBufferData(GL_ARRAY_BUFFER, 4*sizeof(VertexData2D), vertex_data, GL_DYNAMIC_DRAW);
        //Create IBO
        glGenBuffers(1, &IBO_id);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO_id);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4*sizeof(GLfloat), index_data, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
}

void Texture::free_VBO() {
    if (VBO_id != 0) {
        glDeleteBuffers(1, &VBO_id);
        glDeleteBuffers(1, &IBO_id);
    }
}

GLuint* Texture::get_pixel_data() {
    return raw_pixels;
}

GLuint Texture::get_pixel(GLuint x, GLuint y) {
    return raw_pixels[y*texture_width+x];
}

void Texture::set_pixel(GLuint x, GLuint y, GLuint pixel) {
    raw_pixels[y*texture_width+x] = pixel;
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

GLenum Texture::detect_format(SDL_Surface* surface) {
    // std::cout << (int)surface->format->BytesPerPixel << " bytepp " << (int)surface->format->BitsPerPixel << " bitpp" << std::endl;
    if (surface->format->BytesPerPixel == 4) {
        if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
            // std::cout << "GL_BGRA" << std::endl;
            return GL_BGRA;
        } else {
            // std::cout << "GL_RGBA" << std::endl;
            return GL_RGBA;
        }
    } else if (surface->format->BytesPerPixel == 3) {
        if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
            // std::cout << "GL_BGR" << std::endl;
            return GL_BGR;
        } else {
            // std::cout << "GL_RGB" << std::endl;
            return GL_RGB;
        }
    } else if (surface->format->BytesPerPixel == 2) {
        if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
            // std::cout << "GL_BGRA" << std::endl;
            return GL_BGRA;
        } else {
            // std::cout << "GL_RGBA" << std::endl;
            return GL_RGBA;
        }
    } else if (surface->format->BytesPerPixel == 1) {
        if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
            // std::cout << "GL_BGR" << std::endl;
            return GL_BGR;
        } else {
            // std::cout << "GL_RGB" << std::endl;
            return GL_RGB;
        }
    } else {
        // std::cout << "Format not recognized!" << std::endl;
        return 0;
    }
}

bool Texture::load_texture_from_pixels() {
    bool success = true;
    if (texture_id == 0 && raw_pixels != nullptr) {
        glGenTextures(1, &texture_id);
        glBindTexture(GL_TEXTURE_2D, texture_id);
        glTexImage2D(GL_TEXTURE_2D, 0, pixel_mode, texture_width, texture_height, 0, pixel_mode, GL_UNSIGNED_BYTE, raw_pixels);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, DEFAULT_TEXTURE_WRAP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, DEFAULT_TEXTURE_WRAP);
        glBindTexture(GL_TEXTURE_2D, 0);
        GLenum error = glGetError();
        if (error != GL_NO_ERROR) { 
            logGLError(std::cout, "Error loading texture from pixels!", error); 
            success = false;
        } else {
            //release pixels
            delete[] raw_pixels;
            raw_pixels = NULL;
            //Generate VBO - I added this call...
            init_VBO();
        }
    } else {
        std::cout << "Cannot load texture from current pixels! ";
        if (texture_id != 0) {
            std::cout << "A texture is already loaded!" << std::endl;
        } else if (raw_pixels == nullptr) {
            std::cout << "No pixels to create texture from!" << std::endl;
        }
        success = false;
    }
    return success;
}

bool Texture::load_texture_from_pixels(GLuint* pixels, GLuint i_width, GLuint i_height, GLuint t_width, GLuint t_height) {
    bool success = true;
    free_texture();
    image_width = i_width;
    image_height = i_height;
    texture_width = t_width;
    texture_height = t_height;
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, pixel_mode, texture_width, texture_height, 0, pixel_mode, GL_UNSIGNED_BYTE, pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, DEFAULT_TEXTURE_WRAP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, DEFAULT_TEXTURE_WRAP);
    glBindTexture(GL_TEXTURE_2D, 0);
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) { 
        logGLError(std::cout, "Error loading texture from pixels!", error); 
        success = false;
    }
    //Generate VBO
    init_VBO();
    return success;
}

bool Texture::load_texture_from_file(std::string path) {
    bool success = true;
    free_texture();
    file_path = path;
    SDL_Surface* lsurface = IMG_Load(path.c_str());
    if (lsurface == nullptr){
        logSDLError(std::cout, "IMG_Load");
    } else {
        SDL_Surface* csurface = SDL_ConvertSurfaceFormat(lsurface, SDL_PIXELFORMAT_RGBA32, 0);
        image_width = csurface->w;
        image_height = csurface->h;
        // Calculate texture dimensions needed
        texture_width = power_of_two(image_width);
        texture_height = power_of_two(image_height);
        if (image_width != texture_width || image_height != texture_height) {
            // std::cout << "not 2^n size image" << std::endl;
            // create new surface at desired size
            int bitdepth = 32;
            SDL_Surface* nsurface = SDL_CreateRGBSurfaceWithFormat(0, texture_width, texture_height, bitdepth, SDL_PIXELFORMAT_RGBA32);
            if (nsurface == nullptr) {
                logSDLError(std::cout, "SDL_CreateRGBSurfaceWithFormat");
            } else {
                // copy first image into new image, at upper leftW
                int blit = SDL_BlitSurface(csurface, NULL, nsurface, NULL);
                if (blit != 0) {
                    logSDLError(std::cout, "Surface load from file blit");
                }
                pixel_mode = detect_format(nsurface);
                success = load_texture_from_pixels((GLuint*)nsurface->pixels, image_width, image_height, texture_width, texture_height);
            }
            SDL_FreeSurface(nsurface);
        } else {
            // std::cout << "2^n size image" << std::endl;
            pixel_mode = detect_format(csurface);
            success = load_texture_from_pixels((GLuint*)csurface->pixels, image_width, image_height, texture_width, texture_height);
        }
        SDL_FreeSurface(csurface);
    }
    SDL_FreeSurface(lsurface);
    return success;
}

bool Texture::load_texture_from_file_with_colorkey(std::string path, GLubyte r, GLubyte g, GLubyte b, GLubyte a) {
    if (!load_pixels_from_file(path)) {
        return false;
    }
    GLuint size = texture_width * texture_height;
    for (int i = 0; i < size; i++) {
        GLubyte* colors = (GLubyte*)&raw_pixels[i];
        if(colors[0] == r && colors[1] == g && colors[2] == b && (0 == a || colors[3] == a)) {
            // make transparent
            colors[0] = 255;
            colors[1] = 255;
            colors[2] = 255;
            colors[3] = 000;
        }
    }
    return load_texture_from_pixels();
}

bool Texture::load_pixels_from_file(std::string path) {
    bool success = true;
    free_texture();
    file_path = path;
    SDL_Surface* lsurface = IMG_Load(path.c_str());
    if (lsurface == nullptr){
        logSDLError(std::cout, "IMG_Load");
    } else {
        SDL_Surface* csurface = SDL_ConvertSurfaceFormat(lsurface, SDL_PIXELFORMAT_RGBA32, 0);
        //TODO: fix this so we always are in 32 bit RGBA alpha textures
        image_width = csurface->w;
        image_height = csurface->h;
        // Calculate texture dimensions needed
        texture_width = power_of_two(image_width);
        texture_height = power_of_two(image_height);

        GLuint size = texture_width * texture_height;
        raw_pixels = new GLuint[size];

        if (image_width != texture_width || image_height != texture_height) {
            // create new surface at desired size
            int bitdepth = 32;
            SDL_Surface* nsurface = SDL_CreateRGBSurfaceWithFormat(0, texture_width, texture_height, bitdepth, SDL_PIXELFORMAT_RGBA32);
            if (nsurface == nullptr) {
                logSDLError(std::cout, "SDL_CreateRGBSurfaceWithFormat");
            } else {
                // copy first image into new image, at upper leftW
                int blit = SDL_BlitSurface(csurface, NULL, nsurface, NULL);
                if (blit != 0) {
                    logSDLError(std::cout, "Surface load from file blit");
                }
                pixel_mode = detect_format(nsurface);
                memcpy(raw_pixels, (GLuint*)nsurface->pixels, size * nsurface->format->BytesPerPixel);
                success = true;
            }
            SDL_FreeSurface(nsurface);
        } else {
            pixel_mode = detect_format(csurface);
            memcpy(raw_pixels, (GLuint*)csurface->pixels, size * csurface->format->BytesPerPixel);
            success = true;
        }
        SDL_FreeSurface(csurface);
    }
    SDL_FreeSurface(lsurface);
    return success;
}

// bool Texture::load_from_rendered_text(std::string text, SDL_Color color) {
//     free_texture();
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
        //Texture coordinates
        GLfloat texture_top = 0.f;
        GLfloat texture_bottom = (GLfloat)image_height / (GLfloat)texture_height;
        GLfloat texture_left = 0.f;
        GLfloat texture_right = (GLfloat)image_width / (GLfloat)texture_width;
        //Vertex coordinates
        GLfloat quad_width = image_width;
        GLfloat quad_height = image_height;
        //Handle clipping
        if (clip != nullptr) {
            texture_left = clip->x / texture_width;
            texture_right = (clip->x + clip->w) / texture_width;
            texture_top = clip->y / texture_height;
            texture_bottom = (clip->y + clip->h) / texture_height;
            quad_width = clip->w;
            quad_height = clip->h;
        }
        //Move to rendering point
        glTranslatef(x, y, 0.f);
        //Set vertex data
        VertexData2D vertex_data[4];
        //Texture coordinates
        vertex_data[0].texture_coordinate.s =  texture_left; vertex_data[0].texture_coordinate.t =    texture_top;
        vertex_data[1].texture_coordinate.s = texture_right; vertex_data[1].texture_coordinate.t =    texture_top;
        vertex_data[2].texture_coordinate.s = texture_right; vertex_data[2].texture_coordinate.t = texture_bottom;
        vertex_data[3].texture_coordinate.s =  texture_left; vertex_data[3].texture_coordinate.t = texture_bottom;
        //Vertex positions
        vertex_data[0].position.x =        0.f; vertex_data[0].position.y =         0.f;        
        vertex_data[1].position.x = quad_width; vertex_data[1].position.y =         0.f;        
        vertex_data[2].position.x = quad_width; vertex_data[2].position.y = quad_height;        
        vertex_data[3].position.x =        0.f; vertex_data[3].position.y = quad_height;        
        //Set texture ID
        glBindTexture(GL_TEXTURE_2D, texture_id);
        //Enable vertex and texture coordinate arrays
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        //Bind vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, VBO_id);
        //Update vertex buffer data
        glBufferSubData(GL_ARRAY_BUFFER, 0, 4*sizeof(VertexData2D), vertex_data);
        //Set texture coordinate data
        glTexCoordPointer(2, GL_FLOAT, sizeof(VertexData2D), (GLvoid*)offsetof(VertexData2D, texture_coordinate));
        //Set vertex data
        glVertexPointer(2, GL_FLOAT, sizeof(VertexData2D), (GLvoid*)offsetof(VertexData2D, position));
        //Draw quad using vertex data and index data
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO_id);
        glDrawElements(GL_QUADS, 4, GL_UNSIGNED_INT, NULL);
        //Disable Vertex and texture coordinate arrays
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        glDisableClientState(GL_VERTEX_ARRAY);
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


enum SpriteOrigin {
    SPRITE_ORIGIN_CENTER,
    SPRITE_ORIGIN_TOP_LEFT,
    SPRITE_ORIGIN_BOTTOM_LEFT,
    SPRITE_ORIGIN_TOP_RIGHT,
    SPRITE_ORIGIN_BOTTOM_RIGHT
};

class SpriteSheet : public Texture {
public:
    SpriteSheet();
    ~SpriteSheet();
    int add_sprite_clip(FRect& new_clip);
    FRect get_clip(int index);
    bool generate_data_buffer(SpriteOrigin = SPRITE_ORIGIN_CENTER);
    void free_sheet();
    void free_texture();
    void render_sprite(int index);
protected:
    std::vector<FRect> clips;
    GLuint vertex_data_buffer;
    GLuint* index_buffers;
};

SpriteSheet::SpriteSheet() {
    vertex_data_buffer = (GLuint)NULL;
    index_buffers = NULL;
}

SpriteSheet::~SpriteSheet() {
    free_sheet();
}

int SpriteSheet::add_sprite_clip(FRect& new_clip) {
    clips.push_back(new_clip);
    return clips.size() - 1;
}

FRect SpriteSheet::get_clip(int index) {
    return clips[index];
}

bool SpriteSheet::generate_data_buffer(SpriteOrigin origin) {
    if (get_texture_id() != 0 && clips.size() > 0) {
        int total_sprites = clips.size();
        VertexData2D* vertex_data = new VertexData2D[total_sprites*4];
        index_buffers = new GLuint[total_sprites];
        glGenBuffers(1, &vertex_data_buffer);
        glGenBuffers(total_sprites, index_buffers);
        GLfloat tex_width = get_width();
        GLfloat tex_height = get_height();
        GLuint sprite_indicies[4] = {0,0,0,0};
        //for origin calculation
        GLfloat vertex_top = 0.f;
        GLfloat vertex_bottom = 0.f;
        GLfloat vertex_left = 0.f;
        GLfloat vertex_right = 0.f;
        for (int i = 0; i < total_sprites; i++) {
            sprite_indicies[0] = i*4+0;
            sprite_indicies[1] = i*4+1;
            sprite_indicies[2] = i*4+2;
            sprite_indicies[3] = i*4+3;
            //set origin
            switch (origin) {
                case SPRITE_ORIGIN_TOP_LEFT:
                    vertex_top = 0.f;
                    vertex_bottom = clips[i].h;
                    vertex_left = 0.f;
                    vertex_right = clips[i].w;
                    break;
                case SPRITE_ORIGIN_TOP_RIGHT:
                    vertex_top = 0.f;
                    vertex_bottom = clips[i].h;
                    vertex_left = -clips[i].w;
                    vertex_right = 0.f;
                    break;
                case SPRITE_ORIGIN_BOTTOM_LEFT:
                    vertex_top = -clips[i].h;
                    vertex_bottom = 0.f;
                    vertex_left = 0.f;
                    vertex_right = clips[i].w;
                    break;
                case SPRITE_ORIGIN_BOTTOM_RIGHT:
                    vertex_top = -clips[i].h;
                    vertex_bottom = 0.f;
                    vertex_left = -clips[i].w;
                    vertex_right = 0.f;
                    break;
                default: //SPRITE_ORIGIN_CENTER
                    vertex_top = -clips[i].h / 2.f;
                    vertex_bottom = clips[i].h / 2.f;
                    vertex_left = -clips[i].w / 2.f;
                    vertex_right = clips[i].w / 2.f;
                    break;
            }
            vertex_data[sprite_indicies[0]].position.x = vertex_left;
            vertex_data[sprite_indicies[0]].position.y = vertex_top;
            vertex_data[sprite_indicies[0]].texture_coordinate.s = (clips[i].x) / tex_width;
            vertex_data[sprite_indicies[0]].texture_coordinate.t = (clips[i].y) / tex_height;
            vertex_data[sprite_indicies[1]].position.x = vertex_right;
            vertex_data[sprite_indicies[1]].position.y = vertex_top;
            vertex_data[sprite_indicies[1]].texture_coordinate.s = (clips[i].x + clips[i].w) / tex_width;
            vertex_data[sprite_indicies[1]].texture_coordinate.t = (clips[i].y) / tex_height;
            vertex_data[sprite_indicies[2]].position.x = vertex_right;
            vertex_data[sprite_indicies[2]].position.y = vertex_bottom;
            vertex_data[sprite_indicies[2]].texture_coordinate.s = (clips[i].x + clips[i].w) / tex_width;
            vertex_data[sprite_indicies[2]].texture_coordinate.t = (clips[i].y + clips[i].h) / tex_height;
            vertex_data[sprite_indicies[3]].position.x = vertex_left;
            vertex_data[sprite_indicies[3]].position.y = vertex_bottom;
            vertex_data[sprite_indicies[3]].texture_coordinate.s = (clips[i].x) / tex_width;
            vertex_data[sprite_indicies[3]].texture_coordinate.t = (clips[i].y + clips[i].h) / tex_height;
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffers[i]);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4*sizeof(GLuint), sprite_indicies, GL_STATIC_DRAW);
        }
        glBindBuffer(GL_ARRAY_BUFFER, vertex_data_buffer);
        glBufferData(GL_ARRAY_BUFFER, total_sprites*4*sizeof(VertexData2D), vertex_data, GL_STATIC_DRAW);
        delete[] vertex_data;
    } else {
        if (get_texture_id() == 0) {
            printf("No texture to render with!\n");
        }
        if (clips.size() <= 0) {
            printf("No clips to generate vertex data from!\n");
        }
        return false;
    }
    return true;
}

void SpriteSheet::free_sheet() {
    if (vertex_data_buffer != (GLuint)NULL) {
        glDeleteBuffers(1, &vertex_data_buffer);
        vertex_data_buffer = (GLuint)NULL;
    }
    if (index_buffers != NULL) {
        glDeleteBuffers(clips.size(), index_buffers);
        delete[] index_buffers;
        index_buffers = NULL;
    }
    clips.clear();
}

void SpriteSheet::free_texture() {
    free_sheet();
    Texture::free_texture();
}

void SpriteSheet::render_sprite(int index) {
    if (vertex_data_buffer != (GLuint)NULL) {
        glBindTexture(GL_TEXTURE_2D, get_texture_id());
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            glBindBuffer(GL_ARRAY_BUFFER, vertex_data_buffer);
            glTexCoordPointer(2, GL_FLOAT, sizeof(VertexData2D), (GLvoid*)offsetof(VertexData2D, texture_coordinate));
            glVertexPointer(2, GL_FLOAT, sizeof(VertexData2D), (GLvoid*)offsetof(VertexData2D, position));
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffers[index]);
            glDrawElements(GL_QUADS, 4, GL_UNSIGNED_INT, NULL);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        glDisableClientState(GL_VERTEX_ARRAY);
    }
}


class Font : private SpriteSheet {
public:
    Font();
    ~Font();
    bool load_bitmap(std::string path);
    void free_font();
    void render_text(GLfloat x, GLfloat y, std::string text);
private:
    GLfloat space;
    GLfloat line_height;
    GLfloat new_line;
};

Font::Font() {
    space = 0.f;
    line_height = 0.f;
    new_line = 0.f;
}

Font::~Font() {
    free_font();
}

void Font::free_font() {
    free_texture();
    space = 0.f;
    line_height = 0.f;
    new_line = 0.f;
}

bool Font::load_bitmap(std::string path) {
    //expects path to bitmap font image with black, white, shades of grey, black is background color
    //arranged in 16x16 grid in ASCII order
    bool success = true;
    const GLuint BLACK_PIXEL = 0xFF000000;
    free_font();
    if (load_pixels_from_file(path)) {
        //cell dimensions
        GLfloat cell_width = get_image_width() / 16.f;
        GLfloat cell_height = get_image_height() / 16.f;
        //letter top and bottom
        GLuint top = cell_height;
        GLuint bottom = 0;
        GLuint A_bottom = 0;
        //current pixel coordinates
        int pixel_x = 0;
        int pixel_y = 0;
        //base cell offsets
        int base_offset_x = 0;
        int base_offset_y = 0;
        //parsing bitmap
        GLuint current_char = 0;
        FRect next_clip = {0.f, 0.f, cell_width, cell_height};
        //iterate cell rows
        for (unsigned int rows = 0; rows < 16; rows++) {
            for (unsigned int columns = 0; columns < 16; columns++) {
                //parsing cell
                base_offset_x = cell_width * columns;
                base_offset_y = cell_height * rows;
                next_clip.x = cell_width * columns;
                next_clip.y = cell_height * rows;
                next_clip.w = cell_width;
                next_clip.h = cell_height;

                //find left side of character
                for (int pixel_column = 0; pixel_column < cell_width; pixel_column++) {
                    for (int pixel_row = 0; pixel_row < cell_height; pixel_row++) {
                        pixel_x = base_offset_x + pixel_column;
                        pixel_y = base_offset_y + pixel_row;
                        if (get_pixel(pixel_x, pixel_y) != BLACK_PIXEL) {
                            // set sprite x offset
                            next_clip.x = pixel_x;
                            // break loop
                            pixel_column = cell_width;
                            pixel_row = cell_height;
                        } 
                    }
                }
                //find right side of character (width)
                for (int pixel_column_width = cell_width - 1; pixel_column_width >= 0; pixel_column_width--) {
                    for (int pixel_row_width = 0; pixel_row_width < cell_height; pixel_row_width++) {
                        pixel_x = base_offset_x + pixel_column_width;
                        pixel_y = base_offset_y + pixel_row_width;
                        if (get_pixel(pixel_x, pixel_y) != BLACK_PIXEL) {
                            // set sprite width
                            next_clip.w = (pixel_x - next_clip.x) + 1;
                            // break loop
                            pixel_column_width = -1;
                            pixel_row_width = cell_height;
                        } 
                    }
                }
                //find top side of character
                for (int pixel_row = 0; pixel_row < cell_height; pixel_row++) {
                    for (int pixel_column = 0; pixel_column < cell_width; pixel_column++) {
                        pixel_x = base_offset_x + pixel_column;
                        pixel_y = base_offset_y + pixel_row;
                        if (get_pixel(pixel_x, pixel_y) != BLACK_PIXEL) {
                            // new top found
                            if (pixel_row < top) {
                                top = pixel_row;
                            }
                            //break loops
                            pixel_column = cell_width;
                            pixel_row = cell_height;

                        } 
                    }
                }
                //find bottom side of character (height) and baseline A
                for (int pixel_row_base = cell_height - 1; pixel_row_base >= 0; pixel_row_base--) {
                    for (int pixel_column_base = 0; pixel_column_base < cell_width; pixel_column_base++) {
                        pixel_x = base_offset_x + pixel_column_base;
                        pixel_y = base_offset_y + pixel_row_base;
                        if (get_pixel(pixel_x, pixel_y) != BLACK_PIXEL) {
                            // set baseline
                            if (current_char == 'A') {
                                A_bottom = pixel_row_base;
                            }
                            // new bottom
                            if (pixel_row_base > bottom) {
                                bottom = pixel_row_base;
                            }
                            // break loop
                            pixel_column_base = cell_width;
                            pixel_row_base = -1;
                        } 
                    }
                }
                clips.push_back(next_clip);
                current_char++;
            }
        }
        //trip excess height from top of fonts
        for (int t = 0; t < 256; t++) {
            clips[t].y += top;
            clips[t].h -= top;
        }
        //Blend
        const int RED_BYTE = 1; //FIXME: typo?
        const int GREEN_BYTE = 1;
        const int BLUE_BYTE = 2;
        const int ALPHA_BYTE = 3;
        const int PIXEL_COUNT = get_width() * get_height();
        GLuint* pixels = get_pixel_data();
        for (int i = 0; i < PIXEL_COUNT; i++) {
            //Get color components
            GLubyte* colors = (GLubyte*)&pixels[i];
            //White pixel shaded with transparency
            colors[ALPHA_BYTE] = colors[RED_BYTE];
            colors[RED_BYTE] = 0xFF;
            colors[GREEN_BYTE] = 0xFF;
            colors[BLUE_BYTE] = 0xFF;
        }
        //create texture from pixels
        if (load_texture_from_pixels()) {
            if (!generate_data_buffer(SPRITE_ORIGIN_TOP_LEFT)) {
                printf("Unable to create vertex buffer for bitmap font!\n");
                success = false;
            }
        } else {
            printf("Unable to create texture from bitmap font pixels!\n");
            success = false;
        }
        //Set wrap
        glBindTexture(GL_TEXTURE_2D, get_texture_id());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        space = cell_width / 2;
        new_line = A_bottom - top;
        line_height = bottom - top;
    } else {
        printf("Could not load bitmap font image: %s!\n", path.c_str());
        success = false;
    }
    return success;
}

void Font::render_text(GLfloat x, GLfloat y, std::string text) {
    if (get_texture_id() != 0) {
        GLfloat draw_x = x;
        GLfloat draw_y = y;
        glTranslatef(x, y, 0.f);
        glBindTexture(GL_TEXTURE_2D, get_texture_id());
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_data_buffer);
        glTexCoordPointer(2, GL_FLOAT, sizeof(VertexData2D), (GLvoid*)offsetof(VertexData2D, texture_coordinate));
        glVertexPointer(2, GL_FLOAT, sizeof(VertexData2D), (GLvoid*)offsetof(VertexData2D, position));
        for (int i = 0; i < text.length(); i++) {
            if (text[i] == ' ') {
                glTranslatef(space, 0.f, 0.f);
                draw_x += space;
            } else if (text[i] == '\n') {
                glTranslatef(x - draw_x, new_line, 0.f);
                draw_y += new_line;
                draw_x += x - draw_x;
            } else {
                GLuint ascii = (unsigned char)text[i];
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffers[ascii]);
                glDrawElements(GL_QUADS, 4, GL_UNSIGNED_INT, NULL);
                glTranslatef(clips[ascii].w, 0.f, 0.f);
                draw_x += clips[ascii].w;
            }
        }
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        glDisableClientState(GL_VERTEX_ARRAY);
    }
}


class ShaderProgram
{
public:
    ShaderProgram();
    virtual ~ShaderProgram();
    virtual bool load_program() = 0;
    virtual void free_program();
    bool bind();
    void unbind();
    GLuint get_program_id();

protected:
    void print_program_log( GLuint program );
    void print_shader_log( GLuint shader );
    GLuint program_id;
};

ShaderProgram::ShaderProgram() {
    program_id = (GLuint) NULL;
}

ShaderProgram::~ShaderProgram() {
    free_program();
}

void ShaderProgram::free_program() {
    glDeleteProgram(program_id);
}

bool ShaderProgram::bind() {
    bool success = true;
    glUseProgram(program_id);
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) { 
        logGLError(std::cout, "Error binding shader!", error);
        print_program_log(program_id);
        success = false;
    }
    return success;
}

void ShaderProgram::unbind() {
    glUseProgram((GLuint)NULL);
}

GLuint ShaderProgram::get_program_id() {
    return program_id;
}

void ShaderProgram::print_program_log(GLuint program) {
    if(glIsProgram(program)) {
        int info_log_length = 0;
        int max_length = info_log_length;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &max_length);
        char* info_log = new char[max_length];
        glGetProgramInfoLog(program, max_length, &info_log_length, info_log);
        if (info_log_length > 0) {
            printf("%s\n", info_log);
        }
        delete[] info_log;
    } else {
        printf("Name %d is not a program\n", program);
    }
}

void ShaderProgram::print_shader_log(GLuint shader) {
    if(glIsShader(shader)) {
        int info_log_length = 0;
        int max_length = info_log_length;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &max_length);
        char* info_log = new char[max_length];
        glGetShaderInfoLog(program_id, max_length, &info_log_length, info_log);
        if (info_log_length > 0) {
            printf("%s\n", info_log);
        }
        delete[] info_log;
    } else {
        printf("Name %d is not a shader\n", shader);
    }  
}

class PlainPolygonProgram2D : public ShaderProgram {
public:
    bool load_program();
private:
};

bool PlainPolygonProgram2D::load_program() {
    GLint program_success = GL_TRUE;
    program_id = glCreateProgram();
    //Create vertex shader
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    //Get Vertex source
    const GLchar* vertex_shader_source[] = {
        "void main() { gl_Position = gl_Vertex; }"
    };
    //Set vertex source
    glShaderSource(vertex_shader, 1, vertex_shader_source, NULL);
    //compile vertex source
    glCompileShader(vertex_shader);
    //check for errors
    GLint vs_compiled = GL_FALSE;
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &vs_compiled);
    if (vs_compiled != GL_TRUE) {
        printf("Unable to compile vertex shader %d!\n", vertex_shader);
        return false;
    }
    //attach vertex shader to program
    glAttachShader(program_id, vertex_shader);

    //create fragment shader
    

    return program_success;
}