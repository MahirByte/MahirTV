#pragma once
#include "../App.h"
#include <string>

class AccountScreen : public Screen {
public:
    explicit AccountScreen(App* app) : app(app) {}
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
    int   hovered = -1;
    float pulseTime = 0.f;

    enum class Mode { Select, SignIn, SignUp } mode = Mode::Select;

    // Form fields
    std::string inputUser = "";
    std::string inputPass = "";
    int         activeField = 0;   // 0=user, 1=pass
    std::string statusMsg   = "";

    void renderSelectMode();
    void renderSignInMode();
    void renderSignUpMode();
    void drawCard(float x, float y, float w, float h, bool hov);
    void drawInput(const std::string& label, const std::string& val,
                   float x, float y, float w, bool active);
};
