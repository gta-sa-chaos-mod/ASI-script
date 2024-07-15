#include "util/EffectBase.h"
#include "util/GameUtil.h"
#include "util/CPedDamageResponseCalculator.h"
#include "util/hooks/HookMacros.h"

#include <CTimer.h>

class SideWayGravityEffect : public EffectBase
{
private:
    CVector Force;

public:
    void
    OnStart (EffectInstance *inst) override
    {
        GameUtil::SetVehiclesToRealPhysics ();

        Force = CVector(cos(FindPlayerPed()->GetHeading()), sin(FindPlayerPed()->GetHeading()), 0.0f) * 100.0f; // Always Push to Right

        // We Set People to Invicible
        HOOK_METHOD_ARGS (inst, Hooked_KillPedsInVehicle, void (CVehicle *),
                          0x6B3950, 0x6B3D6C, 0x6BEAA3, 0x6C6F44, 0x6CCF83,
                          0x6F225A);

        // CPedDamageResponseCalculator::ComputeWillKillPed
        HOOK_METHOD (inst, Hooked_Empty,
                     void (CPedDamageResponseCalculator *, CPed *, uint8_t *,
                           char),
                     0x4B5B27);

        // Set everyone's Z position a bit higher so gravity triggers
        for (CPed *ped : CPools::ms_pPedPool)
        {
            ped->GetPosition ().z += 0.9f;
            ped->m_vecMoveSpeed.z = 0;
        }
    }

    void
    OnTick (EffectInstance *inst) override
    {
        GameUtil::SetVehiclesToRealPhysics ();

        CPlayerPed *player = FindPlayerPed ();
        if (player) player->m_fHealth = player->m_fMaxHealth;

        for (CPed *ped : CPools::ms_pPedPool)
        {
            SideWayGravity (ped, Force);
        }

        for (CVehicle *vehicle : CPools::ms_pVehiclePool)
        {
            vehicle->m_nPhysicalFlags.bCollisionProof = true;
            SideWayGravity (vehicle, Force);
        }
    }

    void 
    OnEnd (EffectInstance *inst) override
    {
        for (CVehicle *vehicle : CPools::ms_pVehiclePool)
        {
            vehicle->m_nPhysicalFlags.bCollisionProof = false;
        }
    }

    void
    SideWayGravity (CPhysical *physical, CVector force)
    {
        physical->ApplyMoveForce (force);
    }

    static void
    Hooked_KillPedsInVehicle (auto &&cb, CVehicle *thisVehicle)
    {
        thisVehicle->m_nStatus = STATUS_SIMPLE;
    }

    static void
    Hooked_Empty (auto &&cb)
    {
    }
};

DEFINE_EFFECT (SideWayGravityEffect, "effect_sideway_gravity", GROUP_GRAVITY);