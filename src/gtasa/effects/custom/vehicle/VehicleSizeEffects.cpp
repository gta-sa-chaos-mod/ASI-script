#include "util/EffectBase.h"
#include "util/GenericUtil.h"
#include "util/GlobalRenderer.h"

// TODO: Blacklist "Highjack" vehicles as well

template <RwV3d scale, float zAdjustment = 0.0f>
class VehicleSizeEffect : public EffectBase
{
    static inline bool isOnTankingMission  = false;
    static inline bool isOnForkliftMission = false;

public:
    void
    OnStart (EffectInstance *inst) override
    {
        isOnTankingMission  = false;
        isOnForkliftMission = false;
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
        UpdateMissionChecks ();
    }

    void
    UpdateMissionChecks ()
    {
        isOnTankingMission  = false;
        isOnForkliftMission = false;

        for (auto i = CTheScripts::pActiveScripts; i; i = i->m_pNext)
        {
            if (!i->m_bIsMission || !i->m_bIsActive) continue;

            std::string missionName
                = GenericUtil::ToUpper (std::string (i->m_szName));

            if (missionName == "CAT3" || missionName == "TRUCK")
            {
                isOnTankingMission = true;
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
        if (isOnTankingMission)
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

        RwV3d translation = {0.0f, 0.0f, zAdjustment};
        RwFrameTranslate (frame, &translation, rwCOMBINEPRECONCAT);

        RwFrameScale (frame, &scale, rwCOMBINEPRECONCAT);
    }
};

// clang-format off
using VehicleSizeTinyCarsEffect = VehicleSizeEffect<RwV3d {0.5f, 0.5f, 0.5f}, -0.3f>;
DEFINE_EFFECT (VehicleSizeTinyCarsEffect, "effect_vehicle_size_tiny", 0);

using VehicleSizeSuperTinyCarsEffect = VehicleSizeEffect<RwV3d {0.1f, 0.1f, 0.1f}, -0.65f>;
DEFINE_EFFECT (VehicleSizeSuperTinyCarsEffect, "effect_vehicle_size_super_tiny", 0);

using VehicleSizeLargeCarsEffect = VehicleSizeEffect<RwV3d {2.0f, 2.0f, 2.0f}, 0.3f>;
DEFINE_EFFECT (VehicleSizeLargeCarsEffect, "effect_vehicle_size_large", 0);

using VehicleSizeWideEffect = VehicleSizeEffect<RwV3d {2.0f, 1.0f, 1.0f}>;
DEFINE_EFFECT (VehicleSizeWideEffect, "effect_vehicle_size_wide", 0);

using VehicleSizeSuperWideEffect = VehicleSizeEffect<RwV3d {3.0f, 1.0f, 1.0f}>;
DEFINE_EFFECT (VehicleSizeSuperWideEffect, "effect_vehicle_size_super_wide", 0);

using VehicleSizeTallEffect = VehicleSizeEffect<RwV3d {1.0f, 1.0f, 2.0f}, 0.3f>;
DEFINE_EFFECT (VehicleSizeTallEffect, "effect_vehicle_size_tall", 0);

using VehicleSizeLongEffect = VehicleSizeEffect<RwV3d {1.0f, 2.0f, 1.0f}>;
DEFINE_EFFECT (VehicleSizeLongEffect, "effect_vehicle_size_long", 0);

using VehicleSizePaperThinEffect = VehicleSizeEffect<RwV3d {0.01f, 1.0f, 1.0f}>;
DEFINE_EFFECT (VehicleSizePaperThinEffect, "effect_vehicle_size_paper_thin", 0);

using VehicleSizeFlatEffect = VehicleSizeEffect<RwV3d {1.00f, 1.0f, 0.01f}>;
DEFINE_EFFECT (VehicleSizeFlatEffect, "effect_vehicle_size_flat", 0);
// clang-format on