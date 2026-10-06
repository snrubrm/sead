#pragma once

#include "hostio/seadHostIONode.h"
#include "thread/seadCriticalSection.h"

namespace sead
{
class DrawLockContext : public hostio::Node
{
public:
    DrawLockContext();

    void initialize(Heap* heap);
    void lock();
    void unlock();
    void genMessage(hostio::Context* context);

private:
    [[maybe_unused]] u32 _8 = 0;
    CriticalSection mCriticalSection{};
    // Not touched by the constructor; Graphics::initialize allocates 0x60 bytes for this class.
    [[maybe_unused]] u64 _50;
    [[maybe_unused]] u64 _58;
};

}  // namespace sead
