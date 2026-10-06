#pragma once

#include "prim/seadEnum.h"
#include "prim/seadSafeString.h"

namespace sead
{
SEAD_ENUM(RegionLanguageID, JPja, USen, USes, USfr, USpt, EUen, EUes, EUfr, EUde, EUit, EUpt, EUnl, EUru, KRko, CNzh, TWzh)
SEAD_ENUM(RegionID, JP, US, EU, KR, CN, TW)
// The order is the one of EnvUtil::getLanguage's result table (indexed by RegionLanguageID).
SEAD_ENUM(LanguageID, ja, en, es, fr, de, it, pt, nl, ru, ko, zh)

class EnvUtil
{
public:
    static const SafeString& getRomType();
    static LanguageID getLanguage();
    static RegionLanguageID getRegionLanguage();
    static RegionID getRegion();
    static s32 getEnvironmentVariable(BufferedSafeString* out, const SafeString& variable);
    static s32 resolveEnvronmentVariable(BufferedSafeString* out, const SafeString& str);

    /// The region / language of the system settings (nn::oe::GetDesiredLanguage), used until RegionLanguageMgr is
    /// initialized.
    static RegionID getSystemRegion_();
    static LanguageID getSystemLanguage_();
};
}  // namespace sead
