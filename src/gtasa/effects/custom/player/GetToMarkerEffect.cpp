#include "util/EffectBase.h"
#include "util/EffectHandler.h"
#include "util/GenericUtil.h"

#include <CCheckpoints.h>
#include <CRadar.h>
#include <CStreaming.h>
#include <extensions/ScriptCommands.h>

using namespace plugin;

// TODO: (Low Priority) Think about more penalties / penalty effects?

class GetToMarkerEffect : public EffectBase
{
    int     checkpoint        = 0;
    int     coordBlip         = 0;
    CVector position          = {0.0, 0.0f, 0.0f};
    bool    positionGenerated = false;
    bool    reachedMarker     = false;

public:
    bool
    CanActivate () override
    {
        return true;
    }

    void
    OnStart (EffectInstance *inst) override
    {
        checkpoint        = 0;
        coordBlip         = 0;
        positionGenerated = false;
        reachedMarker     = false;
    }

    void
    OnEnd (EffectInstance *inst) override
    {
        if (coordBlip != 0)
        {
            CRadar::ClearBlip (coordBlip);
        }

        if (checkpoint != 0)
        {
            Command<eScriptCommands::COMMAND_DELETE_CHECKPOINT> (checkpoint);
        }

        SetPlanesLocked (false);

        CPlayerPed *player = FindPlayerPed ();
        if (!player) return;

        if (!reachedMarker)
        {
            nlohmann::json json;

            json["effectID"]    = "effect_one_hit_ko";
            json["displayName"] = "One Hit K.O. (Player)";
            json["duration"]    = 1000 * 60 * 5;
            json["subtext"]     = "Chaos Mod";

            EffectHandler::HandleFunction (json);
        }
    }

    void
    OnTick (EffectInstance *inst) override
    {
        CPlayerPed *player = FindPlayerPed ();
        if (!player || !CanActivate ()) return;

        if (!positionGenerated)
        {
            position = FindSuitableMarkerPosition (inst);

            Command<eScriptCommands::COMMAND_ADD_BLIP_FOR_COORD> (position.x,
                                                                  position.y,
                                                                  position.z,
                                                                  &coordBlip);
            CRadar::ChangeBlipScale (coordBlip, 3);
            CRadar::ChangeBlipColour (coordBlip,
                                      CRGBA (0, 150, 255, 255).ToInt ());

            Command<eScriptCommands::COMMAND_CREATE_CHECKPOINT> (
                1, position.x, position.y, position.z, position.x, position.y,
                position.z, 4.0f, &checkpoint);

            positionGenerated = true;
            return;
        }

        eBlipDisplay displayMode = GenericUtil::IsMenuActive ()
                                       ? eBlipDisplay::BLIP_DISPLAY_NEITHER
                                       : eBlipDisplay::BLIP_DISPLAY_BOTH;

        CRadar::ChangeBlipDisplay (coordBlip, displayMode);

        SetPlanesLocked (true);

        if (!Command<eScriptCommands::COMMAND_LOCATE_CHAR_ANY_MEANS_2D> (
                player, position.x, position.y, 4.0f, 4.0f, true))
            return;

        reachedMarker = true;
        inst->Disable ();
    }

    void
    SetPlanesLocked (bool locked)
    {
        for (CVehicle *vehicle : CPools::ms_pVehiclePool)
        {
            switch (vehicle->m_nVehicleSubClass)
            {
                case VEHICLE_HELI:
                case VEHICLE_PLANE:
                case VEHICLE_FHELI:
                case VEHICLE_FPLANE:
                {
                    vehicle->m_nVehicleFlags.bConsideredByPlayer = !locked;
                    continue;
                }

                default: continue;
            }
        }
    }

    CVector
    FindSuitableMarkerPosition (EffectInstance *inst)
    {
        CVector randomPosition
            = CVector (inst->Random (-3000.0f, 3000.0f),
                       inst->Random (-3000.0f, 3000.0f), 0.0f);

        Command<eScriptCommands::COMMAND_REQUEST_COLLISION> (randomPosition.x,
                                                             randomPosition.y);
        CStreaming::StreamZoneModels (&randomPosition);
        CStreaming::LoadAllRequestedModels (false);

        bool groundResult = false;
        randomPosition.z
            = CWorld::FindGroundZFor3DCoord (randomPosition.x, randomPosition.y,
                                             randomPosition.z + 250.0f,
                                             &groundResult, 0)
              + 3.0f;

        if (!groundResult) return FindSuitableMarkerPosition (inst);

        float waterLevel = 0.0f;
        Command<eScriptCommands::COMMAND_GET_WATER_HEIGHT_AT_COORDS> (
            randomPosition.x, randomPosition.y, false, &waterLevel);

        randomPosition.z = std::max (randomPosition.z, waterLevel);

        return randomPosition;
    }
};

DEFINE_EFFECT (GetToMarkerEffect, "effect_get_to_marker",
               GROUP_HEALTH | GROUP_TELEPORT);
