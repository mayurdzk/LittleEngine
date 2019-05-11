#include "stdafx.h"
#include "Core/Logger.h"
#include "SFMLAPI/Rendering/SFLayerID.h"
#include "SFMLAPI/Rendering/SFPrimitive.h"
#include "SFMLAPI/System/SFAssets.h"
#include "LittleEngine/Game/GameManager.h"
#include "LittleEngine/Services/Services.h"
#include "Player.h"
#include "GameFramework/Components/Components.h"

namespace LittleEngine
{
PlayerCollider::PlayerCollider(AABBData bounds, Vector2 offset)
	: bounds(std::move(bounds)), offset(offset)
{
}

PlayerData::PlayerData(TextureAsset& mainTexture, Vec<PlayerCollider> colliders)
	: pMainTexture(&mainTexture), colliders(std::move(colliders))
{
}

void Player::OnCreated()
{
	SetType("Player");
}

void Player::InitPlayer(PlayerData data)
{
	auto pRenderComponent = AddComponent<RenderComponent>();
	LayerID layer = static_cast<LayerID>(LAYER_LIVE + 5);
	pRenderComponent->SetSprite(*data.pMainTexture, layer);

	auto pCollisionComponent = AddComponent<CollisionComponent>();
	for (auto& collider : data.colliders)
	{
		pCollisionComponent->AddAABB(collider.bounds, collider.offset);
	}
	AddComponent<ControllerComponent>();

	LOG_I("%s %s", LogNameStr(), " Initialised");
}
} // namespace LittleEngine
