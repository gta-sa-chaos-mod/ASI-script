#include "util/EffectBase.h"
#include "util/GameUtil.h"

class ExplodeAllCarsEffect : public EffectBase
{
public:
    bool
    CanActivate () override
    {
        if (!GameUtil::IsPlayerSafe ()) return false;

        for (CVehicle *vehicle : CPools::ms_pVehiclePool)
        {
            if (vehicle && vehicle->m_nStatus != STATUS_WRECKED
                && vehicle->m_fHealth > 0.0f)
                return true;
        }

        return true;
    }

    void
    OnStart (EffectInstance *inst) override
    {
        inst->SetIsOneTimeEffect ();
    }

    void
    OnTick (EffectInstance *inst) override
    {
        if (!CanActivate ()) return;

        for (CVehicle *vehicle : CPools::ms_pVehiclePool)
        {
            vehicle->m_nPhysicalFlags.bExplosionProof = false;
            vehicle->BlowUpCar (nullptr, false);
        }

        inst->Disable ();
    }
};

DEFINE_EFFECT (ExplodeAllCarsEffect, "effect_explode_all_cars", 0);