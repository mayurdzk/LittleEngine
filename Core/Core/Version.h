#pragma once
#include "StdTypes.h"

namespace Core
{
struct Version
{
private:
	u32 major = 0;
	u32 minor = 0;
	u32 patch = 0;
	u32 pre = 0;

public:
	Version();
	Version(u32 major, u32 minor, u32 patch, u32 pre);
	Version(const char* szSerialised);

public:
	u32 Major() const;
	u32 Minor() const;
	u32 Patch() const;
	String ToString() const;

	bool operator==(const Version& rhs);
	bool operator!=(const Version& rhs);
	bool operator<(const Version& rhs);
	bool operator<=(const Version& rhs);
	bool operator>(const Version& rhs);
	bool operator>=(const Version& rhs);
};
} // namespace Core