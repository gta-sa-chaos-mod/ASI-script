#include "effects/OneTimeEffect.h"
#include "util/GameUtil.h"
#include "util/GenericUtil.h"
#include "util/GlobalRenderer.h"

class OhNoMyWheel : public OneTimeEffect
{
public:
    bool
    CanActivate () override
    {
        CVehicle *vehicle = FindPlayerVehicle (-1, false);
        return (vehicle && (vehicle->m_nVehicleSubClass == VEHICLE_AUTOMOBILE || vehicle->m_nVehicleSubClass == VEHICLE_QUAD) && vehicle->m_nModelIndex != MODEL_RHINO);
    }

    void
    OnStart (EffectInstance *inst) override
    {
        CVehicle *vehicle = FindPlayerVehicle (-1, false);
        DetachThisWheel(vehicle, eCarNodes::CAR_WHEEL_LF, 0, false);
        DetachThisWheel(vehicle, eCarNodes::CAR_WHEEL_LB, 1, true);
        DetachThisWheel(vehicle, eCarNodes::CAR_WHEEL_RF, 2, false);
        DetachThisWheel(vehicle, eCarNodes::CAR_WHEEL_RB, 3, true);
    }

    void 
    HideAllAtomics(RwFrame *frame)
    {
        if (!rwLinkListEmpty(&frame->objectList))
        {
            RwObjectHasFrame * atomic;

            RwLLLink * current = rwLinkListGetFirstLLLink(&frame->objectList);
            RwLLLink * end = rwLinkListGetTerminator(&frame->objectList);

            current = rwLinkListGetFirstLLLink(&frame->objectList);
            while (current != end) {
                atomic = rwLLLinkGetData(current, RwObjectHasFrame, lFrame);
                atomic->object.flags &= ~0x4; // clear

                current = rwLLLinkGetNext(current);
            }
        }
        return;
    }

    bool 
    IsNotLastWheel(CAutomobile *automobile, int wheelId)
    {
        for (int i = 0; i < 4; ++i)
        {
            if (i != wheelId && automobile->m_damageManager.m_anWheelsStatus[i] != 2)
            {
                return true;
            }
        }
        return false;
    }

    void 
    DetachThisWheel(CVehicle *vehicle, int wheelNode, int wheelId, bool isRear)
    {
        CAutomobile *automobile = reinterpret_cast<CAutomobile*>(vehicle);
        if (vehicle->IsComponentPresent(wheelNode) && IsNotLastWheel(automobile, wheelId))
        {
            if (wheelId != -1)
            {
                if (!isRear)
                {
                    automobile->m_damageManager.SetWheelStatus(wheelId, 2);
                }
            }
            automobile->SpawnFlyingComponent(wheelNode, 1);
            HideAllAtomics(automobile->m_aCarNodes[wheelNode]);
        }
    }

};

DEFINE_EFFECT (OhNoMyWheel, "effect_oh_no_my_wheel", 0);