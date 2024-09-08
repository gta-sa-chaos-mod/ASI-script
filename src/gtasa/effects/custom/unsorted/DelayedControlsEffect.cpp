#include "util/EffectBase.h"
#include "util/hooks/HookMacros.h"

#include <deque>

#include <CMenuManager.h>
#include <CTimer.h>

class DelayedControls : public EffectBase
{
    struct InputData
    {
        std::chrono::steady_clock::time_point chronoTime;
        CControllerState                      state;
        CMouseControllerState                 mouseState;
        CKeyboardState                        keyboardState;
    };

    static inline std::deque<InputData> bufferedInputData = {};

    // 150ms is the minimum for delay
    std::chrono::milliseconds chronoBuffer = std::chrono::milliseconds{100};

public:
    void
    OnStart (EffectInstance *inst) override
    {
        bufferedInputData.clear ();

        HOOK_METHOD_ARGS (inst, Hooked_GetPositionOfAnalogueSticks,
                          void (CRunningScript *, __int16), 0x48AF1F);
    }

    void
    OnProcessScripts (EffectInstance *inst) override
    {
        CPlayerPed *player = FindPlayerPed ();
        if (!player) return;

        CPad *pad = player->GetPadFromPlayer ();
        if (!pad) return;

        std::chrono::steady_clock::time_point chronoTime
            = std::chrono::steady_clock::now ();

        // Add extra random delay between 0 and 100ms, resulting in a random
        // input delay between 150 and 250ms
        auto addedRandomDelay
            = std::chrono::milliseconds{inst->Random (0, 100)};

        InputData frameData
            = {.chronoTime    = chronoTime + chronoBuffer + addedRandomDelay,
               .state         = pad->NewState,
               .mouseState    = pad->NewMouseControllerState,
               .keyboardState = pad->NewKeyState};

        bufferedInputData.push_back (frameData);
        if (chronoTime > bufferedInputData[0].chronoTime)
        {
            InputData data               = bufferedInputData.front ();
            pad->NewState                = data.state;
            pad->NewMouseControllerState = data.mouseState;
            pad->NewKeyState             = data.keyboardState;

            bufferedInputData.pop_front ();
        }
        else
        {
            pad->NewState                = CControllerState ();
            pad->NewMouseControllerState = CMouseControllerState ();
            pad->NewKeyState             = CKeyboardState ();
        }
    }

    static void
    Hooked_GetPositionOfAnalogueSticks (auto &&cb, CRunningScript *script,
                                        __int16 count)
    {
        CTheScripts::ScriptParams[0].iParam = 0;
        CTheScripts::ScriptParams[1].iParam = 0;
        CTheScripts::ScriptParams[2].iParam = 0;
        CTheScripts::ScriptParams[3].iParam = 0;

        if (bufferedInputData.size () > 0)
        {
            InputData data = bufferedInputData.front ();

            std::chrono::steady_clock::time_point chronoTime
                = std::chrono::steady_clock::now ();

            if (chronoTime > data.chronoTime)
            {
                CTheScripts::ScriptParams[0].iParam = data.state.LeftStickX;
                CTheScripts::ScriptParams[1].iParam = data.state.LeftStickY;
                CTheScripts::ScriptParams[2].iParam = data.state.RightStickX;
                CTheScripts::ScriptParams[3].iParam = data.state.RightStickY;
            }
        }

        cb ();
    }
};

DEFINE_EFFECT (DelayedControls, "effect_delayed_controls", GROUP_CONTROLS);