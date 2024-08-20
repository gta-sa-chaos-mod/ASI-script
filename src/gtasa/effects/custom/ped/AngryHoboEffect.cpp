#include "effects/OneTimeEffect.h"
#include "util/Globals.h"

#include <CStreaming.h>
#include <CCivilianPed.h>
#include <CPopulation.h>
#include <CPlayerPed.h>
#include <CWorld.h>
#include <CVector.h>
#include <CPed.h>
#include <extensions/ScriptCommands.h>
#include <CPickups.h>

#include "util/HoboManager.h"

using namespace plugin;

class SpawnAngryHobo : public OneTimeEffect
{
    int hoboType = 0; // 0 - DE, 1 - Rocket, 2 - Homing Rocket, 3 - Minigun, 4 - Spawn Hydra
    eWeaponType weaponType = WEAPON_BRASSKNUCKLE;

public:
    SpawnAngryHobo (int type) : hoboType (type) {
        switch (type) {
            case 0: weaponType = WEAPON_DESERT_EAGLE; break;
            case 1: weaponType = WEAPON_RLAUNCHER; break;
            case 2: weaponType = WEAPON_RLAUNCHER_HS; break;
            case 3: weaponType = WEAPON_MINIGUN; break;
        }
    }

    void
    OnStart (EffectInstance *inst) override
    {
        CPlayerPed *player = FindPlayerPed ();
        if (!player) return;

        CVector position
            = player->TransformFromObjectSpace (CVector (0.0f, -5.0f, 3.0f));

        int modelID = 230; // hobo
        CStreaming::RequestModel(modelID, 0);
        CStreaming::LoadAllRequestedModels(false);
        CPed *ped = new CCivilianPed(PED_TYPE_CIVMALE, modelID);
        if (ped) {
            ped->SetCharCreatedBy(2); // SCM Ped cannot be Removed!
            ped->SetPosn(position);
            ped->SetOrientation(0.0f, 0.0f, 0.0f);
            CWorld::Add(ped);
            ped->m_nWeaponShootingRate = 99999;
            ped->m_nWeaponAccuracy = 99999;
            ped->m_fHealth = hoboType == 3 ? 5000.0f : 1000.0f;

            // Only Explosion and Ramp can kill normal hobo
            ped->m_nPhysicalFlags.bBulletProof = true;
            ped->m_nPhysicalFlags.bFireProof = true;
            ped->m_nPhysicalFlags.bMeleeProof = true;
            if(hoboType >= 1) {
                ped->m_nPhysicalFlags.bExplosionProof = true;
            }
            if(hoboType >= 2) {
                ped->m_nPhysicalFlags.bCollisionProof = true;
            }
            // How to Kill the Undead Hobo (Collsion proof and Explosion Proof)?
            // 1. Delete from the game, (Need Global Variable)
            // 2. Use Parachute and use Stinklikeabee cheat to kill the hobo

            auto model = CPickups::ModelForWeapon(weaponType);
            CStreaming::RequestModel(model, 2);
            CStreaming::LoadAllRequestedModels(false);
            ped->GiveWeapon(weaponType, 99999, false);
            ped->SetCurrentWeapon(weaponType);
            CStreaming::SetModelIsDeletable(model);
            
            ped->PositionAnyPedOutOfCollision();
            Command<eScriptCommands::COMMAND_SET_CHAR_RELATIONSHIP> (ped, 4, ePedType::PED_TYPE_PLAYER1);
        }
        HoboManager::AddAngryHobo(ped);
    }
};

enum {
    ANGRY_HOBO = 0,
    CRAZY_HOBO,
    ULTRA_CRAZY_HOBO,
    GIGACHAD_HOBO,
    PILOT_HOBO // Spawn Angry Hobo with Hydra :)
};

DEFINE_EFFECT (SpawnAngryHobo, "effect_spawn_angry_hobo", 0, ANGRY_HOBO);
DEFINE_EFFECT (SpawnAngryHobo, "effect_spawn_crazy_hobo", 0, CRAZY_HOBO);
DEFINE_EFFECT (SpawnAngryHobo, "effect_spawn_ultra_crazy_hobo", 0, ULTRA_CRAZY_HOBO);
DEFINE_EFFECT (SpawnAngryHobo, "effect_spawn_gigachad_hobo", 0, GIGACHAD_HOBO);