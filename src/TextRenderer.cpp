#include "TextRenderer.h"
#include <SDL2/SDL.h>
#include <unordered_map>
#include <cmath>
#include <vector>
#include <iostream>

static const char* FLAT_VERT = R"(
#version 330 core
layout(location=0) in vec2 pos;
layout(location=1) in vec4 col;
uniform mat4 proj;
out vec4 vCol;
void main(){ gl_Position = proj * vec4(pos,0.0,1.0); vCol = col; }
)";

static const char* FLAT_FRAG = R"(
#version 330 core
in vec4 vCol;
out vec4 fCol;
void main(){ fCol = vCol; }
)";

static const char* TEX_VERT = R"(
#version 330 core
layout(location=0) in vec2 pos;
layout(location=1) in vec2 uv;
uniform mat4 proj;
out vec2 vUV;
void main(){ gl_Position = proj * vec4(pos,0.0,1.0); vUV = uv; }
)";

static const char* TEX_FRAG = R"(
#version 330 core
in vec2 vUV;
uniform sampler2D tex;
uniform float alpha;
out vec4 fCol;
void main(){ fCol = texture(tex,vUV); fCol.a *= alpha; }
)";

bool TextRenderer::init() {
    if (TTF_Init() < 0) { std::cerr << "TTF_Init: " << TTF_GetError() << "\n"; return false; }

    if (!flatShader.load(FLAT_VERT, FLAT_FRAG)) return false;
    if (!texShader.load(TEX_VERT,  TEX_FRAG))  return false;

    glGenVertexArrays(1, &vao); glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*6*6, nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0); glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,6*sizeof(float),(void*)0);
    glEnableVertexAttribArray(1); glVertexAttribPointer(1,4,GL_FLOAT,GL_FALSE,6*sizeof(float),(void*)(2*sizeof(float)));
    glBindVertexArray(0);

    glGenVertexArrays(1, &texVao); glGenBuffers(1, &texVbo);
    glBindVertexArray(texVao);
    glBindBuffer(GL_ARRAY_BUFFER, texVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*6*4, nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0); glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,4*sizeof(float),(void*)0);
    glEnableVertexAttribArray(1); glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,4*sizeof(float),(void*)(2*sizeof(float)));
    glBindVertexArray(0);
    return true;
}

void TextRenderer::destroy() {
    for (auto& [k,f] : fonts) TTF_CloseFont(f);
    fonts.clear();
    if (vao)    { glDeleteVertexArrays(1,&vao);    vao=0; }
    if (vbo)    { glDeleteBuffers(1,&vbo);          vbo=0; }
    if (texVao) { glDeleteVertexArrays(1,&texVao); texVao=0; }
    if (texVbo) { glDeleteBuffers(1,&texVbo);       texVbo=0; }
    flatShader.destroy(); texShader.destroy();
    TTF_Quit();
}

bool TextRenderer::loadFont(const std::string& path, int size, const std::string& alias) {
    TTF_Font* f = TTF_OpenFont(path.c_str(), size);
    if (!f) { std::cerr << "TTF_OpenFont(" << path << "): " << TTF_GetError() << "\n"; return false; }
    fonts[alias] = f;
    return true;
}
TTF_Font* TextRenderer::getFont(const std::string& alias) {
    auto it = fonts.find(alias);
    return it != fonts.end() ? it->second : nullptr;
}

void TextRenderer::beginFrame(int w, int h) {
    sw = w; sh = h;
    proj = mat4Ortho(0.f,(float)w,(float)h,0.f,-1.f,1.f);
}

GLuint TextRenderer::makeTextTexture(const std::string& text, TTF_Font* font,
                                     Uint8 r, Uint8 g, Uint8 b, int& w, int& h) {
    SDL_Color c{r,g,b,255};
    SDL_Surface* surf = TTF_RenderUTF8_Blended(font, text.c_str(), c);
    if (!surf) { w=h=0; return 0; }
    SDL_Surface* conv = SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_ABGR8888, 0);
    SDL_FreeSurface(surf);
    if (!conv) { w=h=0; return 0; }
    w = conv->w; h = conv->h;
    GLuint tex; glGenTextures(1,&tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,w,h,0,GL_RGBA,GL_UNSIGNED_BYTE,conv->pixels);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    SDL_FreeSurface(conv);
    return tex;
}

