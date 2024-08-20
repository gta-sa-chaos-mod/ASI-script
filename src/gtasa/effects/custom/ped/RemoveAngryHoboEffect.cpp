#include "effects/OneTimeEffect.h"

#include <CPlayerPed.h>
#include "util/HoboManager.h"

using namespace plugin;

class RemoveAngryHobo : public OneTimeEffect
{
    int removeType = 0; // 0 - Random, 1 - Nearest, 2 - Farthest, 3 - All
public:
    RemoveAngryHobo (int type) : removeType (type) {}

    bool
    CanActivate () override
    {
        return HoboManager::angryHobo.size () > 0;
    }

    void
    OnStart (EffectInstance *inst) override
    {
        switch (removeType) {
            case 0: HoboManager::RemoveRandomAngryHobo (); break;
            case 1: {
                CPed *player = FindPlayerPed ();
                if (player) {
                    CPed *nearest = nullptr;
                    float nearestDist = 999999.0f;
                    for (CPed *ped : HoboManager::angryHobo) {
                        if (ped) {
                            float dist = DistanceBetweenPoints (player->GetPosition (), ped->GetPosition ());
                            if (dist < nearestDist) {
                                nearest = ped;
                                nearestDist = dist;
                            }
                        }
                    }
                    if (nearest) {
                        HoboManager::RemoveAngryHobo (nearest);
                    }
                }
                break;
            }
            case 2: {
                CPed *player = FindPlayerPed ();
                if (player) {
                    CPed *farthest = nullptr;
                    float farthestDist = 0.0f;
                    for (CPed *ped : HoboManager::angryHobo) {
                        if (ped) {
                            float dist = DistanceBetweenPoints (player->GetPosition (), ped->GetPosition ());
                            if (dist > farthestDist) {
                                farthest = ped;
                                farthestDist = dist;
                            }
                        }
                    }
                    if (farthest) {
                        HoboManager::RemoveAngryHobo (farthest);
                    }
                }
                break;
            }
            case 3: HoboManager::RemoveAllAngryHobo (); break;
        }
    }
};

DEFINE_EFFECT (RemoveAngryHobo, "effect_remove_random_angry_hobo", 0);
DEFINE_EFFECt (RemoveAngryHobo, "effect_remove_nearest_angry_hobo", 1);
DEFINE_EFFECT (RemoveAngryHobo, "effect_remove_farthest_angry_hobo", 2);
DEFINE_EFFECT (RemoveAngryHobo, "effect_remove_all_angry_hobo", 3);