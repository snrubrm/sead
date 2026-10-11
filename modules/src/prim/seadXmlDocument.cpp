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
