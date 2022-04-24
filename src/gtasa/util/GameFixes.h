#pragma once

#include "util/Config.h"
#include "util/EffectHandler.h"
#include "util/GameUtil.h"
#include "util/GenericUtil.h"

#include <CText.h>

using namespace plugin;

class GameFixes
{
public:
    static void
    Initialise ()
    {
        // Properly modify sound pitch and speed
        for (int address : {0x4EA03B, 0x4EF446})
        {
            patch::RedirectCall (address, Hooked_GetIsSlowMotionActive);
        }
        injector::WriteMemory (0x8CBA6C, 1.0f, true);

        // Fix Cesar Vialpando fade-out when it shouldn't happen
        patch::RedirectCall (
            0x47C7C7,
            Hooked_CRunningScript_CollectParameters); // OPCODE: 016A, Fade

        // Fix certain cheats cancelling each other out
        for (int address : {0x43934E, 0x4394C6, 0x4394F6, 0x439856})
        {
            patch::RedirectCall (address, Hooked_CheatCancelEachOther);
        }

        // Fix crash on trying to spawn a train as a boat
        patch::RedirectCall (0x501F3B, Hooked_ProcessTrainTrackSound);

        // Disable radio volume => -100 if slowmo is active
        patch::Nop (0x4EA044, 8);

        // Fix Reefer w/ Invisible Cars
        patch::Nop (0x6F14DE, 3);
    }

    static bool
    Hooked_GetIsSlowMotionActive ()
    {
        return true;
    }

    static __int16 __fastcall Hooked_CRunningScript_CollectParameters (
        CRunningScript *thisScript, void *edx, unsigned __int16 num)
    {
        __int16 result
            = CallMethodAndReturn<__int16, 0x464080, CRunningScript *> (
                thisScript, num);

        std::string missionName
            = GenericUtil::ToUpper (std::string (thisScript->m_szName));
        if (missionName == "CESAR1")
        {
            if (CTheScripts::ScriptParams[0].iParam == 0
                && CTheScripts::ScriptParams[1].iParam == 0)
            {
                CTheScripts::ScriptParams[1].iParam = 1;
            }
        }

        return result;
    }

    static char
    Hooked_CheatCancelEachOther (int id)
    {
        return 0;
    }

    static void __fastcall Hooked_ProcessTrainTrackSound (
        CAEVehicleAudioEntity *thisEntity, void *edx, cVehicleParams *params)
    {
        if (params->m_nVehicleSubclass == VEHICLE_BOAT)
        {
            switch (params->m_pVehicle->m_nModelIndex)
            {
                case MODEL_FREIGHT:
                case MODEL_FREIFLAT: return;
            }
        }

        CallMethod<0x4FA3F0> (thisEntity, params);
    }
};
