#include "EffectDrawHandler.h"

#include "util/ColorHelper.h"
#include "util/Config.h"
#include "util/DrawHelper.h"
#include "util/EffectHandler.h"
#include "util/EffectInstance.h"
#include "util/EffectTwitchHandler.h"
#include "util/GenericUtil.h"
#include "util/Globals.h"

#include <algorithm>

#ifdef GTASA
#include <CMenuSystem.h>
#endif

#include <CFont.h>
#include <extensions/FontPrint.h>

static const float TEXT_SCALE_X = 0.6f;
static const float TEXT_SCALE_Y = 0.8f;

static const float SUBTEXT_SCALE_X = 0.4f;
static const float SUBTEXT_SCALE_Y = 0.6f;

static const float Y_SPACING = 45.0f;

bool
EffectDrawHandler::AreEffectsInset (bool checkOneTimeEffects)
{
    auto &effects = checkOneTimeEffects ? EffectHandler::GetOneTimeEffects ()
                                        : EffectHandler::GetActiveEffects ();

    int i = 0;
    for (const auto &effect : effects)
    {
        if (++i > RECENT_EFFECTS) return false;

        if (effect.DoesEffectDrawTimer ()) return true;
    }

    return false;
}

void
EffectDrawHandler::CalculateDrawPosition ()
{
    float position = isInset ? 70.0f : 25.0f;

    std::string_view name = effect->GetName ();

    if (Globals::enabledEffects["replace_all_text"])
        name = Globals::replaceAllTextString;

#ifdef GTASA
    CFont::SetScaleForCurrentlanguage (TEXT_SCALE_X, TEXT_SCALE_Y);
    float renderWidth
        = CFont::GetStringWidth (const_cast<char *> (name.data ()), true,
                                 false);
#elif GTAVC
    CFont::SetScale (TEXT_SCALE_X, TEXT_SCALE_Y);
    float renderWidth = CFont::GetStringWidth (
        (wchar_t *) std::wstring (name.begin (), name.end ()).c_str (), true);
#elif GTA3
    CFont::SetScale (TEXT_SCALE_X, TEXT_SCALE_Y);
    float renderWidth = CFont::GetStringWidth (name.data (), true);
#endif

    x = GenericUtil::EaseOutBack (transitionTimer, -renderWidth, position);

    if (drawLeft)
        yLeft = ((idx + 1) * Y_SPACING) + 260.0f;
    else
        yRight = ((idx + 1) * Y_SPACING) + 260.0f;

    if (Globals::enabledEffects["screensaver_hud"] && !CONFIG_CC_ENABLED)
    {
        std::string effectName (effect->GetName ());
        auto        element = positions[effectName];

        x = element.pos.x;

        if (drawLeft)
            yLeft = element.pos.y;
        else
            yRight = element.pos.y;
    }
}

void
EffectDrawHandler::PrintEffectName ()
{
    std::string_view name = effect->GetName ();

    if (Globals::enabledEffects["replace_all_text"])
        name = Globals::replaceAllTextString;

    if (drawLeft)
    {
        gamefont::Print (gamefont::LeftBottom, gamefont::AlignLeft,
                         std::string (name), x, yLeft, FONT_DEFAULT,
                         TEXT_SCALE_X, TEXT_SCALE_Y, GetTextColor (), 1,
                         GetDropShadowColor (), false);
    }
    else
    {
        gamefont::Print (gamefont::RightBottom, gamefont::AlignRight,
                         std::string (name), x, yRight, FONT_DEFAULT,
                         TEXT_SCALE_X, TEXT_SCALE_Y, GetTextColor (), 1,
                         GetDropShadowColor (), false);
    }
}

