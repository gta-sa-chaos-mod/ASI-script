#include "util/EffectBase.h"
#include "util/GenericUtil.h"
#include "util/GlobalRenderer.h"

// TODO: Blacklist "Highjack" vehicles as well

using namespace plugin;

class VehicleRotationBasedOnSpeedEffect : public EffectBase
{
    static inline bool                        isOnTankingMission  = false;
    static inline bool                        isOnForkliftMission = false;
    static inline std::map<CVehicle *, RwV3d> rotationAngleMap;

public:
    void
    OnStart (EffectInstance *inst) override
    {
        isOnTankingMission  = false;
        isOnForkliftMission = false;
        rotationAngleMap.clear ();

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

        for (CVehicle *vehicle : CPools::ms_pVehiclePool)
        {
            float speed = vehicle->m_vecMoveSpeed.Magnitude ();

            rotationAngleMap[vehicle].x += speed * inst->Random (1, 5, 5);
            rotationAngleMap[vehicle].y += speed * inst->Random (1, 5, 5);
            rotationAngleMap[vehicle].z += speed * inst->Random (1, 5, 5);

            rotationAngleMap[vehicle].x
                = fmod (rotationAngleMap[vehicle].x, 360.0f);
            rotationAngleMap[vehicle].y
                = fmod (rotationAngleMap[vehicle].y, 360.0f);
            rotationAngleMap[vehicle].z
                = fmod (rotationAngleMap[vehicle].z, 360.0f);
        }

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

        RwV3d vec = rotationAngleMap[vehicle];

        RwV3d rotationX = {1.0f, 0.0f, 0.0f};
        RwFrameRotate (frame, &rotationX, vec.x, rwCOMBINEPRECONCAT);

        RwV3d rotationY = {0.0f, 1.0f, 0.0f};
        RwFrameRotate (frame, &rotationY, vec.y, rwCOMBINEPRECONCAT);

        RwV3d rotationZ = {0.0f, 0.0f, 1.0f};
        RwFrameRotate (frame, &rotationZ, vec.z, rwCOMBINEPRECONCAT);
    }
};

DEFINE_EFFECT (VehicleRotationBasedOnSpeedEffect,
               "effect_vehicle_rotation_based_on_speed", 0);
