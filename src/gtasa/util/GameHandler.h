#pragma once

#include "util/BoneHelper.h"
#include "util/CStuntJump.h"
#include "util/Config.h"
#include "util/GameFixes.h"
#include "util/GameUtil.h"
#include "util/GlobalHooksInstance.h"
#include "util/GlobalRenderer.h"
#include "util/Websocket.h"

#include <CAnimManager.h>
#include <CAudioEngine.h>
#include <CFileMgr.h>
#include <CReferences.h>
#include <CTheScripts.h>
#include <CTimer.h>
#include <extensions/ScriptCommands.h>

using namespace plugin;

class GameHandler
{
    static inline bool initialised = false;

    static inline bool didTryLoadAutoSave = false;

    static inline int                                   lastMissionsPassed = -1;
    static inline std::chrono::steady_clock::time_point lastSaved
        = std::chrono::steady_clock::now ();
    static inline std::chrono::steady_clock::time_point lastQuickSave
        = std::chrono::steady_clock::now ();

    static inline CStuntJump *&currentStuntJump = *(CStuntJump **) 0xA9A88C;

public:
    static void
    Initialise ()
    {
        if (initialised) return;

        // Check if the mod is already loaded / installed once
        if (injector::ReadMemory<bool> (0xBED000, true))
        {
            MessageBox (NULL,
                        "Chaos Mod is already installed - make sure it isn't "
                        "installed multiple times!",
                        "Chaos Mod Error", 0);
            exit (0);
            return;
        }

        // Write to unused memory space so other mods know that the Chaos mod is
        // installed
        injector::WriteMemory<bool> (0xBED000, true, true);

        BoneHelper::Initialise ();
        GlobalRenderer::Initialise ();
        GameFixes::Initialise ();

        // Custom save-file hook for "Slot 9"
        HOOK_ARGS (GlobalHooksInstance::Get (), Hooked_OpenFile,
                   FILE * (const char *, const char *), 0x5D0D66);

        // Make sure to disable effects / delete autosave when starting a new
        // game
        HOOK (GlobalHooksInstance::Get (), Hooked_CTheScripts_Load,
              unsigned int (), 0x5D18F0);

        // Load Game Override
        HOOK_METHOD_ARGS (GlobalHooksInstance::Get (),
                          Hooked_ProcessMenuOptions,
                          void (CMenuManager *, eMenuPage), 0x577244);

        // Overwrite CText::Get call to show custom text on "Load Game" option
        // in the menu
        HOOK_METHOD_ARGS (GlobalHooksInstance::Get (), Hooked_CText_Get,
                          char *(CText *, char *), 0x579D73);

        // Send websocket message for auto-starting
        // Also hook Start New Game menu
        HOOK_METHOD_ARGS (GlobalHooksInstance::Get (),
                          Hooked_CMenuManager_DoSettingsBeforeStartingAGame,
                          signed int (CMenuManager *), 0x573827, 0x57733B);

        // Broken parachute fix where it plays the animation but CJ can't be
        // controlled mid-air
        HOOK (GlobalHooksInstance::Get (), Hooked_BrokenParachuteFix, CPed * (),
              0x443082);

        // Can Ped Jump Out Of Car
        HOOK_METHOD_ARGS (GlobalHooksInstance::Get (), Hooked_CanPedJumpOutCar,
                          bool (CVehicle *, CPed *), 0x6D2030);

        // Fix map crash when trying to load the legend
        HOOK_ARGS (GlobalHooksInstance::Get (), Hooked_FixMapLegendCrash,
                   void (float, float, char *), 0x582DEE);

        // Hook CPhysical::CanPhysicalBeDamaged to allow cutting down of spawned
        // trees with the chainsaw
        HOOK_METHOD_ARGS (GlobalHooksInstance::Get (),
                          Hooked_CanPhysicalBeDamaged,
                          char (CPhysical *, int, char *), 0x5A0DEA);

        // Damaging tree objects with the chainsaw
        HOOK_METHOD_ARGS (GlobalHooksInstance::Get (), Hooked_ObjectDamage,
                          void (CObject *, float, RwV3d *, RwV3d *, CEntity *,
                                eWeaponType),
                          0x61D578);

        // Fix stunt jump crash when player vehicle doesn't exist anymore
        HOOK (GlobalHooksInstance::Get (), Hooked_FixStuntJumpCrash, void (),
              0x53C0C1);

        initialised = true;
    }