void
EffectDrawHandler::PrintSubtext ()
{
    if (effect->HasSubtext ())
    {
        std::string_view subtext = effect->GetSubtext ();

        if (Globals::enabledEffects["replace_all_text"])
            subtext = Globals::replaceAllTextString;

        if (drawLeft)
        {
            gamefont::Print (gamefont::LeftBottom, gamefont::AlignLeft,
                             std::string (subtext), x, yLeft - 20.0f,
                             FONT_DEFAULT, SUBTEXT_SCALE_X, SUBTEXT_SCALE_Y,
                             GetTextColor (), 1, GetDropShadowColor (), true);
        }
        else
        {
            gamefont::Print (gamefont::RightBottom, gamefont::AlignRight,
                             std::string (subtext), x, yRight - 20.0f,
                             FONT_DEFAULT, SUBTEXT_SCALE_X, SUBTEXT_SCALE_Y,
                             GetTextColor (), 1, GetDropShadowColor (), true);
        }
    }
}

void
EffectDrawHandler::PrintEffectTimer ()
{
    int actualRemaining
        = effect->GetEffectRemaining () / Globals::effectTimerSpeed;

    if (CONFIG ("Drawing.DrawCircles", true))
    {
        if (effect->HasSubtext ())
        {
            if (drawLeft)
                yLeft -= 8.0f;
            else
                yRight -= 8.0f;
        }

        CVector2D center
            = CVector2D (SCREEN_COORD_LEFT (x) - SCREEN_COORD (33.0f),
                         SCREEN_COORD_BOTTOM (yLeft) + SCREEN_COORD (12.0f));

        if (!drawLeft)
        {
            center = CVector2D (SCREEN_COORD_RIGHT (x) + SCREEN_COORD (33.0f),
                                SCREEN_COORD_BOTTOM (yRight)
                                    + SCREEN_COORD (12.0f));
        }

        RwTextureFilterMode filter        = rwFILTERLINEAR;
        int                 alphaBlending = true;

        RwRenderStateSet (rwRENDERSTATEVERTEXALPHAENABLE, &alphaBlending);
        RwRenderStateSet (rwRENDERSTATETEXTUREFILTER, (void *) (filter));
        RwRenderStateSet (rwRENDERSTATETEXTURERASTER, NULL);

        float angle = (float) effect->GetEffectRemaining ()
                      / (float) effect->GetEffectDuration () * 360.0f;

        DrawHelper::DrawCircle (center, SCREEN_MULTIPLIER (16.0f), 360.0f,
                                color::White);

        DrawHelper::DrawCircle (center, SCREEN_MULTIPLIER (14.0f), 360.0f,
                                ColorHelper::GetBackgroundColor ());

        DrawHelper::DrawCircle (center, SCREEN_MULTIPLIER (14.0f), angle,
                                GetEffectColor ());

        if (actualRemaining < 60000)
        {
            float timerPosX = x - 40;
            float timerPosY = drawLeft ? yLeft - 2.0f : yRight - 2.0f;

            if (drawLeft)
            {
                gamefont::Print (gamefont::LeftBottom, gamefont::AlignLeft,
                                 GenericUtil::FormatTime (actualRemaining,
                                                          true),
                                 timerPosX, timerPosY, FONT_DEFAULT,
                                 SUBTEXT_SCALE_X, SUBTEXT_SCALE_Y,
                                 GetTextColor (), 1, GetDropShadowColor (),
                                 true, 9999.0f, false);
            }
            else
            {
                gamefont::Print (gamefont::RightBottom, gamefont::AlignRight,
                                 GenericUtil::FormatTime (actualRemaining,
                                                          true),
                                 timerPosX, timerPosY, FONT_DEFAULT,
                                 SUBTEXT_SCALE_X, SUBTEXT_SCALE_Y,
                                 GetTextColor (), 1, GetDropShadowColor (),
                                 true, 9999.0f, false);
            }
        }
    }
    else
    {
        float textPosX = x - 50.0f;
        float textPosY = drawLeft ? yLeft - 4.0f : yRight - 4.0f;
        if (effect->HasSubtext ()) textPosY -= 6.0f;

        if (drawLeft)
        {
            gamefont::Print (gamefont::LeftBottom, gamefont::AlignLeft,
                             GenericUtil::FormatTime (actualRemaining),
                             textPosX, textPosY, FONT_DEFAULT, SUBTEXT_SCALE_X,
                             SUBTEXT_SCALE_Y, GetTextColor (), 1,
                             GetDropShadowColor (), true, 9999.0f, true);
        }
        else
        {
            gamefont::Print (gamefont::RightBottom, gamefont::AlignRight,
                             GenericUtil::FormatTime (actualRemaining),
                             textPosX, textPosY, FONT_DEFAULT, SUBTEXT_SCALE_X,
                             SUBTEXT_SCALE_Y, GetTextColor (), 1,
                             GetDropShadowColor (), true, 9999.0f, true);
        }
    }
}
void
EffectDrawHandler::Draw (EffectInstance *effect, int idx, bool inset,
                         bool drawLeft)
{
    this->effect   = effect;
    this->idx      = idx;
    this->isInset  = inset;
    this->drawLeft = drawLeft;

    UpdateTimers ();

    CalculateDrawPosition ();

    // Print subtext before effect name so it's behind the effect name
    PrintSubtext ();
    PrintEffectName ();

    if (effect->DoesEffectDrawTimer ()) PrintEffectTimer ();

#ifdef GTASA
    CFont::SetProportional (true);
#else
    CFont::SetPropOn ();
#endif
}

