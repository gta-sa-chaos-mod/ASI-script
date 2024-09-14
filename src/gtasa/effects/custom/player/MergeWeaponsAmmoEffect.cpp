#include "util/EffectBase.h"
#include "util/hooks/HookMacros.h"

#include <cstdio>
namespace
{
enum eWeaponSlot
{
    UNARMED,
    MELEE,
    HANDGUN,
    SHOTGUN,
    SMG,
    RIFLE,
    SNIPER,
    HEAVY,
    THROWN,
    SPECIAL,
    GIFT,
    PARACHUTE,
    DETONATOR,
};
} // namespace

using namespace plugin;

class MergeWeaponsAmmoEffect : public EffectBase
{
    static inline bool *infiniteAmmoCheat = nullptr;
    static inline int   totalAmmo         = 0;
    static inline bool  isValidWeapon     = false;

public:
    void
    OnStart (EffectInstance *inst) override
    {
        infiniteAmmoCheat = reinterpret_cast<bool *> (0x969178);
        HOOK_METHOD_ARGS (inst, Hooked_Weapon_Fire,
                          bool (CWeapon *, CPed *, CVector *, CVector *,
                                CEntity *, CVector *, CVector *),
                          0x742300);
        HOOK_METHOD_ARGS (inst, Hooked_Weapon_FireFromCar,
                          bool (CWeapon *, CVehicle *, bool, bool), 0x73FA20);

        patch::RedirectCall (0x589498, Redirect_AsciiToGxtChar);
    }

    void
    OnEnd (EffectInstance *inst)
    {
        patch::RedirectCall (0x589498, AsciiToGxtChar);
    }

    void
    OnTick (EffectInstance *inst) override
    {
        auto *player = FindPlayerPed ();
        if (!player) return;

        totalAmmo = 0;
        for (int i = HANDGUN; i < SPECIAL; i++)
        {
            if (!player->m_aWeapons[i].m_eWeaponType)
            {
                continue;
            }
            totalAmmo += player->m_aWeapons[i].m_nTotalAmmo;
        }
        isValidWeapon         = false;
        auto activeWeaponSlot = player->m_nActiveWeaponSlot;
        if (activeWeaponSlot >= HANDGUN && activeWeaponSlot < SPECIAL)
        {
            isValidWeapon = true;
        }
    }

    static void
    Redirect_AsciiToGxtChar (const char *src, char *dst)
    {
        std::array<char, 32> buff;
        buff.fill (0);
        std::sprintf (buff.data (), "%d", totalAmmo);

        const char *p = isValidWeapon ? buff.data () : src;

        AsciiToGxtChar (p, dst);
    }

    static bool
    Hooked_Weapon_Fire (auto &&cb, CWeapon *thisWeapon, CPed *owner,
                        CVector *vecOrigin, CVector *_vecEffectPosn,
                        CEntity *targetEntity, CVector *vecTarget,
                        CVector *arg_14)
    {
        auto *player = FindPlayerPed ();
        if (owner != player || !player)
        {
            return cb ();
        }

        if (*infiniteAmmoCheat) // already active, skip the next logic
        {
            return cb ();
        }

        if (isValidWeapon)
        {
            int activeSlot     = player->m_nActiveWeaponSlot;
            *infiniteAmmoCheat = true; // hack to decrease ammo manually
            auto hasFire       = cb ();
            *infiniteAmmoCheat = false;

            if (hasFire)
            {
                DecreaseAmmo (activeSlot);
            }

            return hasFire;
        }

        return cb ();
    }

    static bool
    Hooked_Weapon_FireFromCar (auto &&cb, CWeapon *thisWeapon,
                               CVehicle *vehicle, bool leftSide, bool rightSide)
    {
        auto *player = FindPlayerPed ();
        if (vehicle->m_pDriver != player || !player)
        {
            return cb ();
        }

        if (*infiniteAmmoCheat) // already active, skip the next logic
        {
            return cb ();
        }

        if (isValidWeapon)
        {
            int activeSlot     = player->m_nActiveWeaponSlot;
            *infiniteAmmoCheat = true; // hack to decrease ammo manually
            auto hasFire       = cb ();
            *infiniteAmmoCheat = false;

            if (hasFire)
            {
                DecreaseAmmo (activeSlot);
            }

            return hasFire;
        }

        return cb ();
    }

    static void
    DecreaseAmmo (int slot)
    {
        auto *player = FindPlayerPed ();
        auto &weapon = player->m_aWeapons[slot];

        if (weapon.m_nTotalAmmo > 2)
        {
            weapon.m_nTotalAmmo--;
        }
        else
        {
            bool forceDecreaseAmmo = true;
            for (int i = HANDGUN; i < SPECIAL; i++)
            {
                if (!player->m_aWeapons[i].m_eWeaponType || i == slot)
                {
                    continue;
                }
                if (player->m_aWeapons[i].m_nTotalAmmo > 0)
                {
                    player->m_aWeapons[i].m_nTotalAmmo--;
                    forceDecreaseAmmo = false;
                    break;
                }
            }

            if (forceDecreaseAmmo)
            {
                weapon.m_nTotalAmmo--;
            }
        }
    }
};

DEFINE_EFFECT (MergeWeaponsAmmoEffect, "effect_merge_weapons_ammo",
               GROUP_WEAPONS);
