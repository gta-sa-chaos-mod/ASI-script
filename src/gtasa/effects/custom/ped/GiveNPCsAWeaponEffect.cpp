#include "effects/OneTimeEffect.h"

#include <vector>

#include <CPickups.h>
#include <CStreaming.h>
#include <extensions/ScriptCommands.h>

using namespace plugin;

using weapon_list = std::initializer_list<eWeaponType>;

class GiveNPCsAWeaponEffect : public OneTimeEffect
{
private:
    std::vector<eWeaponType> weapons;

public:
    GiveNPCsAWeaponEffect (weapon_list weaponsList) : weapons (weaponsList) {}

    void
    OnStart (EffectInstance *inst) override
    {
        CPlayerPed *player = FindPlayerPed ();

        for (auto weapon : weapons)
        {
            int model = CPickups::ModelForWeapon (weapon);
            CStreaming::RequestModel (model, 2);
            CStreaming::LoadAllRequestedModels (false);

            CStreaming::SetModelIsDeletable (model);
        }

        for (CPed *ped : CPools::ms_pPedPool)
        {
            if (ped == player) continue;

            auto weapon = weapons[inst->Random (1, 100000) % weapons.size ()];
            Command<eScriptCommands::COMMAND_GIVE_WEAPON_TO_CHAR> (ped, weapon,
                                                                   9999);
            Command<eScriptCommands::COMMAND_SET_CURRENT_CHAR_WEAPON> (ped,
                                                                       weapon);
        }
    }
};

DEFINE_EFFECT (GiveNPCsAWeaponEffect, "effect_give_npcs_an_rpg", GROUP_WEAPONS,
               weapon_list{WEAPON_RLAUNCHER});

DEFINE_EFFECT (GiveNPCsAWeaponEffect, "effect_give_npcs_a_sprcan_fire_ex",
               GROUP_WEAPONS,
               weapon_list{WEAPON_SPRAYCAN, WEAPON_EXTINGUISHER});