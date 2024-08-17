#include "util/Config.h"
#include "util/EffectBase.h"
#include "util/GameUtil.h"

#include <CGangWars.h>
#include <CTheScripts.h>

class QuickSaveEffect : public EffectBase
{
    bool didSave = false;

public:
    void
    OnStart (EffectInstance *inst) override
    {
        didSave = false;

        inst->SetIsOneTimeEffect ();
        inst->OverrideName ("Quicksaving...");
    }

    void
    OnEnd (EffectInstance *inst) override
    {
        if (didSave) return;

        inst->OverrideName ("Quicksave Failed");
    }

    void
    OnTick (EffectInstance *inst) override
    {
        CPlayerPed *player = FindPlayerPed ();
        if (player && !didSave && !CTheScripts::IsPlayerOnAMission ()
            && !CGangWars::State)
        {
            CRunningScript *script = CTheScripts::pActiveScripts->m_pNext;
            while (script != nullptr)
            {
                if (script->m_bIsMission && script->m_bIsActive) return;

                script = script->m_pNext;
            }

            bool wasInVehicle              = player->m_nPedFlags.bInVehicle;
            player->m_nPedFlags.bInVehicle = false;

            GameUtil::SaveToFile ("GTASAsf8.b");

            player->m_nPedFlags.bInVehicle = wasInVehicle;

            inst->OverrideName ("Quicksave Completed");

            didSave = true;

            inst->Disable ();
        }
    }
};

DEFINE_EFFECT (QuickSaveEffect, "effect_quicksave", 0);
