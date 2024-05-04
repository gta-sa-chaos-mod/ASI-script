#include "util/EffectBase.h"
#include "util/GameUtil.h"

#include <CTimer.h>

class SimpleGravityEffect : public EffectBase
{
    float gravity = 0.008f;

public:
    SimpleGravityEffect (float gravity) : EffectBase ()
    {
        this->gravity = gravity;
    }

    void
    OnTick (EffectInstance *inst) override
    {
        GameUtil::SetVehiclesToRealPhysics ();

        CPlayerPed *player = FindPlayerPed ();
        if (!player) return;

        for (CPed *ped : CPools::ms_pPedPool)
        {
            if (ped == player)
            {
                // Don't apply gravity effects to the player if they are using a
                // jetpack
                if (player->m_pIntelligence->GetTaskJetPack ()) continue;
            }

            NegateGravity (ped);
            ApplyGravity (ped);
        }

        for (CVehicle *vehicle : CPools::ms_pVehiclePool)
        {
            NegateGravity (vehicle);
            ApplyGravity (vehicle);
        }
    }

    void
    NegateGravity (CPhysical *physical)
    {
        float negativeGravity
            = CTimer::ms_fTimeStep * physical->m_fMass * -0.008f;
        physical->ApplyMoveForce ({0, 0, -negativeGravity});
    }

    void
    ApplyGravity (CPhysical *physical)
    {
        float gravity
            = CTimer::ms_fTimeStep * physical->m_fMass * -this->gravity;
        physical->ApplyMoveForce ({0, 0, gravity});
    }
};

// clang-format off
DEFINE_EFFECT (SimpleGravityEffect, "effect_quarter_gravity",   GROUP_GRAVITY, 0.002f);
DEFINE_EFFECT (SimpleGravityEffect, "effect_half_gravity",      GROUP_GRAVITY, 0.004f);
DEFINE_EFFECT (SimpleGravityEffect, "effect_double_gravity",    GROUP_GRAVITY, 0.016f);
DEFINE_EFFECT (SimpleGravityEffect, "effect_quadruple_gravity", GROUP_GRAVITY, 0.032f);
DEFINE_EFFECT (SimpleGravityEffect, "effect_insane_gravity",    GROUP_GRAVITY, 0.2f);
// clang-format on