#include "stdafx.h"
#include "Maths.h"

namespace Maths
{
Rect2 Lerp(Rect2 min, Rect2 max, Fixed alpha)
{
	Vector2 tl = Maths::Lerp(min.topLeft, max.topLeft, alpha);
	Vector2 br = Maths::Lerp(min.bottomRight, max.bottomRight, alpha);
	return Rect2{tl, br};
}

bool IsNearlyEqual(f32 lhs, f32 rhs, f32 epsilon)
{
	return Abs(lhs - rhs) < epsilon;
}

Fixed ComputeAlpha(Time dt, Time totalTime)
{
	return Fixed(Clamp01(dt.AsSeconds() / totalTime.AsSeconds()));
}

Random::Random(s32 min, s32 max) : m_detMt(1729), m_distribution(min, max)
{
	m_NDetMt = std::mt19937(m_randomDevice());
}

void Random::Seed(s32 seed)
{
	m_detMt = std::mt19937(seed);
}

s32 Random::Range(s32 min, s32 max)
{
	static std::random_device device;
	static std::mt19937 nDetMt(device());
	std::uniform_int_distribution<s32> distribution(min, max);
	return distribution(nDetMt);
}

size_t Random::Range(size_t min, size_t max)
{
	return static_cast<size_t>(Range(static_cast<s32>(min), static_cast<s32>(max)));
}

Fixed Random::Range(Fixed min, Fixed max, u32 precision)
{
	s32 sMin = static_cast<s32>(min.ToF32() * precision);
	s32 sMax = static_cast<s32>(max.ToF32() * precision);
	s32 random = Range(sMin, sMax);
	return Fixed(random, precision);
}

s32 Random::NextDeterministic()
{
	return m_distribution(m_detMt);
}

s32 Random::NextNonDeterministic()
{
	return m_distribution(m_NDetMt);
}

s32 Random::Next(bool bDeterministic)
{
	return bDeterministic ? NextDeterministic() : NextNonDeterministic();
}
} // namespace Maths
