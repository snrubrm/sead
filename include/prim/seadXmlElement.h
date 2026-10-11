#pragma once

#include <container/seadObjList.h>
#include <container/seadTreeNode.h>
#include <prim/seadSafeString.h>

namespace sead
{

// Native B28370 allocates 0xd0 bytes; table 24bfdf0 has the two destructor slots.
class XmlElement : public TreeNode
{
public:
    struct XmlAttribute
    {
        XmlAttribute(Heap* heap, const SafeString& name, const SafeString& value)
            : mName(heap, name), mValue(heap, value)
        {
        }

        HeapSafeString mName;
        HeapSafeString mValue;
    };
    static_assert(sizeof(XmlAttribute) == 0x30, "XmlAttribute size");

    XmlElement();
    virtual ~XmlElement();

    // Inline-only in the original; name is a guess. The effect XML readers
    // 1051604 and 12cbe84 compare the FixedSafeString at +0x28 to their root tag.
    const SafeString& getName() const { return mName; }
    // inline-only in the original; name is a guess. The three native curve
    // writers ba905c, bb424c and bb5dfc copy their root tag into +0x28.
    void setName(const SafeString& name) { mName.copy(name); }
    // inline-only in the original; names are guesses. Recursive deletion
    // b28d9c and independent AGL subtree cleanup 13733a0 traverse these
    // typed child/sibling links owned by the TreeNode base.
    XmlElement* getFirstChild() const { return static_cast<XmlElement*>(mChild); }
    XmlElement* getNextSibling() const { return static_cast<XmlElement*>(mNext); }
    SafeString getContentString() const;
    SafeString findAttributeValue(const SafeString& name) const;
    // Native b29b78 returns the owned list at +0x98 on every exit.
    ObjList<XmlAttribute>* expandAttributeList(s32 capacity, Heap* heap);
    bool addAttribute(const SafeString& name, const SafeString& value, Heap* heap);
    bool updateAttribute(const SafeString& name, const SafeString& value, Heap* heap);
    void setContent(u8* content, u32 size, bool own);
    void setContentString(const SafeString& content, Heap* heap);
    const XmlElement* findElementByAbsolutePath_(const SafeString& path) const;

private:
    // Native XmlUtil creation helpers initialize the owned name/content.
    friend class XmlUtil;

    FixedSafeString<64> mName;
    u8* mContent = nullptr;
    u32 mContentSize = 0;
    bool mOwnContent = false;
    u32 _90 = 0;
    ObjList<XmlAttribute> mAttributes;
    Heap* mHeap = nullptr;
};
static_assert(sizeof(XmlElement) == 0xd0, "XmlElement size");

}  // namespace sead
