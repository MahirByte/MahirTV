#include "App.h"
#include "screens/LanguageScreen.h"
#include "screens/AccountScreen.h"
#include "screens/SaveDataScreen.h"
#include "screens/LoadingScreen.h"
#include "screens/HomeScreen.h"
#include "screens/SettingsScreen.h"
#include "screens/StoreScreen.h"
#include <SDL2/SDL_image.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <cstring>
#include <cstdio>

namespace fs = std::filesystem;

#ifndef ASSETS_PATH
#define ASSETS_PATH "./assets/"
#endif

// ── Locale ───────────────────────────────────────────────────────────────────
void App::initLocale() {
    banglaStrings = {
        // LanguageScreen
        {"Select Language",         u8"ভাষা নির্বাচন করুন"},
        {"Use arrow keys or click to select", u8"তীর কী বা ক্লিক করুন"},
        // AccountScreen
        {"Your Account",            u8"আপনার অ্যাকাউন্ট"},
        {"Sign In",                 u8"সাইন ইন"},
        {"Sign Up",                 u8"নিবন্ধন"},
        {"Continue as Guest",       u8"অতিথি হিসেবে যান"},
        {"Mahir Account",           u8"মাহির অ্যাকাউন্ট"},
        {"Create Account",          u8"অ্যাকাউন্ট তৈরি"},
        {"No Account Needed",       u8"অ্যাকাউন্ট লাগবে না"},
        {"Sign In to Mahir Account",u8"মাহির অ্যাকাউন্টে সাইন ইন"},
        {"Create Mahir Account",    u8"মাহির অ্যাকাউন্ট তৈরি"},
        {"Username",                u8"ব্যবহারকারীর নাম"},
        {"Password",                u8"পাসওয়ার্ড"},
        {"ESC to go back",          u8"ESC ফিরে যান"},
        // SaveData
        {"Select Save Data",        u8"সেভ ডেটা নির্বাচন"},
        {"Draw a line from the center-bottom to a circle",
                                    u8"নীচ থেকে বৃত্তে লাইন টানুন"},
        {"Save Data 1",             u8"সেভ ডেটা ১"},
        {"Save Data 2",             u8"সেভ ডেটা ২"},
        {"Save Data 3",             u8"সেভ ডেটা ৩"},
        {"CONNECTED",               u8"সংযুক্ত"},
        // Loading
        {"Loading...",              u8"লোড হচ্ছে..."},
        // Home
        {"Apps",                    u8"অ্যাপসমূহ"},
        {"Featured",                u8"বৈশিষ্ট্যযুক্ত"},
        {"Welcome to",              u8"স্বাগতম"},
        {"Import App",              u8"অ্যাপ আমদানি"},
        {"Hi, ",                    u8"হ্যালো, "},
        // Settings
        {"Settings",                u8"সেটিংস"},
        {"Display",                 u8"ডিসপ্লে"},
        {"Language",                u8"ভাষা"},
        {"Account",                 u8"অ্যাকাউন্ট"},
        {"Audio",                   u8"অডিও"},
        {"About",                   u8"সম্পর্কে"},
        // Store
        {"Store",                   u8"স্টোর"},
        {"Install",                 u8"ইনস্টল"},
        {"Installed",               u8"ইনস্টল হয়েছে"},
    };
}

std::string App::L(const std::string& key) const {
    if (selectedLanguage == "Bangla") {
        auto it = banglaStrings.find(key);
        if (it != banglaStrings.end()) return it->second;
    }
    return key;
}

// ── Font loading ──────────────────────────────────────────────────────────────
static std::string findFont(const std::vector<std::string>& paths) {
    for (auto& p : paths) if (fs::exists(p)) return p;
    return "";
}

