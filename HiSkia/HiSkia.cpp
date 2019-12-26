#include <iostream>

#include "HiSkia.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkSurface.h"
#include "include/effects/SkGradientShader.h"

using namespace sk_app;

Application* Application::Create(int argc, char** argv, void* platformData) {
    return new HiSkia(argc, argv, platformData);
}

HiSkia::HiSkia(int argc, char** argv, void* platformData) : fBackendType(Window::kNativeGL_BackendType) , fRotationAngle(0) {

    SkGraphics::Init();

    fWindow = Window::CreateNativeWindow(platformData);

    fWindow->setRequestedDisplayParams(DisplayParams());

    fWindow->pushLayer(this);

    fWindow->attach(fBackendType);
}

HiSkia::~HiSkia() {
    fWindow->detach();
    delete fWindow;
}

void HiSkia::updateTitle() {

    if (!fWindow || fWindow->sampleCount() <= 1) {
        return;
    }

    SkString title("Hello World ");
    title.append(Window::kRaster_BackendType == fBackendType ? "Raster" : "OpenGL");
    fWindow->setTitle(title.c_str());
}

void HiSkia::onBackendCreated() {
    this->updateTitle();
    fWindow->show();
    fWindow->inval();
}

void HiSkia::onPaint(SkSurface* surface) {

    auto canvas = surface->getCanvas();

    canvas->clear(SK_ColorWHITE);

    SkPaint paint;
    paint.setColor(SK_ColorRED);

    SkRect rect = SkRect::MakeXYWH(10, 10, 128, 128);

    canvas->drawRect(rect, paint);

    {
        SkPoint linearPoints[] = { { 0, 0 }, { 300, 300 } };
        
        SkColor linearColors[] = { SK_ColorGREEN, SK_ColorRED };

        paint.setShader(SkGradientShader::MakeLinear(linearPoints, linearColors, nullptr, 2, SkTileMode::kMirror));
        paint.setAntiAlias(true);

        canvas->drawCircle(200, 200, 64, paint);

        paint.setShader(nullptr);
    }

    SkFont font;
    font.setSubpixel(true);
    font.setSize(20);
    paint.setColor(SK_ColorBLACK);

    canvas->save();
    static const char message[] = "Hello World";

    canvas->translate(300, 300);
    fRotationAngle += 0.2f;
    if (fRotationAngle > 360) {
        fRotationAngle -= 360;
    }
    canvas->rotate(fRotationAngle);

    canvas->drawSimpleText(message, strlen(message), SkTextEncoding::kUTF8, 0, 0, font, paint);

    canvas->restore();
}

void HiSkia::onIdle() {
    fWindow->inval();
}

bool HiSkia::onChar(SkUnichar c, skui::ModifierKey modifiers) {
    if (' ' == c) {
        fBackendType = Window::kRaster_BackendType == fBackendType ? Window::kNativeGL_BackendType
            : Window::kRaster_BackendType;
        fWindow->detach();
        fWindow->attach(fBackendType);
    }
    return true;
}
