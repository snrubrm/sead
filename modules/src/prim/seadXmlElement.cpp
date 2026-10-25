#include <prim/seadXmlElement.h>
#include <prim/seadXmlUtil.h>

#include <cstring>
#include <basis/seadNew.h>
#include <container/seadObjArray.h>

namespace sead
{

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

bool XmlElement::addAttribute(const SafeString& name, const SafeString& value, Heap* heap)
{
    if (name.isEmpty() || !mAttributes.isBufferReady() || mAttributes.isFull())
        return false;
    for (const auto& attribute : mAttributes)
    {
        if (name == attribute.mName)
            return false;
    }
    mAttributes.emplaceBack(heap, name, value);
    return true;
}

// NON_MATCHING: the comparison operands are reversed after inlining string length.
bool XmlElement::updateAttribute(const SafeString& name, const SafeString& value, Heap* heap)
{
    for (auto& attribute : mAttributes)
    {
        if (attribute.mName != name)
            continue;
        s32 length = value.calcLength();
        if (length < attribute.mValue.calcLength())
            attribute.mValue.copy(value, length);
        else
        {
            mAttributes.erase(&attribute);
            addAttribute(name, value, heap);
        }
        return true;
    }
    return false;
}

// NON_MATCHING: temporary attribute construction schedules the second string argument differently.
ObjList<XmlElement::XmlAttribute>* XmlElement::expandAttributeList(s32 capacity, Heap* heap)
{
    if (!heap)
        heap = mHeap;
    if (!mAttributes.isBufferReady())
    {
        mAttributes.allocBuffer(capacity, heap);
        return &mAttributes;
    }
    if (capacity <= mAttributes.size())
        return &mAttributes;

    // Native b29b78 uses two complete 1024-byte fixed strings per temporary
    // record (0x830 bytes), held in a separate ObjArray while replacing storage.
    struct AttributeCopy
    {
        FixedSafeString<1024> name;
        FixedSafeString<1024> value;
    };
    static_assert(sizeof(AttributeCopy) == 0x830, "AttributeCopy size");
    ObjArray<AttributeCopy> copy;
    copy.allocBuffer(mAttributes.size(), heap, -8);
    for (const auto& attribute : mAttributes)
    {
        auto* item = copy.emplaceBack();
        item->name = attribute.mName;
        item->value = attribute.mValue;
    }
    mAttributes.freeBuffer();
    mAttributes.allocBuffer(capacity, heap);
    for (const auto& item : copy)
        mAttributes.emplaceBack(heap, item.name, item.value);
    copy.freeBuffer();
    return &mAttributes;
}

const XmlElement* XmlElement::findElementByAbsolutePath_(const SafeString& path) const
{
    if (path.comparen("/", 1) == 0)
        return findElementFromRootPath_(path);
    // Native b2a63c tests one character even for the two-character ./ literal.
    if (path.comparen("./", 1) == 0)
        return findElementByRelativePath_(path.getPart(2));
    return findElementByRelativePath_(path);
}

// NON_MATCHING: the existing TreeNode root API is out of line, unlike the native traversal.
const XmlElement* XmlElement::findElementFromRootPath_(const SafeString& path) const
{
    const auto* element = static_cast<const XmlElement*>(TreeNode::findRoot());
    FixedSafeString<256> remaining(path);
    FixedSafeString<256> component;
    while (true)
    {
        component.copy(remaining.getPart(1));
        if (component.isEmpty())
            return element;
        element = element->getFirstChild();
        if (!findSiblingElement_(&element, component))
            return nullptr;
        remaining.copy(component.getPart(element->mName.calcLength()));
        if (remaining.isEmpty())
            return element;
    }
}

// NON_MATCHING: prefix string termination devirtualizes and loop copy blocks group differently.
const XmlElement* XmlElement::findElementByRelativePath_(const SafeString& path) const
{
    FixedSafeString<256> remaining;
    FixedSafeString<256> component(path);
    const XmlElement* element = this;
    while (true)
    {
        if (component.comparen("..", 2) == 0)
        {
            element = static_cast<const XmlElement*>(element->mParent);
            if (!element)
                return nullptr;
            remaining.copy(component.getPart(2));
        }
        else
        {
            if (component.isEmpty())
                return element;
            element = element->getFirstChild();
            if (!findSiblingElement_(&element, component))
                return nullptr;
            remaining.copy(component.getPart(element->mName.calcLength()));
            if (remaining.isEmpty())
                return element;
        }
        component.copy(remaining.getPart(1));
    }
}

XmlElement* XmlElement::findElementByAbsolutePath_(const SafeString& path)
{
    return const_cast<XmlElement*>(
        static_cast<const XmlElement*>(this)->findElementByAbsolutePath_(path));
}

bool XmlElement::findSiblingElement_(const XmlElement** element, SafeString path) const
{
    for (const XmlElement* candidate = *element; candidate; candidate = candidate->getNextSibling())
    {
        const s32 length = candidate->mName.calcLength();
        if (candidate->mName.comparen(path, length) == 0 &&
            (path[length] == '/' || path[length] == '\0'))
        {
            *element = candidate;
            return true;
        }
    }
    return false;
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
