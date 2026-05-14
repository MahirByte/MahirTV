#include "HomeScreen.h"
#include <cmath>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <cstring>

void HomeScreen::enter() {
    fadeIn=0.f; time=0.f; pulseTime=0.f;
    done=false; overlayOpen=false; overlayApp=-1; overlayAnim=0.f;
    app->loadImportedApps();
    initApps();
}

std::string HomeScreen::fontFor(const std::string& key) {
    return (app->selectedLanguage=="Bangla") ? "bangla_body" : key;
}

void HomeScreen::initApps() {
    apps.clear();
    float W=(float)app->screenW, H=(float)app->screenH;

    // Fixed built-in apps
    struct Info { const char* name; const char* icon; float r,g,b;
                  bool isSettings; bool isStore; };
    Info builtin[] = {
        {"YouTube",     "YT",  0.85f,0.05f,0.05f, false, false},
        {"Games",       "GAM", 0.20f,0.55f,0.95f, false, false},
        {"Settings",    "SET", 0.25f,0.30f,0.65f, true,  false},
        {"Store",       "STR", 0.80f,0.45f,0.05f, false, true },
        {"Browser",     "WEB", 0.15f,0.60f,0.40f, false, false},
        {"Music",       "MUS", 0.65f,0.15f,0.80f, false, false},
        {"Import App",  "[+]", 0.25f,0.65f,0.35f, false, false},
    };

    // Collect all: built-in + imported
    int total = 7 + (int)app->importedApps.size();
    int cols  = std::min(total, 4);
    float iconW=170.f, iconH=170.f, gapX=32.f, gapY=32.f;
    float gridW=(float)cols*iconW+(float)(cols-1)*gapX;
    float startX=W*0.5f-gridW*0.5f;
    float startY=H*0.50f-iconH;

    // Adjust cols if many apps
    if (total > 8) cols=5;
    gridW=(float)cols*iconW+(float)(cols-1)*gapX;
    startX=W*0.5f-gridW*0.5f;

    for (int i=0;i<7;++i) {
        int col=i%cols, row=i/cols;
        AppIcon ic;
        ic.name=builtin[i].name; ic.icon=builtin[i].icon;
        ic.r=builtin[i].r; ic.g=builtin[i].g; ic.b=builtin[i].b;
        ic.x=startX+(float)col*(iconW+gapX);
        ic.y=startY+(float)row*(iconH+gapY);
        ic.w=iconW; ic.h=iconH;
        ic.hoverScale=1.f; ic.hovered=false;
        ic.isImported=false; ic.htmlPath="";
        ic.isImportBtn = (std::string(builtin[i].icon)=="[+]");
        ic.isSettings  = builtin[i].isSettings;
        ic.isStore     = builtin[i].isStore;
        apps.push_back(ic);
    }
    // Imported apps
    for (int i=0;i<(int)app->importedApps.size();++i) {
        int gi=7+i;
        int col=gi%cols, row=gi/cols;
        auto& ia=app->importedApps[i];
        AppIcon ic;
        ic.name=ia.name;
        ic.icon=ia.iconText;
        ic.r=ia.r; ic.g=ia.g; ic.b=ia.b;
        ic.x=startX+(float)col*(iconW+gapX);
        ic.y=startY+(float)row*(iconH+gapY);
        ic.w=iconW; ic.h=iconH;
        ic.hoverScale=1.f; ic.hovered=false;
        ic.isImported=true; ic.htmlPath=ia.htmlPath;
        ic.isImportBtn=false; ic.isSettings=false; ic.isStore=false;
        apps.push_back(ic);
    }
    if (focusedApp>=(int)apps.size()) focusedApp=0;
}

