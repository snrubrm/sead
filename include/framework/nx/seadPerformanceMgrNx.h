#pragma once

namespace sead
{
/// The performance (clock) configuration of the Switch. The debug functions do nothing in the retail build.
class PerformanceMgrNx
{
public:
    /// 0x7100af9bc0 (declared only): a tail call into the SDK.
    static void initialize();
    static void printPerformance();
    static void printPerformanceForConfiguration_(int configuration);
    static void setCPUPerformance1122MHz();
};

}  // namespace sead
