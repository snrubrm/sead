#include <devenv/seadDebugMenuSkin.h>
#include <devenv/seadFontMgr.h>

namespace sead
{
Color4f DebugMenuSkinDefault::sBackgroundColor0(0.1328125f, 0.54296875f, 0.1328125f, 1.0f);
Color4f DebugMenuSkinDefault::sBackgroundColor1(0.0f, 0.5f, 0.0f, 1.0f);
Color4f DebugMenuSkinDefault::sTitleBackgroundColor0(0.0f, 0.39f, 0.0f, 1.0f);
Color4f DebugMenuSkinDefault::sTitleBackgroundColor1(0.0f, 0.3f, 0.0f, 1.0f);
Color4f DebugMenuSkinDefault::sItemBackgroundColor1(0.0328125f, 0.4425f, 0.0328125f, 1.0f);
Color4f DebugMenuSkinDefault::sScrollBarColor(1.0f, 1.0f, 1.0f, 0.4f);
SafeString DebugMenuSkinDefault::sCursorText = ">";

// 0x7100af054c
DebugMenuSkinDefault::DebugMenuSkinDefault()
{
    FontBase* font = TextWriter::getDefaultFont();
    if (font)
    {
        mLineHeight = font->getHeight();
        mFontWidth = font->getCharWidth(u'm');
    }
}

// 0x7100af0548 (D1) / 0x7100af05b0 (D0)
DebugMenuSkinDefault::~DebugMenuSkinDefault() = default;

// 0x7100af05b4
void DebugMenuSkinDefault::beginDrawImage(DrawInstance* instance, const Viewport* viewport)
{
    instance->mProjection.setByViewport(*viewport);
    instance->mCamera.setByOrthoProjection(instance->mProjection);
    instance->mCamera.updateViewMatrix();
    instance->mDrawer.setCamera(&instance->mCamera);
    instance->mDrawer.setProjection(&instance->mProjection);
    TextWriter::setupGraphics(instance->mDrawContext);
    instance->mDrawer.begin();
}

// 0x7100af0630
void DebugMenuSkinDefault::endDrawImage(DrawInstance* instance)
{
    instance->mDrawer.end();
}

// 0x7100af0638
void DebugMenuSkinDefault::drawBackground(DrawInstance* instance, const Vector3f& pos, const Vector2f& size)
{
    drawQuad(instance, pos, size, sBackgroundColor0, sBackgroundColor1, false);
}

// 0x7100af0658
void DebugMenuSkinDefault::drawTitleBackground(DrawInstance* instance, const Vector3f& pos,
                                               const Vector2f& size)
{
    drawQuad(instance, pos, size, sTitleBackgroundColor0, sTitleBackgroundColor1, false);
}

// 0x7100af0678
void DebugMenuSkinDefault::drawItemBackground(DrawInstance* instance, const Vector3f& pos,
                                              const Vector2f& size)
{
    drawQuad(instance, pos, size, sBackgroundColor0, sItemBackgroundColor1, false);
}

// 0x7100af0698
void DebugMenuSkinDefault::drawScrollBar(DrawInstance* instance, const Vector3f& pos, const Vector2f& size,
                                         float begin, float end)
{
    drawQuad(instance, pos, size, sScrollBarColor, sScrollBarColor, false);
    drawQuad(instance, Vector3f(pos.x, pos.y - size.y * begin, 0.0f),
             Vector2f(size.x, (end - begin) * size.y), Color4f::cWhite, Color4f::cWhite, false);
}

// 0x7100af075c
void DebugMenuSkinDefault::drawQuad(DrawInstance* instance, const Vector3f& pos, const Vector2f& size,
                                    const Color4f& color0, const Color4f& color1, bool horizontal)
{
    PrimitiveDrawer::QuadArg arg;
    arg.setCornerAndSize(pos, size);
    if (horizontal)
        arg.setColorHorizontal(color0, color1);
    else
        arg.setColor(color0, color1);
    instance->mDrawer.drawQuad(arg);
}

// 0x7100af07fc
void DebugMenuSkinDefault::beginDrawText(DrawInstance* instance, const Viewport* viewport)
{
    instance->mTextWriter.setViewport(viewport);
    TextWriter::setupGraphics(instance->mDrawContext);
    instance->mTextWriter.beginDraw();
}

// 0x7100af0828
void DebugMenuSkinDefault::endDrawText(DrawInstance* instance)
{
    instance->mTextWriter.endDraw();
}

// 0x7100af0830
void DebugMenuSkinDefault::drawTitleText(DrawInstance* instance, const Vector3f& pos, const SafeString& text)
{
    drawText(instance, pos, text, Color4f::cWhite);
}

// 0x7100af0844
void DebugMenuSkinDefault::drawItemText(DrawInstance* instance, const Vector3f& pos, const SafeString& text)
{
    drawText(instance, pos, text, Color4f::cWhite);
}

// 0x7100af0858
void DebugMenuSkinDefault::drawCursorText(DrawInstance* instance, const Vector3f& pos)
{
    drawText(instance, pos, sCursorText, Color4f::cWhite);
}

// 0x7100af0874
s32 DebugMenuSkinDefault::drawFolderText(DrawInstance* instance, const Vector3f& pos, bool open)
{
    const SafeString text = open ? "[+]" : "[-]";
    drawText(instance, pos, text, Color4f::cWhite);
    return text.calcLength();
}

// 0x7100af0940
void DebugMenuSkinDefault::drawText(DrawInstance* instance, const Vector3f& pos, const SafeString& text,
                                    const Color4f& color)
{
    instance->mTextWriter.setCursor(pos);
    instance->mTextWriter.setColor(color);
    instance->mTextWriter.printImpl_(text.cstr(), -1, true, nullptr);
}

// 0x7100af09a4
float DebugMenuSkinDefault::getLineHeight()
{
    return mLineHeight;
}

// 0x7100af09ac
float DebugMenuSkinDefault::getFontWidth()
{
    return mFontWidth;
}

}  // namespace sead
