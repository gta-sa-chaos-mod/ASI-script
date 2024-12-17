#include "util/EffectBase.h"
#include "util/GameUtil.h"

class SwapSteeringAxleEffect : public EffectBase
{
public:
    void
    OnTick (EffectInstance *inst) override
    {
        auto *player = FindPlayerPed ();
        if (!player) return;

        auto *vehicle = player->m_pVehicle;

        if (vehicle && vehicle->m_pDriver == player)
        {
            const int vehicleId   = vehicle->m_nModelIndex;
            auto      vehicleType = CModelInfo::IsVehicleModelType (vehicleId);
            switch (vehicleType)
            {
                case VEHICLE_AUTOMOBILE:
                case VEHICLE_MTRUCK:
                case VEHICLE_QUAD:
                    vehicle->m_nHandlingFlags.bSteerRearwheels = 1;
                    break;
                default: break;
            }
        }
    }
};

DEFINE_EFFECT (SwapSteeringAxleEffect, "effect_swap_steering_axle",
               GROUP_HANDLING);