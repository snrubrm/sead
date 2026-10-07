#pragma once

#include <nn/oe.h>

namespace sead
{
/// The performance (clock) configuration of the Switch. The debug functions do nothing in the retail build.
class PerformanceMgrNx
{
public:
    /// 0x7100af9bc0: a tail call into the SDK.
    static void initialize();
    static void printPerformance();
    static void printPerformanceForConfiguration_(int configuration);
    static void setCPUPerformance1122MHz();
    /// 0x7100af9bc4: remembers the configuration of the mode and sets it.
    static void setPerformanceConfiguration(nn::oe::PerformanceMode mode, nn::oe::PerformanceConfiguration config);

private:
    static nn::oe::PerformanceConfiguration sNormalConfiguration;
    static nn::oe::PerformanceConfiguration sBoostConfiguration;
};

}  // namespace sead
