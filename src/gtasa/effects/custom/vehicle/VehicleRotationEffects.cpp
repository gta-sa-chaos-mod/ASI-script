#include "util/EffectBase.h"
#include "util/GenericUtil.h"
#include "util/GlobalRenderer.h"

// TODO: Blacklist "Highjack" vehicles as well

template <RwV3d rotation, float angle, float perTick = 0.0f>
class VehicleRotationEffect : public EffectBase
{
    static inline bool  isOnTruckingMission = false;
    static inline bool  isOnForkliftMission = false;
    static inline float rotationAngle       = 0.0f;

public:
    void
    OnStart (EffectInstance *inst) override
    {
        isOnTruckingMission = false;
        isOnForkliftMission = false;
        rotationAngle       = angle;

        GlobalRenderer::RenderVehicleEvent += RenderVehicle;
    }

    void
    OnEnd (EffectInstance *inst) override
    {
        GlobalRenderer::RenderVehicleEvent -= RenderVehicle;
    }

    void
    OnTick (EffectInstance *inst) override
    {
        float tick = GenericUtil::CalculateTick (0.025f);

        rotationAngle += tick * perTick;
        rotationAngle = fmod (rotationAngle, 360.0f);

        UpdateMissionChecks ();
    }

    void
    UpdateMissionChecks ()
    {
        isOnTruckingMission = false;
        isOnForkliftMission = false;

        for (auto i = CTheScripts::pActiveScripts; i; i = i->m_pNext)
        {
            if (!i->m_bIsMission || !i->m_bIsActive) continue;

            std::string missionName
                = GenericUtil::ToUpper (std::string (i->m_szName));

            if (missionName == "CAT3" || missionName == "TRUCK"
                || missionName == "TORENO2")
            {
                isOnTruckingMission = true;
            }

            if (missionName == "RYDER2")
            {
                isOnForkliftMission = true;
            }
        }
    }

    static bool
    IsVehicleModelValid (int modelId)
    {
        if (isOnTruckingMission)
        {
            switch (modelId)
            {
                case 403: // Linerunner
                case 514: // Tanker
                case 515: // Roadtrain
                    return false;

                default: break;
            }
        }

        if (isOnForkliftMission)
        {
            switch (modelId)
            {
                case 530: // Forklift
                    return false;

                default: break;
            }
        }

        return true;
    }

    static void
    RenderVehicle (CVehicle *vehicle, RwFrame *frame)
    {
        if (!IsVehicleModelValid (vehicle->m_nModelIndex)) return;

        int offset = perTick != 0.0f ? (int) vehicle % 360 : 0;

        RwFrameRotate (frame, &rotation, rotationAngle + offset,
                       rwCOMBINEPRECONCAT);
    }
};

// clang-format off
using VehicleRotationBackwardsEffect = VehicleRotationEffect<RwV3d {0.0f, 0.0f, 1.0f}, 180.0f>;
DEFINE_EFFECT (VehicleRotationBackwardsEffect, "effect_vehicle_rotation_backwards", 0);

using VehicleRotationFlippedEffect = VehicleRotationEffect<RwV3d {0.0f, 1.0f, 0.0f}, 180.0f>;
DEFINE_EFFECT (VehicleRotationFlippedEffect, "effect_vehicle_rotation_flipped", 0);

using VehicleRotationContinuousXEffect = VehicleRotationEffect<RwV3d {1.0f, 0.0f, 0.0f}, 0.0f, -5.0f>;
DEFINE_EFFECT (VehicleRotationContinuousXEffect, "effect_vehicle_rotation_continuous_x", 0);

using VehicleRotationContinuousYEffect = VehicleRotationEffect<RwV3d {0.0f, 1.0f, 0.0f}, 0.0f, 5.0f>;
DEFINE_EFFECT (VehicleRotationContinuousYEffect, "effect_vehicle_rotation_continuous_y", 0);

using VehicleRotationContinuousZEffect = VehicleRotationEffect<RwV3d {0.0f, 0.0f, 1.0f}, 0.0f, 5.0f>;
DEFINE_EFFECT (VehicleRotationContinuousZEffect, "effect_vehicle_rotation_continuous_z", 0);
// clang-format on