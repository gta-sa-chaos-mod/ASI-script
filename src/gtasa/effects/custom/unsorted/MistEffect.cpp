#include "util/EffectBase.h"

#include <CClock.h>
#include <CTimeCycle.h>
#include <CWeather.h>

class MistEffect : public EffectBase
{
private:
    int forceWeather = WEATHER_FOGGY_SF;

public:
    void
    OnStart (EffectInstance *inst) override
    {
        constexpr int weathers[]
            = {WEATHER_RAINY_SF, WEATHER_FOGGY_SF, WEATHER_RAINY_COUNTRYSIDE,
               WEATHER_SANDSTORM_DESERT};

        auto idx     = inst->Random (0, 10000) % std::size (weathers);
        forceWeather = weathers[idx];
    }

    void
    OnTick (EffectInstance *inst) override
    {
        CClock::ms_nGameClockHours   = 0;
        CClock::ms_nGameClockMinutes = 0;
        CClock::ms_nGameClockSeconds = 0;
        CWeather::ForceWeatherNow (forceWeather);
        CTimeCycle::m_CurrentColours.m_fFarClip = 150.0f;
    }
};

DEFINE_EFFECT (MistEffect, "effect_mist", GROUP_VISION | GROUP_WEATHER);