#include "devenv/seadEnvUtil.h"

#include <nn/oe.h>
#include <nn/settings.h>

#include "container/seadSafeArray.h"
#include "devenv/seadRegionLanguageMgr.h"
#include "filedevice/seadFileDeviceMgr.h"
#include "prim/seadStringBuilder.h"

namespace sead
{
namespace
{
/// {region, language} of every RegionLanguageID.
const SafeArray<u8, 32> sRegionLanguageToRegionAndLanguage = {{
    RegionID::JP, LanguageID::ja,  //
    RegionID::US, LanguageID::en,  //
    RegionID::US, LanguageID::es,  //
    RegionID::US, LanguageID::fr,  //
    RegionID::US, LanguageID::pt,  //
    RegionID::EU, LanguageID::en,  //
    RegionID::EU, LanguageID::es,  //
    RegionID::EU, LanguageID::fr,  //
    RegionID::EU, LanguageID::de,  //
    RegionID::EU, LanguageID::it,  //
    RegionID::EU, LanguageID::pt,  //
    RegionID::EU, LanguageID::nl,  //
    RegionID::EU, LanguageID::ru,  //
    RegionID::KR, LanguageID::ko,  //
    RegionID::CN, LanguageID::zh,  //
    RegionID::TW, LanguageID::zh,  //
}};

/// RegionLanguageID of every language of every region (language * 6 + region), -1 if there is none.
const SafeArray<s8, 66> sLanguageRegionToRegionLanguage = {{
    // ja
    RegionLanguageID::JPja, -1, -1, -1, -1, -1,
    // en
    -1, RegionLanguageID::USen, RegionLanguageID::EUen, -1, -1, -1,
    // es
    -1, RegionLanguageID::USes, RegionLanguageID::EUes, -1, -1, -1,
    // fr
    -1, RegionLanguageID::USfr, RegionLanguageID::EUfr, -1, -1, -1,
    // de
    -1, -1, RegionLanguageID::EUde, -1, -1, -1,
    // it
    -1, -1, RegionLanguageID::EUit, -1, -1, -1,
    // pt
    -1, RegionLanguageID::USpt, RegionLanguageID::EUpt, -1, -1, -1,
    // nl
    -1, -1, RegionLanguageID::EUnl, -1, -1, -1,
    // ru
    -1, -1, RegionLanguageID::EUru, -1, -1, -1,
    // ko
    -1, -1, -1, RegionLanguageID::KRko, -1, -1,
    // zh
    -1, -1, -1, -1, RegionLanguageID::CNzh, RegionLanguageID::TWzh,
}};

inline bool isLanguage(const nn::settings::LanguageCode& code, nn::settings::Language language)
{
    return nn::settings::LanguageCode::Make(language) == code;
}

inline s32 getRegionLanguageIndex(RegionID region, LanguageID language)
{
    return sLanguageRegionToRegionLanguage[region + language * 6];
}

inline LanguageID getDefaultLanguage(RegionID region)
{
    switch (region)
    {
    case RegionID::US:
    case RegionID::EU:
        return LanguageID::en;
    case RegionID::KR:
        return LanguageID::ko;
    case RegionID::CN:
    case RegionID::TW:
        return LanguageID::zh;
    default:
        return LanguageID::ja;
    }
}
}  // namespace

SEAD_SINGLETON_DISPOSER_IMPL(RegionLanguageMgr)

// NON_MATCHING: the original has the token iteration (cutOffGetAndForward, the delimiter search) and the RegionLanguageID
// lookup expanded inline, they are calls here. The definition has to stay in this translation unit: `this` is unused, so
// the original callers (initialize / loadMask_) pass an undefined value for it, which is only done for a visible
// definition.
bool RegionLanguageMgr::parseRegionLanguageMaskStr_(RingBuffer<RegionLanguageID>* mask,
                                                    const SafeString& str) const
{
    mask->clear();

    FixedSafeString<16> token;
    const SafeString delimiter = ",";
    auto it = str.tokenBegin(delimiter);
    const auto end = str.tokenEnd(delimiter);
    while (it != end)
    {
        if (it.cutOffGetAndForward(&token) != 4)
        {
            mask->clear();
            return false;
        }

        RegionLanguageID id;
        if (!id.fromText(token))
        {
            mask->clear();
            return false;
        }

        mask->pushBack(id);
    }

    if (mask->empty())
    {
        mask->clear();
        return false;
    }
    return true;
}

void RegionLanguageMgr::initialize(const InitArg& arg)
{
    if (mInitialized)
        return;

    FixedRingBuffer<RegionLanguageID, 16> mask;
    if (!parseRegionLanguageMaskStr_(&mask, arg.mask_str))
    {
        for (auto it = RegionLanguageID::begin(); it != RegionLanguageID::end(); ++it)
            mask.pushBack(*it);
    }

    mRomType = arg.rom_type;
    if (arg.mask_file)
        loadMask_(&mask, arg);

    setRegionLanguageWithCheckMask_(EnvUtil::getSystemRegion_(), EnvUtil::getSystemLanguage_(), mask);
    mInitialized = true;
}

// NON_MATCHING: the original counts the second loop down and re-reads the element count for the final front() (here the
// loop counts up and the count stays in a register).
// volatile: the original keeps the mask value and the region of the entry in stack slots that are read again in every
// iteration (the SEAD_ENUM volatile conversion operators).
void RegionLanguageMgr::setRegionLanguageWithCheckMask_(RegionID region, LanguageID language,
                                                        const RingBuffer<RegionLanguageID>& mask)
{
    volatile RegionLanguageID id;
    s32 index = getRegionLanguageIndex(region, language);
    if (index >= 0)
    {
        id = RegionLanguageID(index);
        for (auto it = mask.begin(); it != mask.end(); ++it)
        {
            const s32 value = id;
            if (value == *it)
            {
                mRegionLanguage = value;
                return;
            }
        }
    }

    for (auto it = mask.begin(); it != mask.end(); ++it)
    {
        volatile RegionID entry_region;
        entry_region = RegionID(sRegionLanguageToRegionAndLanguage[*it * 2]);
        if (region == entry_region)
        {
            mRegionLanguage = *it;
            return;
        }
    }

    mRegionLanguage = mask.front();
}

RegionID EnvUtil::getSystemRegion_()
{
    const nn::settings::LanguageCode language = nn::oe::GetDesiredLanguage();
    if (isLanguage(language, nn::settings::Language_Japanese))
        return RegionID::JP;
    if (isLanguage(language, nn::settings::Language_English))
        return RegionID::US;
    if (isLanguage(language, nn::settings::Language_CanadianFrench))
        return RegionID::US;
    if (isLanguage(language, nn::settings::Language_LatinAmericanSpanish))
        return RegionID::US;
    if (isLanguage(language, nn::settings::Language_BritishEnglish))
        return RegionID::EU;
    if (isLanguage(language, nn::settings::Language_French))
        return RegionID::EU;
    if (isLanguage(language, nn::settings::Language_German))
        return RegionID::EU;
    if (isLanguage(language, nn::settings::Language_Italian))
        return RegionID::EU;
    if (isLanguage(language, nn::settings::Language_Spanish))
        return RegionID::EU;
    if (isLanguage(language, nn::settings::Language_Dutch))
        return RegionID::EU;
    if (isLanguage(language, nn::settings::Language_Portuguese))
        return RegionID::EU;
    if (isLanguage(language, nn::settings::Language_Russian))
        return RegionID::EU;
    if (isLanguage(language, nn::settings::Language_Korean))
        return RegionID::KR;
    if (isLanguage(language, nn::settings::Language_Chinese))
        return RegionID::CN;
    if (isLanguage(language, nn::settings::Language_SimplifiedChinese))
        return RegionID::CN;
    if (isLanguage(language, nn::settings::Language_Taiwanese))
        return RegionID::TW;
    if (isLanguage(language, nn::settings::Language_TraditionalChinese))
        return RegionID::TW;
    return RegionID::JP;
}

LanguageID EnvUtil::getSystemLanguage_()
{
    const nn::settings::LanguageCode language = nn::oe::GetDesiredLanguage();
    if (isLanguage(language, nn::settings::Language_Japanese))
        return LanguageID::ja;
    if (isLanguage(language, nn::settings::Language_English))
        return LanguageID::en;
    if (isLanguage(language, nn::settings::Language_BritishEnglish))
        return LanguageID::en;
    if (isLanguage(language, nn::settings::Language_French))
        return LanguageID::fr;
    if (isLanguage(language, nn::settings::Language_CanadianFrench))
        return LanguageID::fr;
    if (isLanguage(language, nn::settings::Language_German))
        return LanguageID::de;
    if (isLanguage(language, nn::settings::Language_Italian))
        return LanguageID::it;
    if (isLanguage(language, nn::settings::Language_Spanish))
        return LanguageID::es;
    if (isLanguage(language, nn::settings::Language_LatinAmericanSpanish))
        return LanguageID::es;
    if (isLanguage(language, nn::settings::Language_Chinese))
        return LanguageID::zh;
    if (isLanguage(language, nn::settings::Language_Taiwanese))
        return LanguageID::zh;
    if (isLanguage(language, nn::settings::Language_SimplifiedChinese))
        return LanguageID::zh;
    if (isLanguage(language, nn::settings::Language_TraditionalChinese))
        return LanguageID::zh;
    if (isLanguage(language, nn::settings::Language_Korean))
        return LanguageID::ko;
    if (isLanguage(language, nn::settings::Language_Dutch))
        return LanguageID::nl;
    if (isLanguage(language, nn::settings::Language_Portuguese))
        return LanguageID::pt;
    if (isLanguage(language, nn::settings::Language_Russian))
        return LanguageID::ru;
    return LanguageID::ja;
}

RegionID EnvUtil::getRegion()
{
    if (RegionLanguageMgr* mgr = RegionLanguageMgr::instance(); mgr && mgr->isInitialized())
        return sRegionLanguageToRegionAndLanguage[mgr->getRegionLanguage() * 2];
    return getSystemRegion_();
}

LanguageID EnvUtil::getLanguage()
{
    if (RegionLanguageMgr* mgr = RegionLanguageMgr::instance(); mgr && mgr->isInitialized())
        return sRegionLanguageToRegionAndLanguage[mgr->getRegionLanguage() * 2 + 1];
    return getSystemLanguage_();
}

RegionLanguageID EnvUtil::getRegionLanguage()
{
    if (RegionLanguageMgr* mgr = RegionLanguageMgr::instance(); mgr && mgr->isInitialized())
        return mgr->getRegionLanguage();

    RegionID region = getSystemRegion_();
    LanguageID language = getSystemLanguage_();
    s32 index = getRegionLanguageIndex(region, language);
    if (index < 0)
    {
        language = getDefaultLanguage(region);
        index = getRegionLanguageIndex(region, language);
        if (index < 0)
            index = 0;
    }
    return index;
}

const SafeString& EnvUtil::getRomType()
{
    if (RegionLanguageMgr* mgr = RegionLanguageMgr::instance(); mgr && mgr->isInitialized())
        return mgr->getRomType();
    return SafeString::cEmptyString;
}

s32 EnvUtil::getEnvironmentVariable(BufferedSafeString*, const SafeString&)
{
    return -1;
}

}  // namespace sead
