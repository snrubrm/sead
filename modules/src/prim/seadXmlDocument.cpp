#include <prim/seadXmlDocument.h>
#include <prim/seadXmlElement.h>
#include <prim/seadXmlUtil.h>
#include <prim/seadEndian.h>

#include <basis/seadNew.h>
#include <codec/seadBase64.h>
#include <heap/seadHeapMgr.h>
#include <stream/seadStream.h>

namespace sead
{

// Native default entity tables, indirection pointers and count at 24bfd38..98.
// Both complete reset B2342C and save B24410 consume these original objects.
const char* const sUnk_71024bfd38[] = {"lt", "gt", "amp", "apos", "quot"};
const char* const* sUnk_71024bfd60 = sUnk_71024bfd38;
const char* const sUnk_71024bfd68[] = {"<", ">", "&", "'", "\""};
const char* const* sUnk_71024bfd90 = sUnk_71024bfd68;
u32 sUnk_71024bfd98 = 5;


XmlDocument::~XmlDocument()
{
    if (mRoot)
    {
        deleteElementRecursive_(mRoot);
        mRoot = nullptr;
    }
    mEntities.freeBuffer();
}

// NON_MATCHING: default construction groups the initial member stores differently.
XmlDocument* XmlDocument::create(ReadStream* stream, Heap* heap, bool binary, u32 work_buffer_size)
{
    if (!heap)
        heap = HeapMgr::sInstancePtr->getCurrentHeap();
    auto* document = new (heap, 8) XmlDocument;
    document->mWorkBufferSize = work_buffer_size;
    if (!stream)
    {
        document->mRoot = new (heap, 8) XmlElement;
        document->resetEntity_(heap);
    }
    else
    {
        document->parseXml_(stream, heap, binary);
        if (!document->mRoot)
        {
            delete document;
            return nullptr;
        }
    }
    document->mHeap = heap;
    return document;
}

// NON_MATCHING: fixed-string stack placement adds addressing instructions.
void XmlDocument::parseXml_(ReadStream* stream, Heap* heap, bool binary)
{
    if (mRoot)
    {
        deleteElementRecursive_(mRoot);
        mRoot = nullptr;
    }
    stream->setMode(Stream::Modes::Binary);
    stream->rewind();
    if (binary)
    {
        FixedSafeString<8> name;
        FixedSafeString<128> value;
        u8 length = 0;
        u8 count = 0;
        if (!stream->readMemBlock(&count, 1))
            return;
        resetEntity_(heap);
        expandEntityList(count, heap);
        for (u8 index = 0; index < count; ++index)
        {
            if (!stream->readMemBlock(&length, 1))
                return;
            if (stream->readMemBlock(name.getBuffer(), length) < length)
                return;
            name.trim(length);
            if (!stream->readMemBlock(&length, 1))
                return;
            if (stream->readMemBlock(value.getBuffer(), length) < length)
                return;
            value.trim(length);
            addEntity(name, value);
        }
        auto* element = new (heap, 8) XmlElement;
        FixedSafeString<1024> attribute_name;
        FixedSafeString<1024> attribute_value;
        readXmlInstanceAsBinary_(element, stream, heap, &attribute_name, &attribute_value);
        mRoot = element;
    }
    else
    {
        mWorkBuffer0 = new (heap, -8) u8[mWorkBufferSize];
        mWorkBuffer1 = new (heap, -8) u8[mWorkBufferSize];
        mWorkBuffer2 = new (heap, -8) u8[mWorkBufferSize];
        resetEntity_(heap);
        const s32 offset = parseXmlPreamble_(stream, heap);
        if (offset >= 0)
        {
            stream->rewind();
            stream->skip(offset);
            mRoot = parseXmlTree_(stream, heap);
        }
        // Native B25668..94 releases these raw byte allocations through scalar delete.
        if (mWorkBuffer0)
            ::operator delete(mWorkBuffer0);
        mWorkBuffer0 = nullptr;
        if (mWorkBuffer1)
            ::operator delete(mWorkBuffer1);
        mWorkBuffer1 = nullptr;
        if (mWorkBuffer2)
            ::operator delete(mWorkBuffer2);
        mWorkBuffer2 = nullptr;
    }
}

// NON_MATCHING: name-length normalization uses an extra temporary register.
void XmlDocument::readXmlInstanceAsBinary_(XmlElement* element, ReadStream* stream, Heap* heap,
                                          BufferedSafeString* attribute_name,
                                          BufferedSafeString* attribute_value)
{
    while (true)
    {
        const u8 name_length = stream->readU8();
        if (name_length)
            stream->readString(&element->mName, name_length);
        element->mName.trim(name_length);
        const u8 count = stream->readU8();
        if (count)
        {
            element->expandAttributeList(count, heap);
            for (u8 index = 0; index < count; ++index)
            {
                const u8 attribute_name_length = stream->readU8();
                stream->readString(attribute_name, attribute_name_length);
                attribute_name->trim(attribute_name_length);
                const u8 attribute_value_length = stream->readU8();
                stream->readString(attribute_value, attribute_value_length);
                attribute_value->trim(attribute_value_length);
                element->addAttribute(*attribute_name, *attribute_value, heap);
            }
        }
        const u16 content_size = Endian::toHostU16(Endian::cLittle, stream->readU16());
        if (content_size)
        {
            auto* content = new (heap, 8) u8[content_size + 1];
            stream->readMemBlock(content, content_size);
            content[content_size] = SafeString::cNullChar;
            element->setContent(content, content_size, true);
        }
        switch (stream->readU8())
        {
        case 1:
            element = XmlUtil::createBackChildElement(element, heap);
            break;
        case 2:
            readXmlInstanceAsBinary_(XmlUtil::createBackChildElement(element, heap), stream, heap,
                                     attribute_name, attribute_value);
            [[fallthrough]];
        case 0:
        case 4:
            element = XmlUtil::createBackSiblingElement(element, heap);
            if (!element)
                return;
            break;
        default:
            return;
        }
    }
}

void XmlDocument::resetEntity_(Heap* heap)
{
    mEntities.freeBuffer();
    mEntities.allocBuffer(sUnk_71024bfd98, heap);
    mEntities.clear();
    for (u32 index = 0; index < sUnk_71024bfd98; ++index)
    {
        auto* entity = mEntities.emplaceBack();
        entity->mName.copy(sUnk_71024bfd60[index]);
        entity->mValue.copy(sUnk_71024bfd90[index]);
    }
}

bool XmlDocument::addEntity(const SafeString& name, const SafeString& value)
{
    if (mEntities.isFull())
        return false;
    for (auto& entity : mEntities)
    {
        if (entity.mName == name)
        {
            entity.mValue.copy(value);
            return true;
        }
    }
    auto* entity = mEntities.emplaceBack();
    entity->mName.copy(name);
    entity->mValue.copy(value);
    return true;
}

// NON_MATCHING: existing list allocation width and restored-list capacity checks differ.
ObjList<XmlDocument::Entity>* XmlDocument::expandEntityList(s32 capacity, Heap* heap)
{
    if (!heap)
        heap = mHeap;
    if (!mEntities.isBufferReady())
    {
        mEntities.allocBuffer(capacity, heap);
        return &mEntities;
    }
    if (capacity <= mEntities.size())
        return &mEntities;

    ObjList<Entity> temporary;
    temporary.allocBuffer(mEntities.size(), heap, -8);
    for (auto& entity : mEntities)
    {
        auto* copy = temporary.emplaceBack();
        copy->mName.copy(entity.mName);
        copy->mValue.copy(entity.mValue);
    }
    mEntities.freeBuffer();
    mEntities.allocBuffer(capacity, heap);
    for (auto& entity : temporary)
    {
        auto* copy = mEntities.emplaceBack();
        copy->mName.copy(entity.mName);
        copy->mValue.copy(entity.mValue);
    }
    temporary.freeBuffer();
    return &mEntities;
}

// NON_MATCHING: typed entity-list iterator bases are scheduled differently.
bool XmlDocument::save(WriteStream* stream, Heap* heap, bool binary, XmlElement* element) const
{
    stream->setMode(Stream::Modes::Binary);
    if (!element)
        element = mRoot;
    if (!element)
        return false;
    const s32 extra_count = mEntities.size() - sUnk_71024bfd98;
    if (binary)
    {
        u8 count = extra_count;
        stream->writeMemBlock(&count, 1);
        if (extra_count > 0)
        {
            auto it = mEntities.begin();
            for (u32 index = 0; index < sUnk_71024bfd98; ++index)
                ++it;
            for (; it != mEntities.end(); ++it)
            {
                u8 length = 0;
                length = it->mName.calcLength();
                stream->writeMemBlock(&length, 1);
                stream->writeMemBlock(it->mName.cstr(), length);
                length = it->mValue.calcLength();
                stream->writeMemBlock(&length, 1);
                stream->writeMemBlock(it->mValue.cstr(), length);
            }
        }
        writeXmlInstanceAsBinary_(stream, element);
    }
    else
    {
        const SafeString header("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");
        stream->writeString(header, header.calcLength());
        if (extra_count > 0)
        {
            const SafeString doctype("<!DOCTYPE doc[\n");
            stream->writeString(doctype, doctype.calcLength());
            auto it = mEntities.begin();
            for (u32 index = 0; index < sUnk_71024bfd98; ++index)
                ++it;
            for (; it != mEntities.end(); ++it)
            {
                FixedSafeString<150> declaration;
                declaration.format("<!ENTITY %s \"%s\">\n", it->mName.cstr(), it->mValue.cstr());
                stream->writeString(declaration, declaration.calcLength());
            }
            const SafeString end("]>\n");
            stream->writeString(end, end.calcLength());
        }
        writeXmlInstanceAsText_(stream, element, 0, mWorkBufferSize, heap);
    }
    stream->flush();
    return true;
}

// NON_MATCHING: recursive child/sibling branches and stack allocation differ.
void XmlDocument::writeXmlInstanceAsBinary_(WriteStream* stream, XmlElement* element)
{
    {
        u8 length = element->mName.calcLength();
        stream->writeU8(length);
        stream->writeString(element->mName, length);
        if (element->mAttributes.size() > 0)
        {
            stream->writeU8(element->mAttributes.size());
            for (auto& attribute : element->mAttributes)
            {
                length = attribute.mName.calcLength();
                stream->writeU8(length);
                stream->writeString(attribute.mName, length);
                length = attribute.mValue.calcLength();
                stream->writeU8(length);
                stream->writeString(attribute.mValue, length);
            }
        }
        else
        {
            stream->writeU8(0);
        }
        XmlElement* child = element->getFirstChild();
        XmlElement* sibling = element->getNextSibling();
        if (child)
        {
            stream->writeU16(0);
            stream->writeU8(sibling ? 2 : 1);
            writeXmlInstanceAsBinary_(stream, child);
        }
        else if (element->mContent)
        {
            const u16 size = Endian::fromHostU16(Endian::cLittle, element->mContentSize);
            stream->writeU16(size);
            if (size)
                stream->writeMemBlock(element->mContent, size);
            stream->writeU8(sibling ? 0 : 3);
        }
        else
        {
            stream->writeU16(0);
            stream->writeU8(sibling ? 4 : 5);
        }
        if (sibling)
            writeXmlInstanceAsBinary_(stream, sibling);
    }
}

// NON_MATCHING: native recursive leaf specialization is not inlined here.
void XmlDocument::writeXmlInstanceAsText_(WriteStream* stream, XmlElement* element, s32 depth,
                                           u32 work_buffer_size, Heap* heap)
{
    if (!element)
        return;
    const bool named = !element->mName.isEmpty();
    if (named)
    {
        for (s32 index = 0; index < depth; ++index)
            stream->writeU8('\t');
        stream->writeU8('<');
        stream->writeString(element->mName, element->mName.calcLength());
        writeXmlAttributesAsText_(stream, element);
    }
    if (auto* child = element->getFirstChild())
    {
        stream->writeString(">\n", 2);
        for (; child; child = child->getNextSibling())
            writeXmlInstanceAsText_(stream, child, depth + 1, work_buffer_size, heap);
        if (!named)
            return;
        for (s32 index = 0; index < depth; ++index)
            stream->writeU8('\t');
        stream->writeString("</", 2);
        stream->writeString(element->mName, element->mName.calcLength());
        stream->writeU8('>');
    }
    else if (!element->mContent)
    {
        if (!named)
            return;
        stream->writeString("/>", 2);
    }
    else if (!named)
    {
        for (s32 index = 0; index < depth; ++index)
            stream->writeU8('\t');
        writeXmlContentAsText_(stream, element, element->_90, work_buffer_size, heap);
    }
    else
    {
        stream->writeU8('>');
        writeXmlContentAsText_(stream, element, element->_90, work_buffer_size, heap);
        stream->writeString("</", 2);
        stream->writeString(element->mName, element->mName.calcLength());
        stream->writeU8('>');
    }
    stream->writeU8('\n');
}

// NON_MATCHING: entity-copy output cursor and register allocation differ.
void XmlDocument::writeXmlContentAsText_(WriteStream* stream, XmlElement* element, u32 content_type,
                                         u32 work_buffer_size, Heap* heap)
{
    if (!element->mContent)
        return;
    switch (content_type)
    {
    case 1:
    {
        stream->writeString("<![CDATA[", 9);
        stream->writeMemBlock(element->mContent, element->mContentSize);
        stream->writeString("]]>", 3);
        return;
    }

    case 2:
    {
        u32 encoded_size = element->mContentSize / 3;
        if (element->mContentSize % 3)
            ++encoded_size;
        encoded_size *= 4;
        if (encoded_size > work_buffer_size)
            return;
        auto* encoded = new (heap, -8) char[encoded_size];
        Base64::encode(encoded, element->mContent, element->mContentSize, false);
        stream->writeString("<![CDATA[<!BASE64[", 18);
        stream->writeMemBlock(encoded, encoded_size);
        stream->writeString("]>]]>", 5);
        delete[] encoded;
        return;
    }
    default:
        break;
    }

    const char* content = element->getContentString().cstr();
    const s32 length = element->getContentString().calcLength();
    s32 entity_count = 0;
    if (length > 0 && sUnk_71024bfd98 > 0)
    {
        for (s32 index = 0; index < length; ++index)
        {
            for (u32 entity = 0; entity < sUnk_71024bfd98; ++entity)
            {
                if (sUnk_71024bfd90[entity][0] == content[index])
                {
                    ++entity_count;
                    break;
                }
            }
        }
    }
    if (entity_count == 0)
    {
        stream->writeMemBlock(element->mContent, element->mContentSize);
        return;
    }

    auto* escaped = new (heap, -8) char[length + entity_count * 5];
    s32 written = 0;
    for (s32 index = 0; index < length; ++index)
    {
        char character = content[index];
        if (character == '&' && content[index + 1] == '&')
        {
            ++index;
        }
        else
        {
            for (u32 entity = 0; entity < sUnk_71024bfd98; ++entity)
            {
                if (sUnk_71024bfd90[entity][0] == character)
                {
                    escaped[written] = '&';
                    const s32 name_length = SafeString(sUnk_71024bfd60[entity]).calcLength();
                    for (s32 name_index = 0; name_index < name_length; ++name_index)
                    {
                        escaped[written + name_index + 1] = sUnk_71024bfd60[entity][name_index];
                    }
                    written += name_length + 1;
                    character = ';';
                    break;
                }
            }
        }
        escaped[written] = character;
        ++written;
    }
    escaped[written] = SafeString::cNullChar;
    stream->writeMemBlock(escaped, written);
    delete[] escaped;
}

void XmlDocument::writeXmlAttributesAsText_(WriteStream* stream, XmlElement* element)
{
    if (element->mAttributes.size() > 0)
    {
        for (auto& attribute : element->mAttributes)
        {
            stream->writeU8(' ');
            stream->writeString(attribute.mName, attribute.mName.calcLength());
            stream->writeU8('=');
            stream->writeU8('"');
            stream->writeString(attribute.mValue, attribute.mValue.calcLength());
            stream->writeU8('"');
        }
    }
}

XmlElement* XmlDocument::findElementByAbsolutePath(const SafeString& path)
{
    return mRoot ? mRoot->findElementByAbsolutePath_(path) : nullptr;
}

void XmlDocument::deleteElementRecursive_(XmlElement* element)
{
    if (!element)
        return;
    if (auto* child = element->getFirstChild())
        deleteElementRecursive_(child);
    if (auto* sibling = element->getNextSibling())
        deleteElementRecursive_(sibling);
    element->clearLinks();
    delete element;
}

}  // namespace sead
