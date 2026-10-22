#pragma once

#include <nn/util/MathTypes.h>
#include <nn/vfx/System.h>
#include <ptcl/seadPtclHeap.h>
#include <thread/seadCriticalSection.h>

namespace sead::ptcl {

// gsys::ParticleMgr::initialize 0x7100c51694 allocates 0x29b0 bytes and invokes
// the constructor 0x7100c51c3c, which constructs nn::vfx::System and installs the
// PtclSystem vtable 0x71024cf0a0. The remaining derived members are not modelled.
class PtclSystem : public nn::vfx::System {
public:
    ~PtclSystem() override;

    // Descriptive inline accessor. Effect::updatePlayerPos 0x71011b8808 copies
    // all 48 bytes at +0x2268, or assigns an identity matrix there.
    nn::util::Matrix4x3fType& getCameraMatrix() { return mCameraMatrix; }
    const nn::util::Matrix4x3fType& getCameraMatrix() const { return mCameraMatrix; }

    // Descriptive inline accessor; name is a guess. ForestRenderer::loadModel 0x7100f06010 reads
    // the borrowed heap of the third adapter (mHeaps[2] + 8) and passes it to EffectModelMgr::entry.
    sead::Heap* getHeap(u32 index) { return mHeaps[index].getHeap(); }
    sead::Heap* getHeap(u32 index) const { return mHeaps[index].getHeap(); }

private:
    u8 _17b0[0x1848 - 0x17b0];
    // Five nn::vfx::Heap adapters built by the constructor 0x7100c51c3c from the config heaps
    // (vptrs at +0x1848/+0x1858/+0x1868/+0x1878/+0x1888, borrowed heaps at +8 of each).
    // ForestRenderer::loadModel 0x7100f06010 passes the third adapter's heap (+0x1870) to
    // EffectModelMgr::entry.
    /* 0x1848 */ sead::ptcl::Heap mHeaps[5];
    u8 _1898[0x2268 - 0x1898];
    nn::util::Matrix4x3fType mCameraMatrix;
    u8 _2298[0x2950 - 0x2298];
    // The destructor 0x7100c54b4c destroys this critical section, then the vfx base.
    sead::CriticalSection mCriticalSection;
    u8 _2990[0x29b0 - 0x2990];
};
static_assert(sizeof(PtclSystem) == 0x29b0);

}  // namespace sead::ptcl
