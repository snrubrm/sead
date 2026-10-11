#pragma once

#include <prim/seadSafeString.h>

namespace sead
{
class Heap;
class XmlElement;

class XmlUtil
{
public:
    static XmlElement* createBackSiblingElement(XmlElement* element, Heap* heap);
    static XmlElement* createBackChildElement(XmlElement* element, Heap* heap);
    static XmlElement* createBackSiblingAndSetupElement(XmlElement* element, const SafeString& name,
                                                       const SafeString& content, Heap* heap);
};
}  // namespace sead
