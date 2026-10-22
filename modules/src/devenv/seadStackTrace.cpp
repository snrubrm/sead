#include "devenv/seadStackTrace.h"

#include <nn/diag.h>

namespace sead
{
StackTraceBase::StackTraceBase() = default;

void StackTraceBase::trace(const void*)
{
    clear_();
    uintptr_t addresses[128];
    size_t count = nn::diag::GetBacktrace(addresses, 128);
    for (size_t i = 0; i < count; ++i) {
        if (isFull_())
            break;
        push_(addresses[i]);
    }
}

// NON_MATCHING: the final unsigned comparison reverses operand order.
bool StackSymbolResolver::isIllegalSymbolAddress(uintptr_t symbol_address, uintptr_t address)
{
    if (symbol_address == 0)
        return true;
    const uintptr_t size = nn::diag::GetSymbolSize(symbol_address);
    return address - symbol_address > size;
}

}
