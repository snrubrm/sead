#pragma once

#include <gfx/seadColor.h>
#include <math/seadBoundBox.h>

namespace sead
{
class Viewport;
class Camera;
class Projection;
class DrawContext;
class FontBase;
class DebugFontMgrNvn;

class TextWriter
{
public:
    explicit TextWriter(DrawContext* context);
    TextWriter(DrawContext* context, const Viewport* viewport);
    virtual ~TextWriter();

    // Complete DebugMenu constructor AF00F8 calls this without a TextWriter receiver.
    static FontBase* getDefaultFont();
    static void setDefaultFont(FontBase* font);
    static void setupGraphics(DrawContext* context);
    void getCursorFromTopLeft(Vector2f* pos) const;
    void setCursorFromTopLeft(const Vector2f& pos);
    void setScaleFromFontSize(const Vector2f& fontSize);
    void setScaleFromFontHeight(float fontHeight);
    void setProjectionAndCamera(const Projection* projection, const Camera* camera);
    void setLineSpaceFromLineHeight(float lineHeight);
    void setFormatBuffer(char16_t*, int);
    void setDrawContext(DrawContext* context);
    // Inline in the original: sead::DebugMenuSkinDefault::beginDrawText 0x7100af07fc / drawText 0x7100af0940.
    void setViewport(const Viewport* viewport) { mViewport = viewport; }
    // name is a guess; the original copies x then y (one load and store each).
    void setCursor(const Vector3f& pos)
    {
        mCursor.x = pos.x;
        mCursor.y = pos.y;
    }
    void setColor(const Color4f& color) { mColor = color; }
    void beginDraw();
    void endDraw();
    void printf(const char16_t* format, ...);
    void vprintfImpl_(const char16_t*, std::va_list, bool, BoundBox2f*);
    void printfWithCalcRect(BoundBox2f*, const char16_t*, ...);
    void printf(const char* format, ...);
    void vprintfImpl_(const char*, std::va_list, bool, BoundBox2f*);
    void printfWithCalcRect(BoundBox2f*, const char*, ...);
    void calcFormatStringRect(BoundBox2f*, const char16_t*, ...);
    void calcFormatStringRect(BoundBox2f*, const char*, ...);
    void printImpl_(const char16_t*, int, bool, BoundBox2f*, const Projection*, const Camera*);
    void printImpl_(const char16_t*, int, bool, BoundBox2f*);
    void printImpl_(const char*, int, bool, BoundBox2f*);

private:
    static FontBase* sDefaultFont;

    const Viewport* mViewport;
    Projection* mProjection;
    Camera* mCamera;
    FontBase* mFont;
    Vector2f mCursor;
    Vector2f mScale;
    Color4f mColor;
    int _48;
    float mLineSpace;
    // printImpl_ 0x7100b1f9a8 reads a pointer at +0x50 (null-checked, then pairs of floats) and a width limit at
    // +0x58 that is ignored while it is infinite (the constructors store infinity).
    void* _50;
    float _58;
    char16_t* mFormatBuffer;
    int mFormatBufferSize;
    bool mEndedDrawing;
    DrawContext* mDrawContext;
};
}  // namespace sead
