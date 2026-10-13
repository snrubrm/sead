#pragma once

#include <heap/seadHeap.h>
#include <nn/vfx/Heap.h>

namespace sead::ptcl {

// Native stack producers 1271DE4 and 1272000 store this four-slot adapter
// followed by its borrowed SEAD heap pointer in a 0x10-byte object.
class Heap : public nn::vfx::Heap {
public:
    explicit Heap(sead::Heap* heap) : mHeap(heap) {}
    ~Heap() override = default;

    void* Alloc(size_t size, size_t alignment) override;
    void Free(void* ptr) override;

private:
    sead::Heap* mHeap;
};
static_assert(sizeof(Heap) == 0x10);

}  // namespace sead::ptcl
