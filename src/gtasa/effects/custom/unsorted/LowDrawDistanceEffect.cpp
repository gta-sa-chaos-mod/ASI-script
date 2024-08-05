#include "util/EffectBase.h"

#include <CTimeCycle.h>

class LowDrawDistanceEffect : public EffectBase
{
public:
    void
    OnTick (EffectInstance *inst) override
    {
        CTimeCycle::m_CurrentColours.m_fFarClip = 200.0f;
    }
};

DEFINE_EFFECT (LowDrawDistanceEffect, "effect_low_draw_distance", GROUP_VISION);