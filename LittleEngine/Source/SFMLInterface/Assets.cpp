#include "le_stdafx.h"
#include "Assets.h"
#include "Engine/Logger/Logger.h"
#include "FileRW.h"

namespace LittleEngine {
	using FileRW = GameUtils::FileRW;

	void AssetManifest::AddDefinition(AssetDefinition && definition) {
		definitions.push_back(std::move(definition));
	}

	void AssetManifest::AddDefinition(const AssetType & type, AssetPaths&& resourcePaths) {
		definitions.emplace_back(type, std::move(resourcePaths));
	}

	void AssetManifest::Clear() {
		definitions.clear();
	}

	void AssetManifest::ForEach(std::function<void(const AssetDefinition& definition)> Callback) const {
		for (const auto& definition : definitions) {
			Callback(definition);
		}
	}

	AssetManifestData::AssetManifestData(const std::string& amfPath) {
		Load(amfPath);
	}

	AssetManifest & AssetManifestData::GetManifest() {
		return manifest;
	}

	static AssetPaths Deserialise(GData& gData) {
		return AssetPaths(gData.GetString("pathPrefix"), gData.GetInt("count"), gData.GetString("assetPrefix"), gData.GetString("assetSuffix"));
	}

	void AssetManifestData::Load(const std::string & amfPath) {
		FileRW reader(amfPath);
		GData data(reader.ReadAll(true));
		manifest.Clear();
		auto textures = data.GetVector("textures");
		if (!textures.empty()) {
			for (const auto& tPath : textures) {
				manifest.AddDefinition(AssetType::Texture, tPath);
			}
		}
		auto sounds = data.GetVector("sounds");
		if (!sounds.empty()) {
			for (const auto& sPath : sounds) {
				manifest.AddDefinition(AssetType::Sound, sPath);
			}
		}
		auto music = data.GetVector("music");
		if (!music.empty()) {
			for (const auto& mPath : music) {
				manifest.AddDefinition(AssetType::Music, mPath);
			}
		}
		auto fonts = data.GetVector("fonts");
		if (!fonts.empty()) {
			for (const auto& fPath : fonts) {
				manifest.AddDefinition(AssetType::Font, fPath);
			}
		}
		auto textureSets = data.GetVectorGData("textureSets");
		if (!textureSets.empty()) {
			for (auto& tSet : textureSets) {
				manifest.AddDefinition(AssetType::Texture, Deserialise(tSet));
			}
		}
		auto soundSets = data.GetVectorGData("soundSets");
		if (!soundSets.empty()) {
			for (auto& sSet : soundSets) {
				manifest.AddDefinition(AssetType::Sound, Deserialise(sSet));
			}
		}
	}

	Asset::Asset(const std::string& path) : resourcePath(path) {}

	Asset::~Asset() {
		Logger::Log("[" + resourcePath + "] (Asset) destroyed", Logger::Severity::Debug);
	}

	const std::string & Asset::GetResourcePath() const {
		return resourcePath;
	}

	TextureAsset::TextureAsset(const std::string& path) : Asset(path) {
		if (!sfTexture.loadFromFile(resourcePath)) {
			Logger::Log("Could not load texture from [" + resourcePath + "]!", Logger::Severity::Error);
		}
	}

	FontAsset::FontAsset(const std::string & path) : Asset(path) {
		if (!sfFont.loadFromFile(resourcePath)) {
			Logger::Log("Could not load font from [" + resourcePath + "]!", Logger::Severity::Error);
		}
	}

	SoundAsset::SoundAsset(const std::string & path, const Fixed& volumeScale) : Asset(path), volumeScale(Maths::Clamp01(volumeScale)) {
		if (!sfSoundBuffer.loadFromFile(path)) {
			Logger::Log("Could not load sound from [" + resourcePath + "]!", Logger::Severity::Error);
		}
	}

	MusicAsset::MusicAsset(const std::string & path, const Fixed& volumeScale) : Asset(path), volumeScale(Maths::Clamp01(volumeScale)) {
		valid = music.openFromFile(path);
		if (!valid) {
			Logger::Log("Could not load music from [" + resourcePath + "]!", Logger::Severity::Error);
		}
	}

	Fixed MusicAsset::GetDurationSeconds() const {
		return Fixed(music.getDuration().asSeconds());
	}

	AssetPaths::AssetPaths(const std::string & assetPath) {
		assetPaths.push_back(assetPath);
	}

	AssetPaths::AssetPaths(std::initializer_list<std::string> assetPaths) {
		for (const auto& path : assetPaths) {
			this->assetPaths.push_back(path);
		}
	}

	AssetPaths::AssetPaths(const std::string & pathPrefix, std::initializer_list<std::string> assetPaths) {
		std::string prefix = pathPrefix.empty() ? "" : pathPrefix + "/";
		for (const auto& path : assetPaths) {
			this->assetPaths.emplace_back(prefix + path);
		}
	}

	AssetPaths::AssetPaths(const std::string & pathPrefix, int count, const std::string & assetPrefix, const std::string& assetSuffix) {
		std::string prefix = pathPrefix.empty() ? "" : pathPrefix + "/";
		for (int i = 0; i < count; ++i) {
			std::string suffix = (i < 10) ? "0" + std::to_string(i) : std::to_string(i);
			suffix += assetSuffix;
			this->assetPaths.push_back(prefix + assetPrefix + suffix);
		}
	}

	std::string AssetPaths::GetRandom() const {
		size_t index = Maths::Random::Range(0, static_cast<int>(assetPaths.size()));
		return assetPaths[index];
	}

	AssetManager::AssetManager(const std::string& rootDir) : Object("AssetManager"), rootDir(rootDir) {
		defaultFont = Load<FontAsset>("main.ttf");
		Logger::Log("AssetManager created");
	}

	FontAsset::Ptr AssetManager::GetDefaultFont() const {
		return defaultFont;
	}

	void AssetManager::LoadAll(AssetManifest& manifest) {
		manifest.ForEach(
			[this](const AssetDefinition& definition) {
				switch (definition.type) {
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
					Logger::Log(*this, "Unrecognised asset type!", Logger::Severity::Error);
					break;
				}
			}
		);
	}

	void AssetManager::UnloadAll() {
		loaded.clear();
		Logger::Log(*this, "AssetManager cleared");
	}

	std::string AssetManager::GetPath(const std::string & path) {
		return rootDir.empty() ? path : rootDir + "/" + path;
	}

	AssetManager::~AssetManager() {
		defaultFont = nullptr;
		for (const auto& iter : loaded) {
			if (iter.second.use_count() > 1) {
				Logger::Log(*this, "Unreleased asset! [" + iter.second->GetResourcePath() + "]", Logger::Severity::Warning);
			}
		}
		UnloadAll();
		Logger::Log(*this, "AssetManager destroyed");
	}
}
