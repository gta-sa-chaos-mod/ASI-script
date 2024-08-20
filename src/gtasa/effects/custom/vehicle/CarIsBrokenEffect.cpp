#include "util/EffectBase.h"
#include "util/GameUtil.h"
#include "util/GenericUtil.h"

class CarIsBrokenEffect : public EffectBase
{
    int  wait  = 0;
    bool doBroken = true;

public:
    void
    OnStart (EffectInstance *inst) override
    {
        wait  = 0;
        doBroken = true;
    }

    void
    OnEnd (EffectInstance *inst) override
    {
        FixDoor();
    }

    void
    OnTick (EffectInstance *inst) override
    {
        wait -= (int) GenericUtil::CalculateTick ();
        if (wait > 0) return;

        GameUtil::SetVehiclesToRealPhysics ();

        doBroken = !doBroken;
        if (doBroken)
        {
            BrokeDoor ();
            wait = 25;
        }
        else
        {
            FixDoor ();
            wait = 50;
        }
    }

    void
    BrokeDoor ()
    {
        for (CVehicle *vehicle : CPools::ms_pVehiclePool)
        {
            if (vehicle->m_nVehicleSubClass == VEHICLE_AUTOMOBILE || vehicle->m_nVehicleSubClass == VEHICLE_MTRUCK)
            {
                CAutomobile *car = reinterpret_cast<CAutomobile *> (vehicle);
                car->PopDoor (eCarNodes::CAR_DOOR_LF, eDoors::DOOR_FRONT_LEFT, true);
                car->PopDoor (eCarNodes::CAR_DOOR_RF, eDoors::DOOR_FRONT_RIGHT, true);
                car->PopDoor (eCarNodes::CAR_DOOR_LR, eDoors::DOOR_REAR_LEFT, true);
                car->PopDoor (eCarNodes::CAR_DOOR_RR, eDoors::DOOR_REAR_RIGHT, true);
                car->PopDoor (eCarNodes::CAR_BONNET, eDoors::BONNET, true);
                car->PopDoor (eCarNodes::CAR_BOOT, eDoors::BOOT, true);
            }
        }
    }

    void
    FixDoor ()
    {
        for (CVehicle *vehicle : CPools::ms_pVehiclePool)
        {
            if (vehicle->m_nVehicleSubClass == VEHICLE_AUTOMOBILE || vehicle->m_nVehicleSubClass == VEHICLE_MTRUCK)
            {
                CAutomobile *car = reinterpret_cast<CAutomobile *> (vehicle);
                car->FixDoor (eCarNodes::CAR_DOOR_LF, eDoors::DOOR_FRONT_LEFT);
                car->FixDoor (eCarNodes::CAR_DOOR_RF, eDoors::DOOR_FRONT_RIGHT);
                car->FixDoor (eCarNodes::CAR_DOOR_LR, eDoors::DOOR_REAR_LEFT);
                car->FixDoor (eCarNodes::CAR_DOOR_RR, eDoors::DOOR_REAR_RIGHT);
                car->FixDoor (eCarNodes::CAR_BONNET, eDoors::BONNET);
                car->FixDoor (eCarNodes::CAR_BOOT, eDoors::BOOT);
            }
        }
    }
};

DEFINE_EFFECT (CarIsBrokenEffect, "effect_car_is_broken", 0);