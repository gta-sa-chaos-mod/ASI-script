#pragma once

#include <filesystem>

#define CONFIG(_setting, _default) Config::GetOrDefault (_setting, _default)
#define CONFIG_CC_ENABLED CONFIG ("CrowdControl.Enabled", false)

class Config
{
    static inline bool fileWatcherInitialized = false;

    static inline std::shared_ptr<cpptoml::table> config;
#ifdef GTASA
    static inline std::string configContent
        = R"(########################################################
# Chaos Mod
#######################################################

#######################################################
# Chaos Settings
[Chaos]

# Websocket port for the connection to the GUI
# Min: 1025
# Max: 65535
# Default: 42069
GUIWebsocketPort = 42069

# Effect sound volume between 0 and 100
# Min: 0
# Max: 100
# Default: 100
Volume = 100

# Always count down effects, no matter if it's in a cutscene or the player is currently
# in a specific state (e.g. getting arrested, getting wasted, etc.)
# Default: false
AlwaysCountDownEffects = false

# Autosave after completing a mission
# Default: true
AutosaveAfterMissionPassed = true

# Additionally save the Autosave into Slot 8
# Default: false
SaveToSlot8 = false

# Load the autosave on game load (Can be skipped by holding Left CTRL)
# Default: true
LoadAutosaveOnGameLoad = true

# Whether or not the game should delete the autosave when starting a new game
# This is so when "Reload Autosave" is coming up it won't "wrong-wrap" you to that
# Useful for speedruns, mainly, so they don't get disqualified.
# Default: true
DeleteAutosaveOnNewGame = true

# Whether or not the game should play a simple sound when an effect get's enabled
# Default: true
PlayEffectSound = true

# Whether or not to clear all active effects when a new game is started (Useful for speedruns)
# Default: true
ClearEffectsOnNewGame = true

# Allow Quicksaves at any time by pressing F7
# This is doable every 10 seconds. It will also not save during missions.
# Will be saved to Slot 8.
# Default: false
QuickSave = false

# Switch all vehicles to real game physics every tick
# This will prevent vehicles on highways or other places to go absurdly fast whilst they're on their AI path
# Default: true
SwitchAllVehiclesToRealPhysics = true

#######################################################

#######################################################
# Effect-Specific Settings

# Vehicle One Hit K.O.
[Effects.VehicleOneHitKO]

# When set to true vehicles will explode the moment they take damage.
# When set to false they will just start burning.
# Default: false
ExplodeVehicles = false

[Effects.Buttsbot]
Frequency = 8
LowerCase = "butt"
UpperCase = "Butt"
AllCaps = "BUTT"

[Effects.ReplaceAllText]
Text = "Your Text Could Be Here"

#######################################################

#######################################################
# Drawing Settings
[Drawing]

# Whether or not to enable the drawing functionality in the first place
# Default: true
Enabled = true

# Max. number of recent effects
# Min: 6
# Max: 10
# Default: 8
NumRecentEffects = 8

# Amount of seconds to wait to shift timed effects down by one.
# This will only happen when it shows "And X more..." (as in, NumRecentEffects was hit)
# Min: 3
# Max: 15
# Default: 5
RotationSeconds = 5

# Draw the top bar showing the remaining time until the next effect happens
# Default: true
DrawRemainingTimeBar = true

# Draw the currently active / last effects
# Default: true
DrawActiveEffects = true

# Draw the remaining time of effects as circles instead of a "minute:second" text
# If you don't see circles next to effects, try turning this option off.
# Default: true
DrawCircles = true

# Draw voting for Twitch integration
# Default: true
DrawVoting = true

# Will display the bar and effect circles in rainbow colors
# Default: true
RainbowColors = true

# Will draw effects from the top instead of from the bottom
# The newest effect will be added at the top
# Default: true
EffectsTopToBottom = true

# Will draw votes on the top of the screen instead of the bottom
# Default: false
DrawVotesOnTop = true

#######################################################

#######################################################
# Crowd Control Settings
[CrowdControl]

# Whether Crowd Control is enabled
# WARNING: THIS HAS TO BE ENABLED WHEN PLAYING WITH CROWD CONTROL!
# Default: false
Enabled = false

# Prevents new effects from happening when the recent effect list is full.
# Check Drawing.NumRecentEffects for the amount of effects drawn on screen.
# Default: true
PreventNewEffectsWhenFull = true

# Ensures that effects overriding one another aren't activated at the same time.
#
# Example: One Hit K.O. is still active and someone redeems Infinite Health.
# This would prevent Infinite Health from being activated until One Hit K.O. is over.
#
# Default: false
CheckForIncompatibleEffects = false

#######################################################)";
#else
    static inline std::string configContent = "";
#endif

public:
    static std::string
    GetConfigFilename ()
    {
        const std::string pluginFilename = std::string (PLUGIN_FILENAME);
        const std::string configFilename
            = pluginFilename.substr (0, pluginFilename.size () - 4) + ".toml";

        return configFilename;
    }

    static std::filesystem::path
    GetConfigPath ()
    {
        const std::string configFilename = GetConfigFilename ();

        const std::filesystem::path configPath
            = PLUGIN_PATH ((char *) configFilename.c_str ());

        std::filesystem::create_directories (configPath.parent_path ());

        return configPath;
    }

    static void Init ();

    static void ReloadConfig ();

    template <class T>
    static T
    GetOrDefault (std::string key, T defaultValue)
    {
        if (!config) return defaultValue;

        return config->get_qualified_as<T> (key).value_or (defaultValue);
    }
};