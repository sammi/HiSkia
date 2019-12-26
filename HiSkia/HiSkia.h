#pragma once

#include "tools/sk_app/Application.h"
#include "tools/sk_app/Window.h"

class SkCanvas;

class HiSkia : public sk_app::Application, sk_app::Window::Layer {
public:
    HiSkia(int argc, char** argv, void* platformData);
    ~HiSkia() override;

    void onIdle() override;

    void onBackendCreated() override;
    void onPaint(SkSurface*) override;
    bool onChar(SkUnichar c, skui::ModifierKey modifiers) override;

private:
    void updateTitle();

    sk_app::Window* fWindow;
    sk_app::Window::BackendType fBackendType;

    SkScalar fRotationAngle;
};
