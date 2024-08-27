#include "util/EffectBase.h"
#include "util/GenericUtil.h"

#include <CFireManager.h>

constexpr int SPAWN_FIRE_TIME       = 3000;
constexpr int IGNITE_PED_CHECK_TIME = 1200;

static bool
pointInRadius (const CVector &p1, const CVector &center, const float R)
{
    auto dx = std::abs (p1.x - center.x);
    auto dy = std::abs (p1.y - center.y);
    if ((dx + dy) <= R) return true;

    return (dx * dx + dy * dy <= R * R);
}

class SpawnFireAroundPlayerEffect : public EffectBase
{
    int                  spawnFireTimer = 0;
    int                  ignitePedTimer = 0;
    std::vector<CFire *> fireCoords;

public:
    SpawnFireAroundPlayerEffect ()
    {
        fireCoords.reserve (32);
    }

    void
    OnTick (EffectInstance *inst) override
    {
        CPlayerPed *player = FindPlayerPed ();
        if (!player) return;

        spawnFireTimer += int (GenericUtil::CalculateTick ());
        ignitePedTimer += int (GenericUtil::CalculateTick ());

        if (spawnFireTimer >= SPAWN_FIRE_TIME)
        {
            auto p = player->GetPosition ();
            p.x += inst->Random (-5.0f, 5.0f);
            p.y += inst->Random (-5.0f, 5.0f);
            p.z = CWorld::FindGroundZForCoord (p.x, p.y);
            auto *fire
                = gFireManager.StartFire (p, 1.0f, 0, nullptr, 6000, 0, 1);
            fireCoords.emplace_back (fire);
            spawnFireTimer -= SPAWN_FIRE_TIME;
        }

        if (ignitePedTimer >= IGNITE_PED_CHECK_TIME)
        {
            for (auto *fire : fireCoords)
            {
                if (!fire) continue;

                for (CPed *ped : CPools::ms_pPedPool)
                {
                    if (pointInRadius (ped->GetPosition (), fire->m_vecPosition,
                                       1.5f))
                    {
                        gFireManager.StartFire (ped, nullptr, 1.0f, 0, 500, 0);
                    }
                }
            }
            ignitePedTimer -= IGNITE_PED_CHECK_TIME;
        }
    }
};

DEFINE_EFFECT (SpawnFireAroundPlayerEffect, "effect_spawn_fire_around_player",
               0);