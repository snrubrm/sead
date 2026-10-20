#include "ptcl/seadPtclModelEnumerator.h"
#include "prim/seadMemUtil.h"

namespace sead::ptcl {

// The original emits these with gsys::ParticleMgr (0x7100c543e4..0x7100c5450c); the sead-side
// virtuals are non-const, so the const adapters cast constness away.

int IPtclModelEnumerator::GetModelCount() const {
    return const_cast<IPtclModelEnumerator*>(this)->getNumModel();
}

const char* IPtclModelEnumerator::GetModelName(int index) const {
    return const_cast<IPtclModelEnumerator*>(this)->getModelName(index).cstr();
}

int IPtclModelEnumerator::GetBoneCount(const char* model_name) const {
    return const_cast<IPtclModelEnumerator*>(this)->getNumBone(model_name);
}

const char* IPtclModelEnumerator::GetBoneName(const char* model_name, int index) const {
    return const_cast<IPtclModelEnumerator*>(this)->getBoneName(model_name, index).cstr();
}

void IPtclModelEnumerator::GetBoneMatrix(nn::util::neon::MatrixRowMajor4x3fType* matrix,
                                         const char* model_name, int index) const {
    Matrix34f mtx;
    const_cast<IPtclModelEnumerator*>(this)->getBoneMtx(&mtx, model_name, index);
    MemUtil::copy(matrix, &mtx, sizeof(mtx));
}

void IPtclModelEnumerator::GetModelRootMatrix(nn::util::neon::MatrixRowMajor4x3fType* matrix,
                                              const char* model_name) const {
    Matrix34f mtx;
    const_cast<IPtclModelEnumerator*>(this)->getModelRootMtx(&mtx, model_name);
    MemUtil::copy(matrix, &mtx, sizeof(mtx));
}

}  // namespace sead::ptcl
