#include "LoadingScreen.h"
#include <cmath>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <iostream>

namespace fs = std::filesystem;

void LoadingScreen::enter() {
    fadeIn=0.f; fadeOut=0.f; progress=0.f;
    spinAngle=0.f; holdTime=0.f; workDone=false; exiting=false; done=false;
    taskIdx=0;
    tasks = {
        "Initializing MahirTV...",
        "Creating account data...",
        "Setting up YouTube...",
        "Installing Settings...",
        "Installing Games...",
        "Installing Browser...",
        "Installing Music...",
        "Installing Store...",
        "Finalizing installation...",
        "Ready!"
    };
    currentTask = tasks[0];
}

void LoadingScreen::createAccDat() {
    std::ofstream f("mtv_root/accdat/account.json");
    f << "{\n"
      << "  \"user\":\"" << app->userName << "\",\n"
      << "  \"mode\":\"" << app->accountMode << "\",\n"
      << "  \"saveSlot\":" << app->selectedSaveSlot << ",\n"
      << "  \"language\":\"" << app->selectedLanguage << "\",\n"
      << "  \"firstRun\":false\n"
      << "}";
    f.close();
}

void LoadingScreen::doInstallWork() {
    createAccDat();

    // Create save data in selected slot
    int slot = app->selectedSaveSlot;
    std::string slotPath = "mtv_root/savedata/slot" + std::to_string(slot+1);
    fs::create_directories(slotPath);
    std::ofstream slotF(slotPath + "/savedata.json");
    slotF << "{\n"
          << "  \"slot\":" << slot+1 << ",\n"
          << "  \"user\":\"" << app->userName << "\",\n"
          << "  \"created\":\"2025-05-13\",\n"
          << "  \"apps\":[\"YouTube\",\"Settings\",\"Games\",\"Browser\",\"Music\",\"Store\"]\n"
          << "}";
    slotF.close();

    // Create app data files
    std::vector<std::string> appNames={"YouTube","Settings","Games","Browser","Music","Store"};
    for (auto& a : appNames) {
        fs::create_directories("mtv_root/applications/"+a+"/data");
        std::ofstream af("mtv_root/applications/"+a+"/appdata.json");
        af << "{\"app\":\"" << a << "\",\"installed\":true,\"slot\":" << slot+1 << "}";
        af.close();
    }

    // Log
    std::ofstream log("mtv_root/logs/install.log");
    log << "[MahirTV] Installation complete\n"
        << "[MahirTV] User: " << app->userName << "\n"
        << "[MahirTV] Save slot: " << slot+1 << "\n"
        << "[MahirTV] Language: " << app->selectedLanguage << "\n";
    log.close();
}

void LoadingScreen::update(float dt) {
    spinAngle += dt * 280.f; // degrees

    if (!workDone) {
        fadeIn = std::min(1.f, fadeIn + dt*1.8f);
        progress += dt * 0.12f;
        if (progress >= 1.f) {
            progress = 1.f;
            doInstallWork();
            workDone = true;
        }
        // Advance task label
        float taskProgress = progress * (float)(tasks.size()-1);
        int  newIdx = (int)taskProgress;
        if (newIdx < (int)tasks.size()) {
            taskIdx = newIdx;
            currentTask = tasks[taskIdx];
        }
    } else {
        holdTime += dt;
        if (holdTime > 1.2f && !exiting) {
            exiting = true;
        }
    }

    if (exiting) {
        fadeOut += dt * 2.0f;
        if (fadeOut >= 1.f) done = true;
        nextState = AppState::Home;
    }
}

