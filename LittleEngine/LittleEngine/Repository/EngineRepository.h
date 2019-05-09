#pragma once
#include <mutex>
#include "Core/CoreTypes.h"
#include "Core/Logger.h"

namespace Core
{
class ArchiveReader;
}

namespace LittleEngine
{
// \brief Class that handles all Asset Loading. (Maintains a clearable cache)
class EngineRepository final
{
private:
	using Lock = std::lock_guard<std::mutex>;

private:
	UPtr<Core::ArchiveReader> m_uCooked;
	List<UPtr<class ManifestLoader>> m_uAsyncLoaders;
	std::mutex m_mutex;
	UMap<String, Asset::Ptr> m_loaded;
	String m_rootDir;
	String m_assetPathPrefix;
	class FontAsset* m_pDefaultFont;

public:
	EngineRepository(String archivePath, String rootDir = "");
	~EngineRepository();

	// Loads Asset at path. T must derive from Asset!
	template <typename T>
	T* Load(String id);
	template <typename T>
	Deferred<T*> LoadAsync(String id);

	ManifestLoader* LoadAsync(String manifestPath, std::function<void()> onComplete = nullptr);

	FontAsset* GetDefaultFont() const;

	bool Unload(String id);
	// Unload all assets
	void UnloadAll(bool bUnloadDefaultFont);

public:
	EngineRepository(const EngineRepository&) = delete;
	EngineRepository& operator=(const EngineRepository&) = delete;

private:
	template <typename T>
	T* GetLoaded(const String& id);
	template <typename T>
	T* LoadFromArchive(const String& id);
#if ENABLED(FILESYSTEM_ASSETS)
	template <typename T>
	T* LoadFromFilesystem(const String& id);
#endif

	template <typename T>
	UPtr<T> CreateAsset(const String& id, Vec<u8> buffer);

#if ENABLED(FILESYSTEM_ASSETS)
	template <typename T>
	UPtr<T> RetrieveAsset(const String& id);
#endif

	bool IsLoaded(const String& id);
#if ENABLED(FILESYSTEM_ASSETS)
	bool DoesFileAssetExist(const String& id);
	String GetFileAssetPath(const String& id) const;
#endif

	void Tick(Time dt);

