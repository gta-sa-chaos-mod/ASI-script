#include "util/EffectBase.h"
#include "util/GenericUtil.h"

#include <CAudioEngine.h>
#include <eAudioEvents.h>

using namespace plugin;

// TODO: Explode BMX separately

class MinimumSpeedEffect : public EffectBase
{
    static inline const float VELOCITY_CONST = 0.277778f / 50.f;
    static inline const float MAX_SPEED      = 80.0f;

    static inline CVehicle *lastVehicle;
    static inline int       timeLeft     = 1000 * 10;
    static inline float     currentSpeed = 0.0f;

    int blowUpDelay  = 500;
    int beepCooldown = 1000;

public:
    bool
    CanActivate () override
    {
        return FindPlayerVehicle (-1, false);
    }

    void
    OnStart (EffectInstance *inst) override
    {
        lastVehicle = FindPlayerVehicle (-1, false);
        if (lastVehicle)
        {
            currentSpeed = lastVehicle->m_vecMoveSpeed.Magnitude ();
        }

        timeLeft     = 1000 * 10;
        blowUpDelay  = 500;
        beepCooldown = 1000;

        Events::drawHudEvent.before += OnDraw;
    }

    void
    OnEnd (EffectInstance *inst) override
    {
        Events::drawHudEvent.before -= OnDraw;
    }

    void
    OnTick (EffectInstance *inst) override
    {
        if (!EnsureVehicle ()) return;

        if (GameUtil::IsCutsceneProcessing ()) return;

        currentSpeed = GetVehicleSpeed ();

        int tick = (int) GenericUtil::CalculateTick ();

        if (currentSpeed >= GetMaxAllowedSpeed ())
            timeLeft += tick;
        else
        {
            timeLeft -= tick;
            DoBeepIfNecessary ();
        }

        timeLeft = std::clamp (timeLeft, 0, 1000 * 10);

        if (timeLeft == 0)
        {
            BlowUpVehicle (lastVehicle);

            timeLeft     = 1000 * 10;
            beepCooldown = 1000;
        }
    }

    float
    GetVehicleSpeed ()
    {
        if (!IsVehiclePointerValid (lastVehicle)) return 0.0f;

        float currentSpeed
            = lastVehicle->m_vecMoveSpeed.Magnitude () / VELOCITY_CONST;

        return std::abs (currentSpeed);
    }

    static float
    GetMaxAllowedSpeed ()
    {
        return MAX_SPEED;

        // Calculating vehicle max speeds is a bit unreliable so for now
        // we'll just revert to a set 80km/h as per the movie.

        // if (!IsVehiclePointerValid (lastVehicle)) return MAX_SPEED;

        // return lastVehicle->m_pHandlingData->m_transmissionData
        //            .m_fMaxGearVelocity
        //        / VELOCITY_CONST;
    }

    bool
    EnsureVehicle ()
    {
        CPlayerPed *player = FindPlayerPed ();
        if (!player || !player->IsAlive ())
        {
            lastVehicle = nullptr;
            return false;
        }

        CVehicle *vehicle = FindPlayerVehicle (-1, false);
        if (IsVehiclePointerValid (lastVehicle) && vehicle != lastVehicle)
        {
            blowUpDelay -= (int) GenericUtil::CalculateTick ();
            if (blowUpDelay < 0)
            {
                BlowUpVehicle (lastVehicle);
                lastVehicle = nullptr;
            }
            return false;
        }

        lastVehicle = nullptr;

        if (IsVehiclePointerValid (vehicle) && vehicle->CanBeDriven ()
            && vehicle->IsDriver (FindPlayerPed ())
            && vehicle->m_nStatus != STATUS_WRECKED)
        {
            lastVehicle = vehicle;
            blowUpDelay = 500;
        }
        else
        {
            timeLeft     = 1000 * 10;
            beepCooldown = 1000;
        }

        return lastVehicle != nullptr;
    }

    void
    BlowUpVehicle (CVehicle *vehicle)
    {
        if (IsVehiclePointerValid (vehicle) && vehicle->CanBeDriven ()
            && vehicle->m_nStatus != STATUS_WRECKED)
        {
            vehicle->m_nPhysicalFlags.bExplosionProof = false;
            vehicle->BlowUpCar (nullptr, false);
        }
    }

    void
    DoBeepIfNecessary ()
    {
        beepCooldown -= (int) GenericUtil::CalculateTick ();
        if (beepCooldown > 0) return;

        AudioEngine.ReportFrontendAudioEvent (AE_FRONTEND_PICKUP_INFO, 0.0f,
                                              1.5f);

        beepCooldown = std::round (timeLeft / 10);
        beepCooldown = std::clamp (beepCooldown, 100, 1000);
    }

    static void
    OnDraw ()
    {
        if (GenericUtil::IsMenuActive () || !lastVehicle) return;

        // Calculate between 0.0f and 1.0f
        float timeLeftWidth = (float) timeLeft / (1000 * 10.0f);

        CRGBA rectColor = color::ForestGreen;
        if (timeLeft < 7000) rectColor = color::Orange;
        if (timeLeft < 4000) rectColor = color::Red;
        if (timeLeft < 1000) rectColor = color::DarkRed;

        float x      = 70.0f;
        float y      = 300.0f;
        float width  = 200.0f;
        float height = 30.0f;

        float left   = SCREEN_MULTIPLIER (x);
        float top    = SCREEN_COORD_BOTTOM (y);
        float right  = SCREEN_MULTIPLIER (x + width);
        float bottom = SCREEN_COORD_BOTTOM (y - height);

        float rightFilled = SCREEN_MULTIPLIER (x + width * timeLeftWidth);

        CRect fullWidth   = CRect (left, top, right, bottom);
        CRect filledWidth = CRect (left, top, rightFilled, bottom);

        CSprite2d::DrawRect (fullWidth, color::DarkGray);
        CSprite2d::DrawRect (filledWidth, rectColor);

        char speedBuffer[64];
        sprintf_s (speedBuffer, "%.0f", currentSpeed);

        CRGBA speedColor = currentSpeed > GetMaxAllowedSpeed () ? color::White
                                                                : color::Orange;

        gamefont::PrintUnscaled (speedBuffer,
                                 left + (SCREEN_MULTIPLIER (width) / 2),
                                 top + SCREEN_MULTIPLIER (2.0f), FONT_DEFAULT,
                                 SCREEN_MULTIPLIER (0.8f),
                                 SCREEN_MULTIPLIER (0.8f), speedColor,
                                 gamefont::AlignCenter, 1, color::Black, true,
                                 9999.0F, false);
    }
};

DEFINE_EFFECT (MinimumSpeedEffect, "effect_minimum_speed", 0);