    static void
    ProcessGame ()
    {
        HandleAutoSave ();
        HandleQuickSave ();

        HandleVehicleToRealPhysics ();
        HandleParachutingWithoutParachuteFix ();
    }

private:
    static void
    HandleAutoSave ()
    {
        if (!CONFIG ("Chaos.AutosaveAfterMissionPassed", true)) return;

        int missionsPassed = GameUtil::GetRealMissionsPassed ();

        if (lastMissionsPassed == -1)
        {
            lastMissionsPassed = missionsPassed;
        }
        else if (lastMissionsPassed > missionsPassed)
        {
            lastMissionsPassed = missionsPassed;
        }

        auto currentTime = std::chrono::steady_clock::now ();
        if (missionsPassed > lastMissionsPassed && lastSaved < currentTime
            && !CTheScripts::IsPlayerOnAMission ())
        {
            lastMissionsPassed = missionsPassed;

            nlohmann::json json;

            json["effectID"]                     = "effect_autosave";
            json["duration"]                     = 1000 * 10;
            json["effectData"]["missionsPassed"] = missionsPassed;

            EffectHandler::HandleFunction (json);

            lastSaved = currentTime + std::chrono::milliseconds{10000};
        }
    }

    static void
    HandleQuickSave ()
    {
        if (!CONFIG ("Chaos.QuickSave", false)) return;

        auto currentTime = std::chrono::steady_clock::now ();

        if (!FrontEndMenuManager.m_bMenuActive && KeyPressed (VK_F7)
            && lastQuickSave < currentTime)
        {
            lastQuickSave = currentTime + std::chrono::milliseconds{10000};

            nlohmann::json json;

            json["effectID"] = "effect_quicksave";
            json["duration"] = 1000 * 10;

            EffectHandler::HandleFunction (json);
        }
    }

    static void
    HandleVehicleToRealPhysics ()
    {
        if (CONFIG ("Chaos.SwitchAllVehiclesToRealPhysics", true))
        {
            GameUtil::SetVehiclesToRealPhysics ();
        }
    }

    static void
    HandleParachutingWithoutParachuteFix ()
    {
        CPlayerPed *player = FindPlayerPed ();
        if (!player) return;

        bool hasParachute
            = player->m_aWeapons[player->m_nActiveWeaponSlot].m_eWeaponType
              == WEAPON_PARACHUTE;

        if (hasParachute) return;

        std::vector<std::string> anims = {
            "FALL_SKYDIVE",       "FALL_SKYDIVE_L", "FALL_SKYDIVE_R",
            "FALL_SKYDIVE_ACCEL", "PARA_OPEN",      "PARA_FLOAT",
            "PARA_DECEL",         "PARA_STEERL",    "PARA_STEERR",
        };

        for (std::string anim : anims)
        {
            if (RpAnimBlendClumpGetAssociation (player->m_pRwClump,
                                                anim.c_str ()))
            {
                player->m_pIntelligence->ClearTasks (true, false);
                break;
            }
        }
    }

    static void
    TryLoadAutoSave ()
    {
        GameUtil::LoadFromFile ("chaos_mod\\chaos_autosave.b");
    }

    static FILE *
    Hooked_OpenFile (auto &&cb, const char *&path, const char *mode)
    {
        std::string s_path (path), save_path = "GTASAsf9.b";

        size_t start_pos = s_path.find (save_path);
        if (start_pos != std::string::npos)
        {
            s_path.replace (start_pos, save_path.length (),
                            GameUtil::GetLoadFileName ().c_str ());
        }

        path = s_path.c_str ();

        return cb ();
    }

    static unsigned int
    Hooked_CTheScripts_Load (auto &&cb)
    {
        lastMissionsPassed = -1;

        return cb ();
    }

    static void
    Hooked_ProcessMenuOptions (auto &&cb, CMenuManager *thisManager,
                               eMenuPage page)
    {
        if (page == eMenuPage::MENUPAGE_LOAD_GAME)
        {
            if (KeyPressed (VK_CONTROL))
            {
                TryLoadAutoSave ();
                return;
            }
        }
        else if (page == eMenuPage::MENUPAGE_NEW_GAME_ASK)
        {
            if (CONFIG ("Chaos.DeleteAutosaveOnNewGame", true)
                || KeyPressed (VK_CONTROL))
            {
                GameUtil::DeleteAutoSave ();
            }
        }

        cb ();
    }

