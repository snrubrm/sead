#pragma once

#include <container/seadObjList.h>
#include <prim/seadSafeString.h>

namespace sead
{
class Heap;
class ReadStream;
class XmlElement;

// Native factory B28370 allocates 0x68; table 24bfda0 has two destructor slots.
class XmlDocument
{
public:
    virtual ~XmlDocument();
    static XmlDocument* create(ReadStream* stream, Heap* heap, bool binary, u32 work_buffer_size);

    // Inline-only in the original; name is a guess. Independent effect XML
    // readers 1051604 and 12cbe84 obtain their root element from +8.
    XmlElement* getRoot() const { return mRoot; }

private:
    XmlDocument() = default;
    void resetEntity_(Heap* heap);
    void parseXml_(ReadStream* stream, Heap* heap, bool binary);
    // Descriptive name: native b28d9c recursively releases child/sibling trees.
    static void deleteElementRecursive_(XmlElement* element);

    struct Entity
    {
        FixedSafeString<8> mName;
        FixedSafeString<128> mValue;
    };
    static_assert(sizeof(Entity) == 0xb8, "XmlDocument::Entity size");

    XmlElement* mRoot = nullptr;
    // parseXml_ B25334 allocates and releases three temporary text buffers.
    u8* mWorkBuffer0 = nullptr;
    u8* mWorkBuffer1 = nullptr;
    u8* mWorkBuffer2 = nullptr;
    u32 mWorkBufferSize;
    ObjList<Entity> mEntities;
    Heap* mHeap = nullptr;
};
static_assert(sizeof(XmlDocument) == 0x68, "XmlDocument size");

}  // namespace sead
