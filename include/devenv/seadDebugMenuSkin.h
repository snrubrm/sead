#pragma once

#include <gfx/seadCamera.h>
#include <gfx/seadColor.h>
#include <gfx/seadPrimitiveRenderer.h>
#include <gfx/seadProjection.h>
#include <gfx/seadTextWriter.h>
#include <math/seadVector.h>
#include <prim/seadRuntimeTypeInfo.h>
#include <prim/seadSafeString.h>

namespace sead
{
class DrawContext;
class Viewport;

// The drawing interface of the debug menu. The original has no vtable for this class (an abstract base: only its RTTI
// checks remain, typeinfo 0x71025f89a8).
class DebugMenuSkinBase
{
    SEAD_RTTI_BASE(DebugMenuSkinBase)

public:
    // The draw state that the debug menu passes to the skin (offsets from DebugMenuSkinDefault 0x7100af05b4 /
    // 0x7100af07fc / 0x7100af0940).
    struct DrawInstance
    {
        OrthoProjection mProjection;
        OrthoCamera mCamera;
        PrimitiveDrawer mDrawer;
        TextWriter mTextWriter;
        DrawContext* mDrawContext;
    };

    virtual ~DebugMenuSkinBase() = default;

    virtual void beginDrawImage(DrawInstance* instance, const Viewport* viewport) = 0;
    virtual void endDrawImage(DrawInstance* instance) = 0;
    virtual void drawBackground(DrawInstance* instance, const Vector3f& pos, const Vector2f& size) = 0;
    virtual void drawTitleBackground(DrawInstance* instance, const Vector3f& pos, const Vector2f& size) = 0;
    virtual void drawItemBackground(DrawInstance* instance, const Vector3f& pos, const Vector2f& size) = 0;
    virtual void drawScrollBar(DrawInstance* instance, const Vector3f& pos, const Vector2f& size, float begin,
                               float end) = 0;
    virtual void drawQuad(DrawInstance* instance, const Vector3f& pos, const Vector2f& size,
                          const Color4f& color0, const Color4f& color1, bool horizontal) = 0;
    virtual void beginDrawText(DrawInstance* instance, const Viewport* viewport) = 0;
    virtual void endDrawText(DrawInstance* instance) = 0;
    virtual void drawTitleText(DrawInstance* instance, const Vector3f& pos, const SafeString& text) = 0;
    virtual void drawItemText(DrawInstance* instance, const Vector3f& pos, const SafeString& text) = 0;
    virtual void drawCursorText(DrawInstance* instance, const Vector3f& pos) = 0;
    // Returns the length of the drawn mark.
    virtual s32 drawFolderText(DrawInstance* instance, const Vector3f& pos, bool open) = 0;
    virtual void drawText(DrawInstance* instance, const Vector3f& pos, const SafeString& text,
                          const Color4f& color) = 0;
    virtual float getLineHeight() = 0;
    virtual float getFontWidth() = 0;
};
static_assert(offsetof(DebugMenuSkinBase::DrawInstance, mCamera) == 0xb0);
static_assert(offsetof(DebugMenuSkinBase::DrawInstance, mDrawer) == 0x110);
static_assert(offsetof(DebugMenuSkinBase::DrawInstance, mTextWriter) == 0x138);
static_assert(offsetof(DebugMenuSkinBase::DrawInstance, mDrawContext) == 0x1b0);

// vtable 0x71024bd4f0
class DebugMenuSkinDefault : public DebugMenuSkinBase
{
    SEAD_RTTI_OVERRIDE(DebugMenuSkinDefault, DebugMenuSkinBase)

public:
    DebugMenuSkinDefault();
    ~DebugMenuSkinDefault() override;

    void beginDrawImage(DrawInstance* instance, const Viewport* viewport) override;
    void endDrawImage(DrawInstance* instance) override;
    void drawBackground(DrawInstance* instance, const Vector3f& pos, const Vector2f& size) override;
    void drawTitleBackground(DrawInstance* instance, const Vector3f& pos, const Vector2f& size) override;
    void drawItemBackground(DrawInstance* instance, const Vector3f& pos, const Vector2f& size) override;
    void drawScrollBar(DrawInstance* instance, const Vector3f& pos, const Vector2f& size, float begin,
                       float end) override;
    void drawQuad(DrawInstance* instance, const Vector3f& pos, const Vector2f& size, const Color4f& color0,
                  const Color4f& color1, bool horizontal) override;
    void beginDrawText(DrawInstance* instance, const Viewport* viewport) override;
    void endDrawText(DrawInstance* instance) override;
    void drawTitleText(DrawInstance* instance, const Vector3f& pos, const SafeString& text) override;
    void drawItemText(DrawInstance* instance, const Vector3f& pos, const SafeString& text) override;
    void drawCursorText(DrawInstance* instance, const Vector3f& pos) override;
    s32 drawFolderText(DrawInstance* instance, const Vector3f& pos, bool open) override;
    void drawText(DrawInstance* instance, const Vector3f& pos, const SafeString& text,
                  const Color4f& color) override;
    float getLineHeight() override;
    float getFontWidth() override;

    // The colours and the cursor mark (static initializer 0x7100af2150; the functions read them through the GOT).
    // The original initialises them at run time, so they live in writable data and are not const here.
    // Names are guesses.
    static Color4f sBackgroundColor0;
    static Color4f sBackgroundColor1;
    static Color4f sTitleBackgroundColor0;
    static Color4f sTitleBackgroundColor1;
    static Color4f sItemBackgroundColor1;
    static Color4f sScrollBarColor;
    static SafeString sCursorText;

private:
    // From the default font (its height and the width of 'm').
    float mLineHeight = 0;
    float mFontWidth = 0;
};

}  // namespace sead
