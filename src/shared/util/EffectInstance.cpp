#include "EffectInstance.h"

#include "util/AudioWrapper.h"
#include "util/Config.h"
#include "util/EffectBase.h"
#include "util/EffectHandler.h"
#include "util/GameUtil.h"
#include "util/GenericUtil.h"
#include "util/Globals.h"

EffectInstance::EffectInstance (EffectBase *effect) : effect (effect) {}

void
EffectInstance::Start ()
{
    if (this->effect)
    {
        std::string file = "ChaosMod/audio/";
        file.append (this->GetSoundID ());
        file.append (".ogg");

        const char *path = GAME_PATH ((char *) file.c_str ());

        PlayAudioStream (path);

        this->effect->OnStart (this);
        Globals::enabledEffects[effect->GetID ().substr (7)] = true;
    }
}

void
EffectInstance::End ()
{
    cleanupHandler.DoCleanup ();
    if (this->effect)
    {
        this->effect->OnEnd (this);
        Globals::enabledEffects[effect->GetID ().substr (7)] = false;
    }
}

void
EffectInstance::Tick ()
{
    if (effect->CanTickDown_Internal (this))
    {
        int tick = (int) round (
            GenericUtil::CalculateTick (Globals::effectTimerSpeed));
        this->remaining -= tick;
    }

    if (GetEffectRemaining () < 0) Disable ();

    if (effect && isRunning) effect->OnTick (this);
}

void
EffectInstance::ProcessScripts ()
{
    if (effect && isRunning) effect->OnProcessScripts (this);
}

std::string_view
EffectInstance::GetName () const
{
    if (effect && overrideName.length () == 0)
        return effect->GetMetadata ().name;

    return overrideName;
}

bool
EffectInstance::IsOtherEffectIncompatible (const EffectInstance &other)
{
    if (!this->effect) return false;

    auto &thisMetadata  = this->effect->GetMetadata ();
    auto &otherMetadata = other.effect->GetMetadata ();

    // Check for equal id
    if (thisMetadata.id == otherMetadata.id) return true;

    // Check for common groups
    if ((thisMetadata.groups & otherMetadata.groups).any ()) return true;

    // Check for same description/name (old Chaos code did as well)
    if (this->GetName () == other.GetName ()) return true;

    return false;
}
