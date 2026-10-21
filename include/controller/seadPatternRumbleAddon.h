#pragma once

#include "controller/seadControllerAddon.h"

namespace sead
{
class PatternRumbleAddon : public ControllerAddon
{
    SEAD_RTTI_OVERRIDE(PatternRumbleAddon, ControllerAddon)

public:
    explicit PatternRumbleAddon(Controller* controller);
    ~PatternRumbleAddon() override = default;

    bool calc() override;

protected:
    virtual void startRumbleImpl_() = 0;
    virtual void stopRumbleImpl_() = 0;

public:
    bool isPatternEnable() const;
    void startPattern(const char* pattern, u32 duration);
    void stopPattern();

protected:
    const char* mPattern;
    u32 mPatternIdx;
    u32 mPatternDuration;
    // Ctor 0x7100b1b7a0 clears this byte and both pattern setters set it (startPattern
    // 0x7100b1b7dc / stopPattern 0x7100b1b7f0); its meaning is not recovered, so it keeps the
    // offset name. calc() never reads it.
    bool _38;
};
#ifdef cafe
static_assert(sizeof(PatternRumbleAddon) == 0x20, "sead::PatternRumbleAddon size mismatch");
#endif  // cafe

}  // namespace sead