// ── init ──────────────────────────────────────────────────────────────────────
bool App::init() {
    initLocale();

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
        std::cerr << "SDL_Init: " << SDL_GetError() << "\n"; return false;
    }
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        std::cerr << "IMG_Init: " << IMG_GetError() << "\n"; return false;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

    SDL_DisplayMode dm;
    SDL_GetCurrentDisplayMode(0, &dm);
    screenW = dm.w; screenH = dm.h;

    window = SDL_CreateWindow("MahirTV",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        screenW, screenH,
        SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN_DESKTOP);
    if (!window) { std::cerr << "SDL_CreateWindow: " << SDL_GetError() << "\n"; return false; }

    glCtx = SDL_GL_CreateContext(window);
    if (!glCtx) { std::cerr << "SDL_GL_CreateContext: " << SDL_GetError() << "\n"; return false; }
    SDL_GL_MakeCurrent(window, glCtx);
    SDL_GL_SetSwapInterval(1);

    glewExperimental = GL_TRUE;
    GLenum glewErr = glewInit();
    if (glewErr != GLEW_OK) {
        std::cerr << "GLEW: " << glewGetErrorString(glewErr) << "\n"; return false;
    }
    glGetError();

    glViewport(0, 0, screenW, screenH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_MULTISAMPLE);

    if (!tr.init()) { std::cerr << "TextRenderer::init failed\n"; return false; }

    std::string fp = std::string(ASSETS_PATH) + "fonts/";

    // Latin fonts (Amaranth)
    auto tryLatin = [&](int sz, const std::string& alias, bool bold=false) {
        std::string suf = bold ? "Bold" : "Regular";
        if (!tr.loadFont(fp+"Amaranth-"+suf+".ttf", sz, alias))
            tr.loadFont("/usr/share/fonts/truetype/dejavu/DejaVuSans"+(bold?std::string("-Bold"):std::string(""))+".ttf", sz, alias);
    };
    tryLatin(72,"title");
    tryLatin(48,"heading");
    tryLatin(32,"body");
    tryLatin(22,"small");
    tryLatin(56,"titleBold",true);
    tryLatin(36,"headingBold",true);

    // Bengali fonts — try Alkatra first, then system Noto/Lohit Bengali
    std::string bengaliFont = findFont({
        fp + "Alkatra-Regular.ttf",
        fp + "Alkatra-VariableFont_wght.ttf",
        "/usr/share/fonts/truetype/noto/NotoSansBengali-Regular.ttf",
        "/usr/share/fonts/opentype/noto/NotoSansBengali-Regular.otf",
        "/usr/share/fonts/noto/NotoSansBengali-Regular.ttf",
        "/usr/share/fonts/truetype/lohit-bengali/Lohit-Bengali.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"
    });
    if (!bengaliFont.empty()) {
        tr.loadFont(bengaliFont, 72, "bangla_title");
        tr.loadFont(bengaliFont, 40, "bangla_body");
        tr.loadFont(bengaliFont, 28, "bangla_small");
    } else {
        // last-resort: copy latin fonts
        tr.loadFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 72, "bangla_title");
        tr.loadFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 40, "bangla_body");
        tr.loadFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 28, "bangla_small");
        std::cerr << "WARNING: No Bengali font found. "
                     "Run: sudo apt install fonts-noto-core\n";
    }

    // Use the OS arrow cursor — click point always matches exactly
    sysCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
    if (sysCursor) SDL_SetCursor(sysCursor);
    SDL_ShowCursor(SDL_ENABLE);
    loadSettings();
    loadImportedApps();

    running = true;
    // If a valid session exists, skip setup and go straight to Home
    if (loadSession()) {
        switchState(AppState::Home);
    } else {
        switchState(AppState::Language);
    }
    return true;
}


void App::run() {
    Uint64 prev = SDL_GetPerformanceCounter();
    while (running) {
        Uint64 now = SDL_GetPerformanceCounter();
        float dt = (float)(now-prev)/(float)SDL_GetPerformanceFrequency();
        prev = now;
        if (dt > 0.1f) dt = 0.1f;
        totalTime += dt;

        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) { running=false; break; }
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_F11) {
                isFullscreen = !isFullscreen;
                SDL_SetWindowFullscreen(window, isFullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
                if (!isFullscreen) SDL_GetWindowSize(window, &screenW, &screenH);
                else { SDL_DisplayMode dm; SDL_GetCurrentDisplayMode(0,&dm); screenW=dm.w; screenH=dm.h; }
                glViewport(0,0,screenW,screenH);
            }
            if (e.type == SDL_MOUSEMOTION) { mouseX=e.motion.x; mouseY=e.motion.y; }
            if (currentScreen) currentScreen->handleEvent(e);
        }

        if (currentScreen) {
            currentScreen->update(dt);
            if (currentScreen->done) {
                AppState next = currentScreen->nextState;
                if (next == AppState::Quit) { running=false; break; }
                switchState(next);
            }
        }

        glClearColor(0,0,0,1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        tr.beginFrame(screenW, screenH);
        if (currentScreen) currentScreen->render();
        SDL_GL_SwapWindow(window);
    }
}

