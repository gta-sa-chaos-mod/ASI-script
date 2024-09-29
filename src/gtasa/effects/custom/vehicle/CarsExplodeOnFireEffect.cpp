#include "util/EffectBase.h"

class CarsExplodeOnFireEffect : public EffectBase
{
    // give some space for GhostRider to reset to 0
    static inline const float BURN_TIMER = 4900.0f;

public:
    void
    OnTick (EffectInstance *inst) override
    {
        for (auto *vehicle : CPools::ms_pVehiclePool)
        {
            if (vehicle->m_fHealth < 250.0f)
            {
                SetBurnTimer (vehicle, BURN_TIMER);
            }
        }
    }

    void
    SetBurnTimer (CVehicle *vehicle, float value)
    {
        if (!vehicle || !CPools::ms_pVehiclePool->IsObjectValid (vehicle))
        {
            return;
        }

        auto *model = CModelInfo::ms_modelInfoPtrs[vehicle->m_nModelIndex];
        if (!model) return;

        auto *vehicleModelInfo = reinterpret_cast<CVehicleModelInfo *> (model);
        if (!vehicleModelInfo) return;

        switch (vehicleModelInfo->m_nVehicleType)
        {
            case VEHICLE_BIKE:
            {
                CBike *bike = (CBike *) vehicle;
                if (IsValidTime (bike->m_fBurningTimer))
                {
                    bike->m_fBurningTimer = value;
                }
                break;
            }
            case VEHICLE_AUTOMOBILE:
            case VEHICLE_MTRUCK:
            case VEHICLE_QUAD:
            case VEHICLE_TRAILER:
            {
                CAutomobile *automobile = (CAutomobile *) vehicle;
                if (IsValidTime (automobile->m_fBurningTimer))
                {
                    automobile->m_fBurningTimer = value;
                }
                break;
            }
            case VEHICLE_BOAT:
            {
                CBoat *boat = (CBoat *) vehicle;
                if (IsValidTime (boat->m_fBurningTimer))
                {
                    boat->m_fBurningTimer = value;
                }
                break;
            }
        }

        if (vehicle->m_pTrailer && IsVehiclePointerValid (vehicle->m_pTrailer))
        {
            SetBurnTimer (vehicle->m_pTrailer, value);
        }
    }

    static bool
    IsValidTime (float time)
    {
        return time >= 0.0f && time < BURN_TIMER;
    }
};

DEFINE_EFFECT (CarsExplodeOnFireEffect, "effect_cars_explode_on_fire", 0);