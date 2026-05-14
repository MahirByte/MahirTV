#pragma once
#include "../App.h"
#include <string>
#include <vector>
#include <functional>

class HomeScreen : public Screen {
public:
    explicit HomeScreen(App* app) : app(app) {}
    void enter()  override;
    void exit()   override {}
    void handleEvent(const SDL_Event& e) override;
    void update(float dt) override;
    void render() override;

private:
    App*  app;
    float fadeIn    = 0.f;
    float time      = 0.f;
    float pulseTime = 0.f;

    struct AppIcon {
        std::string name;
        std::string icon;
        float r, g, b;
        float x, y, w, h;
        float hoverScale = 1.f;
        bool  hovered    = false;
        bool  isImported = false;
        std::string htmlPath;
        bool  isImportBtn = false;
        bool  isSettings  = false;
        bool  isStore     = false;
    };
    std::vector<AppIcon> apps;

    bool    overlayOpen  = false;
    int     overlayApp   = -1;
    float   overlayAnim  = 0.f;
    int     focusedApp   = 0;
    bool    importing    = false;

    std::string getTimeStr();
    std::string getDateStr();
    std::string fontFor(const std::string& key);

    void initApps();
    void renderBackground();
    void renderTopBar();
    void renderAppGrid();
    void renderOverlay();
    void renderFeaturedBanner();
    void launchApp(int idx);
    void doImport();
};
