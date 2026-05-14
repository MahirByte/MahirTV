#include "SettingsScreen.h"
#include <cmath>
#include <fstream>
#include <filesystem>
#include <cstdlib>
namespace fs = std::filesystem;

void SettingsScreen::enter() {
    fadeIn=0.f; fadeOut=0.f; exiting=false; done=false; time=0.f;
    selectedCategory=0; hoveredItem=-1;
    // Copy current settings as temp values
    tmpFullscreen  = app->isFullscreen;
    tmpVolume      = app->masterVolume;
    tmpCursorScale = app->cursorScale;
    tmpLang        = app->selectedLanguage;
}

void SettingsScreen::handleEvent(const SDL_Event& e) {
    if (exiting) return;
    float W=(float)app->screenW, H=(float)app->screenH;
    float sideW=280.f;

    if (e.type==SDL_MOUSEMOTION) {
        float mx=(float)e.motion.x, my=(float)e.motion.y;
        hoveredItem=-1;
        // Sidebar hover
        for (int i=0;i<(int)categories.size();++i) {
            float iy=140.f+(float)i*70.f;
            if (mx>=0&&mx<=sideW&&my>=iy&&my<=iy+62.f) hoveredItem=i;
        }
    }
    if (e.type==SDL_MOUSEBUTTONDOWN && e.button.button==SDL_BUTTON_LEFT) {
        float mx=(float)e.button.x, my=(float)e.button.y;

        // ── Sidebar click ─────────────────────────────────────────
        for (int i=0;i<(int)categories.size();++i) {
            float iy=140.f+(float)i*70.f;
            if (mx>=0&&mx<=sideW&&my>=iy&&my<=iy+62.f) { selectedCategory=i; return; }
        }

        // ── Back button (bottom-right) — matches render ───────────
        // renderSidebar draws back btn at W-180..W-20, H-80..H-20 — but
        // the actual Back button is drawn in render() at W-180,H-80,160,50
        if (mx>=W-180.f&&mx<=W-20.f&&my>=H-80.f&&my<=H-20.f) {
            applySettings(); exiting=true; nextState=AppState::Home; return;
        }

        // Content area — all coordinates match renderXxx() exactly
        float cx = sideW + 40.f;   // = 320

        // ── Display (0) ───────────────────────────────────────────
        if (selectedCategory==0) {
            // Fullscreen toggle: drawRectRounded(cx, 260, 180, 44)
            if (mx>=cx&&mx<=cx+180.f&&my>=260.f&&my<=304.f) {
                tmpFullscreen=!tmpFullscreen;
                app->isFullscreen=tmpFullscreen;
                SDL_SetWindowFullscreen(app->window,
                    tmpFullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
                return;
            }
            // Cursor "-" button: drawRectRounded(cx, 368, 90, 40)
            if (mx>=cx&&mx<=cx+90.f&&my>=368.f&&my<=408.f) {
                tmpCursorScale=std::max(0.15f, tmpCursorScale-0.06f);
                app->cursorScale=tmpCursorScale; return;
            }
            // Cursor "+" button: drawRectRounded(cx+100, 368, 90, 40)
            if (mx>=cx+100.f&&mx<=cx+190.f&&my>=368.f&&my<=408.f) {
                tmpCursorScale=std::min(0.70f, tmpCursorScale+0.06f);
                app->cursorScale=tmpCursorScale; return;
            }
        }

        // ── Language (1) ──────────────────────────────────────────
        if (selectedCategory==1) {
            // English: drawRectRounded(cx, 220, 170, 54)
            if (mx>=cx&&mx<=cx+170.f&&my>=220.f&&my<=274.f) {
                tmpLang="English"; app->selectedLanguage="English"; return;
            }
            // Bangla: drawRectRounded(cx+180, 220, 170, 54)
            if (mx>=cx+180.f&&mx<=cx+350.f&&my>=220.f&&my<=274.f) {
                tmpLang="Bangla"; app->selectedLanguage="Bangla"; return;
            }
        }

        // ── Audio (3) — volume bar: drawRectRounded(cx, 265, 400, 20) ─
        if (selectedCategory==3) {
            if (mx>=cx&&mx<=cx+400.f&&my>=255.f&&my<=295.f) {
                tmpVolume=(int)(((mx-cx)/400.f)*100.f);
                tmpVolume=std::max(0,std::min(100,tmpVolume));
                app->masterVolume=tmpVolume; return;
            }
        }

        // ── Danger Zone (5) ───────────────────────────────────────
        // Kill Save Data button: drawRectRounded(W-300, 258, 220, 50)
        if (selectedCategory==5) {
            float bx=W-300.f, by=258.f, bw=220.f, bh=50.f;
            if (mx>=bx&&mx<=bx+bw&&my>=by&&my<=by+bh) {
                int slot = app->selectedSaveSlot + 1;
                std::string msg = "Erase Save Data " + std::to_string(slot) +
                                  "? This cannot be undone.";
                std::string cmd = "zenity --question "
                    "--title='Kill Save Data' "
                    "--text='" + msg + "' "
                    "--ok-label='Yes, Erase' "
                    "--cancel-label='No, Cancel' "
                    "2>/dev/null";
                int ret = system(cmd.c_str());
                if (ret == 0) {
                    app->deleteSlotData(app->selectedSaveSlot);
                    applySettings();
                    exiting=true; nextState=AppState::SaveData;
                }
                return;
            }
        }
    }
    if (e.type==SDL_KEYDOWN && e.key.keysym.sym==SDLK_ESCAPE) {
        applySettings(); exiting=true; nextState=AppState::Home;
    }
}

void SettingsScreen::applySettings() {
    app->selectedLanguage = tmpLang;
    app->isFullscreen     = tmpFullscreen;
    app->masterVolume     = tmpVolume;
    app->cursorScale      = tmpCursorScale;
    app->saveSettings();
}

void SettingsScreen::update(float dt) {
    time+=dt;
    if (!exiting) fadeIn=std::min(1.f,fadeIn+dt*2.f);
    else { fadeOut+=dt*2.f; if(fadeOut>=1.f) done=true; }
}

void SettingsScreen::renderSidebar() {
    auto& tr=app->tr;
    float H=(float)app->screenH;
    float sideW=280.f;
    float alpha=fadeIn;

    tr.drawRect(0,0,sideW,H,0.f,0.02f,0.10f,alpha*0.95f);
    tr.drawRect(sideW-2.f,0.f,2.f,H,0.2f,0.55f,1.f,alpha*0.3f);

    // Logo
    tr.drawTextCentered("MahirTV","titleBold",sideW*0.5f,55.f,0.3f,0.75f,1.f,alpha);
    tr.drawTextCentered(app->L("Settings"),"small",sideW*0.5f,95.f,0.5f,0.75f,1.f,alpha*0.6f);

    for (int i=0;i<(int)categories.size();++i) {
        float iy=140.f+(float)i*70.f;
        bool sel=(i==selectedCategory);
        bool hov=(i==hoveredItem);
        float bg = sel?0.25f : hov?0.12f : 0.f;
        if(sel||hov) tr.drawRect(0.f,iy,sideW,62.f,0.1f,0.4f,0.9f,alpha*bg);
        if(sel) tr.drawRect(0.f,iy,4.f,62.f,0.3f,0.7f,1.f,alpha);
        tr.drawText(categories[i].icon+" "+app->L(categories[i].name),
                    "body",18.f,iy+18.f,sel?1.f:0.7f,sel?1.f:0.85f,1.f,alpha);
    }
}

void SettingsScreen::renderContent() {
    switch(selectedCategory) {
        case 0: renderDisplay();    break;
        case 1: renderLanguage();   break;
        case 2: renderAccount();    break;
        case 3: renderAudio();      break;
        case 4: renderAbout();      break;
        case 5: renderDangerZone(); break;
    }
}

static void label(TextRenderer& tr, float x, float y, const std::string& t, float a) {
    tr.drawText(t,"headingBold",x,y,0.8f,0.92f,1.f,a);
}
static void sublabel(TextRenderer& tr, float x, float y, const std::string& t, float a) {
    tr.drawText(t,"small",x,y+38.f,0.5f,0.7f,0.9f,a*0.7f);
}

void SettingsScreen::renderDisplay() {
    auto& tr=app->tr;
    float alpha=fadeIn;
    float cx=320.f;

    label(tr,cx,160.f,"Display Settings",alpha);
    tr.drawRect(cx,198.f,(float)app->screenW-cx-40.f,1.f,0.2f,0.5f,0.9f,alpha*0.3f);

    // Fullscreen toggle
    label(tr,cx,220.f,"Fullscreen",alpha);
    bool fs=app->isFullscreen;
    tr.drawRectRounded(cx,260.f,180.f,44.f,10.f,
                       fs?0.1f:0.05f, fs?0.5f:0.2f, fs?0.95f:0.4f, alpha*0.9f);
    tr.drawTextCentered(fs?"ON : Fullscreen":"OFF : Windowed","body",cx+90.f,282.f,1.f,1.f,1.f,alpha);

    // Cursor size
    label(tr,cx,330.f,"Cursor Size",alpha);
    tr.drawRectRounded(cx,  368.f,90.f,40.f,8.f,0.1f,0.35f,0.8f,alpha*0.8f);
    tr.drawTextCentered("  -  ","body",cx+45.f,388.f,1.f,1.f,1.f,alpha);
    tr.drawRectRounded(cx+100.f,368.f,90.f,40.f,8.f,0.1f,0.35f,0.8f,alpha*0.8f);
    tr.drawTextCentered("  +  ","body",cx+145.f,388.f,1.f,1.f,1.f,alpha);
    char buf[32]; snprintf(buf,32,"%.0f%%",(app->cursorScale/0.70f)*100.f);
    tr.drawText(buf,"body",cx+210.f,375.f,0.7f,0.9f,1.f,alpha);

    // Resolution info
    label(tr,cx,440.f,"Resolution",alpha);
    char res[64]; snprintf(res,64,"%d x %d",app->screenW,app->screenH);
    tr.drawText(res,"body",cx,480.f,0.6f,0.8f,1.f,alpha*0.8f);
}

void SettingsScreen::renderLanguage() {
    auto& tr=app->tr;
    float alpha=fadeIn;
    float cx=320.f;

    label(tr,cx,160.f,app->L("Language"),alpha);
    tr.drawRect(cx,198.f,(float)app->screenW-cx-40.f,1.f,0.2f,0.5f,0.9f,alpha*0.3f);

    bool engSel=(app->selectedLanguage=="English");
    tr.drawRectRounded(cx,    220.f,170.f,54.f,12.f,
                       engSel?0.1f:0.05f, engSel?0.5f:0.2f, engSel?0.95f:0.4f, alpha*0.9f);
    tr.drawTextCentered("English","heading",cx+85.f,247.f,1.f,1.f,1.f,alpha);

    bool bnSel=(app->selectedLanguage=="Bangla");
    tr.drawRectRounded(cx+180.f,220.f,170.f,54.f,12.f,
                       bnSel?0.1f:0.05f, bnSel?0.5f:0.2f, bnSel?0.95f:0.4f, alpha*0.9f);
    tr.drawTextCentered(u8"বাংলা","bangla_body",cx+180.f+85.f,247.f,1.f,1.f,1.f,alpha);

    tr.drawText("Restart or re-enter each screen for full language change.",
                "small",cx,300.f,0.5f,0.7f,0.9f,alpha*0.5f);
    tr.drawText("Bengali requires: sudo apt install fonts-noto-core",
                "small",cx,330.f,0.5f,0.7f,0.5f,alpha*0.4f);
}

void SettingsScreen::renderAccount() {
    auto& tr=app->tr;
    float alpha=fadeIn;
    float cx=320.f;

    label(tr,cx,160.f,app->L("Account"),alpha);
    tr.drawRect(cx,198.f,(float)app->screenW-cx-40.f,1.f,0.2f,0.5f,0.9f,alpha*0.3f);

    std::string uname = app->userName.empty() ? "Guest" : app->userName;
    std::string mode  = app->accountMode;
    tr.drawRectRounded(cx,230.f,420.f,120.f,16.f,0.08f,0.2f,0.5f,alpha*0.25f);
    tr.drawText("User:","small",cx+20.f,250.f,0.6f,0.8f,1.f,alpha*0.6f);
    tr.drawText(uname,"headingBold",cx+20.f,272.f,1.f,1.f,1.f,alpha);
    tr.drawText("Mode: "+mode,"small",cx+20.f,316.f,0.5f,0.75f,1.f,alpha*0.6f);

    tr.drawText("Save Slot: "+std::to_string(app->selectedSaveSlot+1),
                "body",cx,380.f,0.6f,0.85f,1.f,alpha*0.7f);
}

void SettingsScreen::renderAudio() {
    auto& tr=app->tr;
    float alpha=fadeIn;
    float cx=320.f;

    label(tr,cx,160.f,app->L("Audio"),alpha);
    tr.drawRect(cx,198.f,(float)app->screenW-cx-40.f,1.f,0.2f,0.5f,0.9f,alpha*0.3f);

    label(tr,cx,230.f,"Master Volume",alpha);
    // Volume bar (clickable)
    float barW=400.f;
    tr.drawRectRounded(cx,265.f,barW,20.f,6.f,0.1f,0.2f,0.4f,alpha*0.6f);
    float filled=(float)app->masterVolume/100.f*barW;
    tr.drawRectRounded(cx,265.f,filled,20.f,6.f,0.15f,0.55f,1.f,alpha);
    // Knob
    tr.drawCircle(cx+filled,275.f,12.f,24,0.3f,0.75f,1.f,alpha);
    char vbuf[16]; snprintf(vbuf,16,"%d%%",app->masterVolume);
    tr.drawText(vbuf,"body",cx+barW+18.f,260.f,0.7f,0.9f,1.f,alpha);
    tr.drawText("Click on the bar to adjust volume","small",cx,300.f,0.4f,0.6f,0.8f,alpha*0.5f);
}

void SettingsScreen::renderAbout() {
    auto& tr=app->tr;
    float alpha=fadeIn;
    float cx=320.f;
    float W=(float)app->screenW;

    label(tr,cx,160.f,app->L("About"),alpha);
    tr.drawRect(cx,198.f,W-cx-40.f,1.f,0.2f,0.5f,0.9f,alpha*0.3f);

    tr.drawRectRounded(cx,230.f,480.f,260.f,16.f,0.06f,0.15f,0.4f,alpha*0.2f);
    tr.drawText("MahirTV",         "titleBold",cx+20.f,245.f,0.3f,0.75f,1.f,alpha);
    tr.drawText("Version 1.0.0",   "body",     cx+20.f,305.f,0.7f,0.85f,1.f,alpha*0.8f);
    tr.drawText("Modern Linux TV OS Simulator","body",cx+20.f,340.f,0.6f,0.8f,1.f,alpha*0.7f);
    tr.drawText("Built with: C++ | SDL2 | OpenGL","small",cx+20.f,380.f,0.5f,0.7f,0.9f,alpha*0.5f);
    tr.drawText("Fonts: Amaranth | Alkatra","small",cx+20.f,408.f,0.5f,0.7f,0.9f,alpha*0.5f);
    tr.drawText("(c) 2025 MahirTV Project","small",cx+20.f,440.f,0.4f,0.6f,0.8f,alpha*0.4f);
}

void SettingsScreen::renderDangerZone() {
    auto& tr  = app->tr;
    float alpha = fadeIn;
    float cx  = 320.f;
    float W   = (float)app->screenW;

    // Section header — red tinted
    tr.drawText("[!] Danger Zone","headingBold",cx,160.f,1.f,0.25f,0.25f,alpha);
    tr.drawRect(cx,198.f,W-cx-40.f,1.f,0.8f,0.15f,0.15f,alpha*0.4f);

    tr.drawText("These actions are permanent and cannot be undone.",
                "small",cx,215.f,0.8f,0.5f,0.5f,alpha*0.6f);

    // ── Kill Save Data card ───────────────────────────────────────
    tr.drawRectRounded(cx,242.f,W-cx-60.f,90.f,12.f,0.25f,0.04f,0.04f,alpha*0.18f);
    tr.drawRect(cx,242.f,4.f,90.f,1.f,0.2f,0.2f,alpha*0.9f);   // red left bar

    // Title + description
    tr.drawText("Kill Save Data","headingBold",cx+16.f,252.f,1.f,0.35f,0.35f,alpha);
    std::string slotInfo = "Current slot: Save Data " +
                           std::to_string(app->selectedSaveSlot + 1);
    tr.drawText(slotInfo,"small",cx+16.f,284.f,0.8f,0.55f,0.55f,alpha*0.7f);
    tr.drawText("Erases this save slot and returns you to the save selector.",
                "small",cx+16.f,306.f,0.65f,0.4f,0.4f,alpha*0.55f);

    // Button
    float bx=W-300.f, by=258.f, bw=220.f, bh=50.f;
    tr.drawRectRounded(bx,by,bw,bh,10.f,0.7f,0.06f,0.06f,alpha*0.85f);
    tr.drawRectRounded(bx,by,bw,bh,10.f,1.f,0.15f,0.15f,alpha*0.15f);
    tr.drawTextCentered("[ Kill Save Data ]","body",bx+bw*0.5f,by+bh*0.5f,
                        1.f,0.88f,0.88f,alpha);

    // Warning note below button
    tr.drawTextCentered("Click the button to confirm in a dialog",
                        "small",bx+bw*0.5f,by+bh+14.f,0.6f,0.35f,0.35f,alpha*0.5f);

    // ── Legend ───────────────────────────────────────────────────
    tr.drawRect(cx,360.f,W-cx-40.f,1.f,0.5f,0.1f,0.1f,alpha*0.2f);
    tr.drawText("After erasing: the destroyed slot reappears as NEW in the",
                "small",cx,372.f,0.6f,0.4f,0.4f,alpha*0.45f);
    tr.drawText("save selector. Select it again to create a fresh save.",
                "small",cx,394.f,0.6f,0.4f,0.4f,alpha*0.45f);
}

void SettingsScreen::render() {
    float W=(float)app->screenW, H=(float)app->screenH;
    auto& tr=app->tr;
    float alpha=clamp01(exiting ? 1.f-fadeOut : fadeIn);

    tr.drawGradientRect(0,0,W,H,0.02f,0.04f,0.16f,1.f,0.04f,0.10f,0.32f,1.f);

    renderSidebar();
    renderContent();

    // Back button
    tr.drawRectRounded(W-180.f,H-80.f,160.f,50.f,10.f,0.1f,0.4f,0.85f,alpha*0.8f);
    tr.drawTextCentered("< Back","headingBold",W-100.f,H-55.f,1.f,1.f,1.f,alpha);

    if (exiting) tr.drawRect(0,0,W,H,0.f,0.f,0.f,clamp01(fadeOut));
    else if(fadeIn<1.f) tr.drawRect(0,0,W,H,0.f,0.f,0.f,1.f-fadeIn);
}
