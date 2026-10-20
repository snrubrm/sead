#pragma once

#include <math/seadMatrix.h>
#include <nn/util/MathTypes.h>
#include <prim/seadSafeString.h>

namespace sead::ptcl {

// Adapter from the particle system's model enumerator interface to sead types. The upper-case
// slots (vtable 0x71024cefa0 slots 2..7 of gsys::ParticleMgr) belong to the NintendoSDK
// interface, which is not modelled; each forwards to the matching lower-case slot.
class IPtclModelEnumerator {
public:
    virtual ~IPtclModelEnumerator() = default;

    virtual int GetModelCount() const;
    virtual const char* GetModelName(int index) const;
    virtual int GetBoneCount(const char* model_name) const;
    virtual const char* GetBoneName(const char* model_name, int index) const;
    virtual void GetBoneMatrix(nn::util::neon::MatrixRowMajor4x3fType* matrix,
                               const char* model_name, int index) const;
    virtual void GetModelRootMatrix(nn::util::neon::MatrixRowMajor4x3fType* matrix,
                                    const char* model_name) const;

    virtual int getNumModel() = 0;
    virtual SafeString getModelName(int index) = 0;
    virtual int getNumBone(const SafeString& model_name) = 0;
    virtual SafeString getBoneName(const SafeString& model_name, int index) = 0;
    virtual void getBoneMtx(Matrix34f* matrix, const SafeString& model_name, int index) = 0;
    virtual void getModelRootMtx(Matrix34f* matrix, const SafeString& model_name) = 0;
};

}  // namespace sead::ptcl
