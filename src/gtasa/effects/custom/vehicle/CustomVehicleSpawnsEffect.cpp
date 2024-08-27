// Snippets and some logic taken from the Rainbomizer mod
// https://github.com/Parik27/SA.Rainbomizer/blob/master/src/traffic.cc

#include "util/EffectBase.h"
#include "util/hooks/HookMacros.h"

#include <CCarAI.h>
#include <CCarCtrl.h>
#include <CCarGenerator.h>
#include <CLoadedCarGroup.h>
#include <CModelInfo.h>
#include <CPopulation.h>
#include <CStreaming.h>

using namespace plugin;

class CustomVehicleSpawnsEffect : public EffectBase
{
    static inline int vehicleID = 411; // Default: Infernus

public:
    void
    OnStart (EffectInstance *inst) override
    {
        vehicleID = inst->GetCustomData ().value ("vehicleID", 411);
        vehicleID = std::clamp (vehicleID, 400, 611);

        // Traffic
        HOOK_ARGS (inst, Hooked_RandomizeTrafficCars, signed int (int *),
                   0x43022A);

        // CCarCtrl::ChooseCarModelToLoad
        HOOK (inst, Hooked_RandomizeCarToLoad, signed int (int *), 0x40B4CB,
              0x40B596, 0x40B62F, 0x40ED07);

        // Disable police car spawns for the time being
        HOOK (inst, Hooked_ChoosePoliceCarModel, int (int), 0x424E20, 0x42C320,
              0x43020E, 0x430283);

        HOOK_ARGS (inst, Hooked_FixEmptyPoliceCars, void (uint8_t *, char),
                   0x42BC26, 0x42C620, 0x431EE5, 0x499CBB, 0x499D6A, 0x49A5EB,
                   0x49A85E, 0x49A9AF);

        HOOK_METHOD_ARGS (inst, Hooked_FixCopCrash,
                          CCopPed * (CCopPed *, eCopType), 0x61282F);

        // Parked Cars
        HOOK_METHOD_ARGS (inst, Hooked_RandomizeFixedSpawn,
                          void (CCarGenerator *), 0x6F3EC1);

        HOOK_METHOD (inst, Hooked_RandomizeRandomSpawn,
                     signed int (CLoadedCarGroup *, char, char), 0x6F3583);

        // Roadblocks
        HOOK_METHOD_ARGS (inst, Hooked_RandomizeRoadblocks,
                          CVehicle * (CVehicle *, int, char, char), 0x462217,
                          0x4998F0, 0x42B909);

        // Firetruck occupants
        HOOK_ARGS (inst, Hooked_FixFiretruckAndAmbulanceOccupants,
                   void (uint8_t *), 0x42BC1A, 0x42BBFB);
    }

    static void
    Hooked_FixFiretruckAndAmbulanceOccupants (auto &&cb, uint8_t *vehicle)
    {
        LoadCarModel ();

        // 13 is some ped type I think?
        // TODO: Figure out what 13 *is* and see if we can have it be emergency
        // or fireman instead - for now this should fix crashes though
        CCarCtrl::SetUpDriverAndPassengersForVehicle ((CVehicle *) vehicle, 13,
                                                      0, 0, 0, 99);
    }

    static bool
    IsPoliceModel (int model = -1)
    {
        if (model == -1) model = vehicleID;

        switch (vehicleID)
        {
            case 427: // MODEL_ENFORCER
            case 430: // MODEL_PREDATOR
            case 432: // MODEL_RHINO
            case 433: // MODEL_BARRACKS
            case 490: // MODEL_FBIRANCH
            case 523: // MODEL_COPBIKE
            case 528: // MODEL_FBITRUCK
            case 596: // MODEL_COPCARLA
            case 597: // MODEL_COPCARSF
            case 598: // MODEL_COPCARVG
            case 599: // MODEL_COPCARRU
            case 601: // MODEL_SWATVAN
                return true;
        }
        return false;
    }

    static void
    LoadCarModel ()
    {
        if (CStreamingInfo::ms_pArrayBase[vehicleID].m_nLoadState != 1)
        {
            CStreaming::RequestModel (vehicleID, 2);
            CStreaming::LoadAllRequestedModels (false);
            CStreaming::SetModelIsDeletable (vehicleID);
        }
    }

    static signed int
    Hooked_RandomizeTrafficCars (auto &&cb, int *type)
    {
        LoadCarModel ();

        cb ();

        if (IsPoliceModel ()) *type = 13; // TYPE_POLICECAR

        return vehicleID;
    }

