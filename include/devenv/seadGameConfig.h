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
    // Flag bytes at 0x80a / 0x80b (Player::decreaseStaminaForActionMaybe 0x7100885bd0 skips the stamina cost
    // and the cost scaling; Unk_71008502cc::sub_71008508FC 0x71008508fc reads 0x80b).
    bool get_80a() const { return _80a; }
    bool get_80b() const { return _80b; }
    // Byte 0x80c: AutoPlacementMgr::threadFn 0x7100654e44 tests it before cmd2_c().
    bool get_80c() const { return _80c; }
    // Byte 0x816: GameScene::callSceneStartDemoAuto 0x71008bab0c and the scene-change
    // loaders 0x71007b39dc / 0x71007b9d68 test it; both compare the byte with zero.
    bool get_816() const { return _816; }
    // Byte 0x82d: read by the map static resource loader.
    bool get_82d() const { return _82d; }

    // Type of an XML-backed parameter; updateXmlNodeJustAsTreeMapParamImpl_ 0x7100af2344
    // switches on its first word and the delegate interface takes it by reference.
    enum class ParamValueType;

protected:
    struct FileWriteCallback
    {
        virtual ~FileWriteCallback();
        virtual void save();
    };

    // Members from the singleton disposer (0x6a0 + 0x20) to the end of the object (0x840) that are not modelled
    // except for the bytes at 0x80a, 0x80b and 0x81a.
    u8 _6c0[0x80a - 0x6c0];
    bool _80a;
    bool _80b;
    bool _80c;
    u8 _80d[0x816 - 0x80d];
    bool _816;
    u8 _817[0x81a - 0x817];
    bool _81a;
    u8 _81b[0x82d - 0x81b];
    bool _82d;
    u8 _82e[0x840 - 0x82e];
};
static_assert(sizeof(GameConfig) == 0x840);
}  // namespace sead