void LoadingScreen::render() {
    float alpha = clamp01(fadeIn * (exiting ? (1.f-fadeOut) : 1.f));
    auto& tr = app->tr;
    float W=(float)app->screenW, H=(float)app->screenH;

    // White background
    float whiteness = clamp01(fadeIn);
    tr.drawRect(0,0,W,H, whiteness,whiteness,whiteness,1.f);

    // Faint blue tint at bottom
    tr.drawGradientRect(0,H*0.7f,W,H*0.3f,
                        0.9f,0.95f,1.f,0.f,
                        0.85f,0.92f,1.f,alpha*0.4f);

    // Loading throbber (spinning ring)
    float cx=W*0.5f, cy=H*0.5f;
    float outerR=52.f, innerR=38.f;
    int segs=64;
    float pi=3.14159265f;
    float spinRad = spinAngle * pi / 180.f;
    float arcLen  = pi * 1.3f;

    // Draw arc of ring
    std::vector<float> verts;
    for (int i=0;i<segs;++i) {
        float a0 = spinRad + (float)i/(float)segs * arcLen;
        float a1 = spinRad + (float)(i+1)/(float)segs * arcLen;
        float fade2 = 1.f - (float)i/(float)segs;
        float ba = alpha * fade2;
        float ix0=cx+cosf(a0)*innerR, iy0=cy+sinf(a0)*innerR;
        float ox0=cx+cosf(a0)*outerR, oy0=cy+sinf(a0)*outerR;
        float ix1=cx+cosf(a1)*innerR, iy1=cy+sinf(a1)*innerR;
        float ox1=cx+cosf(a1)*outerR, oy1=cy+sinf(a1)*outerR;
        verts.insert(verts.end(),{ix0,iy0,0.1f,0.55f,1.f,ba});
        verts.insert(verts.end(),{ox0,oy0,0.1f,0.55f,1.f,ba});
        verts.insert(verts.end(),{ox1,oy1,0.1f,0.55f,1.f,ba});
        verts.insert(verts.end(),{ix0,iy0,0.1f,0.55f,1.f,ba});
        verts.insert(verts.end(),{ox1,oy1,0.1f,0.55f,1.f,ba});
        verts.insert(verts.end(),{ix1,iy1,0.1f,0.55f,1.f,ba});
    }

    // Temporary VAO/VBO for the arc
    GLuint tv,tb; glGenVertexArrays(1,&tv); glGenBuffers(1,&tb);
    glBindVertexArray(tv); glBindBuffer(GL_ARRAY_BUFFER,tb);
    glBufferData(GL_ARRAY_BUFFER,verts.size()*sizeof(float),verts.data(),GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0); glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,6*sizeof(float),(void*)0);
    glEnableVertexAttribArray(1); glVertexAttribPointer(1,4,GL_FLOAT,GL_FALSE,6*sizeof(float),(void*)(2*sizeof(float)));

    // Use tr's flat shader
    tr.drawLine(0,0,0,0,0,0,0,0,0); // just to ensure shader binding is fresh
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glDrawArrays(GL_TRIANGLES,0,(GLsizei)(verts.size()/6));
    glBindVertexArray(0); glDeleteVertexArrays(1,&tv); glDeleteBuffers(1,&tb);

    // "Loading..." text
    std::string loadFont = (app->selectedLanguage=="Bangla") ? "bangla_body" : "heading";
    tr.drawTextCentered("Loading...", loadFont, W*0.5f, H*0.5f+90.f,
                        0.1f, 0.5f, 1.f, alpha);

    // Current task
    tr.drawTextCentered(currentTask, "small", W*0.5f, H*0.5f+135.f,
                        0.3f,0.5f,0.8f, alpha*0.7f);

    // Progress bar
    float barW=320.f, barH=6.f;
    float barX=W*0.5f-barW*0.5f, barY=H*0.5f+165.f;
    tr.drawRectRounded(barX,barY,barW,barH,3.f,0.8f,0.88f,0.97f,alpha*0.5f);
    tr.drawRectRounded(barX,barY,barW*progress,barH,3.f,0.1f,0.55f,1.f,alpha);

    // Fade overlay
    if (fadeIn < 1.f) tr.drawRect(0,0,W,H,1.f,1.f,1.f,1.f-fadeIn);
    if (exiting) tr.drawRect(0,0,W,H,0.f,0.f,0.f,clamp01(fadeOut));
}
