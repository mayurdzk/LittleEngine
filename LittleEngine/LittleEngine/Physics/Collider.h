#pragma once
#include "ColliderData.h"

namespace LittleEngine
{
class Collider
{
public:
#if DEBUGGING
	static u32 s_debugShapeWidth;
	static bool s_bShowDebugShape;
#endif

public:
	Vector2 m_position;
	String m_name;
	s32 m_ignoreSig = 0;
	class Entity* m_pOwner = nullptr;
	bool m_bEnabled = true;
	bool m_bDestroyed = false;

public:
	virtual ~Collider();

	void OnHit(const Collider& other);

	virtual bool IsIntersecting(const Collider& other) const = 0;

protected:
	virtual bool IsIntersectAABB(const class AABBCollider& other) const = 0;
	virtual bool IsIntersectCircle(const class CircleCollider& other) const = 0;

	friend class CircleCollider;
	friend class AABBCollider;
};

class CircleCollider : public Collider
{
private:
	CircleData m_circle = CircleData::One;

public:
	CircleCollider(String ownerName = "");

	void SetCircle(Fixed diameter);
	CircleData GetWorldCircle() const;

	bool IsIntersecting(const Collider& other) const override;

protected:
	bool IsIntersectAABB(const class AABBCollider& other) const override;
	bool IsIntersectCircle(const CircleCollider& other) const override;

	friend class Collider;
	friend class AABBCollider;
};

class AABBCollider : public Collider
{
private:
	AABBData m_aabb;

public:
	AABBCollider(String ownerName = "");

	void SetAABB(const AABBData& aabb);
	AABBData GetWorldAABB() const;

	bool IsIntersecting(const Collider& other) const override;

protected:
	bool IsIntersectAABB(const AABBCollider& other) const override;
	bool IsIntersectCircle(const CircleCollider& other) const override;

private:
	friend class Collider;
	friend class CircleCollider;
};
} // namespace LittleEngine
