#include <framework/nx/seadPerformanceMgrNx.h>

namespace sead
{
nn::oe::PerformanceConfiguration PerformanceMgrNx::sNormalConfiguration = 0x20003;
nn::oe::PerformanceConfiguration PerformanceMgrNx::sBoostConfiguration = 0x10001;

// 0x7100af9bc4
void PerformanceMgrNx::setPerformanceConfiguration(nn::oe::PerformanceMode mode,
                                                   nn::oe::PerformanceConfiguration config)
{
    if (mode == nn::oe::PerformanceMode_Boost)
        sBoostConfiguration = config;
    else
        sNormalConfiguration = config;
    nn::oe::SetPerformanceConfiguration(mode, config);
}

// 0x7100af9be4
void PerformanceMgrNx::printPerformance() {}

// 0x7100af9be8
void PerformanceMgrNx::printPerformanceForConfiguration_(int configuration) {}

// 0x7100af9bec
void PerformanceMgrNx::setCPUPerformance1122MHz() {}

}  // namespace sead
