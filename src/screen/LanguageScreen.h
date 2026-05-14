#pragma once
#include "../App.h"

class LanguageScreen : public Screen {
public:
    explicit LanguageScreen(App* app) : app(app) {}
    void enter()  override;
    void exit()   override {}
    void handleEvent(const SDL_Event& e) override;
    void update(float dt) override;
    void render() override;

private:
    App*  app;
    float fadeIn   = 0.f;
    float fadeOut  = 0.f;
    bool  exiting  = false;
    int   hovered  = -1;   // 0=English, 1=Bangla
    float pulseTime = 0.f;

    struct Button {
        float x, y, w, h;
        std::string label;
        std::string font;
        std::string lang;
    };
    Button buttons[2];
    void initButtons();
};
