#pragma once

#include <container/seadObjList.h>
#include <prim/seadSafeString.h>

namespace sead
{
class Heap;
class ReadStream;
class WriteStream;
class XmlElement;

// Native factory B28370 allocates 0x68; table 24bfda0 has two destructor slots.
class XmlDocument
{
public:
    virtual ~XmlDocument();
    static XmlDocument* create(ReadStream* stream, Heap* heap, bool binary, u32 work_buffer_size);
    bool addEntity(const SafeString& name, const SafeString& value);
    bool save(WriteStream* stream, Heap* heap, bool binary, XmlElement* element) const;

    // Inline-only in the original; name is a guess. Independent effect XML
    // readers 1051604 and 12cbe84 obtain their root element from +8.
    XmlElement* getRoot() const { return mRoot; }

private:
    XmlDocument() = default;
    void resetEntity_(Heap* heap);
    struct Entity;
    ObjList<Entity>* expandEntityList(s32 capacity, Heap* heap);
    void parseXml_(ReadStream* stream, Heap* heap, bool binary);
    // Descriptive name: complete B24958 writes the binary element tree.
    static void writeXmlInstanceAsBinary_(WriteStream* stream, XmlElement* element);
    // Descriptive name: complete B292D0 writes the owned attribute list as text.
    static void writeXmlAttributesAsText_(WriteStream* stream, XmlElement* element);
    // Descriptive name: complete B28E14 handles escaped, CDATA and base64 content.
    static void writeXmlContentAsText_(WriteStream* stream, XmlElement* element, u32 content_type,
                                       u32 work_buffer_size, Heap* heap);
    static void writeXmlInstanceAsText_(WriteStream* stream, XmlElement* element, s32 depth,
                                        u32 work_buffer_size, Heap* heap);
    // Descriptive name: native b28d9c recursively releases child/sibling trees.
    static void deleteElementRecursive_(XmlElement* element);

    struct Entity
    {
        // B237EC and independent B2342C default-construct both strings without record zeroing.
        Entity() {}

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
