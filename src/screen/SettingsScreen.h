#pragma once
#include "../App.h"
#include <string>
#include <vector>

class SettingsScreen : public Screen {
public:
    explicit SettingsScreen(App* app) : app(app) {}
    void enter()  override;
    void exit()   override {}
    void handleEvent(const SDL_Event& e) override;
    void update(float dt) override;
    void render() override;

private:
    App*  app;
    float fadeIn  = 0.f;
    float fadeOut = 0.f;
    bool  exiting = false;
    float time    = 0.f;

    int   selectedCategory = 0;
    int   hoveredItem      = -1;

    struct Category { std::string name; std::string icon; };
    std::vector<Category> categories = {
        {"Display",    "[D]"},
        {"Language",   "[L]"},
        {"Account",    "[A]"},
        {"Audio",      "[S]"},
        {"About",      "[?]"},
        {"Danger Zone","[!]"},
    };

    // Temp values (applied on save)
    bool  tmpFullscreen   = true;
    int   tmpVolume       = 80;
    float tmpCursorScale  = 0.32f;
    std::string tmpLang   = "English";

    void renderSidebar();
    void renderContent();
    void renderDisplay();
    void renderLanguage();
    void renderAccount();
    void renderAudio();
    void renderAbout();
    void renderDangerZone();
    void applySettings();
};