void TextRenderer::drawText(const std::string& text, const std::string& fontAlias,
                            float x, float y, float r, float g, float b, float a, float scale) {
    TTF_Font* font = getFont(fontAlias);
    if (!font || text.empty()) return;
    int tw, th;
    GLuint tex = makeTextTexture(text, font,
                                 (Uint8)(r*255),(Uint8)(g*255),(Uint8)(b*255),
                                 tw, th);
    if (!tex) return;
    float w = tw*scale, h = th*scale;
    drawTexture(tex, x, y, w, h, a);
    glDeleteTextures(1,&tex);
}

void TextRenderer::drawTextCentered(const std::string& text, const std::string& fontAlias,
                                    float cx, float cy, float r, float g, float b, float a, float scale) {
    TTF_Font* font = getFont(fontAlias);
    if (!font || text.empty()) return;
    int tw, th;
    GLuint tex = makeTextTexture(text, font,
                                 (Uint8)(r*255),(Uint8)(g*255),(Uint8)(b*255),
                                 tw, th);
    if (!tex) return;
    float w = tw*scale, h = th*scale;
    drawTexture(tex, cx - w*0.5f, cy - h*0.5f, w, h, a);
    glDeleteTextures(1,&tex);
}

void TextRenderer::getTextSize(const std::string& text, const std::string& fontAlias,
                               int& w, int& h, float scale) {
    TTF_Font* font = getFont(fontAlias);
    if (!font) { w=h=0; return; }
    TTF_SizeUTF8(font, text.c_str(), &w, &h);
    w = (int)(w*scale); h = (int)(h*scale);
}

void TextRenderer::uploadQuad(float x, float y, float w, float h) {
    float verts[] = {
        x,   y,   0,0,
        x+w, y,   1,0,
        x,   y+h, 0,1,
        x+w, y,   1,0,
        x+w, y+h, 1,1,
        x,   y+h, 0,1,
    };
    glBindBuffer(GL_ARRAY_BUFFER, texVbo);
    glBufferSubData(GL_ARRAY_BUFFER,0,sizeof(verts),verts);
}

void TextRenderer::drawTexture(GLuint tex, float x, float y, float w, float h, float a) {
    texShader.use();
    texShader.setMat4("proj", proj);
    texShader.setFloat("alpha", a);
    texShader.setInt("tex", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    uploadQuad(x,y,w,h);
    glBindVertexArray(texVao);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDrawArrays(GL_TRIANGLES,0,6);
    glBindVertexArray(0);
}

void TextRenderer::drawRect(float x, float y, float w, float h,
                            float r, float g, float b, float a) {
    float verts[] = {
        x,   y,   r,g,b,a,
        x+w, y,   r,g,b,a,
        x,   y+h, r,g,b,a,
        x+w, y,   r,g,b,a,
        x+w, y+h, r,g,b,a,
        x,   y+h, r,g,b,a,
    };
    flatShader.use();
    flatShader.setMat4("proj", proj);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER,0,sizeof(verts),verts);
    glBindVertexArray(vao);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDrawArrays(GL_TRIANGLES,0,6);
    glBindVertexArray(0);
}

