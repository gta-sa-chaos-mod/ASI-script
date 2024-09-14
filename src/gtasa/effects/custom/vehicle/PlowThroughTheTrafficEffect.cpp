#include "util/EffectBase.h"

namespace
{
bool
pointInRadius (const CVector &p1, const CVector &center, const float R)
{
    auto dx = std::abs (p1.x - center.x);
    auto dy = std::abs (p1.y - center.y);
    auto dz = std::abs (p1.z - center.z);
    if ((dx + dy + dz) <= R) return true;

    return (dx * dx + dy * dy + dz * dz <= R * R);
}

float
dotProduct (const CVector &v1, const CVector &v2)
{
    return (v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z);
}
} // namespace

class PlowThroughTheTrafficEffect : public EffectBase
{
private:
    static inline std::array<float, 5> zFactor
        = {0.2f, 0.15f, 0.25f, 0.15f, 0.3f};
    static inline unsigned int idx = 0;

public:
    void
    OnStart (EffectInstance *inst) override
    {
        idx = 0;
    }

    void
    OnTick (EffectInstance *inst) override
    {
        auto *player = FindPlayerPed ();
        if (!player) return;

        auto *playerCar = player->m_pVehicle;
        if (player->m_nPedFlags.bInVehicle && playerCar)
        {
            auto vehiclePos = playerCar->GetPosition ();
            for (auto *car : CPools::ms_pVehiclePool)
            {
                if (pointInRadius (car->GetPosition (), vehiclePos, 10.0f)
                    && car != playerCar)
                {
                    auto diff = car->GetPosition () - playerCar->GetPosition ();
                    diff.Normalise ();

                    auto pushFactor
                        = playerCar->m_vecMoveSpeed.Magnitude () * 1.5f;
                    auto forward = playerCar->GetMatrix ()->up;
                    if (dotProduct (forward, diff) > 0.8f && pushFactor > 0.05f)
                    {
                        car->m_vecMoveSpeed.x
                            = std::clamp (diff.x, -pushFactor, pushFactor);
                        car->m_vecMoveSpeed.y
                            = std::clamp (diff.y, -pushFactor, pushFactor);
                        car->m_vecMoveSpeed.z
                            = zFactor[idx++ % zFactor.size ()];
                        car->m_vecTurnSpeed.z = inst->Random (-0.2f, 0.2f);
                    }
                }
            }
        }
    }
};

DEFINE_EFFECT (PlowThroughTheTrafficEffect, "effect_plow_through_the_traffic",
               0);