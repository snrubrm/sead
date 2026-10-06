#pragma once

#include "heap/seadHeap.h"

namespace sead
{
/// A heap that allocates blocks of a fixed size.
class UnitHeap : public Heap
{
    SEAD_RTTI_OVERRIDE(UnitHeap, Heap)
public:
    static UnitHeap* create(size_t size, const SafeString& name, u32 unit_size, s32 alignment,
                            Heap* parent, bool enable_lock = false);
    static UnitHeap* tryCreate(size_t size, const SafeString& name, u32 unit_size, s32 alignment,
                               Heap* parent, bool enable_lock = false);

    void destroy() override;
    size_t adjust() override;
    void* tryAlloc(size_t size, s32 alignment) override;
    void free(void* ptr) override;
    void* resizeFront(void* ptr, size_t size) override;
    void* resizeBack(void* ptr, size_t size) override;
    void freeAll() override;
    uintptr_t getStartAddress() const override;
    uintptr_t getEndAddress() const override;
    size_t getSize() const override;
    size_t getFreeSize() const override;
    size_t getMaxAllocatableSize(int alignment) const override;
    bool isInclude(const void* ptr) const override;
    bool isEmpty() const override;
    bool isFreeable() const override;
    bool isResizable() const override;
    bool isAdjustable() const override;
    void dump() const override;
    void dumpYAML(WriteStream& stream, int indent) const override;
    void genInformation_(hostio::Context* context) override;

protected:
    UnitHeap(const SafeString& name, Heap* parent, void* address, size_t size, u32 unit_size,
             bool enable_lock);
    ~UnitHeap() override;

    void initialize_(s32 alignment, bool unk, Heap* parent);
    void initFreeList_();

    u32 mUnitSize;
    /// The memory the units are carved out of (the heap object is at the start of the heap).
    void* mUnitAreaStart;
    size_t mUnitAreaSize;
    size_t mFreeSize;
    /// The first free unit: every free unit starts with a pointer to the next free unit.
    void** mFreeList;
    void* _100;
};
static_assert(sizeof(UnitHeap) == 0x108, "sead::UnitHeap size mismatch");

}  // namespace sead