void TextRenderer::drawGradientRect(float x, float y, float w, float h,
                                    float r1,float g1,float b1,float a1,
                                    float r2,float g2,float b2,float a2,
                                    bool vertical) {
    float verts[6*6];
    auto setV = [&](int i, float px, float py, float r, float g, float b, float a){
        verts[i*6+0]=px; verts[i*6+1]=py;
        verts[i*6+2]=r;  verts[i*6+3]=g; verts[i*6+4]=b; verts[i*6+5]=a;
    };
    if (vertical) {
        setV(0, x,   y,   r1,g1,b1,a1);
        setV(1, x+w, y,   r1,g1,b1,a1);
        setV(2, x,   y+h, r2,g2,b2,a2);
        setV(3, x+w, y,   r1,g1,b1,a1);
        setV(4, x+w, y+h, r2,g2,b2,a2);
        setV(5, x,   y+h, r2,g2,b2,a2);
    } else {
        setV(0, x,   y,   r1,g1,b1,a1);
        setV(1, x+w, y,   r2,g2,b2,a2);
        setV(2, x,   y+h, r1,g1,b1,a1);
        setV(3, x+w, y,   r2,g2,b2,a2);
        setV(4, x+w, y+h, r2,g2,b2,a2);
        setV(5, x,   y+h, r1,g1,b1,a1);
    }
    flatShader.use();
    flatShader.setMat4("proj", proj);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER,0,sizeof(verts),verts);
    glBindVertexArray(vao);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDrawArrays(GL_TRIANGLES,0,6);
    glBindVertexArray(0);
}

void TextRenderer::drawCircle(float cx, float cy, float radius, int segs,
                              float r, float g, float b, float a) {
    std::vector<float> verts;
    float step = 2.f * 3.14159265f / segs;
    for (int i = 0; i < segs; ++i) {
        float a0 = i*step, a1 = (i+1)*step;
        verts.insert(verts.end(), {cx,cy, r,g,b,a});
        verts.insert(verts.end(), {cx+cosf(a0)*radius, cy+sinf(a0)*radius, r,g,b,a});
        verts.insert(verts.end(), {cx+cosf(a1)*radius, cy+sinf(a1)*radius, r,g,b,a});
    }
    flatShader.use();
    flatShader.setMat4("proj", proj);
    GLuint tmpVao, tmpVbo;
    glGenVertexArrays(1,&tmpVao); glGenBuffers(1,&tmpVbo);
    glBindVertexArray(tmpVao);
    glBindBuffer(GL_ARRAY_BUFFER,tmpVbo);
    glBufferData(GL_ARRAY_BUFFER,verts.size()*sizeof(float),verts.data(),GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0); glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,6*sizeof(float),(void*)0);
    glEnableVertexAttribArray(1); glVertexAttribPointer(1,4,GL_FLOAT,GL_FALSE,6*sizeof(float),(void*)(2*sizeof(float)));
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDrawArrays(GL_TRIANGLES,0,(GLsizei)(verts.size()/6));
    glBindVertexArray(0);
    glDeleteVertexArrays(1,&tmpVao); glDeleteBuffers(1,&tmpVbo);
}

void TextRenderer::drawRing(float cx, float cy, float radius, float thickness, int segs,
                            float r, float g, float b, float a) {
    float inner = radius - thickness*0.5f;
    float outer = radius + thickness*0.5f;
    std::vector<float> verts;
    float step = 2.f * 3.14159265f / segs;
    for (int i = 0; i < segs; ++i) {
        float a0 = i*step, a1 = (i+1)*step;
        float ix0=cx+cosf(a0)*inner, iy0=cy+sinf(a0)*inner;
        float ox0=cx+cosf(a0)*outer, oy0=cy+sinf(a0)*outer;
        float ix1=cx+cosf(a1)*inner, iy1=cy+sinf(a1)*inner;
        float ox1=cx+cosf(a1)*outer, oy1=cy+sinf(a1)*outer;
        verts.insert(verts.end(),{ix0,iy0,r,g,b,a});
        verts.insert(verts.end(),{ox0,oy0,r,g,b,a});
        verts.insert(verts.end(),{ox1,oy1,r,g,b,a});
        verts.insert(verts.end(),{ix0,iy0,r,g,b,a});
        verts.insert(verts.end(),{ox1,oy1,r,g,b,a});
        verts.insert(verts.end(),{ix1,iy1,r,g,b,a});
    }
    flatShader.use();
    flatShader.setMat4("proj", proj);
    GLuint tmpVao, tmpVbo;
    glGenVertexArrays(1,&tmpVao); glGenBuffers(1,&tmpVbo);
    glBindVertexArray(tmpVao);
    glBindBuffer(GL_ARRAY_BUFFER,tmpVbo);
    glBufferData(GL_ARRAY_BUFFER,verts.size()*sizeof(float),verts.data(),GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0); glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,6*sizeof(float),(void*)0);
    glEnableVertexAttribArray(1); glVertexAttribPointer(1,4,GL_FLOAT,GL_FALSE,6*sizeof(float),(void*)(2*sizeof(float)));
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDrawArrays(GL_TRIANGLES,0,(GLsizei)(verts.size()/6));
    glBindVertexArray(0);
    glDeleteVertexArrays(1,&tmpVao); glDeleteBuffers(1,&tmpVbo);
}

