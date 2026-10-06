#pragma once

#include "container/seadRingBuffer.h"
#include "devenv/seadEnvUtil.h"
#include "heap/seadDisposer.h"
#include "prim/seadSafeString.h"

namespace sead
{
class Heap;

/// Holds the region / language the game runs with (EnvUtil reads it once the manager is initialized) and the rom type.
class RegionLanguageMgr
{
    SEAD_SINGLETON_DISPOSER(RegionLanguageMgr)

public:
    struct InitArg
    {
        /// The list of the allowed region languages (see parseRegionLanguageMaskStr_), may be null.
        const char* mask_str;
        SafeString rom_type;
        /// Path of a file with the rom type and the mask; ignored without a file device manager.
        const char* mask_file;
        u8 _20[8];
        Heap* heap;
    };

    RegionLanguageMgr() = default;
    ~RegionLanguageMgr() { delete[] mRomTypeBuffer; }

    void initialize(const InitArg& arg);
    bool isInitialized() const { return mInitialized; }
    const RegionLanguageID& getRegionLanguage() const { return mRegionLanguage; }
    const SafeString& getRomType() const { return mRomType; }

private:
    bool loadMask_(RingBuffer<RegionLanguageID>* mask, const InitArg& arg);
    bool parseRegionLanguageMaskStr_(RingBuffer<RegionLanguageID>* mask, const SafeString& str) const;
    void setRegionLanguageWithCheckMask_(RegionID region, LanguageID language,
                                         const RingBuffer<RegionLanguageID>& mask);

    RegionLanguageID mRegionLanguage;
    SafeString mRomType;
    char* mRomTypeBuffer = nullptr;
    bool mInitialized = false;
};
}  // namespace sead
