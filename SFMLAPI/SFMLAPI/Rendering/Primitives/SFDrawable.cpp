#include "stdafx.h"
#include "Core/Logger.h"
#include "SFDrawable.h"
#include "SFMLAPI/System/SFTypes.h"

namespace LittleEngine
{
void ASFDrawable::TDrawableState::Reconcile()
{
	tPivot.min = tPivot.max;
	tSecondaryColour.min = tSecondaryColour.max;
	tOutline.min = tOutline.max;
}

ASFDrawable::ASFDrawable(LayerID layer) : APrimitive(layer)
{
}

ASFDrawable::~ASFDrawable() = default;

ASFDrawable* ASFDrawable::SetPivot(Vector2 pivot, bool bImmediate)
{
	if (bImmediate)
	{
		m_drawableGameState.tPivot.Reset(WorldToScreen(pivot));
	}
	else
	{
		m_drawableGameState.tPivot.Update(WorldToScreen(pivot));
	}
	if (m_bStatic || m_bMakeStatic)
	{
		ReconcileGameState();
		m_bStatic = false;
		m_bMakeStatic = true;
	}
	return this;
}

ASFDrawable* ASFDrawable::SetOutline(Fixed outline, bool bImmediate)
{
	if (bImmediate)
	{
		m_drawableGameState.tOutline.Reset(outline);
	}
	else
	{
		m_drawableGameState.tOutline.Update(outline);
	}
	if (m_bStatic || m_bMakeStatic)
	{
		ReconcileGameState();
		m_bStatic = false;
		m_bMakeStatic = true;
	}
	return this;
}

ASFDrawable* ASFDrawable::SetSecondaryColour(Colour colour, bool bImmediate)
{
	if (bImmediate)
	{
		m_drawableGameState.tSecondaryColour.Reset(colour);
	}
	else
	{
		m_drawableGameState.tSecondaryColour.Update(colour);
	}
	if (m_bStatic || m_bMakeStatic)
	{
		ReconcileGameState();
		m_bStatic = false;
		m_bMakeStatic = true;
	}
	return this;
}

Vector2 ASFDrawable::GetPivot() const
{
	return m_drawableGameState.tPivot.max;
}

Fixed ASFDrawable::GetOutline() const
{
	return m_drawableGameState.tOutline.max;
}

Colour ASFDrawable::GetSecondaryColour() const
{
	return m_drawableGameState.tSecondaryColour.max;
}

void ASFDrawable::OnSwapState()
{
	m_drawableRenderState = m_drawableGameState;
}

void ASFDrawable::ReconcileGameState()
{
	APrimitive::ReconcileGameState();

	m_drawableGameState.Reconcile();
}

ASFDrawable::DrawableState ASFDrawable::GetDrawableState(Fixed alpha) const
{
	DrawableState ret;
	Vector2 offset = Fixed::OneHalf * GetBounds().GetSize();
	Vector2 pivot = m_drawableRenderState.tPivot.Lerp(alpha);
	ret.origin = Vector2(offset.x * pivot.x, offset.y * pivot.y) + offset;
	//ret.origin = m_drawableRenderState.tPivot.Lerp(alpha) + offset;
	ret.secondary = Colour::Lerp(m_drawableRenderState.tSecondaryColour, alpha);
	ret.outline = m_drawableRenderState.tOutline.Lerp(alpha);
	return ret;
}
}