void TextRenderer::drawLine(float x1, float y1, float x2, float y2, float width,
                            float r, float g, float b, float a) {
    float dx = x2-x1, dy = y2-y1;
    float len = sqrtf(dx*dx+dy*dy);
    if (len < 0.001f) return;
    float nx = -dy/len*width*0.5f, ny = dx/len*width*0.5f;
    float verts[] = {
        x1+nx,y1+ny,r,g,b,a,
        x1-nx,y1-ny,r,g,b,a,
        x2-nx,y2-ny,r,g,b,a,
        x1+nx,y1+ny,r,g,b,a,
        x2-nx,y2-ny,r,g,b,a,
        x2+nx,y2+ny,r,g,b,a,
    };
    flatShader.use();
    flatShader.setMat4("proj", proj);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER,0,sizeof(verts),verts);
    glBindVertexArray(vao);
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glDrawArrays(GL_TRIANGLES,0,6);
    glBindVertexArray(0);
}

void TextRenderer::drawRectRounded(float x, float y, float w, float h,
                                   float radius, float r, float g, float b, float a) {
    // Fill center + edge rectangles
    drawRect(x+radius, y,        w-2*radius, h,        r,g,b,a);
    drawRect(x,        y+radius, radius,     h-2*radius,r,g,b,a);
    drawRect(x+w-radius,y+radius,radius,     h-2*radius,r,g,b,a);
    // Corners
    int segs = 12;
    float step = 3.14159265f*0.5f / segs;
    auto corner = [&](float cx, float cy, float startAngle){
        std::vector<float> verts;
        for (int i=0;i<segs;++i){
            float a0=startAngle+i*step, a1=startAngle+(i+1)*step;
            verts.insert(verts.end(),{cx,cy,r,g,b,a});
            verts.insert(verts.end(),{cx+cosf(a0)*radius,cy+sinf(a0)*radius,r,g,b,a});
            verts.insert(verts.end(),{cx+cosf(a1)*radius,cy+sinf(a1)*radius,r,g,b,a});
        }
        flatShader.use(); flatShader.setMat4("proj",proj);
        GLuint tv,tb; glGenVertexArrays(1,&tv); glGenBuffers(1,&tb);
        glBindVertexArray(tv); glBindBuffer(GL_ARRAY_BUFFER,tb);
        glBufferData(GL_ARRAY_BUFFER,verts.size()*sizeof(float),verts.data(),GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0); glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,6*sizeof(float),(void*)0);
        glEnableVertexAttribArray(1); glVertexAttribPointer(1,4,GL_FLOAT,GL_FALSE,6*sizeof(float),(void*)(2*sizeof(float)));
        glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        glDrawArrays(GL_TRIANGLES,0,(GLsizei)(verts.size()/6));
        glBindVertexArray(0); glDeleteVertexArrays(1,&tv); glDeleteBuffers(1,&tb);
    };
    float pi = 3.14159265f;
    corner(x+radius,     y+radius,     pi);
    corner(x+w-radius,   y+radius,     pi*1.5f);
    corner(x+w-radius,   y+h-radius,   0.f);
    corner(x+radius,     y+h-radius,   pi*0.5f);
}
