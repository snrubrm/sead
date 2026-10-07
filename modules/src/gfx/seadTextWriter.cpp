#include <gfx/seadTextWriter.h>
#include <devenv/seadFontMgr.h>
#include <gfx/seadViewport.h>

namespace sead
{
// 0x7100b1f7d0
void TextWriter::setCursorFromTopLeft(const Vector2f& pos)
{
    mCursor.x = pos.x - mViewport->getSizeX() * 0.5f;
    mCursor.y = -pos.y + mViewport->getSizeY() * 0.5f;
}

// 0x7100b1f814
void TextWriter::beginDraw()
{
    mFont->begin(mDrawContext);
    mEndedDrawing = false;
}

// 0x7100b1f848
void TextWriter::endDraw()
{
    mEndedDrawing = true;
    mFont->end(mDrawContext);
}

}  // namespace sead
