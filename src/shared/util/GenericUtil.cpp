#include "GenericUtil.h"

#include "util/Config.h"
#include "util/Globals.h"
#include "util/Version.h"
#include "util/Websocket.h"

std::string
GenericUtil::GetModVersion ()
{
    std::string version = "Chaos Mod v3.2.1";

#ifndef NDEBUG
    version.append ("-debug");
#endif

#ifdef VERSION_SUFFIX
    version.append ("-git.").append (VERSION_SUFFIX);
#endif

    if (CONFIG_CC_ENABLED)
        version.append ("~n~CC Connected: ");
    else
        version.append ("~n~GUI Connected: ");

    bool connected = Websocket::IsClientConnected ();
    version.append (connected ? "~g~true" : "~r~false");

    return version;
}

double
GenericUtil::CalculateTick (double multiplier)
{
    unsigned diff = CTimer::m_snTimeInMilliseconds
                    - CTimer::m_snPreviousTimeInMilliseconds;

    // If the jump is too big, e.g. replays or loading saves
    if (diff <= 0 || diff >= 1000) return 0;

    float timeScale = std::max (0.000001f, CTimer::ms_fTimeScale);

    return diff / timeScale * multiplier;
}

std::string
GenericUtil::FormatTime (int duration, bool onlySeconds)
{
    int seconds = std::max (0, duration) / 1000, minutes = 0;
    while (seconds >= 60)
    {
        minutes += 1;
        seconds -= 60;
    }

    if (minutes > 99)
    {
        minutes = 99;
        seconds = 59;
    }

    std::string time;
    if (!onlySeconds)
    {
        if (minutes < 10) time.append ("0");

        time.append (std::to_string (minutes));
        time.append (":");
    }
    if (seconds < 10) time.append ("0");

    time.append (std::to_string (seconds));
    return time;
}

bool
GenericUtil::IsMenuActive ()
{
    return FrontEndMenuManager.m_bMenuActive;
}

float
GenericUtil::EaseOutBack (float t)
{
    t -= 1;
    return 1 + t * t * (2.70158f * t + 1.70158f);
}

float
GenericUtil::EaseInOutQubic (float t)
{
    if (t < 0.5)
        return 4 * t * t * t;
    else
        return 1 - powf (-2 * t + 2, 3) / 2;
}

std::string
GenericUtil::ToUpper (std::string string)
{
    std::transform (string.begin (), string.end (), string.begin (),
                    [] (unsigned char c) { return std::toupper (c); });
    return string;
}
