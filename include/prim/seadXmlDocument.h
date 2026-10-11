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
    struct Entity
    {
        FixedSafeString<8> mName;
        FixedSafeString<128> mValue;
    };
    static_assert(sizeof(Entity) == 0xb8, "XmlDocument::Entity size");

    XmlElement* mRoot;
    // parseXml_ B25334 allocates and releases three temporary text buffers.
    u8* mWorkBuffer0;
    u8* mWorkBuffer1;
    u8* mWorkBuffer2;
    u32 mWorkBufferSize;
    ObjList<Entity> mEntities;
    Heap* mHeap;
};
static_assert(sizeof(XmlDocument) == 0x68, "XmlDocument size");

}  // namespace sead
