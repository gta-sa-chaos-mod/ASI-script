#include "util/EffectBase.h"
#include "util/GenericUtil.h"

#include <extensions/ScriptCommands.h>

using namespace plugin;

class CJ101Effect : public EffectBase
{
private:
    const int MIN_SPAWN_SEC    = 8;
    const int MAX_SPAWN_SEC    = 13;
    const int planesId[5]      = {511, 512, 476, 593, 513};
    int       timer            = 0;
    int       planeSpawnTimer  = 0;

public:
    bool
    CanActivate () override
    {
        auto *player = FindPlayerPed ();
        return player && !player->m_nAreaCode && GameUtil::IsPlayerSafe ();
    }

    void
    OnStart (EffectInstance *inst) override
    {
        timer           = 0;
        planeSpawnTimer = inst->Random (MIN_SPAWN_SEC, MAX_SPAWN_SEC) * 1000;
    }

    void
    OnTick (EffectInstance *inst) override
    {
        if (!CanActivate ()) return;
        auto *player = FindPlayerPed ();

        timer += int (GenericUtil::CalculateTick ());
        if (timer >= planeSpawnTimer)
        {
            int  id = planesId[inst->Random (0, 10000) % std::size (planesId)];
            auto playerPos = player->GetPosition ();
            auto [pos, angle]
                = GetPositionAndHeading (playerPos, player->GetHeading (),
                                         inst->Random (0, 1));

            pos.z += inst->Random (0, 5);
            auto *plane = reinterpret_cast<CPlane *> (
                GameUtil::CreateVehicle (id, pos, angle, true));

            float force             = inst->Random (0.8f, 1.1f);
            plane->m_vecMoveSpeed.x = -std::sin (angle) * force;
            plane->m_vecMoveSpeed.y = std::cos (angle) * force;
            plane->SetHeading (angle);

            Command<eScriptCommands::COMMAND_PLANE_GOTO_COORDS> (
                plane, playerPos.x, playerPos.y, playerPos.z, 0.0f, 0.0f);

            if (id == 476) plane->SetGearUp ();

            plane->m_fHealth = 249.0f;

            timer = 0;
            planeSpawnTimer = inst->Random (MIN_SPAWN_SEC, MAX_SPAWN_SEC) * 1000;
        }
    }

    std::pair<CVector, float>
    GetPositionAndHeading (const CVector &playerPos, float playerAngle,
                           bool getBehind)
    {
        float dirX = 100.0f * -std::sin (playerAngle);
        float dirY = 100.0f * std::cos (playerAngle);

        if (getBehind)
        {
            float posX = playerPos.x - dirX;
            float posY = playerPos.y - dirY;
            float posZ = CWorld::FindGroundZForCoord (posX, posY);
            return {{posX, posY, posZ}, playerAngle};
        }
        else
        {
            float posX = playerPos.x + dirX;
            float posY = playerPos.y + dirY;
            float posZ = CWorld::FindGroundZForCoord (posX, posY);
            playerAngle -= M_PI;
            return {{posX, posY, posZ}, playerAngle};
        }
    }
};

DEFINE_EFFECT (CJ101Effect, "effect_cj101", 0);
