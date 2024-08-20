#pragma once

#include <extensions/ScriptCommands.h>
#include <CPed.h>

using namespace plugin;

class HoboManager {
public:
    static inline std::vector<CPed *> angryHobo = {};

    static void
    AddAngryHobo (CPed *ped)
    {
        angryHobo.push_back (ped);
    }

    static void
    RemoveAngryHobo(CPed *ped)
    {
        _RemoveAngryHobo (ped);
    }

    static void
    RemoveAllAngryHobo ()
    {
        for (CPed *ped : angryHobo) {
            _RemoveAngryHobo (ped);
        }
    }

    static void
    RemoveRandomAngryHobo ()
    {
        if (angryHobo.size () > 0) {
            _RemoveAngryHobo (rand () % angryHobo.size ());
        }
    }

private:
    static void
    _RemoveAngryHobo (CPed *ped)
    {
        if (ped) {
            Command<eScriptCommands::COMMAND_REMOVE_CHAR_ELEGANTLY> (ped);
            angryHobo.erase (std::remove (angryHobo.begin (), angryHobo.end (), ped), angryHobo.end ());
        }
    }

    static void
    _RemoveAngryHobo (int index)
    {
        if (index >= 0 && index < angryHobo.size ()) {
            CPed *ped = angryHobo.at (index);
            if (ped) {
                Command<eScriptCommands::COMMAND_REMOVE_CHAR_ELEGANTLY> (ped);
                angryHobo.erase (angryHobo.begin () + index); // Remove The Index
            }
        }
    }    
};