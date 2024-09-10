#include "util/CPedDamageResponseCalculator.h"
#include "util/EffectBase.h"
#include "util/hooks/HookMacros.h"

template <float dmgMult> class DeadlyBulletsEffect : public EffectBase
{
    bool isPlayerOnly = false;

public:
    DeadlyBulletsEffect (bool playerOnly) : isPlayerOnly (playerOnly) {}

    void
    OnStart (EffectInstance *inst) override
    {
        if (!isPlayerOnly)
        {
            HOOK_METHOD_ARGS (inst, Hooked_VehicleInflictDamage_everyone,
                              void (CVehicle *, CEntity *, eWeaponType, float,
                                    CVector),
                              0x6D7C90);

            HOOK_METHOD_ARGS (inst, Hooked_AccountForPedArmour_everyone,
                              void (CPedDamageResponseCalculator *, CPed *,
                                    uint8_t *),
                              0x4B5B19);
        }
        else
        {
            HOOK_METHOD_ARGS (inst, Hooked_VehicleInflictDamage_player,
                              void (CVehicle *, CEntity *, eWeaponType, float,
                                    CVector),
                              0x6D7C90);

            HOOK_METHOD_ARGS (inst, Hooked_AccountForPedArmour_player,
                              void (CPedDamageResponseCalculator *, CPed *,
                                    uint8_t *),
                              0x4B5B19);
        }
    }

    static void
    Hooked_VehicleInflictDamage_player (auto &&cb, CVehicle *self,
                                        CEntity *damager, eWeaponType type,
                                        float intensity, CVector coords)
    {
        cb ();

        if (damager != FindPlayerPed ()) return;

        if (!self || self->m_nPhysicalFlags.bBulletProof) return;

        if (type >= WEAPON_PISTOL && type <= WEAPON_SNIPERRIFLE)
        {
            float mult = std::abs (dmgMult - 1.0f);
            self->m_fHealth -= (intensity * mult);
        }
    }

    static void
    Hooked_VehicleInflictDamage_everyone (auto &&cb, CVehicle *self,
                                          CEntity *damager, eWeaponType type,
                                          float intensity, CVector coords)
    {
        cb ();

        if (!self || self->m_nPhysicalFlags.bBulletProof) return;

        if (type >= WEAPON_PISTOL && type <= WEAPON_SNIPERRIFLE)
        {
            float mult = std::abs (dmgMult - 1.0f);
            self->m_fHealth -= (intensity * mult);
        }
    }

    static void
    Hooked_AccountForPedArmour_player (auto                        &&cb,
                                       CPedDamageResponseCalculator *thisCalc,
                                       CPed *ped, uint8_t *data)
    {
        if (ped != FindPlayerPed () && thisCalc->m_weaponType >= WEAPON_PISTOL
            && thisCalc->m_weaponType <= WEAPON_SNIPERRIFLE)
        {
            thisCalc->m_fDamageFactor *= dmgMult;
        }

        cb ();
    }

    static void
    Hooked_AccountForPedArmour_everyone (auto                        &&cb,
                                         CPedDamageResponseCalculator *thisCalc,
                                         CPed *ped, uint8_t *data)
    {
        if (thisCalc->m_weaponType >= WEAPON_PISTOL
            && thisCalc->m_weaponType <= WEAPON_SNIPERRIFLE)
        {
            thisCalc->m_fDamageFactor *= dmgMult;
        }

        cb ();
    }
};

using DeadlyBulletsEffectQuad = DeadlyBulletsEffect<4.0f>;

/* clang-format off */
DEFINE_EFFECT (DeadlyBulletsEffectQuad, "effect_deadly_bullets_quad_dmg_everyone", GROUP_WEAPONS, false);
DEFINE_EFFECT (DeadlyBulletsEffectQuad, "effect_deadly_bullets_quad_dmg_player", GROUP_WEAPONS, true);
/* clang-format on */