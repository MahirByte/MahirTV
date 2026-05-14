#pragma once
#include "../App.h"
#include <string>
#include <vector>

class StoreScreen : public Screen {
public:
    explicit StoreScreen(App* app) : app(app) {}
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
    float time     = 0.f;
    int   hovered  = -1;
    int   scroll   = 0;

    std::string searchText;
    int selectedCategory = 0;
    std::vector<std::string> categories = {"All","Games","Tools","Media"};

    struct StoreApp {
        std::string name;
        std::string description;
        std::string category;
        std::string iconText;
        float r, g, b;
        bool  installed = false;
        const char* htmlContent;
    };
    std::vector<StoreApp> allApps;
    std::vector<int>      filteredIdx;

    void initApps();
    void filterApps();
    void installApp(int idx);
    void renderTopBar();
    void renderCategories();
    void renderGrid();
    void renderSearchBar();
};
