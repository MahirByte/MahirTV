#pragma once
#include <GL/glew.h>
#include <string>
#include "Math.h"

class Shader {
public:
    GLuint program = 0;

    bool load(const char* vertSrc, const char* fragSrc);
    void use() const;
    void setMat4(const char* name, const Mat4& m) const;
    void setVec4(const char* name, float r, float g, float b, float a) const;
    void setFloat(const char* name, float v) const;
    void setInt(const char* name, int v) const;
    void destroy();

private:
    GLuint compile(GLenum type, const char* src);
};
