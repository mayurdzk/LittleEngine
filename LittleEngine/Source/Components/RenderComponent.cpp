#include "stdafx.h"
#include "RenderComponent.h"
#include "Utils/Transform.h"
#include "Entities/Actor.h"
#include "Levels/Level.h"
#include "Engine/World.h"
#include "SFMLInterface/Assets.h"
#include "SFMLInterface/Rendering/RenderFactory.h"
#include "SFMLInterface/Rendering/ShapeRenderer.h"
#include "SFMLInterface/Rendering/SpriteRenderer.h"
#include "SFMLInterface/Rendering/TextRenderer.h"
#include "SFMLInterface/Rendering/RenderParams.h"

namespace Game {
	RenderComponent::RenderComponent(Actor& actor) 
		: Component(actor, "RenderComponent") {
	}

	LayerInfo RenderComponent::GetLayer() const {
		return renderer->layer;
	}

	void RenderComponent::SetLayer(const LayerInfo& layer) {
		renderer->layer = layer;
	}

	void RenderComponent::SetRenderer(std::unique_ptr<Renderer> renderer) {
		this->renderer = std::move(renderer);
	}

	Rect2 RenderComponent::GetBounds() const {
		return renderer == nullptr ? Rect2(Vector2::Zero, Vector2::Zero) : renderer->GetBounds();
	}

	CircleRenderer & RenderComponent::SetCircleRenderer(const ShapeData & shapeData) {
		Fixed radius = shapeData.size.x;
		std::unique_ptr<CircleRenderer> renderer = RenderFactory::NewCircle(radius, shapeData.colour);
		CircleRenderer* c = renderer.get();
		SetRenderer(std::move(renderer));
		return *c;
	}

	RectangleRenderer & RenderComponent::SetRectangleRenderer(const ShapeData & shapeData) {
		std::unique_ptr<RectangleRenderer> renderer = RenderFactory::NewRectangle(shapeData.size, shapeData.colour);
		RectangleRenderer* r = renderer.get();
		SetRenderer(std::move(renderer));
		return *r;
	}
	
	SpriteRenderer& RenderComponent::SetSpriteRenderer(const std::string & texturePath) {
		TextureAsset::Ptr texture = GetActor().GetActiveLevel().GetAssetManager().LoadAsset<TextureAsset>(texturePath);
		SpriteData spriteData(texture);
		SetRenderer(RenderFactory::NewSprite(spriteData));
		return *dynamic_cast<SpriteRenderer*>(renderer.get());
	}

	TextRenderer& RenderComponent::SetTextRenderer(const std::string & text) {
		FontAsset::Ptr font = GetActor().GetActiveLevel().GetAssetManager().GetDefaultFont();
		TextData textData(font, text);
		SetRenderer(RenderFactory::NewText(textData));
		return *dynamic_cast<TextRenderer*>(renderer.get());
	}

	void RenderComponent::Render(RenderParams params) {
		renderer->Render(params);
	}
}