    static signed int
    Hooked_RandomizeCarToLoad (auto &&cb)
    {
        LoadCarModel ();

        return vehicleID;
    }

    static int
    Hooked_ChoosePoliceCarModel (auto &&cb)
    {
        LoadCarModel ();

        return vehicleID;
    }

    static void
    Hooked_FixEmptyPoliceCars (auto &&cb, uint8_t *vehicle, char a3)
    {
        LoadCarModel ();

        uint16_t *modelIndex     = (uint16_t *) (vehicle + 0x22);
        uint16_t  original_index = *modelIndex;

        *modelIndex = ChoosePoliceVehicleBasedOnModel ();
        CCarAI::AddPoliceCarOccupants ((CVehicle *) vehicle, a3);

        *modelIndex = original_index; // restore original model
    }

    static int
    ChoosePoliceVehicleBasedOnModel ()
    {
        if (vehicleID != 528 && vehicleID != 601 && IsPoliceModel (vehicleID))
            return vehicleID;

        // CModelInfo::GetMaximumNumberOfPassengersFromNumberOfDoors
        int seats = CallAndReturn<int, 0x4C89B0> (vehicleID);
        switch (seats)
        {
            case 0: return 523;
            case 1: return 523;
            case 2: return 599;
            default: return 490;
        }
    }

    static CCopPed *
    Hooked_FixCopCrash (auto &&cb, CCopPed *ped, eCopType &type)
    {
        type = COP_TYPE_CITYCOP;

        for (auto i : {
                 std::make_pair (285, COP_TYPE_SWAT2),
                 std::make_pair (286, COP_TYPE_FBI),
                 std::make_pair (287, COP_TYPE_ARMY),
             })
        {
            if (CStreamingInfo::ms_pArrayBase[i.first].m_nLoadState == 1)
            {
                type = i.second;
                break;
            }
        }

        return cb ();
    }

    static int
    RandomizeCarToLoad (auto &&cb)
    {
        LoadCarModel ();

        return vehicleID;
    }

    static void
    Hooked_RandomizeFixedSpawn (auto &&cb, CCarGenerator *carGen)
    {
        short oldModel = carGen->m_nModelId;

        carGen->m_nModelId = vehicleID;
        cb ();
        carGen->m_nModelId = oldModel;
    }

    static signed int
    Hooked_RandomizeRandomSpawn (auto &&cb)
    {
        return vehicleID;
    }

    static CVehicle *
    Hooked_RandomizeRoadblocks (auto &&cb, CVehicle *vehicle, int &modelIndex,
                                char createdBy, char setupSuspensionLines)
    {
        modelIndex = vehicleID;

        if (CModelInfo::IsBoatModel (modelIndex)
            || CModelInfo::IsTrainModel (modelIndex))
            CallMethod<0x6F2940> (vehicle, modelIndex, createdBy);

        else if (CModelInfo::IsPlaneModel (modelIndex))
            CallMethod<0x6C8E20> (vehicle, modelIndex, createdBy);

        else if (CModelInfo::IsHeliModel (modelIndex))
            CallMethod<0x6C4190> (vehicle, modelIndex, createdBy);

        else if (CModelInfo::IsBikeModel (modelIndex))
            CallMethod<0x6BF430> (vehicle, modelIndex, createdBy);

        else if (CModelInfo::IsBmxModel (modelIndex))
            CallMethod<0x6BF820> (vehicle, modelIndex, createdBy);

        else if (CModelInfo::IsTrailerModel (modelIndex))
            CallMethod<0x6D03A0> (vehicle, modelIndex, createdBy);

        else if (CModelInfo::IsQuadBikeModel (modelIndex))
            CallMethod<0x6CE370> (vehicle, modelIndex, createdBy);

        else if (CModelInfo::IsMonsterTruckModel (modelIndex))
            CallMethod<0x6C8D60> (vehicle, modelIndex, createdBy);

        else if (CModelInfo::IsCarModel (modelIndex))
            CallMethod<0x6B0A90> (vehicle, modelIndex, createdBy,
                                  setupSuspensionLines);

        else
            cb ();

        return vehicle;
    }
};

DEFINE_EFFECT (CustomVehicleSpawnsEffect, "effect_custom_vehicle_spawns",
               GROUP_VEHICLE_RARITY);
