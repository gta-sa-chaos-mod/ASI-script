#include "EffectHandler.h"

#include "util/Config.h"
#include "util/EffectDatabase.h"
#include "util/EffectInstance.h"
#include "util/Globals.h"

#ifdef GTASA
#include <CAudioEngine.h>
#include <eAudioEvents.h>
#endif

#include <CMenuManager.h>
#include <CTimer.h>

void
EffectHandler::Tick ()
{
    EmptyQueue ();

    for (auto &effect : timedEffects)
        effect.Tick ();

    for (auto &effect : oneTimeEffects)
        effect.Tick ();
}

void
EffectHandler::ProcessScripts ()
{
    for (auto &effect : timedEffects)
        effect.ProcessScripts ();

    for (auto &effect : oneTimeEffects)
        effect.ProcessScripts ();
}

void
EffectHandler::EmptyQueue ()
{
    if (!effectQueue.empty ())
    {
        effectQueue.front () ();
        effectQueue.pop ();
    }
}

int
EffectHandler::GetActiveEffectCount ()
{
    int count = 0;

    for (auto &effect : timedEffects)
    {
        if (effect.IsRunning ()) count++;
    }

    return count;
}

void
EffectHandler::RemoveStaleEffects (bool checkOneTimeEffects)
{
    auto &effects = checkOneTimeEffects ? oneTimeEffects : timedEffects;

    if (effects.size () > RECENT_EFFECTS)
    {
        int amountToRemove = effects.size () - RECENT_EFFECTS;

        std::set<EffectInstance *> effectsToRemove = {};

        for (int i = effects.size () - 1; i > 1; i--)
        {
            auto &effect = effects[i];

            if (effect.IsRunning ()) continue;

            effectsToRemove.insert (&effect);

            if (effectsToRemove.size () >= amountToRemove) break;
        }

        std::erase_if (effects,
                       [effectsToRemove] (EffectInstance &effect) {
                           return !effect.IsRunning ()
                                  && effectsToRemove.contains (&effect);
                       });
    }
}

template <typename _Callable, typename... _Args>
void
EffectHandler::QueueFunction (_Callable &&__f, _Args &&...__args)
{
    effectQueue.push (std::bind (std::forward<_Callable> (__f),
                                 std::forward<_Args> (__args)...));
}

void
EffectHandler::QueueEffect (EffectBase *effect, const nlohmann::json &data)
{
    if (!effect) return;

    EffectInstance::SubHandlers_t handlers (data);

    if (!handlers.HandleOnQueue ()) return;

    auto effectFunction = [=] ()
    {
        if (!handlers.HandleOnAddEffect (effect)) return;

        auto inst = effect->CreateInstance ();

        /* Check for incompatibilities with timed effects before effect
         * activation */
        for (auto &i : timedEffects)
        {
            if (!i.IsRunning ()) continue;

            if (inst.IsOtherEffectIncompatible (i))
            {
                if (!handlers.HandleOnEffectIncompatibility ()) return;

                i.Disable ();
            }
        }

        /* Check for incompatibilities with fake one-time effects before effect
         * activation */
        for (auto &i : oneTimeEffects)
        {
            if (!i.IsRunning ()) continue;

            if (inst.IsOtherEffectIncompatible (i))
            {
                bool same_effect = i.GetEffect ()->GetMetadata ().id
                                   == inst.GetEffect ()->GetMetadata ().id;

                if (!handlers.HandleOnEffectIncompatibility (same_effect))
                    return;

                i.Disable ();
            }
        }

        /* Actually activate the effect */
        if (!handlers.HandleOnEffectActivated (&inst)) return;

        inst.SetSubHandlers (handlers);
        inst.SetDuration (data["duration"]);

        if (data.contains ("effectData"))
            inst.SetCustomData (data["effectData"]);

        if (data.contains ("displayName"))
            inst.OverrideName (data["displayName"]);

        if (data.contains ("subtext")) inst.SetSubtext (data["subtext"]);

        if (CONFIG ("Chaos.PlayEffectSound", true))
        {
#ifdef GTASA
            AudioEngine.ReportFrontendAudioEvent (AE_FRONTEND_DISPLAY_INFO,
                                                  0.0f, 1.0f);
#endif
        }

        inst.Enable ();
        inst.Tick ();

        if (inst.IsOneTimeEffect ())
        {
            oneTimeEffects.push_front (std::move (inst));
            RemoveStaleEffects (true);
        }
        else
        {
            timedEffects.push_front (std::move (inst));
            RemoveStaleEffects (false);
        }
    };

    QueueFunction (effectFunction);
}

void
EffectHandler::HandleFunction (const nlohmann::json &effectData)
{
    EffectBase *effect = nullptr;

    effect = EffectDatabase::FindEffectById (effectData.at ("effectID"));

    if (effect) QueueEffect (effect, effectData);
    //     else
    // #ifdef _DEBUG
    //         MessageBox (NULL, std::string (effectData.at ("effectID")).c_str
    //         (),
    //                     "Effect not found", MB_ICONHAND);
    // #endif
}
