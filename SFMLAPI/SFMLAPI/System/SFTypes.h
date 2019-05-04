#pragma once
#include "CoreTypes.h"
#include "SFML/Graphics.hpp"
#include "SFMLAPI/Rendering/Colour.h"
#include "SFMLAPI/Windowing/SFWindowData.h"

namespace LittleEngine
{
f32 Cast(Fixed fixed);
Fixed Cast(f32 value);

sf::Vector2f Cast(Vector2 vec2);
template <typename T>
Vector2 Cast(const sf::Vector2<T>& vec2);

sf::Color Cast(Colour colour);
Colour Cast(const sf::Color& sfColor);

sf::Uint8 Cast(SFWindowStyle style);

Vector2 WorldToScreen(Vector2 worldPoint);
Fixed WorldToScreen(Fixed worldOrientation);
Vector2 ScreenToWorld(Vector2 screenPoint);
Fixed ScreenToWorld(Fixed screenOrientation);

template <typename T>
Vector2 Cast(const sf::Vector2<T>& vec2)
{
	return Vector2(vec2.x, vec2.y);
}
} // namespace LittleEngine
