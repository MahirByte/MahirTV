#pragma once
#include "../App.h"
#include <string>
#include <vector>
#include <filesystem>

class LoadingScreen : public Screen {
public:
    explicit LoadingScreen(App* app) : app(app) {}
    void enter()  override;
    void exit()   override {}
    void handleEvent(const SDL_Event& e) override {}
    void update(float dt) override;
    void render() override;

private:
    App*  app;
    float fadeIn  = 0.f;
    float fadeOut = 0.f;
    float progress = 0.f;
    float spinAngle = 0.f;
    float holdTime  = 0.f;
    bool  workDone  = false;
    bool  exiting   = false;

    std::vector<std::string> tasks;
    int taskIdx = 0;
    std::string currentTask;

    void doInstallWork();
    void createAccDat();
};
