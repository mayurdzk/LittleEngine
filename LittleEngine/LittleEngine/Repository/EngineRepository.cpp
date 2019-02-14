#include "stdafx.h"
#include "EngineRepository.h"
#include "AsyncAssetLoader.h"
#include "LittleEngine/Services/Services.h"

namespace LittleEngine
{
EngineRepository::EngineRepository(const String& rootDir) : m_rootDir(rootDir)
{
	m_pDefaultFont = Load<FontAsset>("main.ttf");
	LogI("[AssetRepository] constructed");
}

FontAsset* EngineRepository::GetDefaultFont() const
{
	return m_pDefaultFont;
}

void EngineRepository::LoadAll(AssetManifest& manifest)
{
	manifest.ForEach([this](const AssetDefinition& definition) {
		switch (definition.type)
		{
		case AssetType::Texture:
			Load<TextureAsset>(definition.resourcePaths.assetPaths);
			break;

		case AssetType::Font:
			Load<FontAsset>(definition.resourcePaths.assetPaths);
			break;

		case AssetType::Sound:
			Load<SoundAsset>(definition.resourcePaths.assetPaths);
			break;

		case AssetType::Music:
			Load<MusicAsset>(definition.resourcePaths.assetPaths);
			break;

		default:
			LogE("[AssetRepository] Unrecognised asset type!");
			break;
		}
	});
}

void EngineRepository::LoadAsync(const String& manifestPath, Function(void()) OnCompleted)
{
	UPtr<AsyncAssetLoader> uAsyncLoader = MakeUnique<AsyncAssetLoader>(*this, manifestPath, OnCompleted);
	m_uAsyncLoaders.emplace_back(std::move(uAsyncLoader));
}

void EngineRepository::UnloadAll()
{
	m_loaded.clear();
	LogI("[AssetRepository] cleared");
}

String EngineRepository::GetPath(const String& path) const
{
	return m_rootDir.empty() ? path : m_rootDir + "/" + path;
}

bool EngineRepository::IsLoaded(const String& fullPath)
{
	auto search = m_loaded.find(fullPath);
	return search != m_loaded.end();
}

void EngineRepository::Tick(Time dt)
{
	auto iter = m_uAsyncLoaders.begin();
	while (iter != m_uAsyncLoaders.end())
	{
		(*iter)->Tick(dt);
		if ((*iter)->m_bIdle)
		{
			iter = m_uAsyncLoaders.erase(iter);
			continue;
		}
		++iter;
	}
}

EngineRepository::~EngineRepository()
{
	m_pDefaultFont = nullptr;
	UnloadAll();
	LogI("[AssetRepository] destroyed");
}
} // namespace LittleEngine