	friend class EngineService;
	friend class ManifestLoader;
};

template <typename T>
T* EngineRepository::Load(String id)
{
	static_assert(IsDerived<Asset, T>(),
				  "T must derive from Asset: check Output window for erroneous call");

	T* pT = GetLoaded<T>(id);
	if (pT)
	{
		return pT;
	}

	LOG_W(
		"[EngineRepository] Synchronously loading Asset (add id to manifest or use LoadAsync() to "
		"suppress warning) [%s]",
		id.c_str());

	bool bInCooked = m_uCooked->IsPresent(id.c_str());
#if ENABLED(FILESYSTEM_ASSETS)
	bool bOnFilesystem = DoesFileAssetExist(id);
	// Asset doesn't exist
	if (!bInCooked && !bOnFilesystem)
	{
		LOG_E("[EngineRepository] Asset not present in cooked archive or on filesystem! [%s]", id.c_str());
		return nullptr;
	}
	// Not in cooked archive (but on filesystem)
	if (!bInCooked)
	{
		LOG_W("[EngineRepository] Asset present on filesystem but not in cooked archive! [%s]", id.c_str());
	}
	// Not on filesystem (but in cooked archive)
	if (!bOnFilesystem)
	{
		LOG_W("[EngineRepository] Asset present in cooked archive but not on filesystem! [%s]", id.c_str());
		pT = LoadFromArchive<T>(id);
	}
	// On filesystem: load that regardless of cooked asset
	else
	{
		pT = LoadFromFilesystem<T>(id);
	}
#else
	if (!bInCooked)
	{
		LOG_E("[EngineRepository] Asset not present in cooked archive! [%s]", id.c_str());
	}
	else
	{
		pT = LoadFromArchive<T>(id);
	}
#endif
	return pT;
}

template <typename T>
Deferred<T*> EngineRepository::LoadAsync(String id)
{
	static_assert(IsDerived<Asset, T>(),
				  "T must derive from Asset: check Output window for erroneous call");
	// std::function needs to be copyable, so cannot use UPtr<promise> here
	SPtr<std::promise<T*>> sPromise = MakeShared<std::promise<T*>>();
	Deferred<T*> deferred = sPromise->get_future();

	T* pT = GetLoaded<T>(id);
	if (pT)
	{
		sPromise->set_value(pT);
		return deferred;
	}

	bool bInCooked = m_uCooked->IsPresent(id.c_str());
#if ENABLED(FILESYSTEM_ASSETS)
	bool bOnFilesystem = DoesFileAssetExist(id);
	if (!bInCooked && !bOnFilesystem)
	{
		LOG_E("[EngineRepository] Asset not present in cooked archive or on filesystem! [%s]", id.c_str());
		return deferred;
	}
	// Not in cooked archive (but on filesystem)
	if (!bInCooked)
	{
		LOG_W("[EngineRepository] Asset present on filesystem but not in cooked archive! [%s]", id.c_str());
	}
	// Not on filesystem (but in cooked archive)
	if (!bOnFilesystem)
	{
		LOG_W("[EngineRepository] Asset present in cooked archive but not on filesystem! [%s]", id.c_str());
		Services::Jobs()->Enqueue(
			[&, sPromise, id]() { sPromise->set_value(LoadFromArchive<T>(std::move(id))); }, "", true);
	}
	// On filesystem: load that regardless of cooked asset
	else
	{
		Services::Jobs()->Enqueue(
			[&, sPromise, id]() { sPromise->set_value(LoadFromFilesystem<T>(std::move(id))); }, "", true);
	}
#else
	if (!bInCooked)
	{
		LOG_E("[EngineRepository] Asset not present in cooked archive! [%s]", id.c_str());
		sPromise->set_value(nullptr);
	}
	else
	{
		Services::Jobs()->Enqueue(
			[&, sPromise, id]() { sPromise->set_value(LoadFromArchive<T>(std::move(id))); }, "", true);
	}
#endif
	return deferred;
}

template <typename T>
T* EngineRepository::GetLoaded(const String& id)
{
	Lock lock(m_mutex);
	auto search = m_loaded.find(id);
	if (search != m_loaded.end())
	{
		return dynamic_cast<T*>(search->second.get());
	}
	return nullptr;
}

template <typename T>
T* EngineRepository::LoadFromArchive(const String& id)
{
	T* pT = nullptr;
	UPtr<T> uT = CreateAsset<T>(id, m_uCooked->Decompress(id.c_str()));
	if (uT)
	{
		pT = uT.get();
		Lock lock(m_mutex);
		m_loaded.emplace(id, std::move(uT));
	}
	return pT;
}

#if ENABLED(FILESYSTEM_ASSETS)
template <typename T>
T* EngineRepository::LoadFromFilesystem(const String& id)
{
	T* pT = nullptr;
	UPtr<T> uT = RetrieveAsset<T>(id);
	if (uT)
	{
		pT = uT.get();
		Lock lock(m_mutex);
		m_loaded.emplace(id, std::move(uT));
	}
	return pT;
}
#endif

template <typename T>
UPtr<T> EngineRepository::CreateAsset(const String& id, Vec<u8> buffer)
{
	struct enable_smart : public T
	{
		enable_smart(String id, Vec<u8> buffer) : T(std::move(id), std::move(buffer))
		{
		}
	};

	UPtr<enable_smart> uT;
	uT = MakeUnique<enable_smart>(id, std::move(buffer));
	if (!uT || uT->IsError())
	{
		return nullptr;
	}
	LOG_I("[EngineRepository] Decompressed %s [%s]", g_szAssetType[ToIdx(uT->GetType())], id.c_str());
	return std::move(uT);
}

#if ENABLED(FILESYSTEM_ASSETS)
template <typename T>
UPtr<T> EngineRepository::RetrieveAsset(const String& id)
{
	struct enable_smart : public T
	{
		enable_smart(String id, const String& pathPrefix) : T(std::move(id), pathPrefix)
		{
		}
	};

	UPtr<enable_smart> uT;
	uT = MakeUnique<enable_smart>(id, m_rootDir);
	if (!uT || uT->IsError())
	{
		return nullptr;
	}
	LOG_I("[EngineRepository] Loaded %s from filesystem [%s]", g_szAssetType[ToIdx(uT->GetType())], id.c_str());
	return std::move(uT);
}
#endif
} // namespace LittleEngine
