#include "util/EffectBase.h"
#include "util/GameUtil.h"
#include "util/GenericUtil.h"

#include <eTaskType.h>
#include <extensions/ScriptCommands.h>

using namespace plugin;

class WalkOffEffect : public EffectBase
{
    int wait = 500;

public:
    bool
    CanActivate () override
    {
        return GameUtil::IsPlayerSafe ();
    }

    void
    OnStart (EffectInstance *inst) override
    {
        wait = 500;
    }

    void
    OnEnd (EffectInstance *inst) override
    {
        CPlayerPed *player = FindPlayerPed ();
        if (!player) return;

        Command<eScriptCommands::COMMAND_CLEAR_CHAR_TASKS_IMMEDIATELY> (player);

        CPad *pad = player->GetPadFromPlayer ();
        if (pad) pad->DisablePlayerControls = false;
    }

    void
    OnTick (EffectInstance *inst) override
    {
        wait -= (int) GenericUtil::CalculateTick ();
        if (wait > 0) return;

        CPlayerPed *player = FindPlayerPed ();
        if (!player) return;

        auto task
            = player->m_pIntelligence->FindTaskByType (TASK_COMPLEX_WANDER);
        if (!task && GameUtil::IsPlayerSafe ())
        {
            Command<eScriptCommands::COMMAND_TASK_WANDER_STANDARD> (player);
            wait = 500;
        }

        CPad *pad = player->GetPadFromPlayer ();
        if (pad) pad->DisablePlayerControls = true;
    }
};

DEFINE_EFFECT (WalkOffEffect, "effect_walk_off", GROUP_CONTROLS);