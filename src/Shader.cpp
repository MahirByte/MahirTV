#include "Shader.h"
#include <iostream>
#include <cstring>

GLuint Shader::compile(GLenum type, const char* src) {
    GLuint sh = glCreateShader(type);
    glShaderSource(sh, 1, &src, nullptr);
    glCompileShader(sh);
    GLint ok; glGetShaderiv(sh, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[512]; glGetShaderInfoLog(sh, 512, nullptr, log);
        std::cerr << "Shader compile error: " << log << "\n";
    }
    return sh;
}

bool Shader::load(const char* vertSrc, const char* fragSrc) {
    GLuint v = compile(GL_VERTEX_SHADER,   vertSrc);
    GLuint f = compile(GL_FRAGMENT_SHADER, fragSrc);
    program = glCreateProgram();
    glAttachShader(program, v);
    glAttachShader(program, f);
    glLinkProgram(program);
    glDeleteShader(v);
    glDeleteShader(f);
    GLint ok; glGetProgramiv(program, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[512]; glGetProgramInfoLog(program, 512, nullptr, log);
        std::cerr << "Program link error: " << log << "\n";
        return false;
    }
    return true;
}

void Shader::use() const { glUseProgram(program); }

void Shader::setMat4(const char* name, const Mat4& m) const {
    GLint loc = glGetUniformLocation(program, name);
    glUniformMatrix4fv(loc, 1, GL_FALSE, m.m);
}
void Shader::setVec4(const char* name, float r, float g, float b, float a) const {
    GLint loc = glGetUniformLocation(program, name);
    glUniform4f(loc, r, g, b, a);
}
void Shader::setFloat(const char* name, float v) const {
    glUniform1f(glGetUniformLocation(program, name), v);
}
void Shader::setInt(const char* name, int v) const {
    glUniform1i(glGetUniformLocation(program, name), v);
}
void Shader::destroy() {
    if (program) { glDeleteProgram(program); program = 0; }
}
