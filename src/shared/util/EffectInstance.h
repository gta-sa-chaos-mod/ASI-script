#pragma once

#include "util/EffectCleanupHandler.h"
#include "util/EffectCrowdControlHandler.h"
#include "util/EffectDrawHandler.h"
#include "util/EffectSubHandlers.h"
#include "util/EffectTwitchHandler.h"
#include "util/RandomHelper.h"

#include <memory>
#include <string>

class EffectBase;

class EffectInstance
{
public:
    using SubHandlers_t
        = EffectSubHandlers<EffectCrowdControlHandler, EffectTwitchHandler>;

private:
    EffectBase *effect;

    std::string overrideName;
    std::string subtext;
    std::string soundId;

    int remaining = 0;
    int duration  = 0;

    bool timerVisible    = true;
    bool isRunning       = false;
    bool isOneTimeEffect = false;

    nlohmann::json customData = {{"seed", 0}};

    EffectDrawHandler drawHandler;
    SubHandlers_t     subHandlers;

    RandomHelper         randomHelper;
    EffectCleanupHandler cleanupHandler;

public:
    EffectInstance (EffectBase *effect);

    EffectInstance (const EffectInstance &other)       = delete;
    EffectInstance (EffectInstance &&other)            = default;
    EffectInstance &operator= (EffectInstance &&other) = default;

    /// Returns whether other effect can run together with this effect.
    bool IsOtherEffectIncompatible (const EffectInstance &other);

    EffectBase *
    GetEffect ()
    {
        return this->effect;
    }

    // Setters
    void
    SetDuration (int duration)
    {
        this->duration = remaining = duration;
    }

    void
    SetIsOneTimeEffect ()
    {
        isOneTimeEffect = true;
    }

    bool
    IsOneTimeEffect ()
    {
        return isOneTimeEffect;
    }

    void
    OverrideName (std::string_view name)
    {
        overrideName = name;
    }

    void
    SetSubtext (std::string_view subtext)
    {
        this->subtext = subtext;
    }

    bool
    HasSubtext ()
    {
        return subtext.length () != 0;
    }

    std::string_view
    GetSubtext ()
    {
        return subtext;
    }

    std::string_view
    GetSoundID ()
    {
        return this->soundId;
    }

    void
    SetSoundID (std::string_view soundId)
    {
        this->soundId = soundId;
    }

    void
    SetTimerVisible (bool timerVisible)
    {
        this->timerVisible = timerVisible;
    }

    bool
    IsRunning () const
    {
        return isRunning;
    }

    bool
    IsShownOnScreen () const
    {
        return GetEffectRemaining () > 0;
    }

    bool
    DoesEffectDrawTimer () const
    {
        return IsRunning () && timerVisible;
    }

    int
    GetEffectDuration () const
    {
        return duration;
    }

    int
    GetEffectRemaining () const
    {
        return remaining;
    }

    nlohmann::json &
    GetCustomData ()
    {
        return this->customData;
    }

    void
    SetSubHandlers (const SubHandlers_t &other)
    {
        this->subHandlers = other;
    }

    template <typename T>
    T &
    GetSubhandler ()
    {
        return subHandlers.Get<T> ();
    }

    auto &
    GetCleanupHandler ()
    {
        return cleanupHandler;
    }

    void
    SetCustomData (const nlohmann::json &data)
    {
        if (data.contains ("seed")) randomHelper.SetSeed (data["seed"]);
        if (data.contains ("soundId")) this->SetSoundID (data["soundId"]);

        this->customData = data;
    }

    void
    ResetTimer ()
    {
        remaining = duration;
    }

    template <typename T>
    T
    Random (T min, T max, T amplify = 1)
    {
        return randomHelper.Random (min, max, amplify);
    }

    std::mt19937
    GetRNG ()
    {
        return randomHelper.GetRNG ();
    }

    std::string_view GetName () const;

    // Handler functions
    void Start ();
    void End ();
    void Tick ();
    void ProcessScripts ();

    /* Enable/Disable functions */
    void
    Disable ()
    {
        if (isRunning)
        {
            End ();
            isRunning = false;
        }
    };

    void
    Enable ()
    {
        if (!isRunning)
        {
            isRunning = true;
            Start ();
        }
    };

    void
    Draw (int idx, bool inset, bool drawLeft)
    {
        this->drawHandler.Draw (this, idx, inset, drawLeft);
    }

    template <typename T>
    void
    WriteMemory (uintptr_t addr, const T &value, bool addToCleanup = true,
                 bool vp = true)
    {
        T original = injector::ReadMemory<T> (addr, vp);
        injector::WriteMemory (addr, value, vp);

        if (!addToCleanup) return;

        cleanupHandler.AddFunction (
            [addr, original, vp]
            { injector::WriteMemory (addr, original, vp); });
    }

    template <typename T>
    T
    ReadMemory (uintptr_t addr, bool vp = true)
    {
        return injector::ReadMemory<T> (addr, vp);
    }
};
