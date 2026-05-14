#pragma once
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include "TextRenderer.h"

enum class AppState {
    Language,
    Account,
    SaveData,
    Loading,
    Home,
    Settings,
    Store,
    Quit
};

class Screen {
public:
    virtual ~Screen() = default;
    virtual void enter() {}
    virtual void exit()  {}
    virtual void handleEvent(const SDL_Event& e) = 0;
    virtual void update(float dt) = 0;
    virtual void render() = 0;
    AppState nextState = AppState::Home;
    bool done = false;
};

struct ImportedApp {
    std::string name;
    std::string description;
    std::string appDir;
    std::string htmlPath;
    std::string iconText;
    float r = 0.2f, g = 0.5f, b = 0.9f;
};

class App {
public:
    bool init();
    void run();
    void quit();
    ~App();

    SDL_Window*   window  = nullptr;
    SDL_GLContext glCtx   = {};
    TextRenderer  tr;
    int           screenW = 1920;
    int           screenH = 1080;

    // Shared state
    std::string selectedLanguage = "English";
    std::string accountMode      = "guest";
    int         selectedSaveSlot = 0;
    std::string userName         = "";

    // Settings
    bool  isFullscreen  = true;
    int   masterVolume  = 80;
    float cursorScale   = 0.32f;   // kept for settings UI only
    SDL_Cursor* sysCursor = nullptr;

    int    mouseX     = 0, mouseY  = 0;

    float totalTime = 0.f;

    // Imported / installed apps
    std::vector<ImportedApp> importedApps;
    void loadImportedApps();
    void importMtaApp(const std::string& mtaPath);
    void saveSettings();
    void loadSettings();
    void saveSession();
    bool loadSession();           // returns true if a valid session was found
    void deleteSlotData(int slot); // wipe mtv_root/saves/slot_N/

    // Localization
    std::string L(const std::string& key) const;


private:
    bool      running = false;
    AppState  currentState = AppState::Language;
    std::unique_ptr<Screen> currentScreen;

    std::unordered_map<std::string, std::string> banglaStrings;
    void initLocale();
    void switchState(AppState s);
    std::unique_ptr<Screen> makeScreen(AppState s);
};