void
EffectDrawHandler::UpdateTimers ()
{
    /* Fade-in of the effect text (and timer)  */
    this->transitionTimer += GenericUtil::CalculateTick (0.0015f);

    this->transitionTimer = std::clamp (this->transitionTimer, 0.0f, 1.0f);

    /* Flashing text during effect start */
    if (textFlashTimer >= 0)
    {
        textFlashTimer -= (int) GenericUtil::CalculateTick (1.0f);
        textFlashingThisFrame = (textFlashTimer / 400) % 2;
    }
    else
        textFlashingThisFrame = false;
}

void
EffectDrawHandler::DrawAndXMore ()
{
    auto &effects = EffectHandler::GetActiveEffects ();

    int size = effects.size ();
    if (size <= RECENT_EFFECTS) return;

    int more = 0;
    for (int i = RECENT_EFFECTS; i < size; i++)
    {
        auto &effect = effects[i];
        if (effect.IsRunning ()) more++;
    }

    if (more == 0) return;

    std::string text = "And " + std::to_string (more) + " more...";

    if (Globals::enabledEffects["replace_all_text"])
        text = Globals::replaceAllTextString;

    if (drawLeft)
        yLeft = ((RECENT_EFFECTS + 2) * Y_SPACING) + 260.0f - 20.0f;
    else
        yRight = ((RECENT_EFFECTS + 2) * Y_SPACING) + 260.0f - 20.0f;

    if (drawLeft)
    {
        gamefont::Print (gamefont::LeftBottom, gamefont::AlignLeft, text, x,
                         yLeft, FONT_DEFAULT, SUBTEXT_SCALE_X, SUBTEXT_SCALE_Y,
                         color::DarkGray, 1, GetDropShadowColor (), true);
    }
    else
    {
        gamefont::Print (gamefont::RightBottom, gamefont::AlignRight, text, x,
                         yRight, FONT_DEFAULT, SUBTEXT_SCALE_X, SUBTEXT_SCALE_Y,
                         color::DarkGray, 1, GetDropShadowColor (), true);
    }
}

void
EffectDrawHandler::ResetOffsetCooldown (bool removeStaleEffects)
{
    int seconds = CONFIG ("Drawing.RotationSeconds", 10);
    seconds     = std::clamp (seconds, 3, 15);

    offsetCooldown = 1000 * seconds;

    if (removeStaleEffects)
    {
        EffectHandler::RemoveStaleEffects (false);
    }
}