void App::switchState(AppState s) {
    if (currentScreen) currentScreen->exit();
    currentState  = s;
    currentScreen = makeScreen(s);
    if (currentScreen) currentScreen->enter();
}

std::unique_ptr<Screen> App::makeScreen(AppState s) {
    switch (s) {
        case AppState::Language: return std::make_unique<LanguageScreen>(this);
        case AppState::Account:  return std::make_unique<AccountScreen>(this);
        case AppState::SaveData: return std::make_unique<SaveDataScreen>(this);
        case AppState::Loading:  return std::make_unique<LoadingScreen>(this);
        case AppState::Home:     return std::make_unique<HomeScreen>(this);
        case AppState::Settings: return std::make_unique<SettingsScreen>(this);
        case AppState::Store:    return std::make_unique<StoreScreen>(this);
        default: return nullptr;
    }
}

// ── Import .mta ───────────────────────────────────────────────────────────────
void App::importMtaApp(const std::string& mtaPath) {
    if (mtaPath.empty()) return;

    // Extract to temp dir
    std::string tmpDir = "mtv_root/tmp/mta_" + std::to_string((long)SDL_GetTicks());
    fs::create_directories(tmpDir);
    std::string cmd = "unzip -o '" + mtaPath + "' -d '" + tmpDir + "' > /dev/null 2>&1";
    system(cmd.c_str());

    // Read app.json
    std::ifstream jf(tmpDir + "/app.json");
    if (!jf.is_open()) { std::cerr << "MTA: no app.json\n"; return; }
    std::string json((std::istreambuf_iterator<char>(jf)), std::istreambuf_iterator<char>());
    jf.close();

    // Minimal JSON field extraction
    auto getField = [&](const std::string& key) -> std::string {
        auto pos = json.find("\""+key+"\"");
        if (pos == std::string::npos) return "";
        pos = json.find(":", pos);
        if (pos == std::string::npos) return "";
        pos = json.find("\"", pos);
        if (pos == std::string::npos) return "";
        ++pos;
        auto end = json.find("\"", pos);
        return json.substr(pos, end-pos);
    };

    std::string name = getField("name");
    if (name.empty()) name = "Imported App";
    std::string desc = getField("description");
    std::string main = getField("main");
    if (main.empty()) main = "app.html";

    // Move to permanent location
    std::string appDir = "mtv_root/applications/" + name;
    fs::create_directories(appDir);
    // Copy all files
    for (auto& entry : fs::directory_iterator(tmpDir))
        fs::copy(entry.path(), appDir + "/" + entry.path().filename().string(),
                 fs::copy_options::overwrite_existing);
    fs::remove_all(tmpDir);

    // Mark as imported in app.json
    std::ofstream mf(appDir + "/app.json");
    mf << "{\"name\":\"" << name << "\",\"description\":\"" << desc
       << "\",\"main\":\"" << main << "\",\"imported\":true}";
    mf.close();

    // Add to list
    ImportedApp ia;
    ia.name     = name;
    ia.description = desc;
    ia.appDir   = appDir;
    ia.htmlPath = appDir + "/" + main;
    ia.iconText = name.substr(0, 3);
    ia.r=0.2f; ia.g=0.55f; ia.b=0.9f;
    importedApps.push_back(ia);
}

