#include "util/EffectBase.h"
#include "util/GenericUtil.h"

#include <CAudioEngine.h>
#include <CStreaming.h>
#include <eTaskType.h>
#include <extensions/ScriptCommands.h>

using namespace plugin;

// TODO: (Low Priority) Can still softlock missions such as Just Business when
// it happens right before the player hops on the bike and the cutscene starts.
// Once the cutscene is over and the player regains control, the camera will be
// stuck.

// clang-format off
namespace
{
using DialogueData = std::vector<std::pair<std::string, int>>;

//see id in: GTA San Andreas\data\AudioEvents.txt
DialogueData truthPrankCaller = {{"MTRU02F", 29820}};

DialogueData cesarYay1 = {{"MCES07A", 25280}, {"MCES07B", 25281}, {"MCES07D", 25283}};

DialogueData cesarYay2 = {{"MCES08A", 25285}, {"MCES08B", 25286}, {"MCES08C", 25287},
                          {"MCES08E", 25289}};

DialogueData catalina1 = {{"MCAT04A", 25018}, {"MCAT04B", 25019}, {"MCAT04C", 25020},
                          {"MCAT04D", 25021}, {"MCAT04E", 25022}, {"MCAT04F", 25023}};

DialogueData catalina2 = {{"MCAT07A", 25041}, {"MCAT07B", 25042}, {"MCAT07C", 25043},
                          {"MCAT07D", 25044}, {"MCAT07E", 25045}, {"MCAT07F", 25046},
                          {"MCAT07G", 25047}};

DialogueData catalina3 = {{"MCAT06A", 25031}, {"MCAT06B", 25032}, {"MCAT06C", 25033},
                          {"MCAT06D", 25034}, {"MCAT06E", 25035}, {"MCAT06F", 25036},
                          {"MCAT06G", 25037}, /*{"MCAT06H", 25038},*/ {"MCAT06J", 25039},
                          {"MCAT06K", 25040}};

DialogueData catalina4 = {{"MCAT05B", 25025}, {"MCAT05C", 25026}, {"MCAT05D", 25027},
                          {"MCAT05E", 25028}, {"MCAT05F", 25029}, {"MCAT05G", 25030}};

DialogueData catalina5 = {{"MCAT03A", 25016}, {"MCAT03B", 25017}};

DialogueData hisHolinessBusiness = {{"MCES01A", 25200}, {"MCES01B", 25201},
                                    {"MCES09B", 25291}, {"MCES01F", 25205},
                                    {"MCES09L", 25299}, {"MCES09M", 25300},
                                    {"MCES09N", 25301}, {"MCES09P", 25303}};

const std::array dialogues = {hisHolinessBusiness, catalina4, catalina1,
                              cesarYay2, cesarYay1, catalina2, catalina3,
                              truthPrankCaller, catalina5};
} // namespace
// clang-format on

class AnnoyingPhoneCallsEffect : public EffectBase
{
    int       wait              = 0;
    const int PICKUP_PHONE_TIME = 2200;
    const int NEXT_CALL_TIME    = 15000;
    int       pickupPhoneTimer  = 0;

    int  currentDialogue = 1;
    int  currentPhrase   = 0;
    bool isAudioPlaying  = false;

public:
    void
    OnStart (EffectInstance *inst) override
    {
        wait = inst->Random (3, 5) * 1000;

        currentDialogue = inst->Random (0, 10000) % dialogues.size ();
        currentPhrase   = 0;

        auto [str, id] = dialogues[currentDialogue][currentPhrase];
        Command<Commands::LOAD_MISSION_AUDIO> (1, id);
        Command<Commands::LOAD_MISSION_AUDIO> (3, 23000); // mobile ring-ring
    }

    void
    OnEnd (EffectInstance *inst)
    {
        auto *player = FindPlayerPed ();
        Command<Commands::TASK_USE_MOBILE_PHONE> (player, false);
    }

    bool
    CanActivate () override
    {
        return FindPlayerPed ();
    }

    void
    OnTick (EffectInstance *inst) override
    {
        if (!GameUtil::IsPlayerSafe ()) return;

        auto *player = FindPlayerPed ();

        auto cond = Command<Commands::IS_CHAR_SHOOTING> (player);
        cond |= !Command<Commands::IS_CHAR_ON_FOOT> (player);
        cond |= Command<Commands::IS_PLAYER_USING_JETPACK> (0);
        cond |= Command<Commands::IS_CHAR_IN_WATER> (player);

        if (cond) return;

        wait -= (int) GenericUtil::CalculateTick ();
        if (wait > 0) return;

        auto *phoneTask = player->m_pIntelligence->m_TaskMgr.FindTaskByType (
            3, TASK_COMPLEX_USE_MOBILE_PHONE);

        if (!phoneTask)
        {
            CStreaming::RequestModel (330, 2);
            CStreaming::LoadAllRequestedModels (false);
            CStreaming::SetModelIsDeletable (330);
            Command<Commands::PLAY_MISSION_AUDIO> (3);
            Command<Commands::TASK_USE_MOBILE_PHONE> (player, true);
            pickupPhoneTimer = PICKUP_PHONE_TIME;
        }

        pickupPhoneTimer -= (int) GenericUtil::CalculateTick ();
        if (pickupPhoneTimer > 0) return;

        auto [subtitle, id] = dialogues[currentDialogue][currentPhrase];

        if (!Command<Commands::HAS_MISSION_AUDIO_LOADED> (1)) return;

        if (!isAudioPlaying)
        {
            Command<Commands::PLAY_MISSION_AUDIO> (1);
            Command<Commands::PRINT_NOW> (subtitle.data (), 10000, 1);
            isAudioPlaying = true;
        }

        if (isAudioPlaying && Command<Commands::HAS_MISSION_AUDIO_FINISHED> (1))
        {
            Command<Commands::CLEAR_THIS_PRINT> (subtitle.c_str ());
            Command<Commands::CLEAR_MISSION_AUDIO> (1);
            isAudioPlaying = false;

            currentPhrase++;

            if (currentPhrase >= dialogues[currentDialogue].size ())
            {
                currentDialogue = inst->Random (0, 10000) % dialogues.size ();
                currentPhrase   = 0;

                auto [str, id] = dialogues[currentDialogue][currentPhrase];
                Command<Commands::LOAD_MISSION_AUDIO> (1, id);

                wait           = NEXT_CALL_TIME;
                isAudioPlaying = false;
                Command<Commands::TASK_USE_MOBILE_PHONE> (player, false);
                return;
            }

            auto [str, id] = dialogues[currentDialogue][currentPhrase];
            Command<Commands::LOAD_MISSION_AUDIO> (1, id);
        }

        wait = 100;
    }
};

DEFINE_EFFECT (AnnoyingPhoneCallsEffect, "effect_annoying_phone_calls", 0);
