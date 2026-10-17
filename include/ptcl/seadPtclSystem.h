#pragma once

#include <nn/util/MathTypes.h>
#include <nn/vfx/System.h>

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

private:
    u8 _17b0[0x2268 - 0x17b0];
    nn::util::Matrix4x3fType mCameraMatrix;
    u8 _2298[0x29b0 - 0x2298];
};
static_assert(sizeof(PtclSystem) == 0x29b0);

}  // namespace sead::ptcl
