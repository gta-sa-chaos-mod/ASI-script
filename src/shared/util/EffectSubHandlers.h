#pragma once

#include <tuple>

class EffectBase;
class EffectInstance;

// Subhandler that always returns true
class EffectSubHandler
{
public:
    bool
    HandleOnQueue () const
    {
        return true;
    }

    bool
    HandleOnAddEffect (EffectBase *effect) const
    {
        return true;
    }

    bool
    HandleOnEffectIncompatibility (bool same_effect = false) const
    {
        return true;
    }

    bool
    HandleOnEffectActivated (bool isOneTimeEffect) const
    {
        return true;
    }
};

// Class to manage multiple subhandlers.
template <class... SubHandlers> class EffectSubHandlers
{
    std::tuple<SubHandlers...> handlers;

public:
    EffectSubHandlers (const nlohmann::json &effectData)
    {
        (..., (std::get<SubHandlers> (handlers).Initialise (effectData)));
    }

    EffectSubHandlers () = default;

    bool
    HandleOnQueue () const
    {
        return (... & std::get<SubHandlers> (handlers).HandleOnQueue ());
    }

    bool
    HandleOnAddEffect (EffectBase *effect) const
    {
        return (...
                & std::get<SubHandlers> (handlers).HandleOnAddEffect (effect));
    }

    bool
    HandleOnEffectIncompatibility (bool same_effect = false) const
    {
        return (
            ...
            & std::get<SubHandlers> (handlers).HandleOnEffectIncompatibility (
                same_effect));
    }

    bool
    HandleOnEffectActivated (EffectInstance *inst) const
    {
        return (
            ...
            & std::get<SubHandlers> (handlers).HandleOnEffectActivated (inst));
    }

    template <typename T>
    T &
    Get ()
    {
        return std::get<T> (handlers);
    }
};
