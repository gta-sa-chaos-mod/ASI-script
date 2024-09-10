#include "util/EffectBase.h"
#include "util/GenericUtil.h"
#include "util/hooks/HookMacros.h"

#include <CTimeCycle.h>
#include <extensions/ScriptCommands.h>

using namespace plugin;

class WaterIsLavaEffect : public EffectBase
{
private:
    const int   DAMAGE_TICK_TIME      = 1500;
    const float HEALTH_PRECENT_DAMAGE = 0.09f;
    int         timer                 = 0;

public:
    void
    OnTick (EffectInstance *inst) override
    {
        CTimeCycle::m_CurrentColours.m_fWaterRed   = 255.0f;
        CTimeCycle::m_CurrentColours.m_fWaterGreen = 125.0f;
        CTimeCycle::m_CurrentColours.m_fWaterBlue  = 0.0f;
        CTimeCycle::m_CurrentColours.m_fWaterAlpha = 255.0f;

        timer += int (GenericUtil::CalculateTick ());
        if (timer >= DAMAGE_TICK_TIME)
        {
            for (CPed *ped : CPools::ms_pPedPool)
            {
                if (!ped || ped->m_nPhysicalFlags.bFireProof
                    || !ped->m_nPhysicalFlags.bTouchingWater
                    || !ped->m_nPhysicalFlags.bSubmergedInWater)
                    continue;

                ped->m_fHealth -= ped->m_fMaxHealth * HEALTH_PRECENT_DAMAGE;
                ped->m_fHealth = std::max (0.0f, ped->m_fHealth);

                if (ped->m_fHealth > 0.0f)
                {
                    ped->Say (0x15a, 0, 1.0f, 0, false, false);
                }
            }
            for (auto *vehicle : CPools::ms_pVehiclePool)
            {
                if (!vehicle || vehicle->m_nPhysicalFlags.bFireProof
                    || !(vehicle->m_nPhysicalFlags.bTouchingWater
                         || vehicle->m_nPhysicalFlags.bSubmergedInWater))
                    continue;

                vehicle->m_fHealth -= 1000.0 * HEALTH_PRECENT_DAMAGE;
                vehicle->m_fHealth = std::max (0.0f, vehicle->m_fHealth);
            }
            timer -= DAMAGE_TICK_TIME;
        }
    }
};

DEFINE_EFFECT (WaterIsLavaEffect, "effect_water_is_lava", 0);