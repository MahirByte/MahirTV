#include "AccountScreen.h"
#include <cmath>
#include <fstream>

void AccountScreen::enter() {
    fadeIn = 0.f; fadeOut = 0.f;
    exiting = false; done = false;
    mode = Mode::Select;
    inputUser = ""; inputPass = ""; statusMsg = "";
}

void AccountScreen::handleEvent(const SDL_Event& e) {
    if (exiting) return;
    float W = (float)app->screenW, H = (float)app->screenH;
    float alpha = fadeIn;

    if (mode == Mode::Select) {
        // 3 card options
        struct Card { float x, y, w, h; };
        float cw=360, ch=200, gap=40;
        float startX = W*0.5f - (3*cw + 2*gap)*0.5f;
        float cy2 = H*0.5f;
        Card cards[3] = {
            {startX,              cy2-ch*0.5f, cw, ch},
            {startX+cw+gap,       cy2-ch*0.5f, cw, ch},
            {startX+2*(cw+gap),   cy2-ch*0.5f, cw, ch}
        };
        if (e.type == SDL_MOUSEMOTION) {
            float mx=(float)e.motion.x, my=(float)e.motion.y;
            hovered=-1;
            for (int i=0;i<3;++i)
                if (mx>=cards[i].x && mx<=cards[i].x+cards[i].w
                 && my>=cards[i].y && my<=cards[i].y+cards[i].h)
                    hovered=i;
        }
        if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button==SDL_BUTTON_LEFT) {
            if (hovered==0) { mode=Mode::SignIn; inputUser=""; inputPass=""; statusMsg=""; }
            if (hovered==1) { mode=Mode::SignUp; inputUser=""; inputPass=""; statusMsg=""; }
            if (hovered==2) {
                app->accountMode="guest"; app->userName="Guest";
                exiting=true; nextState=AppState::SaveData;
            }
        }
    } else {
        // Form input
        if (e.type == SDL_MOUSEBUTTONDOWN) {
            float mx=(float)e.button.x, my=(float)e.button.y;
            float fx=W*0.5f-200, fy1=H*0.45f, fw=400, fh=52;
            float fy2=H*0.55f;
            if (mx>=fx&&mx<=fx+fw&&my>=fy1&&my<=fy1+fh) activeField=0;
            else if (mx>=fx&&mx<=fx+fw&&my>=fy2&&my<=fy2+fh) activeField=1;
        }
        if (e.type == SDL_KEYDOWN) {
            if (e.key.keysym.sym == SDLK_TAB) activeField ^= 1;
            if (e.key.keysym.sym == SDLK_BACKSPACE) {
                auto& f = (activeField==0?inputUser:inputPass);
                if (!f.empty()) f.pop_back();
            }
            if (e.key.keysym.sym == SDLK_ESCAPE) { mode=Mode::Select; statusMsg=""; }
            if (e.key.keysym.sym == SDLK_RETURN) {
                if (inputUser.empty()||inputPass.empty()) {
                    statusMsg="Please fill in all fields.";
                } else {
                    app->accountMode = (mode==Mode::SignIn) ? "signin" : "signup";
                    app->userName = inputUser;
                    // Save account data
                    std::ofstream accdat("mtv_root/accdat/account.json");
                    accdat << "{\"user\":\"" << inputUser
                           << "\",\"mode\":\"" << app->accountMode << "\"}";
                    accdat.close();
                    exiting=true; nextState=AppState::SaveData;
                }
            }
        }
        if (e.type == SDL_TEXTINPUT) {
            auto& f = (activeField==0?inputUser:inputPass);
            if (f.size() < 32) f += e.text.text;
        }
    }
}

void AccountScreen::update(float dt) {
    pulseTime += dt;
    if (!exiting) { fadeIn = std::min(1.f, fadeIn + dt*2.0f); }
    else { fadeOut += dt*2.0f; if (fadeOut>=1.f) done=true; }
}

void AccountScreen::drawCard(float x, float y, float w, float h, bool hov) {
    float a = fadeIn;
    float fa = hov ? 0.30f : 0.14f;
    app->tr.drawRectRounded(x+3,y+3,w,h,20.f,0.f,0.f,0.f,a*0.3f);
    app->tr.drawRectRounded(x,y,w,h,20.f,0.1f,0.4f,0.9f,a*fa);
    if (hov) {
        float pulse = 0.6f+0.4f*sinf(pulseTime*3.f);
        app->tr.drawRing(x+w*0.5f,y+h*0.5f,
                         std::max(w,h)*0.5f+4.f,2.f,64,
                         0.3f,0.7f,1.f,a*pulse*0.5f);
    }
}

void AccountScreen::drawInput(const std::string& label, const std::string& val,
                              float x, float y, float w, bool active) {
    float a = fadeIn;
    auto& tr = app->tr;
    float h = 52.f;
    tr.drawRectRounded(x,y,w,h,10.f, active?0.1f:0.05f, active?0.4f:0.2f, active?0.85f:0.5f, a*0.3f);
    if (active) tr.drawRing(x+w*0.5f,y+h*0.5f,std::max(w,h)*0.5f+2.f,2.f,64,0.3f,0.7f,1.f,a*0.6f);
    tr.drawText(label, "small", x, y-24.f, 0.7f,0.85f,1.f, a*0.7f);
    std::string show = val;
    if (label.find("Pass") != std::string::npos) show = std::string(val.size(),'*');
    tr.drawText(show.empty()?"_":show, "body", x+14.f, y+10.f, 1.f,1.f,1.f, a);
}

