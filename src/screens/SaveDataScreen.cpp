#include "SaveDataScreen.h"
#include "../Math.h"
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <filesystem>
namespace fs = std::filesystem;

static const char* VERT3D = R"(
#version 330 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec4 aColor;
uniform mat4 mvp;
out vec4 vColor;
void main(){ gl_Position = mvp * vec4(aPos,1.0); vColor = aColor; }
)";
static const char* FRAG3D = R"(
#version 330 core
in vec4 vColor;
out vec4 fColor;
void main(){ fColor = vColor; }
)";

void SaveDataScreen::enter() {
    fadeIn=0.f; fadeOut=0.f; exiting=false; done=false;
    time=0.f; drawing=false; selectedSlot=-1;
    particles.clear();

    shader3D.load(VERT3D,FRAG3D);
    buildGrid();

    // All 3 slots at the same depth so they project evenly across screen
    slots[0]={ -4.2f, 0.f, 1.0f,  0.f,0.f, 0.f, false };
    slots[1]={  0.0f, 0.f, 1.0f,  0.f,0.f, 0.f, false };
    slots[2]={  4.2f, 0.f, 1.0f,  0.f,0.f, 0.f, false };

    // Check which slots are occupied (have saved data on disk)
    for (int i=0;i<3;++i) {
        std::string d = "mtv_root/saves/slot_" + std::to_string(i) + "/data.json";
        slots[i].occupied = fs::exists(d);
    }
}

void SaveDataScreen::exit() {
    shader3D.destroy();
    if (gridVao) { glDeleteVertexArrays(1,&gridVao); gridVao=0; }
    if (gridVbo) { glDeleteBuffers(1,&gridVbo);      gridVbo=0; }
}

void SaveDataScreen::buildGrid() {
    std::vector<float> verts;
    float ext=8.f, step=1.f;
    float lum=0.07f, a=1.f;
    for (float z=-ext; z<=ext; z+=step) {
        verts.insert(verts.end(),{-ext,0.f,z, lum,lum*1.5f,lum*3.f,a});
        verts.insert(verts.end(),{ ext,0.f,z, lum,lum*1.5f,lum*3.f,a});
    }
    for (float x=-ext; x<=ext; x+=step) {
        verts.insert(verts.end(),{x,0.f,-ext, lum,lum*1.5f,lum*3.f,a});
        verts.insert(verts.end(),{x,0.f, ext, lum,lum*1.5f,lum*3.f,a});
    }
    gridVertCount=(int)(verts.size()/7);
    glGenVertexArrays(1,&gridVao); glGenBuffers(1,&gridVbo);
    glBindVertexArray(gridVao); glBindBuffer(GL_ARRAY_BUFFER,gridVbo);
    glBufferData(GL_ARRAY_BUFFER,verts.size()*sizeof(float),verts.data(),GL_STATIC_DRAW);
    glEnableVertexAttribArray(0); glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,7*sizeof(float),(void*)0);
    glEnableVertexAttribArray(1); glVertexAttribPointer(1,4,GL_FLOAT,GL_FALSE,7*sizeof(float),(void*)(3*sizeof(float)));
    glBindVertexArray(0);
}

void SaveDataScreen::projectSlots() {
    // Place the 3 circles at fixed, evenly-spaced screen positions.
    // The 3D grid is rendered by OpenGL; circles are drawn in 2D by TextRenderer.
    // Hard-coding avoids the CPU-side matrix projection mismatch entirely.
    float W=(float)app->screenW, H=(float)app->screenH;
    slots[0].sx = W * 0.20f;  slots[0].sy = H * 0.50f;
    slots[1].sx = W * 0.50f;  slots[1].sy = H * 0.50f;
    slots[2].sx = W * 0.80f;  slots[2].sy = H * 0.50f;
}

