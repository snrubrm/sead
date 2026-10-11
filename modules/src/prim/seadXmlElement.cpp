#include <prim/seadXmlElement.h>

#include <cstring>
#include <basis/seadNew.h>

namespace sead
{

XmlElement::XmlElement() : mName("") {}

XmlElement::~XmlElement()
{
    if (mContent && mOwnContent)
        delete[] mContent;
    mAttributes.freeBuffer();
}

void XmlElement::setContent(u8* content, u32 size, bool own)
{
    if (mContent == content)
        return;
    if (mContent && mOwnContent)
        delete[] mContent;
    mOwnContent = own;
    mContent = content;
    mContentSize = size;
}

SafeString XmlElement::getContentString() const
{
    return mContent ? SafeString(static_cast<const char*>(static_cast<const void*>(mContent))) :
                      SafeString::cEmptyString;
}

void XmlElement::setContentString(const SafeString& content, Heap* heap)
{
    s32 size = content.calcLength();
    auto* buffer = new (heap, 8) u8[size + 1];
    std::memcpy(buffer, content.cstr(), size);
    buffer[size] = 0;
    setContent(buffer, size, true);
}

}  // namespace sead
