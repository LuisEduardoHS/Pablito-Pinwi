// Standalone CPU regression check. Build in an x86 VS developer prompt with
// the June 2010 DirectX SDK Include/Lib/x86 directories configured.
#include <windows.h>
#include <d3dx10.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include "../DayNightCycle.h"
#pragma comment(lib, "d3dx10.lib")

static void Check(bool condition, const char* message)
{
    if (!condition) { fprintf(stderr, "FAIL: %s\n", message); exit(1); }
}
static float Distance(const D3DXVECTOR3& a, const D3DXVECTOR3& b)
{
    const D3DXVECTOR3 delta = a - b;
    return D3DXVec3Length(&delta);
}
int main()
{
    DayNightCycle cycle;
    cycle.SetHour(12);
    const DayNightState noon = cycle.GetState();
    Check(noon.sunDirection.y > 0.90f && noon.daylight == 1 && noon.starsVisibility == 0,
        "noon must be sunny with no stars");
    Check(noon.sunVisibility == 1 && noon.moonVisibility == 0, "daytime celestial visibility");
    cycle.SetHour(0);
    const DayNightState midnight = cycle.GetState();
    Check(midnight.daylight == 0 && midnight.starsVisibility == 1, "nighttime sky");
    Check(midnight.moonVisibility == 1 && midnight.sunVisibility == 0, "nighttime moon");
    Check(Distance(midnight.lightDirection, midnight.moonDirection) < 0.00001f,
        "moon must drive night lighting and shadows");
    Check(midnight.ambient.x >= 0.04f && midnight.lightIntensity > 0.15f, "night remains playable");
    cycle.SetHour(24);
    Check(Distance(cycle.GetState().sunDirection, midnight.sunDirection) < 0.00001f, "midnight wrap");
    cycle.SetHour(-1);
    Check(cycle.hour == 23, "negative hour normalization");
    cycle.SetHour(9);
    cycle.Advance(300);
    Check(fabs(cycle.hour - 9) < 0.000001, "one real cycle lasts five minutes");
    cycle.paused = true;
    cycle.Advance(120);
    Check(cycle.hour == 9, "pause freezes the clock");
    cycle.paused = false;
    cycle.Advance(std::numeric_limits<double>::quiet_NaN());
    cycle.Advance(-1);
    Check(cycle.hour == 9, "invalid elapsed times are ignored");
    DayNightCycle lowFPS, highFPS;
    for (int i = 0; i < 300 * 15; ++i) lowFPS.Advance(1.0 / 15);
    for (int i = 0; i < 300 * 144; ++i) highFPS.Advance(1.0 / 144);
    Check(fabs(lowFPS.hour - highFPS.hour) < 0.000001, "clock is independent of frame rate");

    cycle.SetHour(0);
    DayNightState previous = cycle.GetState();
    for (int minute = 1; minute <= 24 * 60; ++minute)
    {
        cycle.SetHour(minute / 60.0);
        const DayNightState state = cycle.GetState();
        Check(fabs(D3DXVec3Length(&state.sunDirection) - 1) < 0.00001f, "unit sun direction");
        Check(Distance(state.sunDirection, -state.moonDirection) < 0.00001f, "opposite sun and moon");
        Check(Distance(state.sunDirection, previous.sunDirection) < 0.005f, "continuous orbit");
        Check(Distance(state.ambient, previous.ambient) < 0.02f, "smooth ambient transition");
        Check(Distance(state.lightColor * state.lightIntensity,
            previous.lightColor * previous.lightIntensity) < 0.05f, "smooth sun/moon handoff");
        Check(fabs(state.starsVisibility - previous.starsVisibility) < 0.04f, "smooth stars");
        Check(std::isfinite(state.lightIntensity) && std::isfinite(state.starRotation), "finite cycle state");
        previous = state;
    }
    puts("PASS: day/night, sun/moon handoff, midnight wrap, pause, five-minute cycle, FPS independence, 1440 transition samples.");
    return 0;
}