std::string HomeScreen::getTimeStr() {
    time_t t=::time(nullptr); struct tm* tm2=localtime(&t);
    std::ostringstream ss;
    ss<<std::setfill('0')<<std::setw(2)<<tm2->tm_hour<<":"<<std::setw(2)<<tm2->tm_min;
    return ss.str();
}
std::string HomeScreen::getDateStr() {
    time_t t=::time(nullptr); struct tm* tm2=localtime(&t);
    const char* days[]={"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
    const char* months[]={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
    std::ostringstream ss;
    ss<<days[tm2->tm_wday]<<", "<<months[tm2->tm_mon]<<" "<<tm2->tm_mday<<" "<<(1900+tm2->tm_year);
    return ss.str();
}

void HomeScreen::doImport() {
    // Open file picker via zenity (blocks briefly — dialog is native)
    FILE* fp = popen("zenity --file-selection "
                     "--title='Import MahirTV App' "
                     "--file-filter='MahirTV App (*.mta)|*.mta' "
                     "2>/dev/null", "r");
    if (!fp) {
        // Try kdialog as fallback
        fp = popen("kdialog --getopenfilename . '*.mta' 2>/dev/null", "r");
    }
    if (!fp) return;
    char path[4096]={};
    fgets(path, sizeof(path), fp);
    pclose(fp);
    path[strcspn(path,"\n")] = 0;
    if (strlen(path) > 0) {
        app->importMtaApp(path);
        initApps(); // refresh grid
    }
}

void HomeScreen::launchApp(int idx) {
    if (idx<0||idx>=(int)apps.size()) return;
    auto& ic=apps[idx];

    if (ic.isSettings) { done=true; nextState=AppState::Settings; return; }
    if (ic.isStore)    { done=true; nextState=AppState::Store;    return; }
    if (ic.isImportBtn){ doImport(); return; }

    overlayOpen=true; overlayApp=idx; overlayAnim=0.f; focusedApp=idx;

    // Resolve the webview script path relative to the binary
    std::string script = std::string(ASSETS_PATH) + "mahirtv_webview.py";

    auto launchWeb = [&](const std::string& url, const std::string& label) {
        // Drop to windowed so the WebKit window can sit on top / maximise
        SDL_SetWindowFullscreen(app->window, 0);
        SDL_MinimizeWindow(app->window);
        SDL_GL_MakeCurrent(app->window, nullptr);

        std::string cmd = "python3 '" + script + "' '" + url + "' '" + label + "'";
        system(cmd.c_str());   // blocks until user closes the browser

        // Restore MahirTV window
        SDL_GL_MakeCurrent(app->window, app->glCtx);
        SDL_RestoreWindow(app->window);
        if (app->isFullscreen)
            SDL_SetWindowFullscreen(app->window, SDL_WINDOW_FULLSCREEN_DESKTOP);
        else
            SDL_RaiseWindow(app->window);
        glViewport(0, 0, app->screenW, app->screenH);

        // Close overlay so the home screen is clean on return
        overlayOpen = false; overlayApp = -1;
    };

    if (ic.name=="YouTube")
        launchWeb("https://www.youtube.com", "YouTube");
    else if (ic.name=="Browser")
        launchWeb("https://www.google.com", "Browser");
    else if (ic.name=="Music")
        launchWeb("https://music.youtube.com", "Music");
    else if (ic.isImported && !ic.htmlPath.empty()) {
        // Local .mta HTML — open through the embedded browser too
        launchWeb("file://" + ic.htmlPath, ic.name);
    } else if (ic.name=="Games") {
        // Launch first installed store game if available
        std::string html = "mtv_root/applications/Snake/app.html";
        launchWeb("file://" + html, "Games");
    }
}

void HomeScreen::handleEvent(const SDL_Event& e) {
    if (importing) return;
    if (overlayOpen) {
        if (e.type==SDL_KEYDOWN&&e.key.keysym.sym==SDLK_ESCAPE) { overlayOpen=false; overlayApp=-1; }
        if (e.type==SDL_MOUSEBUTTONDOWN&&e.button.button==SDL_BUTTON_RIGHT) { overlayOpen=false; overlayApp=-1; }
        return;
    }
    if (e.type==SDL_MOUSEMOTION) {
        float mx=(float)e.motion.x, my=(float)e.motion.y;
        for (auto& ic : apps) {
            float s=ic.hoverScale;
            float dx=(ic.w*s-ic.w)*0.5f, dy=(ic.h*s-ic.h)*0.5f;
            ic.hovered=(mx>=ic.x-dx&&mx<=ic.x+ic.w+dx&&my>=ic.y-dy&&my<=ic.y+ic.h+dy);
        }
    }
    if (e.type==SDL_MOUSEBUTTONDOWN&&e.button.button==SDL_BUTTON_LEFT) {
        for (int i=0;i<(int)apps.size();++i)
            if (apps[i].hovered) { launchApp(i); break; }
    }
    if (e.type==SDL_KEYDOWN) {
        int& f=focusedApp;
        int cols=std::min((int)apps.size(),5);
        switch(e.key.keysym.sym) {
            case SDLK_RIGHT: f=(f+1)%(int)apps.size(); break;
            case SDLK_LEFT:  f=(f-1+(int)apps.size())%(int)apps.size(); break;
            case SDLK_DOWN:  f=std::min((int)apps.size()-1,f+cols); break;
            case SDLK_UP:    f=std::max(0,f-cols); break;
            case SDLK_RETURN: launchApp(f); break;
            case SDLK_ESCAPE: app->quit(); break;
        }
    }
}

void HomeScreen::update(float dt) {
    time+=dt; pulseTime+=dt;
    fadeIn=std::min(1.f,fadeIn+dt*1.8f);
    for (int i=0;i<(int)apps.size();++i) {
        auto& ic=apps[i];
        bool focused=(i==focusedApp&&!overlayOpen);
        float target=(ic.hovered||focused)?1.12f:1.f;
        ic.hoverScale+=(target-ic.hoverScale)*dt*8.f;
    }
    if (overlayOpen) overlayAnim=std::min(1.f,overlayAnim+dt*4.f);
}

void HomeScreen::renderBackground() {
    float W=(float)app->screenW, H=(float)app->screenH;
    auto& tr=app->tr;
    tr.drawGradientRect(0,0,W,H,0.01f,0.03f,0.12f,1.f,0.05f,0.12f,0.38f,1.f);
    float px=W*0.3f+sinf(time*0.2f)*W*0.08f;
    float py=H*0.25f+cosf(time*0.15f)*H*0.05f;
    tr.drawCircle(px,py,350.f,64,0.08f,0.22f,0.65f,0.07f);
    tr.drawCircle(W-px,H-py,260.f,64,0.05f,0.16f,0.50f,0.05f);
}

void HomeScreen::renderTopBar() {
    float W=(float)app->screenW;
    auto& tr=app->tr;
    float alpha=fadeIn;
    tr.drawRect(0,0,W,72.f,0.f,0.f,0.f,alpha*0.45f);
    tr.drawRect(0,71.f,W,1.f,0.2f,0.6f,1.f,alpha*0.25f);
    tr.drawText("MahirTV","titleBold",24.f,10.f,0.3f,0.75f,1.f,alpha);
    std::string uname=app->userName.empty()?"Guest":app->userName;
    std::string hi = app->L("Hi, ") + uname;
    tr.drawText(hi,"small",290.f,24.f,0.7f,0.85f,1.f,alpha*0.75f);

    std::string ts=getTimeStr();
    int tw,th; tr.getTextSize(ts,"heading",tw,th);
    tr.drawText(ts,"heading",W-(float)tw-24.f,8.f,1.f,1.f,1.f,alpha);
    std::string ds=getDateStr();
    int dw,dh; tr.getTextSize(ds,"small",dw,dh);
    tr.drawText(ds,"small",W-(float)dw-24.f,56.f,0.6f,0.8f,1.f,alpha*0.6f);
    (void)th; (void)dh;
}

void HomeScreen::renderFeaturedBanner() {
    float W=(float)app->screenW, H=(float)app->screenH;
    auto& tr=app->tr;
    float alpha=fadeIn;
    float bx=80.f, by=90.f, bw=360.f, bh=160.f;
    tr.drawRectRounded(bx,by,bw,bh,14.f,0.08f,0.2f,0.5f,alpha*0.35f);
    std::string feat=app->L("Featured");
    std::string welc=app->L("Welcome to");
    tr.drawText(feat, fontFor("small"),   bx+18.f,by+12.f,0.4f,0.8f,1.f,alpha*0.65f);
    tr.drawText(welc, fontFor("heading"), bx+18.f,by+38.f,0.85f,0.95f,1.f,alpha);
    tr.drawText("MahirTV","titleBold",    bx+18.f,by+82.f,0.3f,0.75f,1.f,alpha);
    float scanY=by+fmodf(time*50.f,bh);
    tr.drawRect(bx,scanY,bw,1.5f,0.3f,0.7f,1.f,alpha*0.12f);
}

void HomeScreen::renderAppGrid() {
    auto& tr=app->tr;
    float alpha=fadeIn;
    if (apps.empty()) return;

    float lblX=apps[0].x, lblY=apps[0].y-44.f;
    tr.drawText(app->L("Apps"), fontFor("headingBold"),lblX,lblY,0.7f,0.85f,1.f,alpha*0.8f);

    for (int i=0;i<(int)apps.size();++i) {
        auto& ic=apps[i];
        float s=ic.hoverScale;
        float iw=ic.w*s, ih=ic.h*s;
        float ix=ic.x+(ic.w-iw)*0.5f, iy=ic.y+(ic.h-ih)*0.5f;
        bool focused=(i==focusedApp);

        // Shadow
        tr.drawRectRounded(ix+5.f,iy+8.f,iw,ih,18.f,0.f,0.f,0.f,alpha*0.22f);

        // Card fill
        float cardA=(ic.hovered||focused)?0.34f:0.17f;
        // Import button special style
        if (ic.isImportBtn) {
            tr.drawRectRounded(ix,iy,iw,ih,18.f,0.f,0.f,0.f,alpha*0.1f);
            // Dashed border effect (just draw ring)
            tr.drawRing(ix+iw*0.5f,iy+ih*0.5f,std::min(iw,ih)*0.45f,2.f,48,
                        0.3f,0.8f,0.4f,alpha*0.7f);
        } else {
            tr.drawRectRounded(ix,iy,iw,ih,18.f,ic.r,ic.g,ic.b,alpha*cardA);
            tr.drawGradientRect(ix,iy,iw,ih*0.45f,1.f,1.f,1.f,alpha*0.06f,1.f,1.f,1.f,0.f);
        }

        // Focus ring
        if (ic.hovered||focused) {
            float pulse=0.6f+0.4f*sinf(pulseTime*3.f);
            tr.drawRing(ix+iw*0.5f,iy+ih*0.5f,std::min(iw,ih)*0.5f+4.f,2.5f,64,
                        ic.r*0.4f+0.6f,ic.g*0.4f+0.6f,ic.b*0.4f+0.6f,alpha*pulse*0.7f);
        }

        // Icon
        tr.drawTextCentered(ic.icon,"headingBold",ix+iw*0.5f,iy+ih*0.38f,1.f,1.f,1.f,alpha);
        // Name
        tr.drawTextCentered(ic.name,fontFor("small"),ix+iw*0.5f,iy+ih*0.76f,
                            0.85f,0.92f,1.f,alpha*0.9f);

        // "Imported" badge
        if (ic.isImported)
            tr.drawTextCentered("custom","small",ix+iw*0.5f,iy+ih*0.90f,0.4f,0.8f,0.5f,alpha*0.6f);
    }
}

void HomeScreen::renderOverlay() {
    if (!overlayOpen||overlayApp<0||(overlayApp>=(int)apps.size())) return;
    auto& tr=app->tr;
    float W=(float)app->screenW, H=(float)app->screenH;
    auto& ic=apps[overlayApp];
    float a=easeOut(overlayAnim)*fadeIn;

    tr.drawRect(0,0,W,H,0.f,0.f,0.f,a*0.75f);
    float pw=600.f, ph=360.f, px2=W*0.5f-pw*0.5f, py2=H*0.5f-ph*0.5f;
    tr.drawRectRounded(px2,py2,pw,ph,22.f,ic.r,ic.g,ic.b,a*0.22f);
    tr.drawRectRounded(px2,py2,pw,ph,22.f,0.05f,0.12f,0.35f,a*0.55f);
    tr.drawTextCentered(ic.icon,"titleBold",W*0.5f,py2+ph*0.26f,
                        ic.r*0.3f+0.7f,ic.g*0.3f+0.7f,ic.b*0.3f+0.7f,a);
    tr.drawTextCentered(ic.name,"titleBold",W*0.5f,py2+ph*0.52f,1.f,1.f,1.f,a);

    std::string sub;
    if (ic.name=="YouTube"||ic.name=="Browser"||ic.name=="Music")
        sub="Opening in system browser...";
    else if (ic.isImported)
        sub="Running "+ic.name+" in browser...";
    else if (ic.isImportBtn)
        sub="";
    else
        sub="Launching "+ic.name+"...";
    tr.drawTextCentered(sub,"body",W*0.5f,py2+ph*0.70f,0.7f,0.85f,1.f,a*0.8f);
    tr.drawTextCentered("ESC or Right-click to close","small",W*0.5f,py2+ph*0.87f,
                        0.5f,0.7f,0.9f,a*0.45f);
}

void HomeScreen::render() {
    float W=(float)app->screenW, H=(float)app->screenH;
    renderBackground();
    renderTopBar();
    renderFeaturedBanner();
    renderAppGrid();
    renderOverlay();
    if (fadeIn<1.f) app->tr.drawRect(0,0,W,H,0.f,0.f,0.f,1.f-fadeIn);
}