void SaveDataScreen::handleEvent(const SDL_Event& e) {
    if (exiting) return;
    if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button==SDL_BUTTON_LEFT) {
        drawStartX=(float)e.button.x; drawStartY=(float)e.button.y;
        drawCurrX=drawStartX; drawCurrY=drawStartY;
        drawing=true;
    }
    if (e.type == SDL_MOUSEMOTION && drawing) {
        drawCurrX=(float)e.motion.x; drawCurrY=(float)e.motion.y;
        tryConnect();
    }
    if (e.type == SDL_MOUSEBUTTONUP && e.button.button==SDL_BUTTON_LEFT) {
        drawing=false;
    }
}

void SaveDataScreen::tryConnect() {
    for (int i=0;i<3;++i) {
        if (slots[i].connected) continue;
        float dx=drawCurrX-slots[i].sx, dy=drawCurrY-slots[i].sy;
        float dist=sqrtf(dx*dx+dy*dy);
        if (dist < 55.f) {
            slots[i].connected=true;
            selectedSlot=i;
            app->selectedSaveSlot=i;
            spawnParticles(i);
            // Persist the session so next launch skips setup
            app->saveSession();
            exiting=true;
            nextState=AppState::Loading;
        }
    }
}

void SaveDataScreen::spawnParticles(int slot) {
    // Spawn many small circles that orbit the slot circle
    for (int i=0;i<40;++i) {
        float angle = (float)i / 40.f * 6.2831f;
        float r2    = 40.f + (rand()%30);
        Particle p;
        p.x      = slots[slot].sx + cosf(angle)*r2;
        p.y      = slots[slot].sy + sinf(angle)*r2;
        p.tx     = slots[slot].sx;
        p.ty     = slots[slot].sy;
        p.t      = 0.f;
        p.speed  = 0.3f + (rand()%40)/100.f;
        p.slotIdx= slot;
        p.alive  = true;
        particles.push_back(p);
    }
}

void SaveDataScreen::update(float dt) {
    time += dt;
    for (auto& p : particles) {
        if (!p.alive) continue;
        // Spiral in toward center
        float angle = p.t * 8.f * 3.14159f;
        float r2    = (1.f-p.t) * 60.f;
        p.x = slots[p.slotIdx].sx + cosf(angle)*r2;
        p.y = slots[p.slotIdx].sy + sinf(angle)*r2;
        p.t += dt * p.speed;
        if (p.t >= 1.f) p.alive=false;
    }
    for (int i=0;i<3;++i) slots[i].pulseAmt=0.5f+0.5f*sinf(time*2.f+(float)i*1.2f);
    if (!exiting) { fadeIn=std::min(1.f,fadeIn+dt*1.5f); }
    else { fadeOut+=dt*2.2f; if (fadeOut>=1.f) { app->selectedSaveSlot=selectedSlot; done=true; } }
}

void SaveDataScreen::renderGrid() {
    float W=(float)app->screenW, H=(float)app->screenH;
    Mat4 view = mat4LookAt({0.f,5.f,9.f},{0.f,0.f,0.f},{0.f,1.f,0.f});
    Mat4 proj = mat4Perspective(0.75f, W/H, 0.1f, 100.f);
    Mat4 mvp  = mat4Multiply(proj,view);
    shader3D.use();
    shader3D.setMat4("mvp",mvp);
    glBindVertexArray(gridVao);
    glLineWidth(1.f);
    glDrawArrays(GL_LINES,0,gridVertCount);
    glBindVertexArray(0);
}

