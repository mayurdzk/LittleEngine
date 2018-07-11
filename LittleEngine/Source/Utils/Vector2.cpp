#include "stdafx.h"
#include <string>
#include <math.h>
#include "Vector2.h"

const Vector2 Vector2::Zero = Vector2(0, 0);
const Vector2 Vector2::One = Vector2(1, 1);

Vector2 Vector2::operator+(const Vector2& rhs) const {
	return Vector2(*this) += rhs;
}

Vector2 Vector2::operator-(const Vector2& rhs) const {
	return Vector2(*this) -= rhs;
}

Vector2& Vector2::operator+=(const Vector2& rhs) {
	x += rhs.x;
	y += rhs.y;
	return *this;
}

Vector2& Vector2::operator-=(const Vector2& rhs) {
	x -= rhs.x;
	y -= rhs.y;
	return *this;
}

Vector2& Vector2::operator*=(const Fixed& fixed) {
	x *= fixed;
	y *= fixed;
	return *this;
}

Vector2& Vector2::operator/=(const Fixed& fixed) {
	x /= fixed;
	y /= fixed;
	return *this;
}

Vector2 Vector2::operator*(const Fixed& rhs) const {
	return Vector2(*this) *= rhs;
}

Vector2 Vector2::operator/(const Fixed & rhs) const {
	return Vector2(*this) /= rhs;
}

Fixed Vector2::Dot(const Vector2 & rhs) const {
	return Fixed((x * rhs.x) + (y * rhs.y));
}

bool Vector2::operator==(const Vector2& rhs) const {
	return x == rhs.x && y == rhs.y;
}

bool Vector2::operator!=(const Vector2& rhs) const {
	return !(*this == rhs);
}

Vector2 Vector2::Normalised() const {
	double sq = SqrMagnitude();
	Fixed coeff = Fixed(std::sqrt(sq));
	coeff = coeff.Inverse();
	return Vector2(x * coeff, y * coeff);
}

void Vector2::Normalise() {
	*this = Normalised();
}

Fixed Vector2::Magnitude() const {
	return Fixed(std::sqrt(SqrMagnitude()));
}

double Vector2::SqrMagnitude() const {
	return (x.GetDouble() * x.GetDouble()) + (y.GetDouble() * y.GetDouble());
}

std::string Vector2::ToString() const {
	return "[" + x.ToString() + ", " + y.ToString() + "]";
}
