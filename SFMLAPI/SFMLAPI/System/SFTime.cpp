#include "stdafx.h"
#include "SFTime.h"
#include "Utils.h"
#include "SFML/System/Time.hpp"
#include "SFML/System/Clock.hpp"

#pragma comment(lib, "winmm.lib")
#if defined(_DEBUG)
#pragma comment(lib, "sfml-system-s-d.lib")
#else
#pragma comment(lib, "sfml-system-s.lib")
#endif

namespace LittleEngine
{
namespace
{
sf::Clock realTimeClock;
}

const Time Time::Zero = Time(0);

String Time::ToString(Time time)
{
	String hours;
	s32 h = time.AsSeconds() / 60 / 60;
	if (h > 0)
		hours = Strings::ToString(h) + ":";
	String mins;
	s32 m = (time.AsSeconds() / 60) - (h * 60);
	if (m > 0)
		mins = Strings::ToString(m) + ":";
	String secs;
	f32 s = (static_cast<f32>(time.AsMilliseconds()) / 1000.0f) - (h * 60 * 60) - (m * 60);
	if (s > 0)
		secs = Strings::ToString(s);
	return "[" + hours + mins + secs + "]";
}

Time Time::Microseconds(s64 microSeconds)
{
	return Time(microSeconds);
}

Time Time::Milliseconds(s32 milliSeconds)
{
	return Time(static_cast<s64>(milliSeconds) * 1000);
}

Time Time::Seconds(f32 seconds)
{
	return Time(static_cast<s64>(seconds * 1000.0f * 1000.0f));
}

Time Time::Now()
{
	return Time(static_cast<s64>(realTimeClock.getElapsedTime().asMicroseconds()));
}

Time Time::Clamp(Time val, Time min, Time max)
{
	if (val.microSeconds < min.microSeconds)
		return min;
	if (val.microSeconds > max.microSeconds)
		return max;
	return val;
}

void Time::RestartRealtimeClock()
{
	realTimeClock.restart();
}

Time::Time() : microSeconds(0)
{
}
Time::Time(s64 microSeconds) : microSeconds(microSeconds)
{
}

Time::Time(sf::Time& sfTime)
{
	microSeconds = sfTime.asMicroseconds();
}

LittleEngine::Time& Time::Scale(const Fixed& magnitude)
{
	microSeconds *= magnitude.ToF32();
	return *this;
}

Time& Time::operator-()
{
	microSeconds = -microSeconds;
	return *this;
}

Time& Time::operator+=(const Time& rhs)
{
	microSeconds += rhs.microSeconds;
	return *this;
}

Time& Time::operator-=(const Time& rhs)
{
	microSeconds -= rhs.microSeconds;
	return *this;
}

Time& Time::operator*=(const Time& rhs)
{
	microSeconds *= rhs.microSeconds;
	return *this;
}

Time& Time::operator/=(const Time& rhs)
{
	microSeconds = (rhs.microSeconds == 0) ? 0 : microSeconds /= rhs.microSeconds;
	return *this;
}

bool Time::operator==(const Time& rhs)
{
	return microSeconds == rhs.microSeconds;
}

bool Time::operator!=(const Time& rhs)
{
	return !(*this == rhs);
}

bool Time::operator<(const Time& rhs)
{
	return microSeconds < rhs.microSeconds;
}

bool Time::operator<=(const Time& rhs)
{
	return microSeconds <= rhs.microSeconds;
}

bool Time::operator>(const Time& rhs)
{
	return microSeconds > rhs.microSeconds;
}

bool Time::operator>=(const Time& rhs)
{
	return microSeconds >= rhs.microSeconds;
}

f32 Time::AsSeconds() const
{
	return static_cast<f32>(microSeconds) / (1000.0f * 1000.0f);
}

s32 Time::AsMilliseconds() const
{
	return static_cast<s32>(microSeconds / 1000);
}

s64 Time::AsMicroseconds() const
{
	return microSeconds;
}

sf::Time Time::_ToSFTime() const
{
	return sf::Time();
}

Time operator+(const Time& lhs, const Time& rhs)
{
	return Time(lhs) -= rhs;
}

Time operator-(const Time& lhs, const Time& rhs)
{
	return Time(lhs) -= rhs;
}

Time operator*(const Time& lhs, const Time& rhs)
{
	return Time(lhs) *= rhs;
}

Time operator/(const Time& lhs, const Time& rhs)
{
	return Time(lhs) /= rhs;
}

} // namespace LittleEngine