void SaveDataScreen::renderSlotCircles() {
    auto& tr = app->tr;
    for (int i=0;i<3;++i) {
        float sx=slots[i].sx, sy=slots[i].sy;
        float pulse=slots[i].pulseAmt;
        float r=50.f+pulse*8.f;
        float alpha=fadeIn;
        bool conn=slots[i].connected;
        bool occ=slots[i].occupied;

        // Colour: occupied=gold, new=blue, connected=green
        float cr = conn?0.f  : occ?0.9f : 0.08f;
        float cg = conn?0.8f : occ?0.7f : 0.3f;
        float cb = conn?1.f  : occ?0.1f : 0.85f;

        // Outer glow
        for (int g=4;g>=1;--g)
            tr.drawCircle(sx,sy,r+(float)g*12.f,48,cr,cg,cb,alpha*0.05f*(5.f-g));

        // Main circle
        tr.drawCircle(sx,sy,r,48, cr,cg,cb, alpha*(conn?0.85f:occ?0.55f:0.40f));

        // Ring
        tr.drawRing(sx,sy,r+5.f,3.f,64,cr*0.4f+0.3f,cg*0.4f+0.5f,cb*0.4f+0.6f,alpha*0.9f);

        // State badge inside circle
        if (conn)
            tr.drawTextCentered(app->L("CONNECTED"),"small",sx,sy,0.1f,1.f,0.4f,alpha);
        else if (occ)
            tr.drawTextCentered("SAVED","small",sx,sy,1.f,0.85f,0.2f,alpha*0.9f);
        else
            tr.drawTextCentered("NEW","small",sx,sy,0.4f,0.85f,1.f,alpha*0.7f);

        // Label below
        std::string label = app->L("Save Data " + std::to_string(i+1));
        tr.drawTextCentered(label,"small",sx,sy+r+22.f,0.7f,0.9f,1.f,alpha*0.8f);
    }
}

void SaveDataScreen::renderParticles() {
    auto& tr = app->tr;
    float alpha=fadeIn;
    for (auto& p : particles) {
        if (!p.alive) continue;
        float sz=5.f*(1.f-p.t)+2.f;
        tr.drawCircle(p.x,p.y,sz,16,0.4f,0.8f,1.f,alpha*(1.f-p.t)*0.9f);
    }
}

void SaveDataScreen::renderUI() {
    auto& tr = app->tr;
    float W=(float)app->screenW, H=(float)app->screenH;
    float alpha=fadeIn;
    tr.drawTextCentered(app->L("Select Save Data"),"heading",W*0.5f,H*0.07f,0.8f,0.95f,1.f,alpha);
    tr.drawTextCentered(app->L("Draw a line from the center-bottom to a circle"),"small",
                        W*0.5f,H*0.92f,0.5f,0.75f,1.f,alpha*0.6f);

    // Draw start point (bottom center connector)
    float sx2=W*0.5f, sy2=H*0.82f;
    float pulse=0.5f+0.5f*sinf(time*3.f);
    tr.drawCircle(sx2,sy2,14.f+pulse*4.f,32,0.5f,0.85f,1.f,alpha*0.6f);
    tr.drawRing(sx2,sy2,18.f,2.5f,32,0.3f,0.7f,1.f,alpha*pulse*0.8f);

    // Draw the drag line
    if (drawing) {
        tr.drawLine(sx2,sy2,drawCurrX,drawCurrY,3.f,0.4f,0.8f,1.f,alpha*0.7f);
        // Small moving dots along the line
        float dx=drawCurrX-sx2, dy=drawCurrY-sy2;
        float len=sqrtf(dx*dx+dy*dy);
        for (int d=0;d<6;++d) {
            float t2=fmodf((float)d/6.f+time*2.f,1.f);
            float px2=sx2+dx*t2, py2=sy2+dy*t2;
            tr.drawCircle(px2,py2,4.f,16,0.5f,0.9f,1.f,alpha*0.8f*(1.f-t2*0.5f));
        }
    }
}

void SaveDataScreen::render() {
    float W=(float)app->screenW, H=(float)app->screenH;
    // Background
    app->tr.drawGradientRect(0,0,W,H,0.01f,0.02f,0.10f,1.f,0.03f,0.08f,0.28f,1.f);
    // Atmospheric glow from bottom
    app->tr.drawGradientRect(0,H*0.5f,W,H*0.5f,0.02f,0.05f,0.20f,0.f,0.06f,0.14f,0.45f,0.3f);

    glEnable(GL_DEPTH_TEST);
    glClear(GL_DEPTH_BUFFER_BIT);
    renderGrid();
    glDisable(GL_DEPTH_TEST);

    projectSlots();
    renderSlotCircles();
    renderParticles();
    renderUI();

    if (exiting) app->tr.drawRect(0,0,W,H,1.f,1.f,1.f,clamp01(fadeOut));
    else if (fadeIn<1.f) app->tr.drawRect(0,0,W,H,0.f,0.f,0.f,1.f-fadeIn);
}
