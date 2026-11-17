#ifndef GC_DAY_NIGHT_CYCLE_H
#define GC_DAY_NIGHT_CYCLE_H

#include <cmath>

struct DayNightState
{
    D3DXVECTOR3 sunDirection, moonDirection, lightDirection;
    D3DXVECTOR3 lightColor, ambient, sunDiskColor;
    float lightIntensity, daylight, twilight, starsVisibility;
    float sunVisibility, moonVisibility, starRotation;
};

// Pure time/lighting calculation, independent of rendering and frame rate.
class DayNightCycle
{
public:
    double hour;
    float dayLengthSeconds;
    bool paused;

    DayNightCycle() : hour(9.0), dayLengthSeconds(300.0f), paused(false) {}

    void SetHour(double value)
    {
        if (!std::isfinite(value)) return;
        hour = std::fmod(value, 24.0);
        if (hour < 0) hour += 24.0;
    }

    void Advance(double seconds)
    {
        if (paused || !std::isfinite(seconds) || seconds <= 0
            || !std::isfinite(dayLengthSeconds) || dayLengthSeconds <= 0) return;
        SetHour(hour + seconds * 24.0 / dayLengthSeconds);
    }

    DayNightState GetState() const
    {
        DayNightState state;
        const float angle = static_cast<float>((hour - 6.0) * D3DX_PI / 12.0);
        // Tilted orbit: rises in the east at 06:00, culminates at 65 degrees,
        // sets in the west at 18:00. The full moon follows the opposite half.
        const D3DXVECTOR3 east(-0.458f, 0, 0.889f);
        const float tilt = D3DXToRadian(25.0f);
        D3DXVECTOR3 rising = east;
        D3DXVec3Normalize(&rising, &rising);
        const D3DXVECTOR3 zenith(rising.z * sinf(tilt), cosf(tilt), -rising.x * sinf(tilt));
        state.sunDirection = rising * cosf(angle) + zenith * sinf(angle);
        D3DXVec3Normalize(&state.sunDirection, &state.sunDirection);
        state.moonDirection = -state.sunDirection;
        const float height = state.sunDirection.y;
        const float moonHeight = state.moonDirection.y;
        state.daylight = Smooth(-0.16f, 0.25f, height);
        state.twilight = Smooth(-0.24f, -0.02f, height) * (1.0f - Smooth(0.02f, 0.36f, height));
        state.starsVisibility = 1.0f - Smooth(-0.30f, -0.07f, height);
        state.sunVisibility = Smooth(-0.045f, 0.035f, height);
        state.moonVisibility = (1.0f - Smooth(-0.12f, 0.10f, height)) * Smooth(-0.045f, 0.04f, moonHeight);
        state.starRotation = static_cast<float>(hour * D3DX_PI / 12.0);
        const float highSun = Smooth(0.02f, 0.42f, height);
        state.sunDiskColor = Mix(D3DXVECTOR3(1.0f, 0.40f, 0.10f), D3DXVECTOR3(1.0f, 0.98f, 0.87f), highSun);
        // Bright moonlit nights keep paths and objects readable, including shadows.
        state.ambient = Mix(D3DXVECTOR3(0.32f, 0.38f, 0.48f), D3DXVECTOR3(0.30f, 0.35f, 0.42f),
            Smooth(-0.22f, 0.30f, height));

        // Switch the shadow map's light only at the horizon, where both direct
        // intensities reach zero. This avoids a visible jump in shadow direction.
        if (height >= 0)
        {
            state.lightDirection = state.sunDirection;
            state.lightColor = Mix(D3DXVECTOR3(1.0f, 0.48f, 0.23f), D3DXVECTOR3(1.0f, 0.95f, 0.84f), highSun);
            state.lightIntensity = 0.95f * Smooth(0.0f, 0.22f, height);
        }
        else
        {
            state.lightDirection = state.moonDirection;
            state.lightColor = D3DXVECTOR3(0.72f, 0.82f, 1.0f);
            state.lightIntensity = 0.60f * Smooth(0.0f, 0.25f, moonHeight);
        }
        return state;
    }

private:
    static float Smooth(float low, float high, float value)
    {
        float t = (value - low) / (high - low);
        if (t < 0) t = 0;
        if (t > 1) t = 1;
        return t * t * (3.0f - 2.0f * t);
    }
    static D3DXVECTOR3 Mix(const D3DXVECTOR3& a, const D3DXVECTOR3& b, float t)
    {
        return a + (b - a) * t;
    }
};

#endif
