#include <basis/seadRawPrint.h>
#include <nn/diag.h>

namespace sead::system
{
// 0x7100aeef54: the release library waits in place while a debugger is attached and aborts with
// an empty message otherwise.
// NON_MATCHING: clang tail-calls the eight-byte-shorter AbortImpl (ldp + b) where the original
// keeps the frame teardown after the call (bl + ldp + ret).
void Halt()
{
    if (nn::diag::IsDebuggerAttached())
    {
        for (;;)
        {
        }
    }

    nn::diag::detail::AbortImpl("", "", "", 0);
}
}  // namespace sead::system