void AccountScreen::render() {
    float alpha = exiting ? (1.f-fadeOut) : fadeIn;
    alpha = clamp01(alpha);
    auto& tr = app->tr;
    float W=(float)app->screenW, H=(float)app->screenH;

    // Background
    tr.drawGradientRect(0,0,W,H, 0.02f,0.04f,0.18f,1.f, 0.04f,0.10f,0.35f,1.f);

    // Subtle glow
    float pulse = 0.5f+0.5f*sinf(pulseTime*1.2f);
    for (int i=4;i>=1;--i)
        tr.drawCircle(W*0.5f,H*0.38f,(float)i*90.f+pulse*30.f,64,0.1f,0.5f,1.f,alpha*0.03f*(5.f-i));

    tr.drawTextCentered("MahirTV",     "titleBold",  W*0.5f, H*0.12f, 0.3f,0.75f,1.f, alpha);
    tr.drawTextCentered("Your Account","headingBold", W*0.5f, H*0.22f, 0.8f,0.9f,1.f, alpha*0.85f);

    if (mode == Mode::Select) renderSelectMode();
    else if (mode == Mode::SignIn) renderSignInMode();
    else renderSignUpMode();

    // Fade overlay
    if (exiting) tr.drawRect(0,0,W,H,0.f,0.f,0.f,clamp01(fadeOut));
    else if (fadeIn<1.f) tr.drawRect(0,0,W,H,0.f,0.f,0.f,1.f-fadeIn);
}

void AccountScreen::renderSelectMode() {
    auto& tr = app->tr;
    float W=(float)app->screenW, H=(float)app->screenH;
    float cw=340, ch=210, gap=50;
    float startX = W*0.5f-(3*cw+2*gap)*0.5f;
    float cy2=H*0.52f;

    const char* titles[] = {"Sign In","Sign Up","Continue as Guest"};
    const char* subs[]   = {"Mahir Account","Create Account","No Account Needed"};
    const char* icons[]  = {"[->]","[+]","[*]"};

    for (int i=0;i<3;++i) {
        float cx3=startX+i*(cw+gap);
        bool hov=(hovered==i);
        drawCard(cx3,cy2,cw,ch,hov);
        tr.drawTextCentered(icons[i],  "heading",      cx3+cw*0.5f, cy2+ch*0.28f, 0.4f,0.8f,1.f,fadeIn);
        tr.drawTextCentered(titles[i], "headingBold",  cx3+cw*0.5f, cy2+ch*0.53f, 1.f,1.f,1.f,fadeIn);
        tr.drawTextCentered(subs[i],   "small",        cx3+cw*0.5f, cy2+ch*0.75f, 0.6f,0.8f,1.f,fadeIn*0.7f);
    }
    tr.drawTextCentered("Select how you want to use MahirTV","small",
                        W*0.5f,H*0.87f,0.5f,0.7f,0.9f,fadeIn*0.5f);
}

void AccountScreen::renderSignInMode() {
    auto& tr = app->tr;
    float W=(float)app->screenW, H=(float)app->screenH;
    float fw=440, fx=W*0.5f-fw*0.5f;
    tr.drawTextCentered("Sign In to Mahir Account","heading",W*0.5f,H*0.33f,1.f,1.f,1.f,fadeIn);
    drawInput("Username", inputUser, fx, H*0.44f, fw, activeField==0);
    drawInput("Password", inputPass, fx, H*0.54f, fw, activeField==1);
    // Login button
    float bw=220,bh=54,bx=W*0.5f-bw*0.5f,by=H*0.66f;
    tr.drawRectRounded(bx,by,bw,bh,14.f,0.15f,0.5f,1.f,fadeIn*0.8f);
    tr.drawTextCentered("Sign In","headingBold",W*0.5f,by+bh*0.5f,1.f,1.f,1.f,fadeIn);
    // Back
    tr.drawTextCentered("ESC to go back","small",W*0.5f,H*0.76f,0.5f,0.7f,0.9f,fadeIn*0.5f);
    if (!statusMsg.empty())
        tr.drawTextCentered(statusMsg,"small",W*0.5f,H*0.72f,1.f,0.4f,0.4f,fadeIn);
}

void AccountScreen::renderSignUpMode() {
    auto& tr = app->tr;
    float W=(float)app->screenW, H=(float)app->screenH;
    float fw=440, fx=W*0.5f-fw*0.5f;
    tr.drawTextCentered("Create Mahir Account","heading",W*0.5f,H*0.33f,1.f,1.f,1.f,fadeIn);
    drawInput("Username", inputUser, fx, H*0.44f, fw, activeField==0);
    drawInput("Password (new)", inputPass, fx, H*0.54f, fw, activeField==1);
    float bw=240,bh=54,bx=W*0.5f-bw*0.5f,by=H*0.66f;
    tr.drawRectRounded(bx,by,bw,bh,14.f,0.15f,0.5f,1.f,fadeIn*0.8f);
    tr.drawTextCentered("Create Account","headingBold",W*0.5f,by+bh*0.5f,1.f,1.f,1.f,fadeIn);
    tr.drawTextCentered("ESC to go back","small",W*0.5f,H*0.76f,0.5f,0.7f,0.9f,fadeIn*0.5f);
    if (!statusMsg.empty())
        tr.drawTextCentered(statusMsg,"small",W*0.5f,H*0.72f,1.f,0.4f,0.4f,fadeIn);
}
