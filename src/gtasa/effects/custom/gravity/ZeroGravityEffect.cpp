#include "util/EffectBase.h"
#include "util/GameUtil.h"

#include <CTimer.h>

class ZeroGravityEffect : public EffectBase
{
public:
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
        }

        for (CVehicle *vehicle : CPools::ms_pVehiclePool)
            NegateGravity (vehicle);
    }

    void
    NegateGravity (CPhysical *physical)
    {
        float negativeGravity
            = CTimer::ms_fTimeStep * physical->m_fMass * -0.008f;
        physical->ApplyMoveForce ({0, 0, -negativeGravity});
    }
};

DEFINE_EFFECT (ZeroGravityEffect, "effect_zero_gravity", GROUP_GRAVITY);