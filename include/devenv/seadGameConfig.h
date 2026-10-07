#pragma once

#include "heap/seadDisposer.h"
#include "hostio/seadHostIONode.h"
#include "prim/seadSafeString.h"

namespace sead
{
class GameConfig : public hostio::Node
{
    // The members before the singleton disposer (0x6a0) are not modelled.
    u8 _8[0x698];

    SEAD_SINGLETON_DISPOSER(GameConfig)
public:
    GameConfig();
    virtual ~GameConfig();

    static const SafeString cNodeName;

    // A flag byte at 0x81a (read through the singleton by the game's gliding / surfing rupee checks).
    bool get_81a() const { return _81a; }

protected:
    struct FileWriteCallback
    {
        virtual ~FileWriteCallback();
        virtual void save();
    };

    // Members from the singleton disposer (0x6a0 + 0x20) to the end of the object (0x840) that are not modelled
    // except for the byte at 0x81a.
    u8 _6c0[0x81a - 0x6c0];
    bool _81a;
    u8 _81b[0x840 - 0x81b];
};
static_assert(sizeof(GameConfig) == 0x840);
}  // namespace sead
