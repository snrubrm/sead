#include <prim/seadXmlElement.h>
#include <prim/seadXmlUtil.h>

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

SafeString XmlElement::findAttributeValue(const SafeString& name) const
{
    for (const auto& attribute : mAttributes)
    {
        if (attribute.mName == name)
            return attribute.mValue;
    }
    return SafeString::cEmptyString;
}

// NON_MATCHING: the compiler inlines XML element construction/content APIs, unlike the original.
XmlElement* XmlUtil::createBackSiblingElement(XmlElement* element, Heap* heap)
{
    if (!heap)
        heap = element->mHeap;
    auto* sibling = new (heap, 8) XmlElement;
    element->pushBackSibling(sibling);
    return sibling;
}

// NON_MATCHING: the compiler inlines XML element construction/content APIs, unlike the original.
XmlElement* XmlUtil::createBackChildElement(XmlElement* element, Heap* heap)
{
    if (!heap)
        heap = element->mHeap;
    auto* child = new (heap, 8) XmlElement;
    element->pushBackChild(child);
    return child;
}

// NON_MATCHING: the compiler inlines XML element construction/content APIs, unlike the original.
XmlElement* XmlUtil::createBackSiblingAndSetupElement(XmlElement* element, const SafeString& name,
                                                    const SafeString& content, Heap* heap)
{
    if (!heap)
        heap = element->mHeap;
    auto* child = createBackChildElement(element, heap);
    child->mName = name;
    if (content.isEmpty())
        child->setContent(nullptr, 0, false);
    else
        child->setContentString(content, heap);
    return child;
}

}  // namespace sead
