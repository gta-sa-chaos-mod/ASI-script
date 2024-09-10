#include "util/EffectBase.h"
#include "util/GenericUtil.h"

#include <map>

#include <CStats.h>

class UnhealthyDrivingEffect : public EffectBase
{
private:
    const float          MAX_FAT            = 750.0f;
    const float          INCREASE_FAT_VALUE = MAX_FAT * 0.08333f;
    const float          DECREASE_FAT_VALUE = MAX_FAT * 0.03333f;
    const int            DRIVING_TICK_TIME  = 10000;
    const int            JOGGING_TICK_TIME  = 8000;
    const int            REBUILD_TICK_TIME  = 40000;
    static constexpr int vehicleDistStats[]
        = {STAT_DISTANCE_TRAVELLED_BY_CAR,
           STAT_DISTANCE_TRAVELLED_BY_MOTORBIKE,
           STAT_DISTANCE_TRAVELLED_BY_BOAT,
           STAT_DISTANCE_TRAVELLED_BY_GOLF_CART,
           STAT_DISTANCE_TRAVELLED_BY_HELICOPTER,
           STAT_DISTANCE_TRAVELLED_BY_PLANE};

    static constexpr int physDistStats[]
        = {STAT_DISTANCE_TRAVELLED_ON_FOOT, STAT_DISTANCE_TRAVELLED_BY_SWIMMING,
           STAT_DISTANCE_TRAVELLED_BY_BICYCLE};

    std::map<int, float> stats{};
    int                  drivingTimer       = 0;
    int                  joggingTimer       = 0;
    int                  rebuildPlayerTimer = 0;

public:
    void
    OnStart (EffectInstance *inst) override
    {
        for (int i : vehicleDistStats)
        {
            stats.try_emplace (i, CStats::GetStatValue (i));
        }

        for (int i : physDistStats)
        {
            stats.try_emplace (i, CStats::GetStatValue (i));
        }

        drivingTimer       = 0;
        joggingTimer       = 0;
        rebuildPlayerTimer = 0;
    }
    void
    OnTick (EffectInstance *inst) override
    {
        if (!GameUtil::IsPlayerSafe ()) return;

        auto *p = FindPlayerPed ();

        bool inVehicle = false;
        if (p->m_nPedFlags.bInVehicle && p->m_pVehicle)
        {
            const int vehicleId = p->m_pVehicle->m_nModelIndex;
            switch (CModelInfo::IsVehicleModelType (vehicleId))
            {
                case VEHICLE_AUTOMOBILE:
                case VEHICLE_MTRUCK:
                case VEHICLE_HELI:
                case VEHICLE_PLANE:
                case VEHICLE_BOAT:
                case VEHICLE_TRAIN:
                case VEHICLE_BIKE:
                case VEHICLE_QUAD: inVehicle = true; break;
                default: break;
            }
        }

        if (inVehicle)
        {
            drivingTimer += int (GenericUtil::CalculateTick ());
            if (drivingTimer >= DRIVING_TICK_TIME)
            {
                if (CStats::GetStatValue (STAT_FAT) < MAX_FAT)
                {
                    for (int i : vehicleDistStats)
                    {
                        ChangeFatValue (i, INCREASE_FAT_VALUE);
                    }
                }
                drivingTimer -= DRIVING_TICK_TIME;
            }
        }
        else
        {
            joggingTimer += int (GenericUtil::CalculateTick ());
            if (joggingTimer >= JOGGING_TICK_TIME)
            {
                for (int i : physDistStats)
                {
                    ChangeFatValue (i, -DECREASE_FAT_VALUE);
                }
                joggingTimer -= JOGGING_TICK_TIME;
            }
        }

        rebuildPlayerTimer += int (GenericUtil::CalculateTick ());
        if (rebuildPlayerTimer >= REBUILD_TICK_TIME)
        {
            GameUtil::RebuildPlayer ();
            rebuildPlayerTimer -= REBUILD_TICK_TIME;
        }
    }

    void
    OnEnd (EffectInstance *inst) override
    {
        GameUtil::RebuildPlayer ();
    }

    void
    ChangeFatValue (int stat, float changeFat)
    {
        float newValue = CStats::GetStatValue (stat);
        if (newValue > stats[stat])
        {
            float fat = CStats::GetStatValue (STAT_FAT) + changeFat;
            CStats::SetStatValue (STAT_FAT, fat);

            if (changeFat > 0.0f)
            {
                CStats::DisplayScriptStatUpdateMessage (true, STAT_FAT,
                                                        changeFat);
            }
            else
            {
                CStats::DisplayScriptStatUpdateMessage (false, STAT_FAT,
                                                        std::abs (changeFat));
            }
        }
        stats[stat] = newValue;
    }
};

DEFINE_EFFECT (UnhealthyDrivingEffect, "effect_unhealthy_driving", 0);