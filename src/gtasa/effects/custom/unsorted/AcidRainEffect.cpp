#include "util/EffectBase.h"
#include "util/GenericUtil.h"

#include <CModelInfo.h>
#include <CWeather.h>
#include <extensions/ScriptCommands.h>
using namespace plugin;

const int RAIN_TICK_TIME = 750;
typedef unsigned int (*GetIsCullzoneDarkAtPoint_f) (float x, float y, float z);
namespace
{
GetIsCullzoneDarkAtPoint_f    isCullZone = nullptr;
constexpr std::array<int, 12> openTopCars
    = {424, 429, 480, 485, 486, 531, 533, 539, 568, 571, 572, 575};
} // namespace

class AcidRainEffect : public EffectBase
{
private:
    eWeatherType oldWeather   = eWeatherType::WEATHER_EXTRASUNNY_LA;
    int          time         = 0;
    int          forceWeather = WEATHER_RAINY_SF;

public:
    void
    OnStart (EffectInstance *inst) override
    {
        this->oldWeather = (eWeatherType) CWeather::OldWeatherType;

        isCullZone = reinterpret_cast<GetIsCullzoneDarkAtPoint_f> (0x72d970);
        time       = 0;

        forceWeather = WEATHER_RAINY_SF;
        if (inst->Random (1, 1000) % 2 == 0)
        {
            forceWeather = WEATHER_RAINY_COUNTRYSIDE;
        }
    }

    void
    OnEnd (EffectInstance *inst) override
    {
        CWeather::ForceWeatherNow (this->oldWeather);
    }

    void
    OnTick (EffectInstance *inst) override
    {
        if (!GameUtil::IsPlayerSafe ()) return;
        auto *player = FindPlayerPed ();
        if (!player || player->m_nAreaCode) return;

        CWeather::ForceWeatherNow (forceWeather);

        time += int (GenericUtil::CalculateTick ());

        if (time >= RAIN_TICK_TIME)
        {
            for (CPed *ped : CPools::ms_pPedPool)
            {
                if (!ped || !ped->IsAlive () || !IsPedUnderRain (ped)) continue;

                if (ped != player && inst->Random (0, 10000) % 5 == 0
                    && ped->m_nCreatedBy != 2)
                {
                    auto p = ped->GetPosition ();
                    if (inst->Random (0, 10000) % 2 == 0)
                    {
                        Command<COMMAND_TASK_FLEE_POINT> (ped, p.x, p.y, p.z,
                                                          15.0f, 3000);
                    }
                    else
                    {
                        Command<COMMAND_TASK_SMART_FLEE_POINT> (ped, p.x, p.y,
                                                                p.z, 15.0f,
                                                                3000);
                    }
                }

                if (ped->m_nPhysicalFlags.bSubmergedInWater)
                {
                    continue;
                }
                float multiplier = 0.0045f;
                if (ped->m_pIntelligence->GetTaskSwim ())
                {
                    multiplier *= 0.5f;
                }
                ped->m_fHealth -= ped->m_fMaxHealth * multiplier;
                ped->m_fHealth = std::max (0.0f, ped->m_fHealth);
            }

            for (auto *vehicle : CPools::ms_pVehiclePool)
            {
                if (!vehicle || vehicle->m_nStatus == STATUS_WRECKED) continue;

                auto p = vehicle->GetPosition ();
                if (!IsUnderRain (isCullZone (p.x, p.y, p.z))) continue;

                vehicle->m_fHealth -= 4.5f;
                vehicle->m_fHealth = std::max (0.0f, vehicle->m_fHealth);
            }

            time -= RAIN_TICK_TIME;
        }
    }

    bool
    IsPedUnderRain (CPed *ped)
    {
        auto pos = ped->GetPosition ();
        if (!IsUnderRain (isCullZone (pos.x, pos.y, pos.z)))
        {
            return false;
        }

        auto *vehicle = ped->m_pVehicle;
        if (ped->m_nPedFlags.bInVehicle && vehicle)
        {
            const int vehicleId = vehicle->m_nModelIndex;
            // boats with a cabin
            if (vehicleId == 453 || vehicleId == 454) return false;

            for (auto id : openTopCars)
            {
                if (id == vehicleId) return true;
            }

            // check blade/savanna mod roof
            if (vehicleId == 536 || vehicleId == 567)
            {
                for (auto upg : vehicle->m_anUpgrades)
                {
                    if (upg == 1128 || upg == 1130 || upg == 1131)
                    {
                        return false;
                    }
                }

                return true;
            }

            // stallion, mesa and super gt can have roof
            if (vehicleId == 439 || vehicleId == 500 || vehicleId == 506)
            {
                switch (vehicle->m_anExtras[0])
                {
                    case 0:
                    case 1: return false;
                    default: return true;
                }
            }

            auto vehicleType = CModelInfo::IsVehicleModelType (vehicleId);
            switch (vehicleType)
            {
                case VEHICLE_QUAD:
                case VEHICLE_BOAT:
                case VEHICLE_BIKE:
                case VEHICLE_BMX: return true;
                default: return false;
            }

            return false;
        }

        return true;
    }

    bool
    IsUnderRain (unsigned char val)
    {
        return !(val >> 3 & 1);
    }
};

DEFINE_EFFECT (AcidRainEffect, "effect_acid_rain", GROUP_WEATHER);
