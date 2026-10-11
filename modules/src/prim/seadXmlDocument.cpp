#include <prim/seadXmlDocument.h>
#include <prim/seadXmlElement.h>

#include <basis/seadNew.h>
#include <heap/seadHeapMgr.h>

namespace sead
{

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
