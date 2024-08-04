#include "util/CStuntJump.h"
#include "util/EffectBase.h"
#include "util/GameUtil.h"
#include "util/GenericUtil.h"
#include "util/Teleportation.h"

#include <CStreaming.h>

class RandomStuntJumpEffect : public EffectBase
{
    struct StuntJump
    {
        CVector position    = {0.0f, 0.0f, 0.0f};
        CVector orientation = {0.0f, 0.0f, 0.0f};
        float   velocity    = 1.0f;
    };

    static inline CStuntJump *&currentStuntJump = *(CStuntJump **) 0xA9A88C;

    // clang-format off
    std::vector<StuntJump> stuntJumps = {
        StuntJump({.position = {1810.96, -2513.99, 13.11}, .orientation = {-0.04, 0.00, -1.59}, .velocity = 1.25f}),
        StuntJump({.position = {2278.81, -1383.99, 23.55}, .orientation = {-0.03, 0.05, 0.04}}),
        StuntJump({.position = {2745.41, -1178.02, 68.90}, .orientation = {-0.02, 0.00, -1.55}}),
        StuntJump({.position = {2155.28, 91.73, 28.81}, .orientation = {0.14, 0.01, 1.45}, .velocity = 0.8f}),
        StuntJump({.position = {-932.63, -237.80, 38.05}, .orientation = {-0.02, 0.00, 1.29}, .velocity = 1.25f}),
        StuntJump({.position = {-2146.68, -136.14, 36.09}, .orientation = {-0.04, 0.00, 0.0}, .velocity = 0.75f}),
        StuntJump({.position = {-1922.50, 223.66, 34.17}, .orientation = {0.05, 0.00, -1.62}, .velocity = 0.75f}),
        StuntJump({.position = {-2145.05, 924.96, 79.42}, .orientation = {0.01, 0.00, -1.56}, .velocity = 1.1f}),
        StuntJump({.position = {-836.64, 2308.32, 151.39}, .orientation = {0.22, 0.00, 1.56}}),
        StuntJump({.position = {-294.16, 2662.55, 62.27}, .orientation = {-0.04, 0.01, -1.57}, .velocity = 0.85f}),
        StuntJump({.position = {2073.23, 2389.82, 49.10}, .orientation = {-0.04, 0.00, 3.14}}),
    };
    // clang-format on

    bool teleported = false;
    int  wait       = 3000;

public:
    bool
    CanActivate () override
    {
        return GameUtil::IsPlayerSafe () && Teleportation::CanTeleport ();
    }

    void
    OnStart (EffectInstance *inst) override
    {
        inst->SetDuration (1000 * 120);
        inst->SetTimerVisible (false);
        inst->SetIsOneTimeEffect ();
        wait       = 3000;
        teleported = false;
    }

    void
    OnEnd (EffectInstance *inst) override
    {
        CPlayerPed *player = FindPlayerPed ();
        if (!player) return;

        player->m_nPedFlags.CantBeKnockedOffBike = 0;
    }

    void
    OnTick (EffectInstance *inst) override
    {
        if (!CanActivate ())
        {
            inst->ResetTimer ();
            return;
        }

        CPlayerPed *player = FindPlayerPed ();
        if (player) player->m_nPedFlags.CantBeKnockedOffBike = 1;

        if (!teleported)
        {
            StuntJump randomJump
                = stuntJumps[inst->Random (0, (int) stuntJumps.size () - 1)];

            Teleport (randomJump);

            teleported = true;
            return;
        }

        wait -= (int) GenericUtil::CalculateTick ();
        if (wait > 0)
        {
            inst->ResetTimer ();
            return;
        }

        if (currentStuntJump)
            inst->ResetTimer ();
        else
            inst->Disable ();
    }

    void
    Teleport (StuntJump jump)
    {
        CPlayerPed *player = FindPlayerPed ();
        if (!player) return;

        CVehicle *playerVehicle = FindPlayerVehicle (-1, false);
        if (playerVehicle)
        {
            Command<eScriptCommands::
                        COMMAND_REMOVE_CHAR_FROM_CAR_MAINTAIN_POSITION> (
                player, playerVehicle);
        }

        Teleportation::Teleport (
            CVector (jump.position.x, jump.position.y, jump.position.z));

        CStreaming::RequestModel (522, 1);
        CStreaming::LoadAllRequestedModels (false);
        CStreaming::SetModelIsDeletable (522);

        CVehicle *stuntVehicle = GameUtil::CreateVehicle (
            522, CVector (jump.position.x, jump.position.y, jump.position.z),
            0.0f, true);
        stuntVehicle->SetOrientation (jump.orientation.x, jump.orientation.y,
                                      jump.orientation.z);

        Command<eScriptCommands::COMMAND_WARP_CHAR_INTO_CAR> (player,
                                                              stuntVehicle);
        Command<eScriptCommands::COMMAND_RESTORE_CAMERA_JUMPCUT> ();

        CMatrix *matrix = stuntVehicle->GetMatrix ();

        stuntVehicle->m_vecMoveSpeed.x = jump.velocity * matrix->up.x;
        stuntVehicle->m_vecMoveSpeed.y = jump.velocity * matrix->up.y;
        stuntVehicle->m_vecMoveSpeed.z = jump.velocity * matrix->up.z;

        teleported = true;
    }
};

DEFINE_EFFECT (RandomStuntJumpEffect, "effect_random_stunt_jump", 0);