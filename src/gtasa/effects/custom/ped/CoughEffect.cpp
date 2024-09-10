#include "util/EffectBase.h"
#include "util/GenericUtil.h"

#include <extensions/ScriptCommands.h>

using namespace plugin;

class CoughEffect : public EffectBase
{
private:
    int       timer             = 0;
    const int COUGH_TIME        = 3000;
    const int MAX_WAIT_TIME_SEC = 8;
    const int MIN_WAIT_TIME_SEC = 6;
    bool      isPlayerOnly      = false;

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

        if (vehicle)
        {
            const int vehicleId   = vehicle->m_nModelIndex;
            auto      vehicleType = CModelInfo::IsVehicleModelType (vehicleId);
            bool      canSpin     = false;
            switch (vehicleType)
            {
                case VEHICLE_AUTOMOBILE:
                case VEHICLE_BIKE: canSpin = true; break;
                default: break;
            }
            canSpin &= vehicle->m_pDriver == ped;
            canSpin &= vehicle->m_vecMoveSpeed.Magnitude () > 0.15f;
            canSpin &= inst->Random (0, 100000) % 2 == 0;

            if (canSpin)
            {
                auto speed = (inst->Random (0, 1) ? 0.025f : -0.025f);
                vehicle->m_vecTurnSpeed.z += speed;
            }
        }

        bool canPlayAnim = Command<Commands::IS_CHAR_ON_FOOT> (ped);
        canPlayAnim &= !ped->m_nPedFlags.bInVehicle;
        canPlayAnim &= !Command<Commands::IS_CHAR_IN_WATER> (ped);
        if (ped == FindPlayerPed ())
        {
            canPlayAnim &= !Command<Commands::IS_PLAYER_USING_JETPACK> (0);
            canPlayAnim &= GameUtil::IsPlayerSafe ();
        }

        if (canPlayAnim)
        {
            Command<eScriptCommands::COMMAND_TASK_PLAY_ANIM_NON_INTERRUPTABLE> (
                ped, "gas_cwr", "ped", 4.0, 1, 1, 1, 0, COUGH_TIME);
        }
        ped->m_fHealth -= ped->m_fMaxHealth * 0.03f;
        ped->m_fHealth = std::max (0.0f, ped->m_fHealth);

        int res        = 0;
        int speechBank = 340;
        Command<eScriptCommands::COMMAND_SET_CHAR_SAY_CONTEXT_IMPORTANT> (
            ped, speechBank, 1, 1, 1, &res);
    }
};

DEFINE_EFFECT (CoughEffect, "effect_cough_player",
               GROUP_HEALTH | GROUP_CONTROLS, true);
DEFINE_EFFECT (CoughEffect, "effect_cough_everyone",
               GROUP_HEALTH | GROUP_CONTROLS, false);