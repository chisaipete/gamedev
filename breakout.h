#include <iostream>
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <SDL.h>
#include <SDL_timer.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

unsigned long long rdtsc(){
    unsigned int lo,hi;
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((unsigned long long)hi << 32) | lo;
}

struct vector2 {
    float x, y;
};

std::ostream &operator<<(std::ostream &os, vector2 const &A) { 
    return os << A.x << "," << A.y;
}

std::ostream &operator<<(std::ostream &os, SDL_Rect const &R) { 
    return os << "x:"<< R.x << " y:" << R.y << " w:" << R.w << " h:" << R.h;
}

// convert pair of floats to vector2
vector2 v2(float A, float B) {
    vector2 r;
    r.x = A;
    r.y = B;
    return r;
};

// negation
vector2 operator-(vector2 A) {
    vector2 r;
    r.x = -A.x;
    r.y = -A.y;
    return r;
};

// scalar multiplication
vector2 operator*(float A, vector2 B) {
    vector2 r;
    r.x = A*B.x;
    r.y = A*B.y;
    return r;
};

// scalar multiplication
vector2 operator*(vector2 A, float B) {
    vector2 r;
    r.x = B*A.x;
    r.y = B*A.y;
    return r;
};

// hadamari product
vector2 operator*(vector2 A, vector2 B) {
    vector2 r;
    r.x = A.x*B.x;
    r.y = A.y*B.y;
    return r;
};

// vector add
vector2 operator+(vector2 A, vector2 B) {
    vector2 r;
    r.x = A.x+B.x;
    r.y = A.y+B.y;
    return r;
};

// vector subtract
vector2 operator-(vector2 A, vector2 B) {
    vector2 r;
    r.x = A.x-B.x;
    r.y = A.y-B.y;
    return r;
};

// dot or inner product
float dot(vector2 A, vector2 B) {
    float r = A.x*B.x + A.y*B.y;
    return r;
};

//square
float square(float A) {
    float r = A*A;
    return r;
};

vector2 reflect(vector2 v, vector2 normal) {
    vector2 r;
    r = v - 2*(dot(normal,v))*normal;
    return r;
};


float length_sq(vector2 v) {
    float r = dot(v,v);
    return r;
};

float length(vector2 v) {
    float r = sqrt(length_sq(v));
    return r;
};

vector2 normalize(vector2 v) {
    vector2 r;
    r = v*(1.0/length(v));
    return r;
};
