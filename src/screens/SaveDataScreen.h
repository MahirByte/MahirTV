#pragma once
#include "../App.h"
#include "../Shader.h"
#include <vector>

class SaveDataScreen : public Screen {
public:
    explicit SaveDataScreen(App* app) : app(app) {}
    void enter()  override;
    void exit()   override;
    void handleEvent(const SDL_Event& e) override;
    void update(float dt) override;
    void render() override;

private:
    App*   app;
    Shader shader3D;
    GLuint vao3D = 0, vbo3D = 0;

    float  fadeIn  = 0.f;
    float  fadeOut = 0.f;
    bool   exiting = false;
    float  time    = 0.f;

    // 3 save slots positioned on the 3D ground
    struct Slot {
        float wx, wy, wz;   // world pos
        float sx, sy;        // projected screen pos
        float pulseAmt;
        bool  connected;
        bool  occupied = false; // has existing save data on disk
    };
    Slot slots[3];

    // Draw mechanic
    bool  drawing     = false;
    float drawStartX  = 0.f;
    float drawStartY  = 0.f;
    float drawCurrX   = 0.f;
    float drawCurrY   = 0.f;

    // Particles
    struct Particle {
        float x, y;        // current screen pos
        float tx, ty;      // target (circle center)
        float t;           // normalized travel 0..1
        float speed;
        int   slotIdx;
        bool  alive;
    };
    std::vector<Particle> particles;
    int selectedSlot = -1;

    // GL geometry helpers
    GLuint gridVao = 0, gridVbo = 0;
    int gridVertCount = 0;

    void buildGrid();
    void renderGrid();
    void renderSlotCircles();
    void renderParticles();
    void renderUI();
    void projectSlots();
    void spawnParticles(int slot);
    void tryConnect();
};
