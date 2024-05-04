#include "util/EffectBase.h"
#include "util/GameUtil.h"

#include <CCamera.h>
#include <extensions/ScriptCommands.h>

using namespace plugin;

class WarpPlayerIntoRandomVehicleEffect : public EffectBase
{
public:
    bool
    CanActivate () override
    {
        int poolSize = CPools::ms_pVehiclePool->m_nSize;

        return !CONFIG_CC_ENABLED || poolSize > 0;
    }

    void
    OnStart (EffectInstance *inst) override
    {
        inst->SetIsOneTimeEffect ();
    }

    void
    OnTick (EffectInstance *inst) override
    {
        if (!GameUtil::IsPlayerSafe ())
        {
            inst->ResetTimer ();
            return;
        }

        CPlayerPed *player  = FindPlayerPed ();
        CVehicle   *vehicle = GetRandomVehicle (inst);

        if (!IsVehicleValid (vehicle))
        {
            inst->ResetTimer ();
            return;
        }

        for (int i = 0; i < vehicle->m_nMaxPassengers; i++)
        {
            if (Command<eScriptCommands::COMMAND_IS_CAR_PASSENGER_SEAT_FREE> (
                    vehicle, i))
            {
                vehicle->m_nVehicleFlags.bHasBeenOwnedByPlayer = true;

                Command<
                    eScriptCommands::COMMAND_WARP_CHAR_INTO_CAR_AS_PASSENGER> (
                    player, vehicle, i);

                Command<eScriptCommands::COMMAND_RESTORE_CAMERA_JUMPCUT> ();

                inst->Disable ();
            }
        }

        CPad *pad = player->GetPadFromPlayer ();
        if (!pad) return;

        pad->bDisablePlayerEnterCar = false;
    }

    bool
    IsVehicleValid (CVehicle *vehicle)
    {
        CPlayerPed *player = FindPlayerPed ();

        if (!vehicle || !vehicle->CanBeDriven ()
            || vehicle->m_nStatus == STATUS_WRECKED
            || vehicle->m_pDriver == player)
        {
            return false;
        }

        return true;
    }

    CVehicle *
    GetRandomVehicle (EffectInstance *inst, int attempts = 0)
    {
        int randomNumber  = inst->Random (0, CPools::ms_pVehiclePool->m_nSize);
        CVehicle *vehicle = CPools::ms_pVehiclePool->GetAt (randomNumber);

        if (IsVehiclePointerValid (vehicle) && vehicle->m_pDriver)
            return vehicle;

        attempts += 1;
        if (attempts > 10) return nullptr;

        return GetRandomVehicle (inst, attempts);
    }
};

DEFINE_EFFECT (WarpPlayerIntoRandomVehicleEffect,
               "effect_warp_player_into_random_vehicle", 0);