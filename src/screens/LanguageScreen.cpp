#include "LanguageScreen.h"
#include <cmath>

void LanguageScreen::enter() {
    fadeIn  = 0.f;
    fadeOut = 0.f;
    exiting = false;
    done    = false;
    initButtons();
}

void LanguageScreen::initButtons() {
    float cx = app->screenW * 0.5f;
    float cy = app->screenH * 0.5f;
    float bw = 420.f, bh = 110.f, gap = 50.f;
    buttons[0] = { cx - bw - gap*0.5f, cy - bh*0.5f, bw, bh, "English", "heading", "English" };
    buttons[1] = { cx +       gap*0.5f, cy - bh*0.5f, bw, bh, u8"\u09AC\u09BE\u0982\u09B2\u09BE", "bangla_body", "Bangla" };
}

void LanguageScreen::handleEvent(const SDL_Event& e) {
    if (exiting) return;
    if (e.type == SDL_MOUSEMOTION) {
        float mx = (float)e.motion.x, my = (float)e.motion.y;
        hovered = -1;
        for (int i = 0; i < 2; ++i) {
            auto& b = buttons[i];
            if (mx >= b.x && mx <= b.x+b.w && my >= b.y && my <= b.y+b.h)
                hovered = i;
        }
    }
    if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
        if (hovered >= 0) {
            app->selectedLanguage = buttons[hovered].lang;
            exiting = true;
            nextState = AppState::Account;
        }
    }
    if (e.type == SDL_KEYDOWN) {
        if (e.key.keysym.sym == SDLK_LEFT || e.key.keysym.sym == SDLK_RIGHT)
            hovered = (hovered == 0) ? 1 : 0;
        if (e.key.keysym.sym == SDLK_RETURN && hovered >= 0) {
            app->selectedLanguage = buttons[hovered].lang;
            exiting = true;
            nextState = AppState::Account;
        }
    }
}

void LanguageScreen::update(float dt) {
    pulseTime += dt;
    if (!exiting) {
        fadeIn = std::min(1.f, fadeIn + dt * 1.5f);
    } else {
        fadeOut += dt * 2.0f;
        if (fadeOut >= 1.f) done = true;
    }
}

void LanguageScreen::render() {
    float alpha = exiting ? (1.f - fadeOut) : fadeIn;
    alpha = clamp01(alpha);
    auto& tr = app->tr;
    float W = (float)app->screenW, H = (float)app->screenH;

    // Background: dark blue gradient (top) to deep blue (bottom)
    tr.drawGradientRect(0, 0, W, H,
        0.02f, 0.04f, 0.18f, 1.f,
        0.04f, 0.10f, 0.35f, 1.f, true);

    // Animated radial glow at center
    float pulse = 0.5f + 0.5f*sinf(pulseTime * 1.5f);
    for (int i = 5; i >= 1; --i) {
        float r = (float)i * 80.f + pulse*40.f;
        float a2 = alpha * 0.04f * (6.f - i);
        tr.drawCircle(W*0.5f, H*0.5f, r, 64, 0.1f, 0.5f, 1.f, a2);
    }

    // Logo / Title
    std::string titleFont = (app->selectedLanguage == "Bangla") ? "bangla_title" : "title";
    tr.drawTextCentered("MahirTV", "title", W*0.5f, H*0.28f, 0.3f, 0.75f, 1.f, alpha);

    // Subtitle
    tr.drawTextCentered("Select Language", "body", W*0.5f, H*0.40f,
                        0.7f, 0.85f, 1.f, alpha * 0.7f);

    // Buttons
    for (int i = 0; i < 2; ++i) {
        auto& b = buttons[i];
        bool hov = (hovered == i);
        float scale = hov ? (1.f + 0.04f*sinf(pulseTime*4.f)) : 1.f;
        float bw2 = b.w * scale, bh2 = b.h * scale;
        float bx = b.x + (b.w - bw2)*0.5f;
        float by = b.y + (b.h - bh2)*0.5f;

        // Card shadow
        tr.drawRectRounded(bx+4, by+4, bw2, bh2, 18.f, 0.f,0.f,0.f, alpha*0.35f);

        // Card fill
        float fgA = hov ? 0.30f : 0.15f;
        tr.drawRectRounded(bx, by, bw2, bh2, 18.f, 0.1f, 0.45f, 0.95f, alpha * fgA);

        // Border glow
        float bA = hov ? 0.9f : 0.4f;
        tr.drawRing(bx+bw2*0.5f, by+bh2*0.5f,
                    std::max(bw2,bh2)*0.5f, 2.f, 64,
                    0.2f, 0.65f, 1.f, alpha*bA*0.4f);

        // Label
        tr.drawTextCentered(b.label, b.font,
                            bx+bw2*0.5f, by+bh2*0.5f,
                            1.f, 1.f, 1.f, alpha);

        // "Selected" indicator
        if (i == 0) {
            tr.drawTextCentered("English", "small",
                                bx+bw2*0.5f, by+bh2+16.f,
                                0.5f,0.8f,1.f, alpha*0.6f);
        } else {
            // Small "Bangla" hint in latin under button
            tr.drawTextCentered("Bangla", "small",
                                bx+bw2*0.5f, by+bh2+16.f,
                                0.5f,0.8f,1.f, alpha*0.6f);
        }
    }

    // Footer hint
    tr.drawTextCentered("Use arrow keys or click to select", "small",
                        W*0.5f, H*0.88f, 0.5f,0.7f,0.9f, alpha*0.5f);

    // Fade overlay
    if (exiting) {
        tr.drawRect(0,0,W,H, 0.f,0.f,0.f, clamp01(fadeOut));
    } else if (fadeIn < 1.f) {
        tr.drawRect(0,0,W,H, 0.f,0.f,0.f, 1.f - fadeIn);
    }
}
