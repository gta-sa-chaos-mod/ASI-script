#pragma once

#include "util/EffectSubHandlers.h"

class EffectCrowdControlHandler : public EffectSubHandler
{
    int crowdControlId = -1;

public:
    static bool IsCrowdControlEnabled ();

    void Initialise (const nlohmann::json &data);

    int
    GetCrowdControlId () const
    {
        return crowdControlId;
    }

    void SendRetry () const;
    void SendSucceeded () const;

    // These functions return whether to continue execution
    bool HandleOnQueue () const;
    bool HandleOnAddEffect (EffectBase *effect) const;
    bool HandleOnEffectIncompatibility (bool same_effect) const;
    bool HandleOnEffectActivated (bool isOneTimeEffect) const;

    explicit
    operator bool () const
    {
        return IsCrowdControlEnabled () && crowdControlId != -1;
    }
};
