#include "util/EffectBase.h"
#include "util/GenericUtil.h"
#include "util/hooks/HookMacros.h"

#include <set>

#include <CCamera.h>
#include <CStreaming.h>
#include <CWeaponInfo.h>

namespace
{
typedef void (*CWaterCannons_UpdateOne_f) (uint32_t vehicle, CVector *start,
                                           CVector *end);
} // namespace

class WaterCannonEffect : public EffectBase
{
private:
    const int                               PED_DAMAGE_TIME = 750;
    static inline std::set<CPed *>          pedsSet{};
    static inline int                       timer                 = 0;
    static inline CWaterCannons_UpdateOne_f WaterCannonsUpdateOne = nullptr;

public:
    void
    OnStart (EffectInstance *inst) override
    {
        if (!WaterCannonsUpdateOne)
        {
            WaterCannonsUpdateOne
                = reinterpret_cast<CWaterCannons_UpdateOne_f> (0x728CB0);
        }
        HOOK_METHOD_ARGS (inst, Hooked_CWeapon_Fire,
                          char (CWeapon *, CPed *, CVector *, CVector *,
                                CEntity *, CVector *, CVector *),
                          0x742300);

        HOOK_ARGS (inst, Hook_RwIm3DTransform,
                   void *(RwIm3DVertex *, RwUInt32, RwMatrix *, RwUInt32),
                   0x729551);

        HOOK_METHOD_ARGS (inst, Hooked_CPhysical_ApplyMoveForce,
                          void (CPhysical *, CVector), 0x729906);

        auto *player = FindPlayerPed ();
        if (!player) return;

        CStreaming::RequestModel (MODEL_FIRE_EX, 2);
        CStreaming::LoadAllRequestedModels (false);

        player->GiveWeapon (WEAPON_EXTINGUISHER, 500, 1);
        const auto *info = CWeaponInfo::GetWeaponInfo (WEAPON_EXTINGUISHER, 1);
        player->SetCurrentWeapon (info->m_nSlot);

        CStreaming::SetModelIsDeletable (MODEL_FIRE_EX);

        timer = 0;
    }

    void
    OnTick (EffectInstance *inst) override
    {
        timer += int (GenericUtil::CalculateTick ());

        if (timer >= PED_DAMAGE_TIME)
        {
            bool pedDamaged = false;
            for (auto *ped : pedsSet)
            {
                if (!ped) continue;
                ped->m_fHealth -= inst->Random (7.0f, 11.0f);
                ped->m_fHealth = std::max (0.0f, ped->m_fHealth);
                pedDamaged     = true;
            }

            auto *player = FindPlayerPed ();
            if (pedDamaged && player && inst->Random (0, 1))
            {
                plugin::Call<0x532010, eCrimeType, CEntity *, CPed *> (
                    CRIME_FIRE_WEAPON_HIT_PED, player, player);
            }

            pedsSet.clear ();

            timer -= PED_DAMAGE_TIME;
        }
    }

    static void
    Hooked_CPhysical_ApplyMoveForce (auto &&cb, CPhysical *self, CVector force)
    {
        cb ();

        auto *player = FindPlayerPed ();
        for (CPed *ped : CPools::ms_pPedPool)
        {
            if (ped->m_nPhysicalFlags.bInfiniteMass
                && ped->m_nPhysicalFlags.bDisableMoveForce)
            {
                continue;
            }
            if (ped == self && ped != player && ped->m_fHealth > 0.0f)
            {
                pedsSet.emplace (ped);
                break;
            }
        }
    }

    static void *
    Hook_RwIm3DTransform (auto &&cb, RwIm3DVertex *pVerts, RwUInt32 numVerts,
                          RwMatrix *ltm, RwUInt32 flags)
    {
        if (pVerts)
        {
            RwRGBA waterColor{240, 240, 100, 0};
            for (int i = 0; i < numVerts; i++)
            {
                RwUInt8 alpha = (pVerts[i].color >> 24) & 0xFF;

                pVerts[i].color = RwUInt32 (alpha << 24)
                                  | RwUInt32 (waterColor.red << 16)
                                  | RwUInt32 (waterColor.green << 8)
                                  | RwUInt32 (waterColor.blue);
            }
        }
        return cb ();
    }

    static char
    Hooked_CWeapon_Fire (auto &&cb, CWeapon *thisWeapon, CPed *owner,
                         CVector *vecOrigin, CVector *_vecEffectPosn,
                         CEntity *targetEntity, CVector *vecTarget,
                         CVector *arg_14)
    {
        auto *player = FindPlayerPed ();
        if (player && owner == player && WaterCannonsUpdateOne && thisWeapon
            && thisWeapon->m_eWeaponType == WEAPON_EXTINGUISHER)
        {
            auto beginPoint
                = player->TransformFromObjectSpace ({0.0f, 0.0f, -0.085f});
            float dirX = 0.75f * -std::sin (player->GetHeading ());
            float dirY = 0.75f * std::cos (player->GetHeading ());
            beginPoint.x += dirX;
            beginPoint.y += dirY;
            CVector endPoint = TheCamera.m_matrix->up;
            endPoint /= 1.5f;
            WaterCannonsUpdateOne ((uint32_t) player, &beginPoint, &endPoint);

            return 1;
        }

        return cb ();
    }
};

DEFINE_EFFECT (WaterCannonEffect, "effect_water_cannon", GROUP_WEAPONS);