void
EffectDrawHandler::DrawRecentEffects ()
{
    bool inset = AreEffectsInset (false);

    /* Timed Effects */
    auto &effects     = EffectHandler::GetActiveEffects ();
    int   effectsSize = effects.size ();

    if (effectsSize > RECENT_EFFECTS)
    {
        offsetCooldown -= GenericUtil::CalculateTick ();
        if (offsetCooldown <= 0)
        {
            ResetOffsetCooldown (true);

            offset = (offset + RECENT_EFFECTS) % effectsSize;
        }
    }
    else
    {
        offset = 0;

        ResetOffsetCooldown ();
    }

    int drawn = 0;
    for (int i = 0; i < effectsSize; i++)
    {
        int   index  = (i + offset) % effectsSize;
        auto &effect = effects[index];

        if (!Globals::enabledEffects["hide_chaos_ui"]
            || effect.GetEffect ()->GetID () == "effect_hide_chaos_ui")
        {
            if (++drawn > RECENT_EFFECTS) break;

            effect.Draw (CONFIG ("Drawing.EffectsTopToBottom", true)
                             ? RECENT_EFFECTS - drawn + 1
                             : drawn,
                         inset, true);
        }
    }

    if (!Globals::enabledEffects["hide_chaos_ui"]) DrawAndXMore ();

    int i = 0;
    inset = AreEffectsInset (true);
    for (auto &effect : EffectHandler::GetOneTimeEffects ())
    {
        if (++i > RECENT_EFFECTS) break;

        if (!Globals::enabledEffects["hide_chaos_ui"])
            effect.Draw (CONFIG ("Drawing.EffectsTopToBottom", true)
                             ? RECENT_EFFECTS - i + 1
                             : i,
                         inset, false);
    }
}

EffectDrawHandler::ScreensaverHUDElement
EffectDrawHandler::CreateHUDElement (EffectInstance *effect)
{
    return ScreensaverHUDElement{
        .pos           = CVector2D (effect->Random (SCREEN_COORD_LEFT (10.0f),
                                                    SCREEN_COORD_RIGHT (10.0f)),
                                    effect->Random (SCREEN_COORD_TOP (10.0f),
                                                    SCREEN_COORD_BOTTOM (10.0f))),
        .speedModifier = effect->Random (0.5f, 2.0f),
        .goingRight    = effect->Random (0, 1) == 0,
        .goingDown     = effect->Random (0, 1) == 0};
}

void
EffectDrawHandler::Tick ()
{
    if (!Globals::enabledEffects["screensaver_hud"] || CONFIG_CC_ENABLED)
        return;

    for (auto &effect : EffectHandler::GetActiveEffects ())
    {
        std::string name (effect.GetName ());
        if (!positions.contains (name))
        {
            positions[name] = CreateHUDElement (&effect);
        }
    }

    float tick = GenericUtil::CalculateTick (0.2f);

    for (auto &[name, element] : positions)
    {
        CVector2D &pos          = element.pos;
        float      adjustedTick = tick * element.speedModifier;

        if (element.goingRight)
        {
            pos.x += adjustedTick;
            if (pos.x >= SCREEN_COORD_RIGHT (20.0f)) element.goingRight = false;
        }
        else
        {
            pos.x -= adjustedTick;
            if (pos.x < 0.0f) element.goingRight = true;
        }

        if (element.goingDown)
        {
            pos.y += adjustedTick;
            if (pos.y >= SCREEN_COORD_BOTTOM (20.0f)) element.goingDown = false;
        }
        else
        {
            pos.y -= adjustedTick;
            if (pos.y < 0.0f) element.goingDown = true;
        }
    }
}

void
EffectDrawHandler::ClearScreensaverHUDMap ()
{
    positions.clear ();
}

CRGBA
EffectDrawHandler::GetTextColor () const
{
    CRGBA color (textColor);

    if (textFlashingThisFrame) color = flashColor;

    if (!effect->IsRunning () || !effect->DoesEffectDrawTimer ())
        color = disabledColor;

#ifdef GTASA
    if (CMenuSystem::num_menus_in_use && !effect->IsOneTimeEffect ())
        color.a = 10;
#endif

    return color;
};

CRGBA
EffectDrawHandler::GetDropShadowColor ()
{
    CRGBA color (color::Black);

#ifdef GTASA
    if (CMenuSystem::num_menus_in_use && !effect->IsOneTimeEffect ())
        color.a = 10;
#endif

    return color;
};

CRGBA
EffectDrawHandler::GetEffectColor () const
{
    return ColorHelper::GetForegroundColor ();
};
