#pragma once
#include <cmath>
#include <algorithm>

struct Vec2 { float x, y; };
struct Vec3 { float x, y, z; };
struct Vec4 { float x, y, z, w; };

struct Mat4 {
    float m[16] = {};
    float& operator[](int i) { return m[i]; }
    const float& operator[](int i) const { return m[i]; }
};

inline Mat4 mat4Identity() {
    Mat4 r;
    r[0]=1; r[5]=1; r[10]=1; r[15]=1;
    return r;
}

inline Mat4 mat4Multiply(const Mat4& a, const Mat4& b) {
    Mat4 r;
    for (int row = 0; row < 4; ++row)
        for (int col = 0; col < 4; ++col)
            for (int k = 0; k < 4; ++k)
                r.m[row*4+col] += a.m[row*4+k] * b.m[k*4+col];
    return r;
}

inline Mat4 mat4Ortho(float l, float r, float b, float t, float zn, float zf) {
    Mat4 m = mat4Identity();
    m[0]  =  2.0f/(r-l);
    m[5]  =  2.0f/(t-b);
    m[10] = -2.0f/(zf-zn);
    m[12] = -(r+l)/(r-l);
    m[13] = -(t+b)/(t-b);
    m[14] = -(zf+zn)/(zf-zn);
    return m;
}

inline Mat4 mat4Perspective(float fovY, float aspect, float zn, float zf) {
    float f = 1.0f / tanf(fovY * 0.5f);
    Mat4 m;
    m[0]  = f / aspect;
    m[5]  = f;
    m[10] = (zf+zn)/(zn-zf);
    m[11] = -1.0f;
    m[14] = (2.0f*zf*zn)/(zn-zf);
    return m;
}

inline Mat4 mat4LookAt(Vec3 eye, Vec3 center, Vec3 up) {
    Vec3 f = { center.x-eye.x, center.y-eye.y, center.z-eye.z };
    float flen = sqrtf(f.x*f.x + f.y*f.y + f.z*f.z);
    f = { f.x/flen, f.y/flen, f.z/flen };

    Vec3 s = { f.y*up.z - f.z*up.y, f.z*up.x - f.x*up.z, f.x*up.y - f.y*up.x };
    float slen = sqrtf(s.x*s.x + s.y*s.y + s.z*s.z);
    s = { s.x/slen, s.y/slen, s.z/slen };

    Vec3 u = { s.y*f.z - s.z*f.y, s.z*f.x - s.x*f.z, s.x*f.y - s.y*f.x };

    Mat4 m;
    m[0]=s.x; m[4]=s.y; m[8]=s.z;    m[12]=-(s.x*eye.x+s.y*eye.y+s.z*eye.z);
    m[1]=u.x; m[5]=u.y; m[9]=u.z;    m[13]=-(u.x*eye.x+u.y*eye.y+u.z*eye.z);
    m[2]=-f.x;m[6]=-f.y;m[10]=-f.z;  m[14]= (f.x*eye.x+f.y*eye.y+f.z*eye.z);
    m[15]=1.0f;
    return m;
}

inline float lerp(float a, float b, float t) { return a + (b-a)*t; }
inline float clamp01(float v) { return v < 0.f ? 0.f : v > 1.f ? 1.f : v; }
inline float easeInOut(float t) { return t*t*(3.0f - 2.0f*t); }
inline float easeOut(float t) { return 1.0f - (1.0f-t)*(1.0f-t); }