    static char *
    Hooked_CText_Get (auto &&cb, CText *thisText, char *&key)
    {
        std::string key_str (key);
        if (key_str == "FES_NGA")
        {
            if (CONFIG ("Chaos.DeleteAutosaveOnNewGame", true)
                || KeyPressed (VK_CONTROL))
                return (char *) "New Game (Delete Autosave)";
        }
        else if (key_str == "FES_LOA")
        {
            if (KeyPressed (VK_CONTROL)) return (char *) "Load Autosave";
        }
        else if (key_str == "FEP_STG")
        {
            if (!didTryLoadAutoSave)
            {
                didTryLoadAutoSave = true;

                if (CONFIG ("Chaos.LoadAutosaveOnGameLoad", true))
                {
                    if (!KeyPressed (VK_CONTROL)) TryLoadAutoSave ();
                }
            }
        }

        return cb ();
    }

    static signed int
    Hooked_CMenuManager_DoSettingsBeforeStartingAGame (
        auto &&cb, CMenuManager *thisManager)
    {
        if (CTimer::m_snTimeInMilliseconds < 1000 * 60)
        {
            nlohmann::json json;
            json["type"]  = "ChaosMod";
            json["state"] = "auto_start";

            Websocket::SendWebsocketMessage (json);
        }

        if (CONFIG ("Chaos.DeleteAutosaveOnNewGame", true)
            || KeyPressed (VK_CONTROL))
        {
            GameUtil::DeleteAutoSave ();
        }

        if (CONFIG ("Chaos.ClearEffectsOnNewGame", true) && !CONFIG_CC_ENABLED)
        {
            for (auto &effect : EffectHandler::GetActiveEffects ())
                effect.Disable ();

            EffectHandler::Clear ();
        }

        return cb ();
    }

    static CPed *
    Hooked_BrokenParachuteFix (auto &&cb)
    {
        int &parachuteCreationStage = GameUtil::GetGlobalVariable<int> (1497);
        int &freefallStage          = GameUtil::GetGlobalVariable<int> (1513);

        parachuteCreationStage = 0;
        freefallStage          = 0;

        return cb ();
    }

    static bool
    Hooked_CanPedJumpOutCar (auto &&cb, CVehicle *vehicle, CPed *ped)
    {
        if (!GameUtil::IsPlayerSafe ()) return cb ();

        if (vehicle->IsDriver (ped)) return cb ();

        // Get vehicle speed in km/h
        float speed = vehicle->m_vecMoveSpeed.Magnitude () * 175.0f;

        return speed > 25.0f || cb ();
    }

    // Thanks to Parik's Rainbomizer code for this
    // https://github.com/Parik27/SA.Rainbomizer/blob/master/src/blips.cc#L39
    static void
    Hooked_FixMapLegendCrash (auto &&cb, float x, float y, char *&text)
    {
        if (int (text) < 0x2000) text = (char *) "Absolute Legend.";

        cb ();
    }

    static char
    Hooked_CanPhysicalBeDamaged (auto &&cb, CPhysical *physical, int weaponType,
                                 char *unknown)
    {
        if (physical->m_nModelIndex == 708
            && weaponType != eWeaponType::WEAPON_CHAINSAW)
            return false;

        return cb ();
    }

    static void
    Hooked_ObjectDamage (auto &&cb, CObject *object, float damage,
                         RwV3d *fxOrigin, RwV3d *fxDirection, CEntity *entity,
                         eWeaponType weaponType)
    {
        cb ();

        if (object->m_nModelIndex == 708
            && weaponType == eWeaponType::WEAPON_CHAINSAW)
        {
            object->m_pObjectInfo->m_fColDamageMultiplier = 0.5f;

            if (object->m_fHealth <= 0.0f)
                Command<eScriptCommands::COMMAND_DELETE_OBJECT> (object);
        }
    }

    static void
    Hooked_FixStuntJumpCrash (auto &&cb)
    {
        if (currentStuntJump && !FindPlayerVehicle (-1, false))
        {
            currentStuntJump = nullptr;
            injector::WriteMemory<int> (0xA9A898, 0, true);
            CTimer::ms_fTimeScale = 1.0f;
            TheCamera.RestoreWithJumpCut ();
        }

        cb ();
    }
};
