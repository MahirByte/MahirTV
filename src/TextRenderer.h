#pragma once
#include <GL/glew.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <unordered_map>
#include "Math.h"
#include "Shader.h"

class TextRenderer {
public:
    bool init();
    void destroy();

    bool loadFont(const std::string& path, int size, const std::string& alias);
    TTF_Font* getFont(const std::string& alias);

    // Draw text at screen pixel coords (top-left origin).
    // Call beginFrame first, endFrame when done.
    void beginFrame(int screenW, int screenH);
    void drawText(const std::string& text, const std::string& fontAlias,
                  float x, float y,
                  float r, float g, float b, float a = 1.0f,
                  float scale = 1.0f);
    void drawTextCentered(const std::string& text, const std::string& fontAlias,
                          float cx, float cy,
                          float r, float g, float b, float a = 1.0f,
                          float scale = 1.0f);
    void getTextSize(const std::string& text, const std::string& fontAlias,
                     int& w, int& h, float scale = 1.0f);

    // 2D primitive helpers (uses same shader)
    void drawRect(float x, float y, float w, float h,
                  float r, float g, float b, float a);
    void drawRectRounded(float x, float y, float w, float h,
                         float radius,
                         float r, float g, float b, float a);
    void drawCircle(float cx, float cy, float radius, int segs,
                    float r, float g, float b, float a);
    void drawRing(float cx, float cy, float radius, float thickness, int segs,
                  float r, float g, float b, float a);
    void drawLine(float x1, float y1, float x2, float y2, float width,
                  float r, float g, float b, float a);
    void drawTexture(GLuint tex, float x, float y, float w, float h, float a = 1.0f);
    void drawGradientRect(float x, float y, float w, float h,
                          float r1,float g1,float b1,float a1,
                          float r2,float g2,float b2,float a2,
                          bool vertical = true);

private:
    Shader flatShader, texShader;
    GLuint vao = 0, vbo = 0;
    GLuint texVao = 0, texVbo = 0;
    int sw = 1920, sh = 1080;
    Mat4 proj;

    std::unordered_map<std::string, TTF_Font*> fonts;

    GLuint makeTextTexture(const std::string& text, TTF_Font* font,
                           Uint8 r, Uint8 g, Uint8 b, int& w, int& h);
    void uploadQuad(float x, float y, float w, float h);
    void uploadColoredQuad(float x, float y, float w, float h,
                           float r1,float g1,float b1,float a1,
                           float r2,float g2,float b2,float a2,
                           bool vertical);
};
