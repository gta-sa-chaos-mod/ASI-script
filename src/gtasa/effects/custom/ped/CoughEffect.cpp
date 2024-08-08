#include "util/EffectBase.h"
#include "util/GenericUtil.h"

#include <extensions/ScriptCommands.h>

using namespace plugin;

class CoughEffect : public EffectBase
{
private:
    int       timer              = 0;
    const int MIN_COUGH_TIME_SEC = 3;
    const int MAX_COUGH_TIME_SEC = 4;
    const int MAX_WAIT_TIME_SEC  = 7;
    const int MIN_WAIT_TIME_SEC  = 5;
    bool      isPlayerOnly       = false;

public:
    CoughEffect (bool playerOnly) : isPlayerOnly (playerOnly) {}

    void
    OnStart (EffectInstance *inst) override
    {
        timer = inst->Random (MIN_WAIT_TIME_SEC, MAX_WAIT_TIME_SEC) * 1000;
    }

    void
    OnTick (EffectInstance *inst) override
    {
        timer -= int (GenericUtil::CalculateTick ());
        if (timer > 0) return;

        timer = inst->Random (MIN_WAIT_TIME_SEC, MAX_WAIT_TIME_SEC) * 1000;
        GameUtil::SetVehiclesToRealPhysics ();

        CPlayerPed *player = FindPlayerPed ();

        if (isPlayerOnly)
        {
            if (!GameUtil::IsPlayerSafe ()) return;

            DoCoughEffect (player, inst);
        }
        else
        {
            for (CPed *ped : CPools::ms_pPedPool)
            {
                if (ped == player && !GameUtil::IsPlayerSafe ()) continue;

                if (inst->Random (0, 100000) % 3 == 0) continue;

                DoCoughEffect (ped, inst);
            }
        }
    }

    void
    DoCoughEffect (CPed *ped, EffectInstance *inst)
    {
        if (!ped) return;

        auto *vehicle = ped->m_pVehicle;
        if (vehicle && vehicle->m_pDriver == ped
            && vehicle->m_vecMoveSpeed.Magnitude () > 0.15f)
        {
            auto speed = vehicle->m_vecMoveSpeed.Magnitude ()
                         * (inst->Random (0, 1) ? 0.12f : -0.12f);
            vehicle->m_vecTurnSpeed.z = speed;
        }
        if (!ped->m_nPedFlags.bInVehicle)
        {
            int sec
                = inst->Random (MIN_COUGH_TIME_SEC, MAX_COUGH_TIME_SEC) * 1000;
            Command<eScriptCommands::COMMAND_TASK_PLAY_ANIM_NON_INTERRUPTABLE> (
                ped, "gas_cwr", "ped", 4.0, 1, 1, 1, 0, sec);
        }
        ped->m_fHealth -= ped->m_fMaxHealth * 0.02f;
        ped->m_fHealth = std::max (0.0f, ped->m_fHealth);

        int res        = 0;
        int speechBank = 340;
        Command<eScriptCommands::COMMAND_SET_CHAR_SAY_CONTEXT_IMPORTANT> (
            ped, speechBank, 1, 1, 1 & res);
    }
};

DEFINE_EFFECT (CoughEffect, "effect_cough_player",
               GROUP_HEALTH | GROUP_CONTROLS, true);
DEFINE_EFFECT (CoughEffect, "effect_cough_everyone",
               GROUP_HEALTH | GROUP_CONTROLS, false);