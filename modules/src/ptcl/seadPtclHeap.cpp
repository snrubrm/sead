#include <ptcl/seadPtclHeap.h>

namespace sead::ptcl {

void* Heap::Alloc(size_t size, size_t alignment) {
    return mHeap->tryAlloc(size, alignment);
}

void Heap::Free(void* ptr) {
    mHeap->free(ptr);
}

}  // namespace sead::ptcl