void App::loadImportedApps() {
    importedApps.clear();
    if (!fs::exists("mtv_root/applications")) return;
    for (auto& entry : fs::directory_iterator("mtv_root/applications")) {
        std::string ajPath = entry.path().string() + "/app.json";
        if (!fs::exists(ajPath)) continue;
        std::ifstream f(ajPath);
        std::string json((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
        if (json.find("\"imported\":true") == std::string::npos) continue;
        auto getF = [&](const std::string& key) -> std::string {
            auto pos = json.find("\""+key+"\"");
            if (pos==std::string::npos) return "";
            pos=json.find(":",pos); if(pos==std::string::npos) return "";
            pos=json.find("\"",pos); if(pos==std::string::npos) return "";
            ++pos; auto end=json.find("\"",pos);
            return json.substr(pos,end-pos);
        };
        ImportedApp ia;
        ia.name      = getF("name");
        ia.description = getF("description");
        ia.appDir    = entry.path().string();
        std::string main = getF("main"); if(main.empty()) main="app.html";
        ia.htmlPath  = entry.path().string() + "/" + main;
        ia.iconText  = ia.name.size()>=3 ? ia.name.substr(0,3) : ia.name;
        ia.r=0.2f; ia.g=0.55f; ia.b=0.9f;
        importedApps.push_back(ia);
    }
}

void App::saveSettings() {
    fs::create_directories("mtv_root/accdat");
    std::ofstream f("mtv_root/accdat/settings.json");
    f << "{\n"
      << "  \"language\":\""    << selectedLanguage << "\",\n"
      << "  \"fullscreen\":"    << (isFullscreen?"true":"false") << ",\n"
      << "  \"volume\":"        << masterVolume << ",\n"
      << "  \"cursorScale\":"   << cursorScale << "\n"
      << "}";
}

void App::saveSession() {
    fs::create_directories("mtv_root/accdat");
    // Create the slot directory so it's marked occupied
    std::string slotDir = "mtv_root/saves/slot_" + std::to_string(selectedSaveSlot);
    fs::create_directories(slotDir);
    std::ofstream sd(slotDir + "/data.json");
    sd << "{\"slot\":" << selectedSaveSlot
       << ",\"accountMode\":\"" << accountMode << "\""
       << ",\"userName\":\"" << userName << "\""
       << ",\"language\":\"" << selectedLanguage << "\""
       << ",\"active\":true}";
    sd.close();
    // Write session file
    std::ofstream f("mtv_root/accdat/session.json");
    f << "{\n"
      << "  \"setupComplete\":true,\n"
      << "  \"language\":\""        << selectedLanguage  << "\",\n"
      << "  \"accountMode\":\""     << accountMode       << "\",\n"
      << "  \"userName\":\""        << userName          << "\",\n"
      << "  \"selectedSaveSlot\":"  << selectedSaveSlot  << "\n"
      << "}";
}

bool App::loadSession() {
    std::ifstream f("mtv_root/accdat/session.json");
    if (!f.is_open()) return false;
    std::string json((std::istreambuf_iterator<char>(f)),
                      std::istreambuf_iterator<char>());
    if (json.find("\"setupComplete\":true") == std::string::npos) return false;

    auto getStr = [&](const std::string& k) -> std::string {
        auto p = json.find("\""+k+"\"");
        if (p == std::string::npos) return "";
        p = json.find(":", p); if (p == std::string::npos) return "";
        p = json.find("\"", p); if (p == std::string::npos) return "";
        ++p; auto e = json.find("\"", p);
        return json.substr(p, e-p);
    };
    auto getInt = [&](const std::string& k, int def) -> int {
        auto p = json.find("\""+k+"\"");
        if (p == std::string::npos) return def;
        p = json.find(":", p); if (p == std::string::npos) return def;
        ++p; while (p < json.size() && (json[p]==' '||json[p]=='\n')) ++p;
        try { return std::stoi(json.substr(p)); } catch(...) { return def; }
    };

    std::string lang = getStr("language");
    if (!lang.empty()) selectedLanguage = lang;
    std::string mode = getStr("accountMode");
    if (!mode.empty()) accountMode = mode;
    userName = getStr("userName");
    selectedSaveSlot = getInt("selectedSaveSlot", 0);
    return true;
}

void App::deleteSlotData(int slot) {
    std::string slotDir = "mtv_root/saves/slot_" + std::to_string(slot);
    fs::remove_all(slotDir);
    // Remove session so next boot goes through setup
    fs::remove("mtv_root/accdat/session.json");
}

void App::loadSettings() {
    std::ifstream f("mtv_root/accdat/settings.json");
    if (!f.is_open()) return;
    std::string json((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    auto getStr = [&](const std::string& k) {
        auto p=json.find("\""+k+"\""); if(p==std::string::npos) return std::string("");
        p=json.find("\"",json.find(":",p)); if(p==std::string::npos) return std::string("");
        ++p; auto e=json.find("\"",p); return json.substr(p,e-p);
    };
    auto getBool = [&](const std::string& k, bool def) {
        auto p=json.find("\""+k+"\""); if(p==std::string::npos) return def;
        p=json.find(":",p); if(p==std::string::npos) return def;
        return json.find("true",p) < json.find(",",p);
    };
    std::string lang = getStr("language");
    if (!lang.empty()) selectedLanguage = lang;
    isFullscreen = getBool("fullscreen", true);
}

void App::quit() { running = false; }

App::~App() {
    if (sysCursor) SDL_FreeCursor(sysCursor);
    tr.destroy();
    if (glCtx)  SDL_GL_DeleteContext(glCtx);
    if (window) SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
}
