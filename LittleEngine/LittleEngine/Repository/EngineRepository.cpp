#include "stdafx.h"
#include "EngineRepository.h"
#include "AsyncAssetLoader.h"
#include "LittleEngine/Services/Services.h"

namespace LittleEngine
{
EngineRepository::EngineRepository(const String& archivePath, const String& rootDir)
	: m_rootDir(rootDir)
{
	String fontID = "Fonts/main.ttf";
	m_archiveReader.Load(archivePath.c_str());
	if (!m_archiveReader.IsPresent(fontID.c_str()))
	{
		LOG_E("[EngineRepository] Cooked assets does not contain %s!", fontID.c_str());
	}
	else
	{
		UPtr<FontAsset> uDefaultFont =
			LoadInternal<FontAsset>(fontID, m_archiveReader.Decompress(fontID));
		if (uDefaultFont)
		{
			m_pDefaultFont = uDefaultFont.get();
			m_loaded.emplace(fontID, std::move(uDefaultFont));
		}
	}

#if DEBUGGING
	if (!m_pDefaultFont)
	{
		LOG_W("[EngineRepository] Could not load %s from Cooked assets!", fontID.c_str());
		m_pDefaultFont = Load<FontAsset>(fontID);
	}
#endif
	LOG_I("[EngineRepository] constructed");
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
			Load<TextureAsset>(definition.assetIDs.assetIDs);
			break;

		case AssetType::Font:
			Load<FontAsset>(definition.assetIDs.assetIDs);
			break;

		case AssetType::Sound:
			Load<SoundAsset>(definition.assetIDs.assetIDs);
			break;

		default:
			LOG_E("[EngineRepository] Unrecognised asset type [%d]!", definition.type);
			break;
		}
	});
}

AsyncAssetLoader* EngineRepository::LoadAsync(const String& manifestPath, const std::function<void()>& onComplete)
{
	UPtr<AsyncAssetLoader> uAsyncLoader = MakeUnique<AsyncAssetLoader>(*this, manifestPath, onComplete);
	AsyncAssetLoader* pLoader = uAsyncLoader.get();
	m_uAsyncLoaders.emplace_back(std::move(uAsyncLoader));
	return pLoader;
}

AsyncAssetLoader* EngineRepository::LoadAsync(const String& archivePath,
											  const String& manifestPath,
											  const std::function<void()>& onComplete)
{
	UPtr<AsyncAssetLoader> uAsyncLoader =
		MakeUnique<AsyncAssetLoader>(*this, archivePath, manifestPath, onComplete);
	AsyncAssetLoader* pLoader = uAsyncLoader.get();
	m_uAsyncLoaders.emplace_back(std::move(uAsyncLoader));
	return pLoader;
}

void EngineRepository::UnloadAll(bool bUnloadDefaultFont)
{
	if (bUnloadDefaultFont || !m_pDefaultFont)
	{
		m_loaded.clear();
	}
	else
	{
		String fontID = m_pDefaultFont->GetID();
		Core::CleanMap<String, UPtr<Asset>>(
			m_loaded, [fontID](UPtr<Asset>& uAsset) { return uAsset->GetID() != fontID; });
	}
	LOG_I("[EngineRepository] cleared");
}

bool EngineRepository::IsLoaded(const String& id)
{
	auto search = m_loaded.find(id);
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
	UnloadAll(true);
	Core::ArchiveReader::UnInit();
	LOG_I("[EngineRepository] destroyed");
}
} // namespace LittleEngine
