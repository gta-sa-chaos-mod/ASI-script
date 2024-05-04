#include "util/EffectBase.h"
#include "util/GameUtil.h"

#include <CTimer.h>

class InvertedGravityEffect : public EffectBase
{
public:
    void
    OnStart (EffectInstance *inst) override
    {
        GameUtil::SetVehiclesToRealPhysics ();

        // Set everyone's Z position a bit higher so gravity triggers
        for (CPed *ped : CPools::ms_pPedPool)
        {
            ped->GetPosition ().z += 0.2f;
            ped->m_vecMoveSpeed.z = 0;
        }

        for (CVehicle *vehicle : CPools::ms_pVehiclePool)
        {
            vehicle->GetPosition ().z += 0.2f;
            vehicle->m_vecMoveSpeed.z = 0;
        }
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
            ApplyGravity (ped, -0.002f);
        }

        for (CVehicle *vehicle : CPools::ms_pVehiclePool)
        {
            NegateGravity (vehicle);
            ApplyGravity (vehicle, -0.002f);
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
    ApplyGravity (CPhysical *physical, float gravity)
    {
        float newGravity = CTimer::ms_fTimeStep * physical->m_fMass * -gravity;
        physical->ApplyMoveForce ({0, 0, newGravity});
    }
};

DEFINE_EFFECT (InvertedGravityEffect, "effect_inverted_gravity", GROUP_GRAVITY);